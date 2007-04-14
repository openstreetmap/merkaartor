#include "Interaction/CreateWayInteraction.h"

#include "MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "Command/DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

CreateWayInteraction::CreateWayInteraction(MainWindow* aMain, MapView* aView, bool aBezierWay)
: TrackPointSnapInteraction(aView), HaveFirstPoint(false), P1(0,0), P2(0,0),
  From(0), BezierWay(aBezierWay), Main(aMain)
{
	view()->properties()->setSelection(0);
}

CreateWayInteraction::~CreateWayInteraction(void)
{
}

void CreateWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (HaveFirstPoint)
	{
		QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
		thePainter.setPen(TP);
		thePainter.drawLine(projection().project(P1),projection().project(P2));
	}
	TrackPointSnapInteraction::paintEvent(anEvent, thePainter);
}

void CreateWayInteraction::snapMouseReleaseEvent(QMouseEvent * event, TrackPoint* aLast)
{
	if (event->button() == Qt::LeftButton)
	{
		if (!HaveFirstPoint)
		{
			From = aLast;
			P1 = projection().inverse(event->pos());
			if (From)
				P1 = From->position();
			HaveFirstPoint = true;
			view()->update();
		}
		else
		{
			P2 = projection().inverse(event->pos());
			CommandList* L = new CommandList;
			if (!From)
			{
				From = new TrackPoint(P1);
				L->add(new AddFeatureCommand(Main->activeLayer(),From,true));
			}
			TrackPoint* To = aLast;
			if (!To)
			{
				To = new TrackPoint(P2);
				L->add(new AddFeatureCommand(Main->activeLayer(),To,true));
			}
			Way* W;
			if (BezierWay)
			{
				double DLat = To->position().lat()-From->position().lat();
				double DLon = To->position().lon()-From->position().lon();
				TrackPoint* ControlFrom = new TrackPoint(
					Coord(From->position().lat()+DLat/3,From->position().lon()+DLon/3));
				TrackPoint* ControlTo = new TrackPoint(
					Coord(From->position().lat()+2*DLat/3,From->position().lon()+2*DLon/3));
				W = new Way(From,ControlFrom,ControlTo,To);
				L->add(new AddFeatureCommand(Main->activeLayer(),ControlFrom,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),ControlTo,true));
				L->add(new AddFeatureCommand(Main->activeLayer(),W,true));
			}
			else
			{
				W = new Way(From,To);
				L->add(new AddFeatureCommand(Main->activeLayer(),W,true));
			}
			document()->history().add(L);
			view()->invalidate();
			view()->properties()->setSelection(W);
			HaveFirstPoint = false;
		}
	}
}

void CreateWayInteraction::snapMouseMoveEvent(QMouseEvent* event, TrackPoint* aLast)
{
	if (HaveFirstPoint)
	{
		P2 = projection().inverse(event->pos());
		if (aLast)
			P2 = aLast->position();
		view()->update();
	}
}

QCursor CreateWayInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}


