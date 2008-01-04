#include "Interaction/CreateAreaInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RelationCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Painting.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"

#include <QtCore/QSettings>
#include <QtGui/QDockWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>

CreateAreaInteraction::CreateAreaInteraction(MainWindow* aMain, MapView* aView)
	: GenericFeatureSnapInteraction<MapFeature>(aView), Main(aMain), 
	  theRelation(0), theRoad(0), LastRoad(0), FirstPoint(0,0), 
	  FirstNode(0), HaveFirst(false), EndNow(false)
{
}

CreateAreaInteraction::~CreateAreaInteraction()
{
}

void CreateAreaInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (HaveFirst)
	{
		QPen TP(QBrush(QColor(0xff,0x77,0x11,128)),projection().pixelPerM()*4);
		QPointF PreviousPoint = view()->projection().project(FirstPoint);
		::draw(thePainter,TP,MapFeature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());
	}
	GenericFeatureSnapInteraction<MapFeature>::paintEvent(anEvent,thePainter);
}

void CreateAreaInteraction::snapMouseMoveEvent(QMouseEvent* ev, MapFeature* aFeature)
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

void CreateAreaInteraction::startNewRoad(QMouseEvent* anEvent, MapFeature* aFeature)
{
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aFeature))
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

void CreateAreaInteraction::createNewRoad(CommandList* L)
{
	TrackPoint* From = 0;
	theRoad = new Road;
	if (FirstNode)
	{
		From = FirstNode;
		FirstNode = 0;
	}
	else
	{
		From = new TrackPoint(FirstPoint);
		L->add(new AddFeatureCommand(Main->activeLayer(),From,true));
	}
	L->add(new AddFeatureCommand(Main->activeLayer(),theRoad,true));
	L->add(new RoadAddTrackPointCommand(theRoad,From));
}

void CreateAreaInteraction::finishRoad(CommandList* L)
{
	if (theRelation)
		L->add(new RelationAddFeatureCommand(theRelation,"inner",theRoad));
	else if (LastRoad)
	{
		theRelation = new Relation;
		theRelation->setTag("type","multipolygon");
		theRelation->add("outer",LastRoad);
		theRelation->add("inner",theRoad);
		L->add(new AddFeatureCommand(Main->activeLayer(),theRelation,true));
		LastRoad = 0;
	}
	else
		LastRoad = theRoad;
	HaveFirst = false;
	LastRoad = theRoad;
	theRoad = 0;

	if (QMessageBox::question(Main, tr("Add a hole?"), 
		tr("Do you want to add a(nother) hole to this area?"), 
		QMessageBox::Yes, QMessageBox::No)  == QMessageBox::No)
	{
		EndNow = true;
	}
}

void CreateAreaInteraction::addToRoad(QMouseEvent* anEvent, MapFeature* Snap, CommandList* L)
{
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(Snap);
	TrackPoint* To = 0;
	if (Pt)
		To = Pt;
	else if (Road* aRoad = dynamic_cast<Road*>(Snap))
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
	if (To == theRoad->get(0))
		finishRoad(L);
}
	
void CreateAreaInteraction::snapMousePressEvent(QMouseEvent* anEvent, MapFeature* aFeature)
{
	if (anEvent->buttons() & Qt::LeftButton)
	{
		if (!HaveFirst)
		{
			HaveFirst = true;
			startNewRoad(anEvent, aFeature);
		}
		else 
		{
			CommandList* L = new CommandList;
			if (!theRoad)
				createNewRoad(L);
			addToRoad(anEvent, aFeature, L);
			document()->history().add(L);
			view()->invalidate();
			if (theRelation)
				Main->properties()->setSelection(theRelation);
			else
				Main->properties()->setSelection(theRoad);
		}
		FirstPoint = view()->projection().inverse(anEvent->pos());
	}
	else
		Interaction::mousePressEvent(anEvent);
	if (EndNow)
		view()->launch(0);
}
