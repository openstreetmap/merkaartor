#include "FeatureCommands.h"

#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"
#include "Sync/DirtyList.h"

TagCommand::TagCommand(MapFeature* aF, MapLayer* aLayer)
: Command(aF), theFeature(aF), FirstRun(true), theLayer(aLayer), oldLayer(0)
{
}

TagCommand::TagCommand(MapFeature* aF)
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
//	for (unsigned int i=0; i<Before.size(); ++i)
//		theFeature->setTag(Before[i].first,Before[i].second);
//}
//
//void TagCommand::redo()
//{
//	if (FirstRun)
//		for (unsigned int i=0; i<theFeature->tagSize(); ++i)
//			After.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
//	else
//	{
//		theFeature->clearTags();
//		for (unsigned int i=0; i<After.size(); ++i)
//			theFeature->setTag(After[i].first,After[i].second);
//	}
//	FirstRun = false;
//}
//
bool TagCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theFeature->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theFeature);
	if (theLayer->isUploadable())
		return theList.update(theFeature);

	return theList.noop(theFeature);
}

SetTagCommand::SetTagCommand(MapFeature* aF)
: TagCommand(aF, 0), theIdx(0), theK(""), theV("")
{
	oldLayer = theFeature->layer();
}

SetTagCommand::SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v, MapLayer* aLayer)
: TagCommand(aF, aLayer), theIdx(idx), theK(k), theV(v)
{
	description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(k).arg(v).arg(aF->description());
	oldLayer = theFeature->layer();
	redo();
}

SetTagCommand::SetTagCommand(MapFeature* aF, const QString& k, const QString& v, MapLayer* aLayer)
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
		decDirtyLevel(oldLayer);
	}
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
		incDirtyLevel(oldLayer);
		theLayer->add(theFeature);

	}
	Command::redo();
}

bool SetTagCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theFeature->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theFeature);
	if (theK.startsWith('_') && (theK.endsWith('_')))
		return theList.noop(theFeature);
	else
		return theList.update(theFeature);
}

bool SetTagCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("SetTagCommand");
	xParent.appendChild(e);

	e.setAttribute("oldkey", oldK);
	e.setAttribute("xml:id", id());
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("idx", QString::number(theIdx));
	e.setAttribute("key", theK);
	e.setAttribute("value", theV);
	e.setAttribute("oldvalue", oldV);
	if (theLayer)
	    e.setAttribute("layer", theLayer->id());
	if (oldLayer)
	    e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

SetTagCommand * SetTagCommand::fromXML(MapDocument * d, QDomElement e)
{
	MapFeature* F;
	if (!(F = d->getFeature(e.attribute("feature"), false)))
		return NULL;

	SetTagCommand* a = new SetTagCommand(F);
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("oldkey"))
		a->oldK = e.attribute("oldkey");
	if (!(F = d->getFeature(e.attribute("feature"), false)))
		return NULL;
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

ClearTagsCommand::ClearTagsCommand(MapFeature* F, MapLayer* aLayer)
: TagCommand(F, aLayer)
{
	for (unsigned int i=0; i<theFeature->tagSize(); ++i)
		Before.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
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
	for (unsigned int i=0; i<Before.size(); ++i)
		theFeature->setTag(Before[i].first,Before[i].second);

	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theFeature);
		oldLayer->add(theFeature);
		decDirtyLevel(oldLayer);
	}
}

void ClearTagsCommand::redo()
{
	theFeature->clearTags();

	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theFeature);
		incDirtyLevel(oldLayer);
		theLayer->add(theFeature);
	}
	Command::redo();
}

bool ClearTagsCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("ClearTagsCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("feature", theFeature->xmlId());
	if (theLayer)
	    e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	for (unsigned int i=0; i<Before.size(); ++i)
	{
		QDomElement c = e.ownerDocument().createElement("tag");
		e.appendChild(c);

		c.setAttribute("k", Before[i].first);
		c.setAttribute("v", Before[i].second);
	}

	Command::toXML(e);

	return OK;
}

ClearTagsCommand * ClearTagsCommand::fromXML(MapDocument * d, QDomElement e)
{
	MapFeature* F;
	if (!(F = d->getFeature(e.attribute("feature"), false)))
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
			a->Before.push_back(std::make_pair(c.attribute("k"),c.attribute("v")));
		}
		c = c.nextSiblingElement();
	}

	Command::fromXML(d, e, a);

	return a;
}

/* CLEARTAGCOMMAND */

ClearTagCommand::ClearTagCommand(MapFeature* F)
: TagCommand(F, 0), theIdx(0), theK(""), theV("")
{
	oldLayer = theFeature->layer();
}

ClearTagCommand::ClearTagCommand(MapFeature* F, const QString& k, MapLayer* aLayer)
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
		decDirtyLevel(oldLayer);
	}
}

void ClearTagCommand::redo()
{
	theFeature->clearTag(theK);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theFeature);
		incDirtyLevel(oldLayer);
		theLayer->add(theFeature);
	}
	Command::redo();
}

bool ClearTagCommand::buildDirtyList(DirtyList& theList)
{
	if (isUndone)
		return false;
	if (theFeature->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theFeature);
	if (theLayer->isUploadable())
		if (theK.startsWith('_') && (theK.endsWith('_')))
		return theList.noop(theFeature);
		else
			return theList.update(theFeature);
	else
		return theList.noop(theFeature);
}

bool ClearTagCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("ClearTagCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("idx", QString::number(theIdx));
	e.setAttribute("key", theK);
	e.setAttribute("value", theV);
	if (theLayer)
	    e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

ClearTagCommand * ClearTagCommand::fromXML(MapDocument * d, QDomElement e)
{
	MapFeature* F;
	if (!(F = d->getFeature(e.attribute("feature"), false)))
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


