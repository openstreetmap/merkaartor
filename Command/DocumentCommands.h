#ifndef MERKATOR_DOCUMENTCOMMANDS_H_
#define MERKATOR_DOCUMENTCOMMANDS_H_

#include "Command/Command.h"

#include <vector>

class MapDocument;
class MapLayer;
class MapFeature;

class AddFeatureCommand : public Command
{
	public:
		AddFeatureCommand(MapLayer* aDocument, MapFeature* aFeature, bool aUserAdded);
		virtual ~AddFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

	private:
		MapLayer* theLayer;
		MapFeature* theFeature;
		bool UserAdded;
		bool RemoveOnDelete;
};

class RemoveFeatureCommand : public Command
{
	public:
		RemoveFeatureCommand(MapDocument* theDocument, MapFeature* aFeature);
		RemoveFeatureCommand(MapDocument* theDocument, MapFeature* aFeature, const std::vector<MapFeature*>& Alternatives);
		virtual ~RemoveFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

	private:
		MapLayer* theLayer;
		unsigned int Idx;
		MapFeature* theFeature;
		CommandList* CascadedCleanUp;
		bool RemoveExecuted;
		bool RemoveOnDelete;
};

#endif


