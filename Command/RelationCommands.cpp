#include "RelationCommands.h"

#include "Map/Relation.h"
#include "Map/MapFeature.h"
#include "Map/MapLayer.h"
#include "Sync/DirtyList.h"

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R)
: Command(R), theLayer(0), oldLayer(0), theRelation(R), Role(""), theMapFeature(0), Position(0)
{
}

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, MapFeature* W, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRelation(R), Role(aRole), theMapFeature(W), Position(theRelation->size())
{
	redo();
}

RelationAddFeatureCommand::RelationAddFeatureCommand(Relation* R, const QString& aRole, MapFeature* W, unsigned int aPos, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), theRelation(R), Role(aRole), theMapFeature(W), Position(aPos)
{
	redo();
}

RelationAddFeatureCommand::~RelationAddFeatureCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void RelationAddFeatureCommand::undo()
{
	Command::undo();
	theRelation->remove(Position);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theRelation);
		oldLayer->add(theRelation);
		decDirtyLevel(oldLayer);
	}
}

void RelationAddFeatureCommand::redo()
{
	theRelation->add(Role, theMapFeature, Position);
	oldLayer = theRelation->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theRelation);
		incDirtyLevel(oldLayer);
		theLayer->add(theRelation);
	}
	Command::redo();
}

bool RelationAddFeatureCommand::buildDirtyList(DirtyList& theList)
{
	if (theRelation->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theRelation);
	if (!theRelation->layer())
		return theList.update(theRelation);
	if (theRelation->layer()->isUploadable())
		return theList.update(theRelation);

	return theList.noop(theRelation);
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
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

RelationAddFeatureCommand * RelationAddFeatureCommand::fromXML(MapDocument * d, QDomElement e)
{
	RelationAddFeatureCommand* a = new RelationAddFeatureCommand();
	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRelation = MapFeature::getRelationOrCreatePlaceHolder(d, a->theLayer, e.attribute("relation"));
	MapFeature* F;
	if (e.attribute("featureclass") == "TrackPoint") {
		F = (MapFeature*) MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));
	} else 
	if (e.attribute("featureclass") == "Road") {
		F = (MapFeature*) MapFeature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	} else 
	if (e.attribute("featureclass") == "Relation") {
		F = (MapFeature*) MapFeature::getRelationOrCreatePlaceHolder(d, a->theLayer, e.attribute("road"));
	} else {
		if (!(F = d->getFeature(e.attribute("feature"), false)))
			return NULL;
	}
	a->Role = e.attribute("role");
	a->theMapFeature = F;
	a->Position = e.attribute("pos").toUInt();

	Command::fromXML(d, e, a);

	return a;
}

/* ROADREMOVEMapFeatureCOMMAND */

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R)
: Command(R), theLayer(0), oldLayer(0), Idx(0), theRelation(R), theMapFeature(0)
{
}

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, MapFeature* W, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(R->find(W)), theRelation(R), theMapFeature(W)
{
	Role = R->getRole(Idx);
	redo();
}

RelationRemoveFeatureCommand::RelationRemoveFeatureCommand(Relation* R, unsigned int anIdx, MapLayer* aLayer)
: Command(R), theLayer(aLayer), oldLayer(0), Idx(anIdx), theRelation(R), Role(R->getRole(anIdx)), theMapFeature(R->get(anIdx))
{
	redo();
}

RelationRemoveFeatureCommand::~RelationRemoveFeatureCommand(void)
{
	oldLayer->decDirtyLevel(commandDirtyLevel);
}


void RelationRemoveFeatureCommand::undo()
{
	Command::undo();
	theRelation->add(Role,theMapFeature,Idx);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(theRelation);
		oldLayer->add(theRelation);
		decDirtyLevel(oldLayer);
	}
}

void RelationRemoveFeatureCommand::redo()
{
	theRelation->remove(Idx);
	oldLayer = theRelation->layer();
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(theRelation);
		incDirtyLevel(oldLayer);
		theLayer->add(theRelation);
	}
	Command::redo();
}

bool RelationRemoveFeatureCommand::buildDirtyList(DirtyList& theList)
{
	if (theRelation->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(theRelation);
	if (!theRelation->layer())
		return theList.update(theRelation);
	if (theRelation->layer()->isUploadable())
		return theList.update(theRelation);

	return theList.noop(theRelation);
}

bool RelationRemoveFeatureCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RelationRemoveFeatureCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("relation", theRelation->xmlId());
	e.setAttribute("feature", theMapFeature->xmlId());
	e.setAttribute("featureclass", theMapFeature->getClass());
	e.setAttribute("index", QString::number(Idx));
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

RelationRemoveFeatureCommand * RelationRemoveFeatureCommand::fromXML(MapDocument * d, QDomElement e)
{
	RelationRemoveFeatureCommand* a = new RelationRemoveFeatureCommand();

	a->setId(e.attribute("xml:id"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = d->getDirtyLayer();
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->theRelation = MapFeature::getRelationOrCreatePlaceHolder(d, a->theLayer, e.attribute("relation"));
	MapFeature* F = NULL;
	if (e.attribute("featureclass") == "TrackPoint") {
		F = (MapFeature*) MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("feature"));
	} else 
	if (e.attribute("featureclass") == "Road") {
		F = (MapFeature*) MapFeature::getWayOrCreatePlaceHolder(d, a->theLayer, e.attribute("feature"));
	} else 
	if (e.attribute("featureclass") == "Relation") {
		F = (MapFeature*) MapFeature::getRelationOrCreatePlaceHolder(d, a->theLayer, e.attribute("feature"));
	}
	a->theMapFeature = F;
	a->Idx = e.attribute("index").toInt();
	a->Role = a->theRelation->getRole(a->Idx);

	Command::fromXML(d, e, a);

	return a;
}




