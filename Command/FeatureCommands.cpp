#include "FeatureCommands.h"

#include "Map/MapFeature.h"
#include "Sync/DirtyList.h"

TagCommand::TagCommand(MapFeature* aF)
: theFeature(aF), FirstRun(true)
{
	for (unsigned int i=0; i<theFeature->tagSize(); ++i)
		Before.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
}

void TagCommand::undo()
{
	theFeature->clearTags();
	for (unsigned int i=0; i<Before.size(); ++i)
		theFeature->setTag(Before[i].first,Before[i].second);
}

void TagCommand::redo()
{
	if (FirstRun)
		for (unsigned int i=0; i<theFeature->tagSize(); ++i)
			After.push_back(std::make_pair(theFeature->tagKey(i),theFeature->tagValue(i)));
	else
	{
		theFeature->clearTags();
		for (unsigned int i=0; i<After.size(); ++i)
			theFeature->setTag(After[i].first,After[i].second);
	}
	FirstRun = false;
}

bool TagCommand::buildDirtyList(DirtyList& theList)
{
	if (Before == After) return true;
	return theList.update(theFeature);
}

SetTagCommand::SetTagCommand(MapFeature* aF, unsigned int idx, const QString& k, const QString& v)
: TagCommand(aF)
{
	aF->setTag(idx,k,v);
	redo();
}

SetTagCommand::SetTagCommand(MapFeature* aF, const QString& k, const QString& v)
: TagCommand(aF)
{
	aF->setTag(k,v);
	redo();
}

/* CLEARTAGSCOMMAND */

ClearTagsCommand::ClearTagsCommand(MapFeature* F)
: TagCommand(F)
{
	F->clearTags();
	redo();
}


/* CLEARTAGCOMMAND */

ClearTagCommand::ClearTagCommand(MapFeature* F, const QString& k)
: TagCommand(F)
{
	F->clearTag(k);
	redo();
}
