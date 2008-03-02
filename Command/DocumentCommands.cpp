#include "DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Sync/DirtyList.h"

AddFeatureCommand::AddFeatureCommand(MapLayer* aDocument, MapFeature* aFeature, bool aUserAdded)
: theLayer(aDocument), theFeature(aFeature), UserAdded(aUserAdded), RemoveOnDelete(false)
{
	redo();
}

AddFeatureCommand::~AddFeatureCommand()
{
	if (RemoveOnDelete)
		delete theFeature;
}

void AddFeatureCommand::undo()
{
	theLayer->remove(theFeature);
	RemoveOnDelete = true;
}

void AddFeatureCommand::redo()
{
	theLayer->add(theFeature);
	RemoveOnDelete = false;
}

bool AddFeatureCommand::buildDirtyList(DirtyList& theList)
{
	if (UserAdded)
		return theList.add(theFeature);
	return false;
}

/* REMOVEFEATURECOMMAND */

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), RemoveOnDelete(true)
{
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
	{
		if (it.get() == aFeature)
		{
			theLayer = it.layer();
			Idx = it.index();
			break;
		}
	}
	redo();
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature, const std::vector<MapFeature*>& Alternatives)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), RemoveOnDelete(true)
{
	CascadedCleanUp = new CommandList;
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		it.get()->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
	if (CascadedCleanUp->empty())
	{
		delete CascadedCleanUp;
		CascadedCleanUp = 0;
	}
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
	{
		if (it.get() == aFeature)
		{
			theLayer = it.layer();
			Idx = it.index();
			break;
		}
	}
//	redo();
	theLayer->remove(theFeature);
}

RemoveFeatureCommand::~RemoveFeatureCommand()
{
	delete CascadedCleanUp;
	if (RemoveOnDelete)
		delete theFeature;
}

void RemoveFeatureCommand::redo()
{
	if (CascadedCleanUp)
		CascadedCleanUp->redo();
	theLayer->remove(theFeature);
	RemoveOnDelete = true;
}

void RemoveFeatureCommand::undo()
{
	theLayer->add(theFeature,Idx);
	if (CascadedCleanUp)
		CascadedCleanUp->undo();
	RemoveOnDelete = false;
}

bool RemoveFeatureCommand::buildDirtyList(DirtyList &theList)
{
	if (CascadedCleanUp && CascadedCleanUp->buildDirtyList(theList))
	{
		delete CascadedCleanUp;
		CascadedCleanUp = 0;
	}
	if (!RemoveExecuted)
		RemoveExecuted = theList.erase(theFeature);
	return RemoveExecuted && (CascadedCleanUp == 0);
}
