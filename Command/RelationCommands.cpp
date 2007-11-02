#include "RelationCommands.h"

#include "Map/Relation.h"
#include "Map/MapFeature.h"
#include "Sync/DirtyList.h"

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, MapFeature* W)
: theRelation(R), Role(aRole), theMapFeature(W), Position(theRelation->size())
{
	redo();
}

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, MapFeature* W, unsigned int aPos)
: theRelation(R), Role(aRole), theMapFeature(W), Position(aPos)
{
	redo();
}

void RelationAddFeatureCommand::undo()
{
	theRelation->remove(Position);
}

void RelationAddFeatureCommand::redo()
{
	theRelation->add(Role, theMapFeature, Position);
}

bool RelationAddFeatureCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRelation);
}


/* ROADREMOVEMapFeatureCOMMAND */

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, MapFeature* W)
: Idx(R->find(W)), theRelation(R), theMapFeature(W)
{
	Role = R->getRole(Idx);
	redo();
}

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, unsigned int anIdx)
: Idx(anIdx), theRelation(R), Role(R->getRole(anIdx)), theMapFeature(R->get(anIdx))
{
	redo();
}


void RelationRemoveFeatureCommand::undo()
{
	theRelation->add(Role,theMapFeature,Idx);
}

void RelationRemoveFeatureCommand::redo()
{
	theRelation->remove(Idx);
}

bool RelationRemoveFeatureCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRelation);
}




