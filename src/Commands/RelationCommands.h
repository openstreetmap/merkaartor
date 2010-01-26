#ifndef MERKAARTOR_RELATIONCOMMANDS_H_
#define MERKAARTOR_RELATIONCOMMANDS_H_

#include "Command.h"

#include <QString>

class Relation;
class Feature;
class Layer;

class RelationAddFeatureCommand : public Command
{
	public:
		RelationAddFeatureCommand(Relation* R = NULL);
		RelationAddFeatureCommand(Relation* R, const QString& Role, Feature* W, Layer* aLayer=NULL);
		RelationAddFeatureCommand(Relation* R, const QString& Role, Feature* W, int Position, Layer* aLayer=NULL);
		~RelationAddFeatureCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RelationAddFeatureCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		Relation* theRelation;
		QString Role;
		Feature* theMapFeature;
		int Position;
};

class RelationRemoveFeatureCommand : public Command
{
	public:
		RelationRemoveFeatureCommand(Relation* R = NULL);
		RelationRemoveFeatureCommand(Relation* R, Feature* W, Layer* aLayer=NULL);
		RelationRemoveFeatureCommand(Relation* R, int anIdx, Layer* aLayer=NULL);
		~RelationRemoveFeatureCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RelationRemoveFeatureCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		int Idx;
		Relation* theRelation;
		QString Role;
		Feature* theMapFeature;
};



#endif


