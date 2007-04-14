#include "Interaction/MoveTrackPointInteraction.h"

#include "MapView.h"
#include "Command/DocumentCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>

#include <vector>

MoveTrackPointInteraction::MoveTrackPointInteraction(MapView* aView)
: TrackPointSnapInteraction(aView), Moving(0), Orig(0,0)
{
}

MoveTrackPointInteraction::~MoveTrackPointInteraction(void)
{
}

void MoveTrackPointInteraction::snapMousePressEvent(QMouseEvent *, TrackPoint* aLast)
{
	Moving = aLast;
	if (Moving)
	{
		Orig = Moving->position();
		addToNoSnap(Moving);
	}
}

void MoveTrackPointInteraction::snapMouseReleaseEvent(QMouseEvent * event, TrackPoint* Closer)
{
	if (Moving)
	{
		Moving->setPosition(Orig);
		Moving->setLastUpdated(MapFeature::User);
		if (Closer)
		{
			CommandList* theList = new CommandList;
			theList->add(new MoveTrackPointCommand(Moving,Closer->position()));
			std::vector<MapFeature*> Alternative;
			Alternative.push_back(Moving);
			theList->add(new RemoveFeatureCommand(document(),Closer,Alternative));
			document()->history().add(theList);
		}
		else
			document()->history().add(new MoveTrackPointCommand(Moving,projection().inverse(event->pos())));
		view()->invalidate();
		Moving = 0;
	}
	clearNoSnap();
}

void MoveTrackPointInteraction::snapMouseMoveEvent(QMouseEvent* event, TrackPoint* Closer)
{
	if (Moving)
	{
		if (Closer)
			Moving->setPosition(Closer->position());
		else
			Moving->setPosition(projection().inverse(event->pos()));
		view()->invalidate();
	}
}




