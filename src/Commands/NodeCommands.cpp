#include "NodeCommands.h"

#include "Node.h"
#include "Layer.h"
#include "DirtyList.h"

MoveNodeCommand::MoveNodeCommand()
: Command(0), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
}

MoveNodeCommand::MoveNodeCommand(Node* aPt)
: Command(aPt), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
    if (!theLayer)
        theLayer = thePoint->layer();
    description = MainWindow::tr("Move node %1").arg(aPt->description());
}

MoveNodeCommand::MoveNodeCommand(Node* aPt, const Coord& aPos, Layer* aLayer)
: Command(aPt), theLayer(aLayer), oldLayer(0), thePoint(aPt), OldPos(aPt->position()), NewPos(aPos)
{
    if (!theLayer)
        theLayer = thePoint->layer();
    description = MainWindow::tr("Move node %1").arg(aPt->description());
    redo();
}

MoveNodeCommand::~MoveNodeCommand(void)
{
    if (oldLayer)
        oldLayer->decDirtyLevel(commandDirtyLevel);
}

void MoveNodeCommand::undo()
{
    Command::undo();
    thePoint->setPosition(OldPos);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(thePoint);
        oldLayer->add(thePoint);
    }
    decDirtyLevel(oldLayer, thePoint);
}

void MoveNodeCommand::redo()
{
    oldLayer = thePoint->layer();
    thePoint->setPosition(NewPos);
    thePoint->setVirtual(false);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(thePoint);
        theLayer->add(thePoint);
    }
    incDirtyLevel(oldLayer, thePoint);
    Command::redo();
}

bool MoveNodeCommand::buildDirtyList(DirtyList &theList)
{
    if (isUndone)
        return false;
    if (thePoint->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(thePoint);
    if (!thePoint->layer())
        return theList.update(thePoint);
    if (thePoint->isUploadable())
        return theList.update(thePoint);

    return theList.noop(thePoint);
}

bool MoveNodeCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("MoveTrackPointCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("trackpoint", thePoint->xmlId());
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());
    OldPos.toXML("oldpos", stream);
    NewPos.toXML("newpos", stream);

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

MoveNodeCommand * MoveNodeCommand::fromXML(Document * d, QXmlStreamReader& stream)
{
    MoveNodeCommand* a = new MoveNodeCommand();
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

    a->thePoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, stream.attributes().value("trackpoint").toString().toLongLong()));
    a->description = MainWindow::tr("Move node %1").arg(a->thePoint->description());

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "oldpos") {
            a->OldPos = Coord::fromXML(stream);
        } else if (stream.name() == "newpos") {
            a->NewPos = Coord::fromXML(stream);
        }
        stream.readNext();
    }

    Command::fromXML(d, stream, a);

    return a;
}



