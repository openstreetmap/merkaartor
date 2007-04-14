#include "RoadCommands.h"

#include "Map/Road.h"
#include "Map/Way.h"
#include "Sync/DirtyList.h"

RoadAddWayCommand::RoadAddWayCommand(Road* R, Way* W)
: theRoad(R), theWay(W), Position(theRoad->size())
{
	redo();
}

RoadAddWayCommand::RoadAddWayCommand(Road* R, Way* W, unsigned int aPos)
: theRoad(R), theWay(W), Position(aPos)
{
	redo();
}

void RoadAddWayCommand::undo()
{
	theRoad->erase(theWay);
	theWay->removeAsPartOf(theRoad);
}

void RoadAddWayCommand::redo()
{
	theRoad->add(theWay, Position);
	theWay->addAsPartOf(theRoad);
}

bool RoadAddWayCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}


/* ROADREMOVEWAYCOMMAND */

RoadRemoveWayCommand::RoadRemoveWayCommand(Road* R, Way* W)
: Idx(R->find(W)), theRoad(R), theWay(W)
{
	redo();
}

void RoadRemoveWayCommand::undo()
{
	theRoad->add(theWay,Idx);
	theWay->addAsPartOf(theRoad);
}

void RoadRemoveWayCommand::redo()
{
	theRoad->erase(theWay);
	theWay->removeAsPartOf(theRoad);
}

bool RoadRemoveWayCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}




