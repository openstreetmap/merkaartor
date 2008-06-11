#include "FeatureCommands.h"

#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"
#include "Sync/DirtyList.h"

#define KEY_UNDEF_VALUE "%%%%%"
#define TAG_UNDEF_VALUE "%%%%%"

TagCommand::TagCommand(MapFeature* aF, MapLayer* aLayer)
: theFeature(aF), FirstRun(true), theLayer(aLayer), oldLayer(0)
{
}

TagCommand::TagCommand()
: theLayer(0), oldLayer(0)
{
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
	return theList.update(theFeature);
}

SetTagCommand::SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v, MapLayer* aLayer)
: TagCommand(aF, aLayer), theIdx(idx), theK(k), theV(v)
{
	description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(k).arg(v).arg(aF->description());
	mainFeature = aF;
	redo();
}

SetTagCommand::SetTagCommand(MapFeature* aF, const QString& k, const QString& v, MapLayer* aLayer)
: TagCommand(aF, aLayer), theIdx(-1), theK(k), theV(v)
{
	description = MainWindow::tr("Set Tag '%1=%2' on %3").arg(k).arg(v).arg(aF->description());
	mainFeature = aF;
	redo();
}

void SetTagCommand::undo()
{
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

	oldLayer = theFeature->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theFeature);
		theLayer->add(theFeature);
	}
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
	if (oldLayer)
	    e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

SetTagCommand * SetTagCommand::fromXML(MapDocument * d, QDomElement e)
{
	SetTagCommand* a = new SetTagCommand();
	a->setId(e.attribute("xml:id"));
	MapFeature* F;
	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
	if (e.hasAttribute("oldkey"))
		a->oldK = e.attribute("oldkey");
			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
				return NULL;
	a->theFeature = F;
	a->theIdx = e.attribute("idx").toUInt();
	a->theK = e.attribute("key");
	a->theV = e.attribute("value");
	a->theLayer = d->getDirtyLayer();
    if (e.hasAttribute("oldvalue"))
        a->oldV = e.attribute("oldvalue");
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;

	return a;
}


/* CLEARTAGSCOMMAND */

//ClearTagsCommand::ClearTagsCommand(MapFeature* F)
//: TagCommand(F)
//{
//	for (unsigned int i=0; i<theFeature->tagSize(); ++i)
//		Before.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
//	redo();
//}
//
//void ClearTagsCommand::undo()
//{
//	for (unsigned int i=0; i<Before.size(); ++i)
//		theFeature->setTag(Before[i].first,Before[i].second);
//}
//
//void ClearTagsCommand::redo()
//{
//	theFeature->clearTags();
//}
//
//bool ClearTagsCommand::toXML(QDomElement& xParent) const
//{
//	bool OK = true;
//
//	QDomElement e = xParent.ownerDocument().createElement("ClearTagsCommand");
//	xParent.appendChild(e);
//
//	e.setAttribute("xml:id", id());
//	e.setAttribute("feature", theFeature->xmlId());
//
//	return OK;
//}
//
//ClearTagsCommand * ClearTagsCommand::fromXML(MapDocument * d, QDomElement e)
//{
//	ClearTagsCommand* a = new ClearTagsCommand();
//	a->setId(e.attribute("xml:id"));
//	MapFeature* F;
//	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
//		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
//			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
//				return NULL;
//	a->theFeature = F;
//
//	return a;
//}
//
/* CLEARTAGCOMMAND */

ClearTagCommand::ClearTagCommand(MapFeature* F, const QString& k, MapLayer* aLayer)
: TagCommand(F, aLayer), theIdx(F->findKey(k)), theK(k), theV(F->tagValue(k, ""))
{
	description = MainWindow::tr("Clear Tag '%1' on %2").arg(k).arg(F->description());
	mainFeature = F;
	redo();
}

void ClearTagCommand::undo()
{
	theFeature->setTag(theIdx,theK,theV);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theFeature);
		oldLayer->add(theFeature);
	}
}

void ClearTagCommand::redo()
{
	theFeature->clearTag(theK);
	oldLayer = theFeature->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theFeature);
		theLayer->add(theFeature);
	}
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
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	return OK;
}

ClearTagCommand * ClearTagCommand::fromXML(MapDocument * d, QDomElement e)
{
	ClearTagCommand* a = new ClearTagCommand();
	a->setId(e.attribute("xml:id"));
	MapFeature* F;
	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
				return NULL;
	a->theFeature = F;
	a->theIdx = e.attribute("idx").toUInt();
	a->theK = e.attribute("key");
	a->theV = e.attribute("value");
	a->theLayer = d->getDirtyLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;

	return a;
}


