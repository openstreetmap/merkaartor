#include "RoadCommands.h"

#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Sync/DirtyList.h"

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W)
: theRoad(R), theTrackPoint(W), Position(theRoad->size())
{
	redo();
}

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W, unsigned int aPos)
: theRoad(R), theTrackPoint(W), Position(aPos)
{
	redo();
}

void RoadAddTrackPointCommand::undo()
{
	theRoad->remove(Position);
}

void RoadAddTrackPointCommand::redo()
{
	theRoad->add(theTrackPoint, Position);
}

bool RoadAddTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}


/* ROADREMOVETRACKPOINTCOMMAND */

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, TrackPoint* W)
: Idx(R->find(W)), theRoad(R), theTrackPoint(W)
{
	redo();
}

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, unsigned int anIdx)
: Idx(anIdx), theRoad(R), theTrackPoint(R->get(anIdx))
{
	redo();
}


void RoadRemoveTrackPointCommand::undo()
{
	theRoad->add(theTrackPoint,Idx);
}

void RoadRemoveTrackPointCommand::redo()
{
	theRoad->remove(Idx);
}

bool RoadRemoveTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}




