#include "FeatureCommands.h"

#include "Document.h"
#include "Feature.h"
#include "Layer.h"
#include "DirtyList.h"

TagCommand::TagCommand(Feature* aF, Layer* aLayer)
: Command(aF), theFeature(aF), FirstRun(true), theLayer(aLayer), oldLayer(0)
{
}

TagCommand::TagCommand(Feature* aF)
: Command(aF), theLayer(0), oldLayer(0)
{
}

TagCommand::~TagCommand(void)
{
    if (oldLayer)
        oldLayer->decDirtyLevel(commandDirtyLevel);
}

//void TagCommand::undo()
//{
//	theFeature->clearTags();
//	for (int i=0; i<Before.size(); ++i)
//		theFeature->setTag(Before[i].first,Before[i].second);
//}
//
//void TagCommand::redo()
//{
//	if (FirstRun)
//		for (int i=0; i<theFeature->tagSize(); ++i)
//			After.push_back(qMakePair(theFeature->tagKey(i),theFeature->tagValue(i)));
//	else
//	{
//		theFeature->clearTags();
//		for (int i=0; i<After.size(); ++i)
//			theFeature->setTag(After[i].first,After[i].second);
//	}
//	FirstRun = false;
//}
//
bool TagCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;
    if (theFeature->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(theFeature);
    if (theLayer->isUploadable())
        return theList.update(theFeature);

    return theList.noop(theFeature);
}

SetTagCommand::SetTagCommand(Feature* aF)
: TagCommand(aF, 0), theIdx(0), theK(""), theV("")
{
    oldLayer = theFeature->layer();
}

SetTagCommand::SetTagCommand(Feature* aF, int idx, const QString& k, const QString& v, Layer* aLayer)
: TagCommand(aF, aLayer), theIdx(idx), theK(k), theV(v)
{
    description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(k).arg(v).arg(aF->description());
    oldLayer = theFeature->layer();
    redo();
}

SetTagCommand::SetTagCommand(Feature* aF, const QString& k, const QString& v, Layer* aLayer)
: TagCommand(aF, aLayer), theIdx(-1), theK(k), theV(v)
{
    description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(k).arg(v).arg(aF->description());
    oldLayer = theFeature->layer();
    redo();
}

void SetTagCommand::undo()
{
    Command::undo();
    if (theFeature->isUploaded()) {
        theLayer = theLayer->getDocument()->getUploadedLayer();
        oldLayer = theLayer->getDocument()->getDirtyOrOriginLayer();
    }
    if (oldV != TAG_UNDEF_VALUE && oldK != KEY_UNDEF_VALUE)
    {
        if(oldK!=theK) theFeature->clearTag(theK);
        theFeature->setTag(theIdx,oldK,oldV);
    }
    else
        theFeature->clearTag(theK);

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theFeature);
        oldLayer->add(theFeature);
    }
    decDirtyLevel(oldLayer, theFeature);
}

void SetTagCommand::redo()
{
    oldV = TAG_UNDEF_VALUE;
    oldK = KEY_UNDEF_VALUE;
    if (theIdx < 0) {
        oldK = theK;
        oldV = theFeature->tagValue(theK, TAG_UNDEF_VALUE);
        theFeature->setTag(theK,theV);
    } else {
        oldV = theFeature->tagValue(theIdx);
        oldK = theFeature->tagKey(theIdx);
        if(oldK!=theK) theFeature->clearTag(oldK);
        theFeature->setTag(theIdx,theK,theV);
    }

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theFeature);
        theLayer->add(theFeature);
    }
    incDirtyLevel(oldLayer, theFeature);
    Command::redo();
}

bool SetTagCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;
    if (theFeature->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(theFeature);
    if (theK.startsWith('_') && (theK.endsWith('_')))
        return theList.noop(theFeature);
    if (!theFeature->isUploadable())
        return theList.noop(theFeature);

    return theList.update(theFeature);
}

bool SetTagCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("SetTagCommand");
    stream.writeAttribute("oldkey", oldK);
    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("feature", theFeature->xmlId());
    stream.writeAttribute("idx", QString::number(theIdx));
    stream.writeAttribute("key", theK);
    stream.writeAttribute("value", theV);
    stream.writeAttribute("oldvalue", oldV);
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

SetTagCommand * SetTagCommand::fromXML(Document * d, QDomElement e)
{
    Feature* F;
    if (!(F = d->getFeature(IFeature::FId(IFeature::All, e.attribute("feature").toLongLong())))) {
        qDebug() << "SetTagCommand::fromXML: Undefined feature: " << e.attribute("feature");
        return NULL;
    }

    SetTagCommand* a = new SetTagCommand(F);
    a->setId(e.attribute("xml:id"));
    if (e.hasAttribute("oldkey"))
        a->oldK = e.attribute("oldkey");
    a->theFeature = F;
    a->theIdx = e.attribute("idx").toInt();
    a->theK = e.attribute("key");
    a->theV = e.attribute("value");
    if (e.hasAttribute("oldvalue"))
        a->oldV = e.attribute("oldvalue");
    if (e.hasAttribute("layer"))
        a->theLayer = d->getLayer(e.attribute("layer"));
    else
        a->theLayer = NULL;
    if (e.hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(e.attribute("oldlayer"));
    else
        a->oldLayer = NULL;

    a->description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(a->theK).arg(a->theV).arg(a->theFeature->description());

    Command::fromXML(d, e, a);

    return a;
}


/* CLEARTAGSCOMMAND */

ClearTagsCommand::ClearTagsCommand(Feature* F, Layer* aLayer)
: TagCommand(F, aLayer)
{
    for (int i=0; i<theFeature->tagSize(); ++i)
        Before.push_back(qMakePair(theFeature->tagKey(i),theFeature->tagValue(i)));
    oldLayer = theFeature->layer();
    redo();
}

void ClearTagsCommand::undo()
{
    Command::undo();
    if (theFeature->isUploaded()) {
        theLayer = theLayer->getDocument()->getUploadedLayer();
        oldLayer = theLayer->getDocument()->getDirtyOrOriginLayer();
    }
    theFeature->clearTags();
    for (int i=0; i<Before.size(); ++i)
        theFeature->setTag(Before[i].first,Before[i].second);

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theFeature);
        oldLayer->add(theFeature);
    }
    decDirtyLevel(oldLayer, theFeature);
}

