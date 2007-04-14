#include "Command/TrackPointCommands.h"
#include "Map/TrackPoint.h"
#include "Sync/DirtyList.h"

MoveTrackPointCommand::MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos)
: thePoint(aPt), OldPos(aPt->position()), NewPos(aPos)
{
	redo();
}

void MoveTrackPointCommand::undo()
{
	thePoint->setPosition(OldPos);
}

void MoveTrackPointCommand::redo()
{
	thePoint->setPosition(NewPos);
}

bool MoveTrackPointCommand::buildDirtyList(DirtyList &theList)
{
	return theList.update(thePoint);
}


