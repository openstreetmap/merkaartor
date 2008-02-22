#include "Interaction/CreateSingleWayInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"

#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

CreateSingleWayInteraction::CreateSingleWayInteraction(MainWindow* aMain, MapView* aView, bool aCurved)
	: GenericFeatureSnapInteraction<MapFeature>(aView), Main(aMain), theRoad(0), FirstPoint(0,0),
	  HaveFirst(false), FirstNode(0), IsCurved(aCurved)
{
}

CreateSingleWayInteraction::~CreateSingleWayInteraction()
{
}

void CreateSingleWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (HaveFirst)
	{
	  QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,projection().pixelPerM()*4+2);
		QPointF PreviousPoint = view()->projection().project(FirstPoint);
		::draw(thePainter,TP,MapFeature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());
	}
	GenericFeatureSnapInteraction<MapFeature>::paintEvent(anEvent,thePainter);
}

void CreateSingleWayInteraction::snapMouseMoveEvent(QMouseEvent* ev, MapFeature* aFeature)
{
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aFeature))
		LastCursor = view()->projection().project(Pt->position());
	else if (Road* R = dynamic_cast<Road*>(aFeature))
	{
		Coord P(projection().inverse(ev->pos()));
		findSnapPointIndex(R, P);
		LastCursor = projection().project(P);
	}
	else
		LastCursor = ev->pos();
	view()->update();
}

void CreateSingleWayInteraction::snapMousePressEvent(QMouseEvent* anEvent, MapFeature* aFeature)
{
	if (anEvent->buttons() & Qt::LeftButton)
	{
		TrackPoint* Pt = dynamic_cast<TrackPoint*>(aFeature);
		if (!HaveFirst)
		{
			HaveFirst = true;
			if (Pt)
				FirstNode = Pt;
			else if (Road* aRoad = dynamic_cast<Road*>(aFeature))
			{
				Coord P(projection().inverse(anEvent->pos()));
				CommandList* theList = new CommandList;
				unsigned int SnapIdx = findSnapPointIndex(aRoad, P);
				TrackPoint* N = new TrackPoint(P);
				theList->add(new AddFeatureCommand(main()->activeLayer(),N,true));
				theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
				document()->history().add(theList);
				view()->invalidate();
				FirstNode = N;
			}
		}
		else
		{
			CommandList* L = new CommandList;
			if (!theRoad)
			{
				TrackPoint* From = 0;
				theRoad = new Road;
				if (IsCurved)
					theRoad->setTag("smooth","yes");
				if (FirstNode)
					From = FirstNode;
				else
				{
					From = new TrackPoint(FirstPoint);
					L->add(new AddFeatureCommand(Main->activeLayer(),From,true));
				}
				L->add(new AddFeatureCommand(Main->activeLayer(),theRoad,true));
				L->add(new RoadAddTrackPointCommand(theRoad,From));
			}
			TrackPoint* To = 0;
			if (Pt)
				To = Pt;
			else if (Road* aRoad = dynamic_cast<Road*>(aFeature))
			{
				Coord P(projection().inverse(anEvent->pos()));
				CommandList* theList = new CommandList;
				unsigned int SnapIdx = findSnapPointIndex(aRoad, P);
				TrackPoint* N = new TrackPoint(P);
				theList->add(new AddFeatureCommand(main()->activeLayer(),N,true));
				theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
				document()->history().add(theList);
				view()->invalidate();
				To = N;
			}
			if (!To)
			{
				To = new TrackPoint(view()->projection().inverse(anEvent->pos()));
				L->add(new AddFeatureCommand(Main->activeLayer(),To,true));
			}
			L->add(new RoadAddTrackPointCommand(theRoad,To));
			document()->history().add(L);
			view()->invalidate();
			Main->properties()->setSelection(theRoad);
		}
		FirstPoint = view()->projection().inverse(anEvent->pos());
	}
	else
		Interaction::mousePressEvent(anEvent);
}