void ClearTagsCommand::redo()
{
    theFeature->clearTags();

    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theFeature);
        theLayer->add(theFeature);
    }
    incDirtyLevel(oldLayer, theFeature);
    Command::redo();
}

bool ClearTagsCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("ClearTagsCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("feature", theFeature->xmlId());
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    for (int i=0; i<Before.size(); ++i)
    {
        stream.writeStartElement("tag");
        stream.writeAttribute("k", Before[i].first);
        stream.writeAttribute("v", Before[i].second);
        stream.writeEndElement();
    }

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

ClearTagsCommand * ClearTagsCommand::fromXML(Document * d, QDomElement e)
{
    Feature* F;
    if (!(F = d->getFeature(IFeature::FId(IFeature::All, e.attribute("feature").toLongLong()))))
        return NULL;

    ClearTagsCommand* a = new ClearTagsCommand(F);
    a->setId(e.attribute("xml:id"));
    a->theFeature = F;
    if (e.hasAttribute("layer"))
        a->theLayer = d->getLayer(e.attribute("layer"));
    else
        a->theLayer = NULL;
    if (e.hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(e.attribute("oldlayer"));
    else
        a->oldLayer = NULL;

    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "tag") {
            a->Before.push_back(qMakePair(c.attribute("k"),c.attribute("v")));
        }
        c = c.nextSiblingElement();
    }

    Command::fromXML(d, e, a);

    return a;
}

/* CLEARTAGCOMMAND */

ClearTagCommand::ClearTagCommand(Feature* F)
: TagCommand(F, 0), theIdx(0), theK(""), theV("")
{
    oldLayer = theFeature->layer();
}

ClearTagCommand::ClearTagCommand(Feature* F, const QString& k, Layer* aLayer)
: TagCommand(F, aLayer), theIdx(F->findKey(k)), theK(k), theV(F->tagValue(k, ""))
{
    description = MainWindow::tr("Clear Tag '%1' on %2").arg(k).arg(F->description());
    oldLayer = theFeature->layer();
    redo();
}

void ClearTagCommand::undo()
{
    Command::undo();
    if (theFeature->isUploaded()) {
        theLayer = theLayer->getDocument()->getUploadedLayer();
        oldLayer = theLayer->getDocument()->getDirtyOrOriginLayer();
    }
    theFeature->setTag(theIdx,theK,theV);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        theLayer->remove(theFeature);
        oldLayer->add(theFeature);
    }
    decDirtyLevel(oldLayer, theFeature);
}

void ClearTagCommand::redo()
{
    theFeature->clearTag(theK);
    if (theLayer && oldLayer && (theLayer != oldLayer)) {
        oldLayer->remove(theFeature);
        theLayer->add(theFeature);
    }
    incDirtyLevel(oldLayer, theFeature);
    Command::redo();
}

bool ClearTagCommand::buildDirtyList(DirtyList& theList)
{
    if (isUndone)
        return false;
    if (theFeature->lastUpdated() == Feature::NotYetDownloaded)
        return theList.noop(theFeature);
    if (theK.startsWith('_') && (theK.endsWith('_')))
        return theList.noop(theFeature);
    if (!theFeature->isUploadable())
        return theList.noop(theFeature);

    return theList.update(theFeature);
}

bool ClearTagCommand::toXML(QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement("ClearTagCommand");

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("feature", theFeature->xmlId());
    stream.writeAttribute("idx", QString::number(theIdx));
    stream.writeAttribute("key", theK);
    stream.writeAttribute("value", theV);
    if (theLayer)
        stream.writeAttribute("layer", theLayer->id());
    if (oldLayer)
        stream.writeAttribute("oldlayer", oldLayer->id());

    Command::toXML(stream);
    stream.writeEndElement();

    return OK;
}

ClearTagCommand * ClearTagCommand::fromXML(Document * d, QDomElement e)
{
    Feature* F;
    if (!(F = d->getFeature(IFeature::FId(IFeature::All, e.attribute("feature").toLongLong()))))
        return NULL;

    ClearTagCommand* a = new ClearTagCommand(F);
    a->setId(e.attribute("xml:id"));
    a->theFeature = F;
    a->theIdx = e.attribute("idx").toInt();
    a->theK = e.attribute("key");
    a->theV = e.attribute("value");
    if (e.hasAttribute("layer"))
        a->theLayer = d->getLayer(e.attribute("layer"));
    else
        a->theLayer = NULL;
    if (e.hasAttribute("oldlayer"))
        a->oldLayer = d->getLayer(e.attribute("oldlayer"));
    else
        a->oldLayer = NULL;

    a->description = MainWindow::tr("Clear Tag '%1' on %2").arg(a->theK).arg(a->theFeature->description());

    Command::fromXML(d, e, a);

    return a;
}


