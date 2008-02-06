#include "MapView.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/Interaction.h"
#include "PaintStyle/EditPaintStyle.h"

#include "QMapControl/layermanager.h"
#include "QMapControl/imagemanager.h"


#include <QtCore/QTime>
#include <QtGui/QMainWindow>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStatusBar>

MapView::MapView(MainWindow * aMain)
:
Main(aMain), theDocument(0), theInteraction(0), StaticBuffer(0),
StaticBufferUpToDate(false)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);
	theProjection = new ImageProjection();

	QSettings Sets;
	Sets.beginGroup("downloadosm");
	if (Sets.value("useproxy").toBool()) {
		ImageManager::instance()->setProxy(Sets.value("proxyhost").
						   toString(),
						   Sets.value("proxyport").
						   toInt());
	}

	layermanager = new LayerManager((QWidget *) this, size());

	connect(ImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(updateRequestNew()));

	connect(ImageManager::instance(), SIGNAL(loadingFinished()),
		this, SLOT(loadingFinished()));
}

MapView::~MapView()
{
	delete layermanager;
	delete StaticBuffer;
}

MainWindow *MapView::main()
{
	return Main;
}

PropertiesDock *MapView::properties()
{
	return Main->properties();
}

void MapView::setDocument(MapDocument * aDoc)
{
	theDocument = aDoc;

	delete layermanager;
	layermanager = new LayerManager((QWidget *) this, size());

	layermanager->addLayer(theDocument->layer(0)->imageLayer());
	theDocument->layer(0)->layermanager = layermanager;
	((ImageProjection *) projection())->setLayerManager(layermanager);
	projection()->setViewport(CoordBox(Coord(1, -1), Coord(-1, 1)),
				  rect());
}

MapDocument *MapView::document()
{
	return theDocument;
}

void MapView::invalidate()
{
	StaticBufferUpToDate = false;
	layermanager->forceRedraw();
	update();
}

void MapView::paintEvent(QPaintEvent * anEvent)
{
	updateStaticBuffer(anEvent);
	QPainter P(this);
	P.drawPixmap(QPoint(0, 0), *StaticBuffer);
	if (theInteraction) {
		P.setRenderHint(QPainter::Antialiasing);
		theInteraction->paintEvent(anEvent, P);
	}
}

void MapView::updateStaticBuffer(QPaintEvent * anEvent)
{
	if (!StaticBuffer || (StaticBuffer->width() != width())
	    || (StaticBuffer->height() != height())) {
		delete StaticBuffer;
		StaticBuffer = new QPixmap(width(), height());
		StaticBufferUpToDate = false;
	}
	if (StaticBufferUpToDate)
		return;
	QTime Start(QTime::currentTime());
	QPainter P(StaticBuffer);
	P.setRenderHint(QPainter::Antialiasing);
	P.fillRect(StaticBuffer->rect(), QBrush(QColor(255, 255, 255)));
	if (theDocument) {
		EditPaintStyle EP(P, projection());

		for (unsigned int i = 0; i < theDocument->numLayers(); ++i) {
			theDocument->layer(i)->
				sortRenderingPriority(projection()->
						      pixelPerM());
		}

		if (layermanager->getLayers().size() > 0) {
			layermanager->drawImage(&P);
		}
		for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			i.get()->draw(P, projection());
		for (unsigned int i = 0; i < EP.size(); ++i) {
			PaintStyleLayer *Current = EP.get(i);
			for (VisibleFeatureIterator i(theDocument);
			     !i.isEnd(); ++i) {
				if (Road * R =
				    dynamic_cast < Road * >(i.get()))
					Current->draw(R);
				else if (TrackPoint * Pt =
					 dynamic_cast <
					 TrackPoint * >(i.get()))
					Current->draw(Pt);
				else if (Relation * RR =
					 dynamic_cast < Relation * >(i.get()))
					Current->draw(RR);
			}
		}
	}
	QTime Stop(QTime::currentTime());
	main()->statusBar()->clearMessage();
	main()->statusBar()->showMessage(QString("Paint took %1ms").
					 arg(Start.msecsTo(Stop)));
	StaticBufferUpToDate = true;
}

void MapView::mousePressEvent(QMouseEvent * event)
{
	if (theInteraction)
		theInteraction->mousePressEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent * event)
{
	if (theInteraction)
		theInteraction->mouseReleaseEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent * anEvent)
{
	if (!updatesEnabled())
		return;
	if (theInteraction)
		theInteraction->mouseMoveEvent(anEvent);
}

void MapView::wheelEvent(QWheelEvent * ev)
{
	int Steps = ev->delta() / 120;
	if (Steps > 0) {
		for (int i = 0; i < Steps; ++i) {
			projection()->zoom(1 / 0.75, ev->pos(), rect());
		}
		invalidate();
	} else if (Steps < 0) {
		for (int i = 0; i < -Steps; ++i) {
			projection()->zoom(0.75, ev->pos(), rect());
		}
		invalidate();
	}
}

void MapView::launch(Interaction * anInteraction)
{
	if (theInteraction)
		delete theInteraction;
	theInteraction = anInteraction;
	if (theInteraction)
		setCursor(theInteraction->cursor());
	else {
		setCursor(QCursor(Qt::ArrowCursor));
		launch(new EditInteraction(this));
	}
}

Interaction *MapView::interaction()
{
	return theInteraction;
}

Projection *MapView::projection()
{
	return theProjection;
}

void MapView::updateRequestNew()
{
//      qDebug() << "MapControl::updateRequestNew()";
	layermanager->forceRedraw();
	invalidate();
}

void MapView::loadingFinished()
{
//      qDebug() << "MapControl::loadingFinished()";
	layermanager->removeZoomImage();
}

void MapView::resizeEvent(QResizeEvent * event)
{
	StaticBufferUpToDate = false;
	layermanager->setSize(size());
	projection()->zoom(1, QPoint(width() / 2, height() / 2), rect());

	QWidget::resizeEvent(event);
}
