#include "RoadCommands.h"

#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Maps/MapLayer.h"
#include "Sync/DirtyList.h"

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R)
: Command (R), theLayer(0), oldLayer(0), theRoad(R), theTrackPoint(0), Position(0)
{
}

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W, MapLayer* aLayer)
: Command (R), theLayer(aLayer), oldLayer(0), theRoad(R), theTrackPoint(W), Position(theRoad->size())
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W, int aPos, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRoad(R), theTrackPoint(W), Position(aPos)
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

RoadAddTrackPointCommand::~RoadAddTrackPointCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void RoadAddTrackPointCommand::undo()
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

void RoadAddTrackPointCommand::redo()
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

bool RoadAddTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theRoad->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theRoad);
	if (!theRoad->layer())
		return theList.update(theRoad);
	if (theRoad->layer()->isUploadable() && theTrackPoint->layer()->isUploadable())
		return theList.update(theRoad);

	return theList.noop(theRoad);
}

bool RoadAddTrackPointCommand::toXML(QDomElement& xParent) const
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

RoadAddTrackPointCommand * RoadAddTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	RoadAddTrackPointCommand* a = new RoadAddTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyOrOriginLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRoad = MapFeature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	a->theTrackPoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Position = e.attribute("pos").toUInt();

	Command::fromXML(d, e, a);

	return a;
}

/* ROADREMOVETRACKPOINTCOMMAND */

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R)
: Command(R), theLayer(0), oldLayer(0), Idx(0), theRoad(R), theTrackPoint(0)
{
}

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, TrackPoint* W, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theRoad(R), theTrackPoint(W)
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, int anIdx, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), theRoad(R), theTrackPoint(R->getNode(anIdx))
{
	if (!theLayer)
		theLayer = theRoad->layer();
	redo();
}

RoadRemoveTrackPointCommand::~RoadRemoveTrackPointCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void RoadRemoveTrackPointCommand::undo()
{
	Command::undo();
	theRoad->add(theTrackPoint,Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theRoad);
		oldLayer->add(theRoad);
		decDirtyLevel(oldLayer);
	}
	theRoad->updateVirtuals();
}

void RoadRemoveTrackPointCommand::redo()
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

bool RoadRemoveTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theRoad->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theRoad);
	if (!theRoad->layer())
		return theList.update(theRoad);
	if (theRoad->layer()->isUploadable() && (theTrackPoint->layer()->isUploadable() || theTrackPoint->hasOSMId()))
		return theList.update(theRoad);

	return theList.noop(theRoad);
}

bool RoadRemoveTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RoadRemoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("road", theRoad->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("index", QString::number(Idx));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

RoadRemoveTrackPointCommand * RoadRemoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	RoadRemoveTrackPointCommand* a = new RoadRemoveTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyOrOriginLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRoad = MapFeature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	a->theTrackPoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Idx = e.attribute("index").toUInt();

	Command::fromXML(d, e, a);

	return a;
}




