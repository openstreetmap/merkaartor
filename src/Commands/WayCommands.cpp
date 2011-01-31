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
    }
    decDirtyLevel(oldLayer, theRoad);
}

void WayAddNodeCommand::redo()
{
    oldLayer = theRoad->layer();
    theRoad->add(theTrackPoint, Position);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theRoad);
        theLayer->add(theRoad);
    }
    incDirtyLevel(oldLayer, theRoad);
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
    if (theRoad->isUploadable() && theTrackPoint->isUploadable())
        return theList.update(theRoad);

    return theList.noop(theRoad);
}

bool WayAddNodeCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("RoadAddTrackPointCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("road", theRoad->xmlId());
    stream.writeAttribute("trackpoint", theTrackPoint->xmlId());
    stream.writeAttribute("pos", QString::number(Position));
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

WayAddNodeCommand * WayAddNodeCommand::fromXML(Document * d, QXmlStreamReader& stream)
{
    WayAddNodeCommand* a = new WayAddNodeCommand();
    a->setId(stream.attributes().value("xml:id").toString());
    if (stream.attributes().hasAttribute("layer"))
        a->theLayer = d->getLayer(stream.attributes().value("layer").toString());
    else
        a->theLayer = d->getDirtyOrOriginLayer();
    if (stream.attributes().hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(stream.attributes().value("oldlayer").toString());
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->theRoad = Feature::getWayOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::LineString, stream.attributes().value("road").toString().toLongLong()));
    a->theTrackPoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, stream.attributes().value("trackpoint").toString().toLongLong()));
    a->Position = stream.attributes().value("pos").toString().toUInt();

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Command") {
            Command::fromXML(d, stream, a);
        }
        stream.readNext();
    }

    return a;
}

/* ROADREMOVETRACKPOINTCOMMAND */

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R)
: Command(R), theLayer(0), oldLayer(0), Idx(0), wasClosed(R ? R->isClosed() : false), theRoad(R), theNode(0)
{
}

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R, Node* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), wasClosed(R ? R->isClosed() : false), theRoad(R), theNode(W)
{
    if (!theLayer)
        theLayer = theRoad->layer();
    redo();
}

WayRemoveNodeCommand::WayRemoveNodeCommand(Way* R, int anIdx, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), wasClosed(R ? R->isClosed() : false), theRoad(R), theNode(R->getNode(anIdx))
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
    if (wasClosed && Idx == 0) {
        theRoad->remove(theRoad->size()-1);
        theRoad->add(theNode);
    }

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theRoad);
        oldLayer->add(theRoad);
    }
    decDirtyLevel(oldLayer, theRoad);
}

void WayRemoveNodeCommand::redo()
{
    oldLayer = theRoad->layer();
    theRoad->remove(Idx);
    if (wasClosed && Idx == 0) {
        theRoad->remove(theRoad->size()-1);
        theRoad->add(theRoad->getNode(0));
    }

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theRoad);
        theLayer->add(theRoad);
    }
    incDirtyLevel(oldLayer, theRoad);
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
    if (theRoad->isUploadable() && (theNode->isUploadable() || theNode->hasOSMId()))
        return theList.update(theRoad);

    return theList.noop(theRoad);
}

bool WayRemoveNodeCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("RoadRemoveTrackPointCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("road", theRoad->xmlId());
    stream.writeAttribute("trackpoint", theNode->xmlId());
    stream.writeAttribute("index", QString::number(Idx));
    if (wasClosed)
        stream.writeAttribute("closed", "true");
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

WayRemoveNodeCommand * WayRemoveNodeCommand::fromXML(Document * d, QXmlStreamReader& stream)
{
    WayRemoveNodeCommand* a = new WayRemoveNodeCommand();
    a->setId(stream.attributes().value("xml:id").toString());
    if (stream.attributes().hasAttribute("layer"))
        a->theLayer = d->getLayer(stream.attributes().value("layer").toString());
    else
        a->theLayer = d->getDirtyOrOriginLayer();
    if (stream.attributes().hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(stream.attributes().value("oldlayer").toString());
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->wasClosed = (stream.attributes().hasAttribute("closed") && stream.attributes().value("closed") == "true");
    a->theRoad = Feature::getWayOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::LineString, stream.attributes().value("road").toString().toLongLong()));
    a->theNode = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, stream.attributes().value("trackpoint").toString().toLongLong()));
    a->Idx = stream.attributes().value("index").toString().toUInt();

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Command") {
            Command::fromXML(d, stream, a);
        }
        stream.readNext();
    }

    return a;
}




