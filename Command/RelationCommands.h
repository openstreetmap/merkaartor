#ifndef MERKAARTOR_RELATIONCOMMANDS_H_
#define MERKAARTOR_RELATIONCOMMANDS_H_

#include "Command/Command.h"

#include <QtCore/QString>

class Relation;
class MapFeature;

class RelationAddFeatureCommand : public Command
{
	public:
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W);
		RelationAddFeatureCommand(Relation* R, const QString& Role, MapFeature* W, unsigned int Position);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		Relation* theRelation;
		QString Role;
		MapFeature* theMapFeature;
		unsigned int Position;
};

class RelationRemoveFeatureCommand : public Command
{
	public:
		RelationRemoveFeatureCommand(Relation* R, MapFeature* W);
		RelationRemoveFeatureCommand(Relation* R, unsigned int anIdx);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		unsigned int Idx;
		Relation* theRelation;
		QString Role;
		MapFeature* theMapFeature;
};



#endif


