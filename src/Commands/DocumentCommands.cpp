#include "DocumentCommands.h"
#include "Document.h"
#include "Layer.h"
#include "Features.h"
#include "DirtyList.h"

AddFeatureCommand::AddFeatureCommand(Feature* aFeature)
: Command(aFeature), theLayer(0), theFeature(0), UserAdded(false)
{
}

AddFeatureCommand::AddFeatureCommand(Layer* aLayer, Feature* aFeature, bool aUserAdded)
: Command(aFeature), theLayer(aLayer), theFeature(aFeature), UserAdded(aUserAdded)
{
    redo();
}

AddFeatureCommand::~AddFeatureCommand()
{
    if (theLayer)
        theLayer->decDirtyLevel(commandDirtyLevel);
}

void AddFeatureCommand::undo()
{
    Command::undo();
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theFeature);
        oldLayer->add(theFeature);
    } else
        theFeature->setDeleted(true);

    decDirtyLevel(theLayer, theFeature);
}

void AddFeatureCommand::redo()
{
    oldLayer = theFeature->layer();
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theFeature);
        theLayer->add(theFeature);
    } else {
        if (!oldLayer)
            theLayer->add(theFeature);
        else {
            theFeature->setDeleted(false);
            oldLayer = NULL;
        }
        incDirtyLevel(theLayer, theFeature);
    }
    Command::redo();
}

bool AddFeatureCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;

    if (UserAdded)
        if (theFeature->isUploadable())
            return theList.add(theFeature);
    return false;
}

bool AddFeatureCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("AddFeatureCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());
    stream.writeAttribute("feature", theFeature->xmlId());
    stream.writeAttribute("useradded", QString(UserAdded ? "true" : "false"));

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

AddFeatureCommand * AddFeatureCommand::fromXML(Document* d, QXmlStreamReader& stream)
{
    AddFeatureCommand* a = new AddFeatureCommand();

    a->setId(stream.attributes().value("xml:id").toString());
    a->theLayer = d->getLayer(stream.attributes().value("layer").toString());
    if (stream.attributes().hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(stream.attributes().value("oldlayer").toString());
    else
        a->oldLayer = NULL;
    if (!a->theLayer)
        return NULL;

    Feature* F;
    if (!(F = d->getFeature(IFeature::FId(IFeature::All, stream.attributes().value("feature").toString().toLongLong()))))
        return NULL;

    a->theFeature = F;
    a->UserAdded = (stream.attributes().value("useradded") == "true" ? true : false);

    Command::fromXML(d, stream, a);

    return a;
}

/* REMOVEFEATURECOMMAND */

RemoveFeatureCommand::RemoveFeatureCommand(Feature *aFeature)
: Command(aFeature), theLayer(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false)
{
}

RemoveFeatureCommand::RemoveFeatureCommand(Document *theDocument, Feature *aFeature)
: Command(aFeature), theLayer(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false)
{
    theLayer = theDocument->getDirtyOrOriginLayer(aFeature->layer());
    redo();
}

RemoveFeatureCommand::RemoveFeatureCommand(Document *theDocument, Feature *aFeature, const QList<Feature*>& Alternatives)
: Command(aFeature), theLayer(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), theAlternatives(Alternatives)
{
    CascadedCleanUp  = new CommandList(MainWindow::tr("Cascaded cleanup"), NULL);
    for (int i=0; i<aFeature->sizeParents(); ++i) {
        Feature* f = CAST_FEATURE(aFeature->getParent(i));
        if (f)
            f->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
    }
    for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
        it.get()->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
    if (CascadedCleanUp->empty())
    {
        SAFE_DELETE(CascadedCleanUp);
        CascadedCleanUp = 0;
    } else
        CascadedCleanUp->undo();
    theLayer = theDocument->getDirtyOrOriginLayer(aFeature->layer());
    redo();
}

RemoveFeatureCommand::~RemoveFeatureCommand()
{
    if (oldLayer)
        oldLayer->decDirtyLevel(commandDirtyLevel);
    SAFE_DELETE(CascadedCleanUp);
    if (theLayer->getDocument()->exists(theFeature) && theFeature->isDeleted()) {
        theLayer->getDocument()->deleteFeature(theFeature);
    }
}

void RemoveFeatureCommand::redo()
{
    if (!theFeature)
        return;
    if (CascadedCleanUp)
        CascadedCleanUp->redo();
    oldLayer = theFeature->layer();
    oldLayer->remove(theFeature);
    theLayer->add(theFeature);
    theFeature->setDeleted(true);
    incDirtyLevel(oldLayer, theFeature);
    Command::redo();
}

void RemoveFeatureCommand::undo()
{
    if (!theFeature)
        return;
    Command::undo();
    theLayer->remove(theFeature);
    oldLayer->add(theFeature);
    theFeature->setDeleted(false);
    decDirtyLevel(oldLayer, theFeature);
    if (CascadedCleanUp)
        CascadedCleanUp->undo();
}

bool RemoveFeatureCommand::buildDirtyList(DirtyList &theList)
{
    if (!theFeature)
        return false;
    if (isUndone)
        return false;
    if (!oldLayer->isUploadable())
        return false;

    if (theFeature->lastUpdated() == Feature::OSMServerConflict)
        return false;

    //if (!theFeature->hasOSMId())
    //	return false;

    bool CascadedResult = true;
    if (CascadedCleanUp)
        CascadedResult = CascadedCleanUp->buildDirtyList(theList);

    if (!RemoveExecuted)
        RemoveExecuted = theList.erase(theFeature);
    return RemoveExecuted && CascadedResult;
}

bool RemoveFeatureCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("RemoveFeatureCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("layer", oldLayer->id());
    stream.writeAttribute("newlayer", theLayer->id());
    stream.writeAttribute("feature", theFeature->xmlId());

    if (CascadedCleanUp) {
        stream.writeStartElement("Cascaded");
        CascadedCleanUp->toXML(stream);
        stream.writeEndElement();
    }
// 	if (theAlternatives.size() > 0) {
// 		QList<MapFeature*>::const_iterator myFeatIter;
// 		for(myFeatIter = theAlternatives.begin();
// 			myFeatIter != theAlternatives.end();
// 			myFeatIter++)
// 		{
// 			QDomElement alt = xParent.ownerDocument().createElement("Alternative");
// 			e.appendChild(alt);
//
// 			alt.setAttribute("xml:id", id());
// 		}
// 	}

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

RemoveFeatureCommand * RemoveFeatureCommand::fromXML(Document* d, QXmlStreamReader& stream)
{
    RemoveFeatureCommand* a = new RemoveFeatureCommand();

    a->setId(stream.attributes().value("xml:id").toString());
    a->oldLayer = d->getLayer(stream.attributes().value("layer").toString());
    if (stream.attributes().hasAttribute("newlayer"))
        a->theLayer = d->getLayer(stream.attributes().value("newlayer").toString());
    else
        a->theLayer = d->getDirtyOrOriginLayer();
    if (!a->theLayer)
        return NULL;

    Feature* F;
    if (!(F = d->getFeature(IFeature::FId(IFeature::All, stream.attributes().value("feature").toString().toLongLong()))))
        return NULL;
    a->theFeature = F;

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Cascaded") {
            a->CascadedCleanUp = CommandList::fromXML(d, stream);
        }
        stream.readNext();
    }

    Command::fromXML(d, stream, a);

    return a;
}


