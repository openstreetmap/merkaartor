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

CreateSingleWayInteraction::CreateSingleWayInteraction(MainWindow* aMain, MapView* aView, TrackPoint *firstNode, bool aCurved)
	: GenericFeatureSnapInteraction<MapFeature>(aView), Main(aMain), theRoad(0), FirstPoint(0,0),
	 FirstNode(firstNode), HaveFirst(false), Prepend(false), IsCurved(aCurved), Creating(false)
{
	if (firstNode)
	{
		FirstPoint = firstNode->position();
		LastCursor = view()->projection().project(FirstPoint);
		HaveFirst = true;
		if ((theRoad = Road::GetSingleParentRoad(firstNode))) {
			if (theRoad->isExtrimity(firstNode)) {
				Prepend = (theRoad->get(0) == firstNode) ? true : false;
			} else
				theRoad = NULL;

		}
	}
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

		Coord NewPoint = view()->projection().inverse(LastCursor);
		const double distance = FirstPoint.distanceFrom(NewPoint);

		QString distanceTag;
		if (distance < 1.0)
			distanceTag = QString("%1 m").arg(int(distance * 1000));
		else
			distanceTag = QString("%1 km").arg(distance, 0, 'f', 3);

		thePainter.drawText(LastCursor + QPointF(10,-10), distanceTag);
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
	Q_UNUSED(aFeature)
	if ((anEvent->buttons() & Qt::LeftButton) )
		Creating = true;
}

void CreateSingleWayInteraction::snapMouseReleaseEvent(QMouseEvent* anEvent, MapFeature* aFeature)
{
	if ( Creating && !panning() )
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
				unsigned int SnapIdx = findSnapPointIndex(aRoad, P);
				TrackPoint* N = new TrackPoint(P);
				N->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
				theList->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),N,true));
				theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx));
				document()->addHistory(theList);
				view()->invalidate(true, false);
				FirstNode = N;
			}
		}
		else
		{
			CommandList* L  = new CommandList();
			if (!theRoad)
			{
				TrackPoint* From = 0;
				theRoad = new Road;
				theRoad->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				if (IsCurved)
					theRoad->setTag("smooth","yes");
				if (FirstNode) {
					From = FirstNode;
					if (!From->isDirty() && !From->hasOSMId() && From->isUploadable())
						L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),From,true));
				}
				else
				{
					From = new TrackPoint(FirstPoint);
					From->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
					L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),From,true));
				}
				L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),theRoad,true));
				L->add(new RoadAddTrackPointCommand(theRoad,From));
				L->setDescription(MainWindow::tr("Create Road: %1").arg(theRoad->description()));
				L->setFeature(theRoad);
			}
			TrackPoint* To = 0;
			if (Pt) {
				To = Pt;
				if (!To->isDirty() && !To->hasOSMId() && To->isUploadable()) {
					L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),To,true));
					L->setDescription(MainWindow::tr("Create Node: %1").arg(To->description()));
				}
			}
			else if (Road* aRoad = dynamic_cast<Road*>(aFeature))
			{
				Coord P(projection().inverse(anEvent->pos()));
				unsigned int SnapIdx = findSnapPointIndex(aRoad, P);
				TrackPoint* N = new TrackPoint(P);
				N->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
				theList->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),N,true));
				theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx,Main->document()->getDirtyLayer()));
				document()->addHistory(theList);
				view()->invalidate(true, false);
				To = N;
			}
			if (!To)
			{
				To = new TrackPoint(view()->projection().inverse(anEvent->pos()));
				To->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				L->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),To,true));
				L->setDescription(MainWindow::tr("Create Node %1 in Road %2").arg(To->description()).arg(theRoad->description()));
				L->setFeature(To);
			}
			L->setDescription(MainWindow::tr("Add Node %1 to Road %2").arg(To->description()).arg(theRoad->description()));
			if (Prepend)
				L->add(new RoadAddTrackPointCommand(theRoad,To,(unsigned int)0,Main->document()->getDirtyLayer()));
			else
				L->add(new RoadAddTrackPointCommand(theRoad,To,Main->document()->getDirtyLayer()));
			document()->addHistory(L);
			view()->invalidate(true, false);
			Main->properties()->setSelection(theRoad);
		}
		FirstPoint = view()->projection().inverse(anEvent->pos());
	}
	Creating = false;
	LastCursor = anEvent->pos();
}

QCursor CreateSingleWayInteraction::cursor() const
{
	return QCursor(Qt::CrossCursor);
}
