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

bool MoveTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("MoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("trackpoint", thePoint->xmlId());
	OldPos.toXML("oldpos", e);
	NewPos.toXML("newpos", e);

	return OK;
}

MoveTrackPointCommand * MoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	MoveTrackPointCommand* a = new MoveTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	a->thePoint = dynamic_cast<TrackPoint*>(d->getFeature("node_"+e.attribute("trackpoint")));
	a->OldPos = Coord::fromXML(e.firstChildElement("oldpos"));
	a->NewPos = Coord::fromXML(e.firstChildElement("newpos"));

	return a;
}



