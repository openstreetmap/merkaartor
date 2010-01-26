#include "WayCommands.h"

#include "Way.h"
#include "Node.h"
#include "Layer.h"
#include "DirtyList.h"

WayAddNodeCommand::WayAddNodeCommand(Way* R)
: Command (R), theLayer(0), oldLayer(0), theRoad(R), theTrackPoint(0), Position(0)
{
}

WayAddNodeCommand::WayAddNodeCommand(Way* R, Node* W, Layer* aLayer)
: Command (R), theLayer(aLayer), oldLayer(0), theRoad(R), theTrackPoint(W), Position(theRoad->size())
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

WayAddNodeCommand::WayAddNodeCommand(Way* R, Node* W, int aPos, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRoad(R), theTrackPoint(W), Position(aPos)
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

WayAddNodeCommand::~WayAddNodeCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void WayAddNodeCommand::undo()
{
	Command::undo();
	theRoad->remove(Position);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theRoad);
		oldLayer->add(theRoad);
		decDirtyLevel(oldLayer);
	}
	theRoad->updateVirtuals();
}

void WayAddNodeCommand::redo()
{
	oldLayer = theRoad->layer();
	theRoad->add(theTrackPoint, Position);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theRoad);
		incDirtyLevel(oldLayer);
		theLayer->add(theRoad);
	}
	theRoad->updateVirtuals();
	Command::redo();
}

bool WayAddNodeCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theRoad->lastUpdated() == Feature::NotYetDownloaded)
		return theList.noop(theRoad);
	if (!theRoad->layer())
		return theList.update(theRoad);
	if (theRoad->layer()->isUploadable() && theTrackPoint->layer()->isUploadable())
		return theList.update(theRoad);

	return theList.noop(theRoad);
}

bool WayAddNodeCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RoadAddTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("road", theRoad->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("pos", QString::number(Position));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

WayAddNodeCommand * WayAddNodeCommand::fromXML(Document * d, QDomElement e)
{
	WayAddNodeCommand* a = new WayAddNodeCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyOrOriginLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRoad = Feature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	a->theTrackPoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Position = e.attribute("pos").toUInt();

	Command::fromXML(d, e, a);

	return a;
}

/* ROADREMOVETRACKPOINTCOMMAND */

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R)
: Command(R), theLayer(0), oldLayer(0), Idx(0), theRoad(R), theNode(0)
{
}

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R, Node* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theRoad(R), theNode(W)
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R, int anIdx, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), theRoad(R), theNode(R->getNode(anIdx))
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

WayRemoveNodeCommand::~WayRemoveNodeCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void WayRemoveNodeCommand::undo()
{
	Command::undo();
	theRoad->add(theNode,Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theRoad);
		oldLayer->add(theRoad);
		decDirtyLevel(oldLayer);
	}
	theRoad->updateVirtuals();
}

void WayRemoveNodeCommand::redo()
{
	oldLayer = theRoad->layer();
	theRoad->remove(Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theRoad);
		incDirtyLevel(oldLayer);
		theLayer->add(theRoad);
	}
	theRoad->updateVirtuals();
	Command::redo();
}

bool WayRemoveNodeCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theRoad->lastUpdated() == Feature::NotYetDownloaded)
		return theList.noop(theRoad);
	if (!theRoad->layer())
		return theList.update(theRoad);
	if (theRoad->layer()->isUploadable() && (theNode->layer()->isUploadable() || theNode->hasOSMId()))
		return theList.update(theRoad);

	return theList.noop(theRoad);
}

bool WayRemoveNodeCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RoadRemoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("road", theRoad->xmlId());
	e.setAttribute("trackpoint", theNode->xmlId());
	e.setAttribute("index", QString::number(Idx));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

WayRemoveNodeCommand * WayRemoveNodeCommand::fromXML(Document * d, QDomElement e)
{
	WayRemoveNodeCommand* a = new WayRemoveNodeCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyOrOriginLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRoad = Feature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	a->theNode = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Idx = e.attribute("index").toUInt();

	Command::fromXML(d, e, a);

	return a;
}




