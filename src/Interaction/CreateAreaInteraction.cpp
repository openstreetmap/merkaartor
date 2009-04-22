#include "Interaction/CreateAreaInteraction.h"
#include "Command/DocumentCommands.h"
#include "Command/RelationCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Maps/Painting.h"
#include "Maps/Relation.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"
#include "Utils/MDiscardableDialog.h"

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
	if (theRoad && (!theRoad->layer() || theRoad->isDeleted())) { // The road was begon and then undoed. Restarting....
		HaveFirst = false;
		theRoad = NULL;
	}

	if (HaveFirst)
	{
		QPointF PreviousPoint;
		if (theRoad && theRoad->size())
			PreviousPoint = view()->projection().project(CAST_NODE(theRoad->get(theRoad->size()-1))->position());
		else
			PreviousPoint = view()->projection().project(FirstPoint);
		QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
		QPen TP(SomeBrush,projection().pixelPerM()*4);
		::draw(thePainter,TP,MapFeature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());
	}
	GenericFeatureSnapInteraction<MapFeature>::paintEvent(anEvent,thePainter);
}

void CreateAreaInteraction::snapMouseMoveEvent(QMouseEvent* ev, MapFeature* aFeature)
{
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aFeature))
		LastCursor = view()->projection().project(Pt);
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
		CommandList* theList  = new CommandList(MainWindow::tr("Create Area %1").arg(aRoad->description()), aRoad);
		int SnapIdx = findSnapPointIndex(aRoad, P);
		TrackPoint* N = new TrackPoint(P);
		N->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
		theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(),N,true));
		theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
		document()->addHistory(theList);
		view()->invalidate(true, false);
		FirstNode = N;
	}
}

void CreateAreaInteraction::createNewRoad(CommandList* L)
{
	TrackPoint* From = 0;
	theRoad = new Road;
	theRoad->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
	if (FirstNode)
	{
		From = FirstNode;
		FirstNode = 0;
		if (!From->isDirty() && !From->hasOSMId() && From->isUploadable())
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),From,true));
	}
	else
	{
		From = new TrackPoint(FirstPoint);
		From->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
		L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),From,true));
	}
	L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),theRoad,true));
	L->add(new RoadAddTrackPointCommand(theRoad,From));
	L->setDescription(MainWindow::tr("Area: Create Road %1").arg(theRoad->description()));
	L->setFeature(theRoad);
}

void CreateAreaInteraction::finishRoad(CommandList* L)
{
	if (theRelation)
		L->add(new RelationAddFeatureCommand(theRelation,"inner",theRoad));
	else if (LastRoad)
	{
		theRelation = new Relation;
		theRelation->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
		theRelation->setTag("type","multipolygon");
		theRelation->add("outer",LastRoad);
		theRelation->add("inner",theRoad);
		L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),theRelation,true));
		LastRoad = 0;
	}
	else
		LastRoad = theRoad;
	HaveFirst = false;
	LastRoad = theRoad;
	theRoad = 0;

	MDiscardableMessage dlg(NULL,
		MainWindow::tr("Add a hole."),
		MainWindow::tr("Do you want to add a(nother) hole to this area?"));
	if (dlg.check() == QDialog::Rejected) {
		EndNow = true;
	}
	L->setDescription(MainWindow::tr("Area: Finish Road %1").arg(LastRoad->description()));
	L->setFeature(LastRoad);
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
		int SnapIdx = findSnapPointIndex(aRoad, P);
		TrackPoint* N = new TrackPoint(P);
		N->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
		CommandList* theList  = new CommandList(MainWindow::tr("Area: Add node %1 to Road %2").arg(N->description()).arg(theRoad->description()), N);
		theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(),N,true));
		theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
		document()->addHistory(theList);
		view()->invalidate(true, false);
		To = N;
	}
	if (!To)
	{
		To = new TrackPoint(view()->projection().inverse(anEvent->pos()));
		To->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
		L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),To,true));
		L->setDescription(MainWindow::tr("Area: Add node %1 to Road %2").arg(To->description().arg(theRoad->description())));
		L->setFeature(To);
	} else {
		if (!To->isDirty() && !To->hasOSMId() && To->isUploadable())
			L->add(new AddFeatureCommand(Main->document()->getDirtyOrOriginLayer(),To,true));
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
 			CommandList* L  = new CommandList();
			if (!theRoad)
				createNewRoad(L);
			addToRoad(anEvent, aFeature, L);
			document()->addHistory(L);
			view()->invalidate(true, false);
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

#ifndef Q_OS_SYMBIAN
QCursor CreateAreaInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
#endif
