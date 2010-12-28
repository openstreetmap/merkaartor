#include "RelationCommands.h"

#include "Relation.h"
#include "Feature.h"
#include "Layer.h"
#include "DirtyList.h"

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R)
: Command(R), theLayer(0), oldLayer(0), theRelation(R), Role(""), theMapFeature(0), Position(0)
{
}

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, Feature* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRelation(R), Role(aRole), theMapFeature(W), Position(theRelation->size())
{
    if (!theLayer)
        theLayer = theRelation->layer();
    redo();
}

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, Feature* W, int aPos, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRelation(R), Role(aRole), theMapFeature(W), Position(aPos)
{
    if (!theLayer)
        theLayer = theRelation->layer();
    redo();
}

RelationAddFeatureCommand::~RelationAddFeatureCommand(void)
{
    if (oldLayer)
        oldLayer->decDirtyLevel(commandDirtyLevel);
}

void RelationAddFeatureCommand::undo()
{
    Command::undo();
    theRelation->remove(Position);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theRelation);
        oldLayer->add(theRelation);
    }
    decDirtyLevel(oldLayer, theRelation);
}

void RelationAddFeatureCommand::redo()
{
    oldLayer = theRelation->layer();
    theRelation->add(Role, theMapFeature, Position);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theRelation);
        theLayer->add(theRelation);
    }
    incDirtyLevel(oldLayer, theRelation);
    Command::redo();
}

bool RelationAddFeatureCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;
    if (theRelation->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(theRelation);
    if (!theRelation->layer())
        return theList.update(theRelation);
    if (theRelation->isUploadable())
        return theList.update(theRelation);

    return theList.noop(theRelation);
}

bool RelationAddFeatureCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("RelationAddFeatureCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("relation", theRelation->xmlId());
    stream.writeAttribute("role", Role);
    stream.writeAttribute("feature", theMapFeature->xmlId());
    stream.writeAttribute("pos", QString::number(Position));
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

RelationAddFeatureCommand * RelationAddFeatureCommand::fromXML(Document * d, QDomElement e)
{
    RelationAddFeatureCommand* a = new RelationAddFeatureCommand();
    a->setId(e.attribute("xml:id"));
    if (e.hasAttribute("layer"))
        a->theLayer = d->getLayer(e.attribute("layer"));
    else
        a->theLayer = d->getDirtyOrOriginLayer();
    if (e.hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(e.attribute("oldlayer"));
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->theRelation = Feature::getRelationOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::OsmRelation, e.attribute("relation").toLongLong()));
    Feature* F;
    if (e.attribute("featureclass") == "TrackPoint") {
        F = (Feature*) Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, e.attribute("trackpoint").toLongLong()));
    } else
    if (e.attribute("featureclass") == "Road") {
        F = (Feature*) Feature::getWayOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::LineString, e.attribute("road").toLongLong()));
    } else
    if (e.attribute("featureclass") == "Relation") {
        F = (Feature*) Feature::getRelationOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::OsmRelation, e.attribute("road").toLongLong()));
    } else {
        if (!(F = d->getFeature(IFeature::FId(IFeature::All, e.attribute("feature").toLongLong()))))
            return NULL;
    }
    a->Role = e.attribute("role");
    a->theMapFeature = F;
    a->Position = e.attribute("pos").toUInt();

    Command::fromXML(d, e, a);

    return a;
}

/* RelationRemoveFeatureCommand */

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R)
: Command(R), theLayer(0), oldLayer(0), Idx(0), theRelation(R), theMapFeature(0)
{
}

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, Feature* W, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theRelation(R), theMapFeature(W)
{
    if (!theLayer)
        theLayer = theRelation->layer();
    Role = R->getRole(Idx);
    redo();
}

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, int anIdx, Layer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), theRelation(R), Role(R->getRole(anIdx)), theMapFeature(R->get(anIdx))
{
    if (!theLayer)
        theLayer = theRelation->layer();
    redo();
}

RelationRemoveFeatureCommand::~RelationRemoveFeatureCommand(void)
{
    oldLayer->decDirtyLevel(commandDirtyLevel);
}


void RelationRemoveFeatureCommand::undo()
{
    Command::undo();
    theRelation->add(Role,theMapFeature,Idx);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theRelation);
        oldLayer->add(theRelation);
    }
    decDirtyLevel(oldLayer, theRelation);
}

void RelationRemoveFeatureCommand::redo()
{
    oldLayer = theRelation->layer();
    theRelation->remove(Idx);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theRelation);
        theLayer->add(theRelation);
    }
    incDirtyLevel(oldLayer, theRelation);
    Command::redo();
}

bool RelationRemoveFeatureCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;
    if (theRelation->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(theRelation);
    if (!theRelation->layer())
        return theList.update(theRelation);
    if (theRelation->isUploadable())
        return theList.update(theRelation);

    return theList.noop(theRelation);
}

bool RelationRemoveFeatureCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("RelationRemoveFeatureCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("relation", theRelation->xmlId());
    stream.writeAttribute("feature", theMapFeature->xmlId());
    stream.writeAttribute("featureclass", theMapFeature->getClass());
    stream.writeAttribute("index", QString::number(Idx));
    stream.writeAttribute("role", Role);
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

RelationRemoveFeatureCommand * RelationRemoveFeatureCommand::fromXML(Document * d, QDomElement e)
{
    RelationRemoveFeatureCommand* a = new RelationRemoveFeatureCommand();

    a->setId(e.attribute("xml:id"));
    if (e.hasAttribute("layer"))
        a->theLayer = d->getLayer(e.attribute("layer"));
    else
        a->theLayer = d->getDirtyOrOriginLayer();
    if (e.hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(e.attribute("oldlayer"));
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    a->theRelation = Feature::getRelationOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::OsmRelation, e.attribute("relation").toLongLong()));
    Feature* F = NULL;
    if (e.attribute("featureclass") == "Node") {
        F = (Feature*) Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::Point, e.attribute("feature").toLongLong()));
    } else
    if (e.attribute("featureclass") == "Way") {
        F = (Feature*) Feature::getWayOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::LineString, e.attribute("feature").toLongLong()));
    } else
    if (e.attribute("featureclass") == "Relation") {
        F = (Feature*) Feature::getRelationOrCreatePlaceHolder(d, a->theLayer, IFeature::FId(IFeature::OsmRelation, e.attribute("feature").toLongLong()));
    } else {
        if (!(F = d->getFeature(IFeature::FId(IFeature::All, e.attribute("feature").toLongLong()))))
            return NULL;
    }
    a->theMapFeature = F;
    a->Idx = e.attribute("index").toInt();
    a->Role = e.attribute("role");

    Command::fromXML(d, e, a);

    return a;
}




