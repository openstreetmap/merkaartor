#include "TrackSegmentCommands.h"

#include "Map/TrackSegment.h"
#include "Map/TrackPoint.h"
#include "Map/MapLayer.h"
#include "Sync/DirtyList.h"

TrackSegmentAddTrackPointCommand::TrackSegmentAddTrackPointCommand(TrackSegment* R, TrackPoint* W, MapLayer* aLayer)
: theLayer(aLayer), oldLayer(0), theTrackSegment(R), theTrackPoint(W), Position(theTrackSegment->size())
{
	redo();
}

TrackSegmentAddTrackPointCommand::TrackSegmentAddTrackPointCommand(TrackSegment* R, TrackPoint* W, unsigned int aPos, MapLayer* aLayer)
: theLayer(aLayer), oldLayer(0), theTrackSegment(R), theTrackPoint(W), Position(aPos)
{
	redo();
}

TrackSegmentAddTrackPointCommand::~TrackSegmentAddTrackPointCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void TrackSegmentAddTrackPointCommand::undo()
{
	theTrackSegment->remove(Position);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theTrackSegment);
		oldLayer->add(theTrackSegment);
		decDirtyLevel(oldLayer);
	}
}

void TrackSegmentAddTrackPointCommand::redo()
{
	theTrackSegment->add(theTrackPoint, Position);
	oldLayer = theTrackSegment->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theTrackSegment);
		incDirtyLevel(oldLayer);
		theLayer->add(theTrackSegment);
	}
}

bool TrackSegmentAddTrackPointCommand::buildDirtyList(DirtyList& /* theList */)
{
	return false;
}

bool TrackSegmentAddTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("TrackSegmentAddTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("tracksegment", theTrackSegment->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("pos", QString::number(Position));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

TrackSegmentAddTrackPointCommand * TrackSegmentAddTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	TrackSegmentAddTrackPointCommand* a = new TrackSegmentAddTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = NULL;
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theTrackSegment = dynamic_cast<TrackSegment*>(d->getFeature(e.attribute("tracksegment")));
	a->theTrackPoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, NULL, e.attribute("trackpoint"));
	a->Position = e.attribute("pos").toUInt();

	return a;
}

/* TRACKSEGMENTREMOVETRACKPOINTCOMMAND */

TrackSegmentRemoveTrackPointCommand::TrackSegmentRemoveTrackPointCommand(TrackSegment* R, TrackPoint* W, MapLayer* aLayer)
: theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theTrackSegment(R), theTrackPoint(W)
{
	redo();
}

TrackSegmentRemoveTrackPointCommand::TrackSegmentRemoveTrackPointCommand(TrackSegment* R, unsigned int anIdx, MapLayer* aLayer)
: theLayer(aLayer), oldLayer(0), Idx(anIdx), theTrackSegment(R), theTrackPoint(R->get(anIdx))
{
	redo();
}

TrackSegmentRemoveTrackPointCommand::~TrackSegmentRemoveTrackPointCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void TrackSegmentRemoveTrackPointCommand::undo()
{
	theTrackSegment->add(theTrackPoint,Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theTrackSegment);
		oldLayer->add(theTrackSegment);
		decDirtyLevel(oldLayer);
	}
}

void TrackSegmentRemoveTrackPointCommand::redo()
{
	theTrackSegment->remove(Idx);
	oldLayer = theTrackSegment->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theTrackSegment);
		incDirtyLevel(oldLayer);
		theLayer->add(theTrackSegment);
	}
}

bool TrackSegmentRemoveTrackPointCommand::buildDirtyList(DirtyList& /* theList */)
{
	return false;
}

bool TrackSegmentRemoveTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("TrackSegmentRemoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("tracksegment", theTrackSegment->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("index", QString::number(Idx));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

TrackSegmentRemoveTrackPointCommand * TrackSegmentRemoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	TrackSegmentRemoveTrackPointCommand* a = new TrackSegmentRemoveTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = NULL;
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theTrackSegment = dynamic_cast<TrackSegment*>(d->getFeature(e.attribute("tracksegment")));
	a->theTrackPoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, NULL, e.attribute("trackpoint"));
	a->Idx = e.attribute("index").toUInt();

	return a;
}




