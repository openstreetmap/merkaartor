#ifndef MERKAARTOR_RELATIONCOMMANDS_H_
#define MERKAARTOR_RELATIONCOMMANDS_H_

#include "Command/Command.h"

#include <QString>

class Relation;
class MapFeature;
class MapLayer;

class RelationAddFeatureCommand : public Command
{
	public:
		RelationAddFeatureCommand() {};
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W, MapLayer* aLayer=NULL);
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W, unsigned int Position, MapLayer* aLayer=NULL);
		~RelationAddFeatureCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RelationAddFeatureCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		Relation* theRelation;
		QString Role;
		MapFeature* theMapFeature;
		unsigned int Position;
};

class RelationRemoveFeatureCommand : public Command
{
	public:
		RelationRemoveFeatureCommand() {};
		RelationRemoveFeatureCommand(Relation* R, MapFeature* W, MapLayer* aLayer=NULL);
		RelationRemoveFeatureCommand(Relation* R, unsigned int anIdx, MapLayer* aLayer=NULL);
		~RelationRemoveFeatureCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RelationRemoveFeatureCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		unsigned int Idx;
		Relation* theRelation;
		QString Role;
		MapFeature* theMapFeature;
};



#endif


