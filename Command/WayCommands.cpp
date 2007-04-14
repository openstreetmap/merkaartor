#include "Command/WayCommands.h"
#include "Map/Way.h"
#include "Sync/DirtyList.h"

WaySetWidthCommand::WaySetWidthCommand(Way* W, double w)
: theWay(W), OldWidth(W->width()), NewWidth(w)
{
	redo();
}

void WaySetWidthCommand::redo()
{
	theWay->setWidth(NewWidth);
}

void WaySetWidthCommand::undo()
{
	theWay->setWidth(OldWidth);
}

bool WaySetWidthCommand::buildDirtyList(DirtyList &theList)
{
	return theList.update(theWay);
}

/* WAYSETFROMTOCOMMAND */

WaySetFromToCommand::WaySetFromToCommand(Way* W, TrackPoint* aFrom, TrackPoint* aTo)
: theWay(W), OldFrom(W->from()), OldTo(W->to()), NewFrom(aFrom), NewTo(aTo),
  OldControlFrom(W->controlFrom()), OldControlTo(W->controlTo()), NewControlFrom(0), NewControlTo(0)
{
	redo();
}

WaySetFromToCommand::WaySetFromToCommand(Way* W, TrackPoint* aFrom, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* aTo)
: theWay(W), OldFrom(W->from()), OldTo(W->to()), NewFrom(aFrom), NewTo(aTo),
  OldControlFrom(W->controlFrom()), OldControlTo(W->controlTo()), NewControlFrom(aC1), NewControlTo(aC2)
{
	redo();
}


void WaySetFromToCommand::undo()
{
	theWay->setFromTo(OldFrom,OldControlFrom,OldControlTo,OldTo);
}

void WaySetFromToCommand::redo()
{
	theWay->setFromTo(NewFrom, NewControlFrom, NewControlTo, NewTo);
}

bool WaySetFromToCommand::buildDirtyList(DirtyList &theList)
{
	return theList.update(theWay);
}



