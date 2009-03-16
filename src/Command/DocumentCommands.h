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
		AddFeatureCommand(MapFeature* aFeature = NULL);
		AddFeatureCommand(MapLayer* aDocument, MapFeature* aFeature, bool aUserAdded);
		virtual ~AddFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static AddFeatureCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		MapFeature* theFeature;
		bool UserAdded;
};

class RemoveFeatureCommand : public Command
{
	public:
		RemoveFeatureCommand(MapFeature* aFeature = NULL);
		RemoveFeatureCommand(MapDocument* theDocument, MapFeature* aFeature);
		RemoveFeatureCommand(MapDocument* theDocument, MapFeature* aFeature, const std::vector<MapFeature*>& Alternatives);
		virtual ~RemoveFeatureCommand();

		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RemoveFeatureCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		unsigned int Idx;
		MapFeature* theFeature;
		CommandList* CascadedCleanUp;
		bool RemoveExecuted;
		std::vector<MapFeature*> theAlternatives;
};

#endif


