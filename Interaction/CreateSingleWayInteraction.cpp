#include "Interaction/CreateSingleWayInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"

#include <QtCore/QSettings>
#include <QtGui/QDockWidget>
#include <QtGui/QPainter>

CreateSingleWayInteraction::CreateSingleWayInteraction(MainWindow* aMain, MapView* aView)
	: GenericFeatureSnapInteraction<MapFeature>(aView), Main(aMain), theRoad(0), FirstPoint(0,0), 
	  HaveFirst(false), FirstNode(0)
{
	setDontSelectRoads(true);
}

CreateSingleWayInteraction::~CreateSingleWayInteraction()
{
}

void CreateSingleWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (HaveFirst)
	{
		QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
		QPointF PreviousPoint = view()->projection().project(FirstPoint);
		::draw(thePainter,TP,MapFeature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());
	}
	GenericFeatureSnapInteraction<MapFeature>::paintEvent(anEvent,thePainter);
}

void CreateSingleWayInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* aFeature)
{
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aFeature))
		LastCursor = view()->projection().project(Pt->position());	
	else if (Way* W = dynamic_cast<Way*>(aFeature))
	{
		LineF L(view()->projection().project(W->from()->position()),
			view()->projection().project(W->to()->position()));
		LastCursor = L.project(event->pos());
	}
	else
		LastCursor = event->pos();
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
			else if (Way* aWay = dynamic_cast<Way*>(aFeature))
			{
				Coord P(projection().inverse(anEvent->pos()));
				CommandList* theList = new CommandList;
				LineF L(aWay->from()->position(),aWay->to()->position());
				TrackPoint* N = new TrackPoint(L.project(P));
				theList->add(new AddFeatureCommand(main()->activeLayer(),N,true));
				Way* W1 = new Way(aWay->from(),N);
				theList->add(new AddFeatureCommand(main()->activeLayer(),W1,true));
				Way* W2 = new Way(N,aWay->to());
				theList->add(new AddFeatureCommand(main()->activeLayer(),W2,true));
				std::vector<MapFeature*> Alternatives;
				Alternatives.push_back(W1);
				Alternatives.push_back(W2);
				theList->add(new RemoveFeatureCommand(document(),aWay, Alternatives));
				document()->history().add(theList);
				view()->invalidate();
				FirstNode = N;
			}
		}
		else 
		{
			CommandList* L = new CommandList;
			TrackPoint* From = 0;
			if (!theRoad)
			{
				theRoad = new Road;
				if (FirstNode)
					From = FirstNode;
				else
				{
					From = new TrackPoint(FirstPoint);
					L->add(new AddFeatureCommand(Main->activeLayer(),From,true));
				}
				L->add(new AddFeatureCommand(Main->activeLayer(),theRoad,true));
			}
			else
				From = theRoad->get(theRoad->size()-1)->to();
			TrackPoint* To = 0;
			if (Pt)
				To = Pt;
			else if (Way* aWay = dynamic_cast<Way*>(aFeature))
			{
				Coord P(projection().inverse(anEvent->pos()));
				LineF LL(aWay->from()->position(),aWay->to()->position());
				TrackPoint* N = new TrackPoint(LL.project(P));
				L->add(new AddFeatureCommand(main()->activeLayer(),N,true));
				Way* W1 = new Way(aWay->from(),N);
				L->add(new AddFeatureCommand(main()->activeLayer(),W1,true));
				Way* W2 = new Way(N,aWay->to());
				L->add(new AddFeatureCommand(main()->activeLayer(),W2,true));
				std::vector<MapFeature*> Alternatives;
				Alternatives.push_back(W1);
				Alternatives.push_back(W2);
				L->add(new RemoveFeatureCommand(document(),aWay, Alternatives));
				To = N;
			}
			if (!To)
			{
				To = new TrackPoint(view()->projection().inverse(anEvent->pos()));
				L->add(new AddFeatureCommand(Main->activeLayer(),To,true));
			}
			Way* W = new Way(From,To);
			L->add(new AddFeatureCommand(Main->activeLayer(),W,true));
			L->add(new RoadAddWayCommand(theRoad,W));
			document()->history().add(L);
			view()->invalidate();
			Main->properties()->setSelection(theRoad);
		}
		FirstPoint = view()->projection().inverse(anEvent->pos());
	}
	else
		Interaction::mousePressEvent(anEvent);
}
