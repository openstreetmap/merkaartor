#include "Command/TrackPointCommands.h"
#include "Map/TrackPoint.h"
#include "Map/MapLayer.h"
#include "Sync/DirtyList.h"

MoveTrackPointCommand::MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos, MapLayer* aLayer)
: theLayer(aLayer), oldLayer(0), thePoint(aPt), OldPos(aPt->position()), NewPos(aPos)
{
	redo();
}

MoveTrackPointCommand::~MoveTrackPointCommand(void)
{
	oldLayer->decDirtyLevel(commandDirtyLevel);
}

void MoveTrackPointCommand::undo()
{
	thePoint->setPosition(OldPos);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(thePoint);
		oldLayer->add(thePoint);
		decDirtyLevel(oldLayer);
	}
}

void MoveTrackPointCommand::redo()
{
	thePoint->setPosition(NewPos);
	oldLayer = thePoint->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(thePoint);
		incDirtyLevel(oldLayer);
		theLayer->add(thePoint);
	}
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
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

MoveTrackPointCommand * MoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	MoveTrackPointCommand* a = new MoveTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	a->theLayer = d->getDirtyLayer();
	a->thePoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, NULL, e.attribute("trackpoint"));
	a->OldPos = Coord::fromXML(e.firstChildElement("oldpos"));
	a->NewPos = Coord::fromXML(e.firstChildElement("newpos"));
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;

	return a;
}



