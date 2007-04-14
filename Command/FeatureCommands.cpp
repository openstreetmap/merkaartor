#include "FeatureCommands.h"

#include "Map/MapFeature.h"
#include "Sync/DirtyList.h"

SetTagCommand::SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v)
: theFeature(aF), Idx(idx), Key(k), Value(v), OldKey(aF->tagKey(idx)), OldValue(aF->tagValue(idx)), Remove(false)
{
	redo();
}

SetTagCommand::SetTagCommand(MapFeature* aF, const QString& k, const QString& v)
: theFeature(aF), Idx(aF->findKey(k)), Key(k), Value(v), Remove(true)
{
	if (Idx < theFeature->tagSize())
	{
		OldKey = theFeature->tagKey(Idx);
		OldValue = theFeature->tagValue(Idx);
		Remove = false;
	}
	redo();
}


void SetTagCommand::redo()
{
	if (Idx < theFeature->tagSize())
		theFeature->setTag(Idx,Key,Value);
	else
		theFeature->setTag(Key,Value);
}

void SetTagCommand::undo()
{
	if (!Remove)
		theFeature->setTag(Idx,OldKey, OldValue);
	else
		theFeature->removeTag(Idx);
}

bool SetTagCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theFeature);
}

/* CLEARTAGSCOMMAND */

ClearTagsCommand::ClearTagsCommand(MapFeature* F)
: theFeature(F)
{
	for (unsigned int i=0; i<theFeature->tagSize(); ++i)
		Tags.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
	redo();
}

void ClearTagsCommand::redo()
{
	theFeature->clearTags();
}

void ClearTagsCommand::undo()
{
	for (unsigned int i=0; i<Tags.size(); ++i)
		theFeature->setTag(Tags[i].first,Tags[i].second);
}

bool ClearTagsCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theFeature);
}

/* CLEARTAGCOMMAND */

ClearTagCommand::ClearTagCommand(MapFeature* F, const QString& k)
: theFeature(F), Existed(false), Key(k)
{
	unsigned int Idx = theFeature->findKey(k);
	if (Idx < theFeature->tagSize())
	{
		Existed = true;
		Value = theFeature->tagValue(Idx);
	}
	redo();
}

void ClearTagCommand::undo()
{
	if (Existed)
		theFeature->setTag(Key,Value);
}

void ClearTagCommand::redo()
{
	theFeature->clearTag(Key);
}

bool ClearTagCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theFeature);
}


