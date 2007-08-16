#include "Command/WayCommands.h"
#include "Command/FeatureCommands.h"
#include "Map/Way.h"
#include "Sync/DirtyList.h"

#include <math.h>

WaySetWidthCommand::WaySetWidthCommand(Way* W, double NewWidth)
: Sub(0)
{
	double Default = widthOf(W);
	if (fabs(NewWidth-Default) > 0.01)
		Sub = new SetTagCommand(W,"width", QString::number(NewWidth));
	else
		Sub = new ClearTagCommand(W,"width");
}

WaySetWidthCommand::~WaySetWidthCommand()
{
	delete Sub;
}

void WaySetWidthCommand::redo()
{
	Sub->redo();
}

void WaySetWidthCommand::undo()
{
	Sub->undo();
}

bool WaySetWidthCommand::buildDirtyList(DirtyList &theList)
{
	return Sub->buildDirtyList(theList);
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



