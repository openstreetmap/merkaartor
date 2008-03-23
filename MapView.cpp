#include "MapView.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/Interaction.h"
#include "PaintStyle/EditPaintStyle.h"

#include "QMapControl/layermanager.h"
#include "QMapControl/imagemanager.h"
#include "Preferences/MerkaartorPreferences.h"


#include <QtCore/QTime>
#include <QtGui/QMainWindow>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStatusBar>

MapView::MapView(MainWindow* aMain) :
	Main(aMain), theDocument(0), theInteraction(0), StaticBuffer(0),
		StaticBufferUpToDate(false), numImages(0)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);

	ImageManager::instance()->setCacheDir(MerkaartorPreferences::instance()->getCacheDir());
	ImageManager::instance()->setCacheMaxSize(MerkaartorPreferences::instance()->getCacheSize());
	if (MerkaartorPreferences::instance()->getProxyUse()) {
		ImageManager::instance()->setProxy(MerkaartorPreferences::instance()->getProxyHost(),
			MerkaartorPreferences::instance()->getProxyPort());
	} else {
		ImageManager::instance()->setProxy("",0);
	}

	layermanager = new LayerManager((QWidget *) this, size());

	pbImages = new QProgressBar(Main);
	pbImages->setFormat("tile %v / %m");
	Main->statusBar()->addPermanentWidget(pbImages);

	connect(ImageManager::instance(), SIGNAL(imageRequested()),
		this, SLOT(imageRequested()));
	connect(ImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(imageReceived()));
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

void MapView::setDocument(MapDocument* aDoc)
{
	theDocument = aDoc;

	delete layermanager;
	layermanager = new LayerManager((QWidget *) this, size());

	if (theDocument->getImageLayer()->imageLayer())
		layermanager->addLayer(theDocument->getImageLayer()->imageLayer());
	theDocument->getImageLayer()->layermanager = layermanager;
	projection().setLayerManager(layermanager);

	projection().setViewport(WORLD_COORDBOX, rect());
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

void MapView::updateStaticBuffer(QPaintEvent * /* anEvent */)
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

		for (unsigned int i = 0; i < theDocument->layerSize(); ++i) {
			theDocument->getLayer(i)->
				sortRenderingPriority(projection().pixelPerM());
		}

		if (layermanager) {
			if (layermanager->getLayers().size() > 0) {
				if (MerkaartorPreferences::instance()->getProjectionType() == Proj_Merkaartor) {
					QRectF vlm = layermanager->getViewport();
					Coord ctl = Coord(angToRad(vlm.bottomLeft().y()), angToRad(vlm.bottomLeft().x()));
					Coord cbr = Coord(angToRad(vlm.topRight().y()), angToRad(vlm.topRight().x()));
					QPointF tl = projection().project(ctl);
					QPointF br = projection().project(cbr);

					QRect pr = QRectF(tl, br).toRect();
					QSize ps = pr.size();
					QPixmap pm(size());
					QPainter pmp(&pm);
					layermanager->drawImage(&pmp);

					qreal ratio = qMax((qreal)width()/ps.width()*1.0, (qreal)height()/ps.height());
					QPixmap pms;
					if (ratio > 1.0) {
						pms = pm.scaled(ps /*, Qt::IgnoreAspectRatio, Qt::SmoothTransformation */ );
					} else {
						QSizeF ds;
						QRect dr;
						ds = QSizeF(ratio*pm.width(), ratio*pm.height());
						dr = QRect(QPoint((pm.width()/2)-(ds.width()/2), (pm.height()/2)-(ds.height()/2)), ds.toSize());
						pms = pm.copy(dr).scaled(ps*ratio /*, Qt::IgnoreAspectRatio, Qt::SmoothTransformation */ );
					}
					P.drawPixmap((width()-pms.width())/2, (height()-pms.height())/2, pms);
				} else {
					layermanager->drawImage(&P);
				}
			}
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

void MapView::mousePressEvent(QMouseEvent* event)
{
	if (theInteraction)
		theInteraction->mousePressEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
	if (theInteraction)
		theInteraction->mouseReleaseEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent* anEvent)
{
	if (!updatesEnabled())
		return;
	if (theInteraction)
		theInteraction->mouseMoveEvent(anEvent);
}

void MapView::wheelEvent(QWheelEvent* ev)
{
	int Steps = ev->delta() / 120;
	if (Steps > 0) {
		for (int i = 0; i < Steps; ++i) {
			projection().zoom(MerkaartorPreferences::instance()->getZoomInPerc()/100.0, ev->pos(), rect());
		}
		invalidate();
	} else if (Steps < 0) {
		for (int i = 0; i < -Steps; ++i) {
			projection().zoom(MerkaartorPreferences::instance()->getZoomOutPerc()/100.0, ev->pos(), rect());
		}
		invalidate();
	}
}

void MapView::launch(Interaction* anInteraction)
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

Projection& MapView::projection()
{
	return theProjection;
}

void MapView::imageRequested()
{
	++numImages;
	pbImages->setRange(0, numImages);
	//pbImages->setValue(0);
	pbImages->update();
}

void MapView::imageReceived()
{
	pbImages->setValue(pbImages->value()+1);

	invalidate();
}

void MapView::loadingFinished()
{
	layermanager->removeZoomImage();
	numImages = 0;
	pbImages->reset();

	invalidate();
}

void MapView::resizeEvent(QResizeEvent * event)
{
	StaticBufferUpToDate = false;
	layermanager->setSize(size());
	projection().zoom(1, QPoint(width() / 2, height() / 2), rect());

	QWidget::resizeEvent(event);
}
