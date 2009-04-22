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
		RelationAddFeatureCommand(Relation* R = NULL);
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W, MapLayer* aLayer=NULL);
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W, int Position, MapLayer* aLayer=NULL);
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
		int Position;
};

class RelationRemoveFeatureCommand : public Command
{
	public:
		RelationRemoveFeatureCommand(Relation* R = NULL);
		RelationRemoveFeatureCommand(Relation* R, MapFeature* W, MapLayer* aLayer=NULL);
		RelationRemoveFeatureCommand(Relation* R, int anIdx, MapLayer* aLayer=NULL);
		~RelationRemoveFeatureCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RelationRemoveFeatureCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		int Idx;
		Relation* theRelation;
		QString Role;
		MapFeature* theMapFeature;
};



#endif


