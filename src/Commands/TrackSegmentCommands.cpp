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
    }
    decDirtyLevel(oldLayer, theTrackSegment);
}

void TrackSegmentAddNodeCommand::redo()
{
    theTrackSegment->add(theNode, Position);
    oldLayer = theTrackSegment->layer();
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theTrackSegment);
        theLayer->add(theTrackSegment);
    }
    incDirtyLevel(oldLayer, theTrackSegment);
    Command::redo();
}

bool TrackSegmentAddNodeCommand::buildDirtyList(DirtyList& /* theList */)
{
    return false;
}

bool TrackSegmentAddNodeCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("TrackSegmentAddTrackPointCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("tracksegment", theTrackSegment->xmlId());
    stream.writeAttribute("trackpoint", theNode->xmlId());
    stream.writeAttribute("pos", QString::number(Position));
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    stream.writeEndElement();
    return OK;
}

TrackSegmentAddNodeCommand * TrackSegmentAddNodeCommand::fromXML(Document * d, QXmlStreamReader& stream)
{
    TrackSegmentAddNodeCommand* a = new TrackSegmentAddNodeCommand();
    a->setId(stream.attributes().value("xml:id").toString());
    if (stream.attributes().hasAttribute("layer"))
        a->theLayer = d->getLayer(stream.attributes().value("layer").toString());
    else
        a->theLayer = NULL;
    if (stream.attributes().hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(stream.attributes().value("oldlayer").toString());
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->theTrackSegment = dynamic_cast<TrackSegment*>(d->getFeature(IFeature::FId(IFeature::GpxSegment, stream.attributes().value("tracksegment").toString().toLongLong())));
    a->theNode = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, stream.attributes().value("trackpoint").toString().toLongLong()));
    a->Position = stream.attributes().value("pos").toString().toUInt();

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
    }
    decDirtyLevel(oldLayer, theTrackSegment);
}

void TrackSegmentRemoveNodeCommand::redo()
{
    theTrackSegment->remove(Idx);
    oldLayer = theTrackSegment->layer();
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theTrackSegment);
        theLayer->add(theTrackSegment);
    }
    incDirtyLevel(oldLayer, theTrackSegment);
    Command::redo();
}

bool TrackSegmentRemoveNodeCommand::buildDirtyList(DirtyList& /* theList */)
{
    return false;
}

bool TrackSegmentRemoveNodeCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("TrackSegmentRemoveTrackPointCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("tracksegment", theTrackSegment->xmlId());
    stream.writeAttribute("trackpoint", theTrackPoint->xmlId());
    stream.writeAttribute("index", QString::number(Idx));
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    stream.writeEndElement();
    return OK;
}

TrackSegmentRemoveNodeCommand * TrackSegmentRemoveNodeCommand::fromXML(Document * d, QXmlStreamReader& stream)
{
    TrackSegmentRemoveNodeCommand* a = new TrackSegmentRemoveNodeCommand();
    a->setId(stream.attributes().value("xml:id").toString());
    if (stream.attributes().hasAttribute("layer"))
        a->theLayer = d->getLayer(stream.attributes().value("layer").toString());
    else
        a->theLayer = NULL;
    if (stream.attributes().hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(stream.attributes().value("oldlayer").toString());
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->theTrackSegment = dynamic_cast<TrackSegment*>(d->getFeature(IFeature::FId(IFeature::GpxSegment, stream.attributes().value("tracksegment").toString().toLongLong())));
    a->theTrackPoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, stream.attributes().value("trackpoint").toString().toLongLong()));
    a->Idx = stream.attributes().value("index").toString().toUInt();

    return a;
}




