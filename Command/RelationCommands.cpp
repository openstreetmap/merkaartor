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

bool RelationAddFeatureCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RelationAddFeatureCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("relation", theRelation->xmlId());
	e.setAttribute("role", Role);
	e.setAttribute("feature", theMapFeature->xmlId());
	e.setAttribute("pos", QString::number(Position));

	return OK;
}

RelationAddFeatureCommand * RelationAddFeatureCommand::fromXML(MapDocument * d, QDomElement e)
{
	RelationAddFeatureCommand* a = new RelationAddFeatureCommand();
	a->setId(e.attribute("xml:id"));
	a->theRelation = dynamic_cast<Relation*>(d->getFeature("rel"+e.attribute("relation")));
	a->Role = e.attribute("role");
	MapFeature* F;
	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
				return NULL;
	a->theMapFeature = F;
	a->Position = e.attribute("pos").toUInt();

	return a;
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

bool RelationRemoveFeatureCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RelationRemoveFeatureCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("relation", theRelation->xmlId());
	e.setAttribute("feature", theMapFeature->xmlId());
	e.setAttribute("index", QString::number(Idx));

	return OK;
}

RelationRemoveFeatureCommand * RelationRemoveFeatureCommand::fromXML(MapDocument * d, QDomElement e)
{
	RelationRemoveFeatureCommand* a = new RelationRemoveFeatureCommand();

	a->setId(e.attribute("xml:id"));
	a->theRelation = dynamic_cast<Relation*>(d->getFeature("rel"+e.attribute("relation")));
	MapFeature* F;
	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
				return NULL;
	a->theMapFeature = F;
	a->Idx = e.attribute("index").toInt();
	a->Role = a->theRelation->getRole(a->Idx);

	return a;
}




