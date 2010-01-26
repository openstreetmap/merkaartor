#include "TrackSegmentCommands.h"

#include "TrackSegment.h"
#include "Node.h"
#include "Layer.h"
#include "DirtyList.h"

TrackSegmentAddNodeCommand::TrackSegmentAddNodeCommand(TrackSegment* R) 
: Command(R), theLayer(0), oldLayer(0), theTrackSegment(R), theNode(0), Position(0)
{
}

TrackSegmentAddNodeCommand::TrackSegmentAddNodeCommand(TrackSegment* R, Node* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theTrackSegment(R), theNode(W), Position(theTrackSegment->size())
{
	redo();
}

TrackSegmentAddNodeCommand::TrackSegmentAddNodeCommand(TrackSegment* R, Node* W, int aPos, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theTrackSegment(R), theNode(W), Position(aPos)
{
	redo();
}

TrackSegmentAddNodeCommand::~TrackSegmentAddNodeCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void TrackSegmentAddNodeCommand::undo()
{
	Command::undo();
	theTrackSegment->remove(Position);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theTrackSegment);
		oldLayer->add(theTrackSegment);
		decDirtyLevel(oldLayer);
	}
}

void TrackSegmentAddNodeCommand::redo()
{
	theTrackSegment->add(theNode, Position);
	oldLayer = theTrackSegment->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theTrackSegment);
		incDirtyLevel(oldLayer);
		theLayer->add(theTrackSegment);
	}
	Command::redo();
}

bool TrackSegmentAddNodeCommand::buildDirtyList(DirtyList& /* theList */)
{
	return false;
}

bool TrackSegmentAddNodeCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("TrackSegmentAddTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("tracksegment", theTrackSegment->xmlId());
	e.setAttribute("trackpoint", theNode->xmlId());
	e.setAttribute("pos", QString::number(Position));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

TrackSegmentAddNodeCommand * TrackSegmentAddNodeCommand::fromXML(Document * d, QDomElement e)
{
	TrackSegmentAddNodeCommand* a = new TrackSegmentAddNodeCommand();
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
	a->theNode = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Position = e.attribute("pos").toUInt();

	return a;
}

/* TRACKSEGMENTREMOVETRACKPOINTCOMMAND */

TrackSegmentRemoveNodeCommand::TrackSegmentRemoveNodeCommand(TrackSegment* R) 
: Command(R), theLayer(0), oldLayer(0), Idx(0), theTrackSegment(R), theTrackPoint(0)
{
};

TrackSegmentRemoveNodeCommand::TrackSegmentRemoveNodeCommand(TrackSegment* R, Node* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theTrackSegment(R), theTrackPoint(W)
{
	redo();
}

TrackSegmentRemoveNodeCommand::TrackSegmentRemoveNodeCommand(TrackSegment* R, int anIdx, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), theTrackSegment(R), theTrackPoint(dynamic_cast <Node*> (R->get(anIdx)))
{
	redo();
}

TrackSegmentRemoveNodeCommand::~TrackSegmentRemoveNodeCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void TrackSegmentRemoveNodeCommand::undo()
{
	Command::undo();
	theTrackSegment->add(theTrackPoint,Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theTrackSegment);
		oldLayer->add(theTrackSegment);
		decDirtyLevel(oldLayer);
	}
}

void TrackSegmentRemoveNodeCommand::redo()
{
	theTrackSegment->remove(Idx);
	oldLayer = theTrackSegment->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theTrackSegment);
		incDirtyLevel(oldLayer);
		theLayer->add(theTrackSegment);
	}
	Command::redo();
}

bool TrackSegmentRemoveNodeCommand::buildDirtyList(DirtyList& /* theList */)
{
	return false;
}

bool TrackSegmentRemoveNodeCommand::toXML(QDomElement& xParent) const
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

TrackSegmentRemoveNodeCommand * TrackSegmentRemoveNodeCommand::fromXML(Document * d, QDomElement e)
{
	TrackSegmentRemoveNodeCommand* a = new TrackSegmentRemoveNodeCommand();
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
	a->theTrackPoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	a->Idx = e.attribute("index").toUInt();

	return a;
}




