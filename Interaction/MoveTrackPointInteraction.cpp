#include "Interaction/MoveTrackPointInteraction.h"

#include "MapView.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Coord.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>

#include <vector>

MoveTrackPointInteraction::MoveTrackPointInteraction(MapView* aView)
: FeatureSnapInteraction(aView), StartDragPosition(0,0)
{
}

MoveTrackPointInteraction::~MoveTrackPointInteraction(void)
{
}

QCursor MoveTrackPointInteraction::cursor() const
{
	QPixmap pm(":/Icons/move.xpm");
	return QCursor(pm);
}


void MoveTrackPointInteraction::snapMousePressEvent(QMouseEvent * event, MapFeature* aLast)
{
	clearNoSnap();
	Moving.clear();
	OriginalPosition.clear();
	StartDragPosition = projection().inverse(event->pos());
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aLast))
	{
		Moving.push_back(Pt);
		StartDragPosition = Pt->position();
	}
	else if (Road* R = dynamic_cast<Road*>(aLast))
		for (unsigned int i=0; i<R->size(); ++i)
			if (std::find(Moving.begin(),Moving.end(),R->get(i)) == Moving.end())
				Moving.push_back(R->get(i));
	for (unsigned int i=0; i<Moving.size(); ++i)
	{
		OriginalPosition.push_back(Moving[i]->position());
		addToNoSnap(Moving[i]);
	}
}

void MoveTrackPointInteraction::snapMouseReleaseEvent(QMouseEvent * event, MapFeature* Closer)
{
	if (Moving.size())
	{
		CommandList* theList = new CommandList(MainWindow::tr("Move Point %1").arg(Moving[0]->id()), Moving[0]);
		Coord Diff(calculateNewPosition(event,Closer, theList)-StartDragPosition);
		for (unsigned int i=0; i<Moving.size(); ++i)
		{
			Moving[i]->setPosition(OriginalPosition[i]);
			theList->add(new MoveTrackPointCommand(Moving[i],OriginalPosition[i]+Diff, document()->getDirtyOrOriginLayer(Moving[i]->layer())));
		}
		document()->addHistory(theList);
		view()->invalidate();
	}
	Moving.clear();
	OriginalPosition.clear();
	clearNoSnap();
}

void MoveTrackPointInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* Closer)
{
	if (Moving.size())
	{
		Coord Diff = calculateNewPosition(event,Closer,0)-StartDragPosition;
		for (unsigned int i=0; i<Moving.size(); ++i)
			Moving[i]->setPosition(OriginalPosition[i]+Diff);
		view()->invalidate();
	}
}

Coord MoveTrackPointInteraction::calculateNewPosition(QMouseEvent *event, MapFeature *aLast, CommandList* theList)
{
	Coord TargetC = projection().inverse(event->pos());
	QPointF Target(TargetC.lat(),TargetC.lon());
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aLast))
		return Pt->position();
	else if (Road* R = dynamic_cast<Road*>(aLast))
	{
		LineF L1(R->get(0)->position(),R->get(1)->position());
		double Dist = L1.distance(Target);
		QPointF BestTarget = L1.project(Target);
		unsigned int BestIdx = 1;
		for (unsigned int i=2; i<R->size(); ++i)
		{
			LineF L2(R->get(i-1)->position(),R->get(i)->position());
			double Dist2 = L2.distance(Target);
			if (Dist2 < Dist)
			{
				Dist = Dist2;
				BestTarget = L2.project(Target);
				BestIdx = i;
			}
		}
		if (theList && (Moving.size() == 1))
			theList->add(new
				RoadAddTrackPointCommand(R,Moving[0],BestIdx,document()->getDirtyOrOriginLayer(R->layer())));
		return Coord(BestTarget.x(),BestTarget.y());
	}
	return projection().inverse(event->pos());
}



