#include "FeatureCommands.h"

#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Sync/DirtyList.h"

#define TAG_UNDEF_VALUE "%%%%%"

TagCommand::TagCommand(MapFeature* aF)
: theFeature(aF), FirstRun(true)
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

SetTagCommand::SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v)
: TagCommand(aF), theIdx(idx), theK(k), theV(v)
{
	redo();
}

SetTagCommand::SetTagCommand(MapFeature* aF, const QString& k, const QString& v)
: TagCommand(aF), theIdx(-1), theK(k), theV(v)
{
	redo();
}

void SetTagCommand::undo()
{
	if (oldV != TAG_UNDEF_VALUE)
		theFeature->setTag(theK,oldV);
	else
		theFeature->clearTag(theK);
}

void SetTagCommand::redo()
{
	oldV = TAG_UNDEF_VALUE;
	if (theIdx < 0) {
		oldV = theFeature->tagValue(theK, TAG_UNDEF_VALUE);
		theFeature->setTag(theK,theV);
	} else {
		oldV = theFeature->tagValue(theIdx);
		theFeature->setTag(theIdx,theK,theV);
	}
}

bool SetTagCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("SetTagCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("idx", QString::number(theIdx));
	e.setAttribute("key", theK);
	e.setAttribute("value", theV);
	e.setAttribute("oldvalue", oldV);

	return OK;
}

SetTagCommand * SetTagCommand::fromXML(MapDocument * d, QDomElement e)
{
	SetTagCommand* a = new SetTagCommand();
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
	if (e.hasAttribute("oldvalue"))
		a->oldV = e.attribute("oldvalue");

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

ClearTagCommand::ClearTagCommand(MapFeature* F, const QString& k)
: TagCommand(F), theIdx(F->findKey(k)), theK(k), theV(F->tagValue(k, ""))
{
	redo();
}

void ClearTagCommand::undo()
{
	theFeature->setTag(theIdx,theK,theV);
}

void ClearTagCommand::redo()
{
	theFeature->clearTag(theK);
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

	return a;
}


