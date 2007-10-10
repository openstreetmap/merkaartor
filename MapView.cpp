#include "MapView.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/Interaction.h"
#include "PaintStyle/EditPaintStyle.h"

#include <QtCore/QTime>
#include <QtGui/QMainWindow>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStatusBar>

MapView::MapView(MainWindow* aMain)
: Main(aMain), theDocument(0), theInteraction(0), StaticBuffer(0), StaticBufferUpToDate(false)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);
}

MapView::~MapView(void)
{
	delete StaticBuffer;
}

MainWindow* MapView::main()
{
	return Main;
}

PropertiesDock* MapView::properties()
{
	return Main->properties();
}

void MapView::setDocument(MapDocument* aDoc)
{
	theDocument = aDoc;
}

MapDocument* MapView::document()
{
	return theDocument;
}

void MapView::invalidate()
{
	StaticBufferUpToDate = false;
	update();
}

void MapView::paintEvent(QPaintEvent* anEvent)
{
	updateStaticBuffer(anEvent);
	QPainter P(this);
	P.drawPixmap(QPoint(0,0),*StaticBuffer);
	if (theInteraction)
	{
		P.setRenderHint(QPainter::Antialiasing);
		theInteraction->paintEvent(anEvent,P);
	}
}

void MapView::updateStaticBuffer(QPaintEvent*)
{
	if (!StaticBuffer || (StaticBuffer->width() != width()) || (StaticBuffer->height() != height()))
	{
		delete StaticBuffer;
		StaticBuffer = new QPixmap(width(),height());
		StaticBufferUpToDate = false;
	}
	if (StaticBufferUpToDate)
		return;
	QTime Start(QTime::currentTime());
	QPainter P(StaticBuffer);
	P.setRenderHint(QPainter::Antialiasing);
	P.fillRect(StaticBuffer->rect(),QBrush(QColor(255,255,255)));
	if (theDocument)
	{
		for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			i.get()->draw(P,projection());
		EditPaintStyle EP(P,projection());
		PaintStyle* Current = EP.firstLayer();
		while (Current)
		{
			for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			{
				if (Road* R = dynamic_cast<Road*>(i.get()))
					Current->draw(R);
			}
			Current = Current->nextLayer();
		}
	}
/*	QTime Stop(QTime::currentTime());
	main()->statusBar()->clearMessage();
	main()->statusBar()->showMessage(QString("Paint took %1ms").arg(Start.msecsTo(Stop))); */
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

void MapView::mouseMoveEvent(QMouseEvent* anEvent)
{
	if (!updatesEnabled()) return;
	if (theInteraction)
		theInteraction->mouseMoveEvent(anEvent);
}

void MapView::wheelEvent(QWheelEvent* ev)
{
	int Steps = ev->delta()/120;
	if (Steps > 0)
	{
		for (int i=0; i<Steps; ++i)
			projection().zoom(0.75,rect());
		invalidate();
	}
	else if (Steps < 0)
	{
		for (int i=0; i<-Steps; ++i)
			projection().zoom(1/0.75,rect());
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
	else
	{
		setCursor(QCursor(Qt::ArrowCursor));
		launch(new EditInteraction(this));
	}
}

Interaction* MapView::interaction()
{
	return theInteraction;
}

Projection& MapView::projection()
{
	return theProjection;
}



