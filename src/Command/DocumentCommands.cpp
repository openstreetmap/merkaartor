#include "DocumentCommands.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"
#include "Maps/MapFeature.h"
#include "Maps/Road.h"
#include "Maps/TrackPoint.h"
#include "Maps/Relation.h"
#include "Sync/DirtyList.h"

AddFeatureCommand::AddFeatureCommand(MapFeature* aFeature)
: Command(aFeature), theLayer(0), theFeature(0), UserAdded(false)
{
}

AddFeatureCommand::AddFeatureCommand(MapLayer* aLayer, MapFeature* aFeature, bool aUserAdded)
: Command(aFeature), theLayer(aLayer), theFeature(aFeature), UserAdded(aUserAdded)
{
	redo();
}

AddFeatureCommand::~AddFeatureCommand()
{
	if (theLayer)
		theLayer->decDirtyLevel(commandDirtyLevel);
}

void AddFeatureCommand::undo()
{
	Command::undo();
	if (theFeature->isUploaded() || postUploadCommand) {
		if (postUploadCommand)
			postUploadCommand->redo();
		else
			postUploadCommand = new RemoveFeatureCommand(theLayer->getDocument(), theFeature);
	} else {
		if (theLayer && oldLayer && (theLayer != oldLayer)) {
			theLayer->remove(theFeature);
			oldLayer->add(theFeature);
		} else
			theFeature->setDeleted(true);

		decDirtyLevel(theLayer);
	}
}

void AddFeatureCommand::redo()
{
	if (postUploadCommand)
		postUploadCommand->undo();
	else {
		oldLayer = theFeature->layer();
		if (theLayer && oldLayer && (theLayer != oldLayer)) {
			oldLayer->remove(theFeature);
			theLayer->add(theFeature);
		} else {
			if (!oldLayer)
				theLayer->add(theFeature);
			else {
				theFeature->setDeleted(false);
				oldLayer = NULL;
			}
		}
		incDirtyLevel(theLayer);
	}
	Command::redo();
}

bool AddFeatureCommand::buildDirtyList(DirtyList& theList)
{
	//if (isUndone && postUploadCommand) {
	//	return postUploadCommand->buildDirtyList(theList);
	//}
	if (isUndone)
		return false;

	if (UserAdded)
		if (theLayer->isUploadable())
			return theList.add(theFeature);
	return false;
}

bool AddFeatureCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("AddFeatureCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("useradded", QString(UserAdded ? "true" : "false"));

	Command::toXML(e);

	return OK;
}

AddFeatureCommand * AddFeatureCommand::fromXML(MapDocument* d, QDomElement e)
{
	AddFeatureCommand* a = new AddFeatureCommand();

	a->setId(e.attribute("xml:id"));
	a->theLayer = d->getLayer(e.attribute("layer"));
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;

	MapFeature* F;
	if (!(F = d->getFeature(e.attribute("feature"), false)))
		return NULL;

	a->theFeature = F;
	a->UserAdded = (e.attribute("useradded") == "true" ? true : false);

	Command::fromXML(d, e, a);

	return a;
}

/* REMOVEFEATURECOMMAND */

RemoveFeatureCommand::RemoveFeatureCommand(MapFeature *aFeature)
: Command(aFeature), theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false)
{
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature)
: Command(aFeature), theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false)
{
	theLayer = theDocument->getDirtyOrOriginLayer(aFeature->layer());
	redo();
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature, const QList<MapFeature*>& Alternatives)
: Command(aFeature), theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), theAlternatives(Alternatives)
{
	CascadedCleanUp  = new CommandList(MainWindow::tr("Cascaded cleanup"), NULL);
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		it.get()->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
	if (CascadedCleanUp->empty())
	{
		SAFE_DELETE(CascadedCleanUp);
		CascadedCleanUp = 0;
	} else
		CascadedCleanUp->undo();
	theLayer = theDocument->getDirtyOrOriginLayer(aFeature->layer());
	redo();
}

RemoveFeatureCommand::~RemoveFeatureCommand()
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
	SAFE_DELETE(CascadedCleanUp);
	if (theLayer->getDocument()->exists(theFeature) && theFeature->isDeleted()) {
		theLayer->getDocument()->deleteFeature(theFeature);
	}
}

void RemoveFeatureCommand::redo()
{
	if (postUploadCommand)
		postUploadCommand->undo();
	else {
		if (CascadedCleanUp)
			CascadedCleanUp->redo();
		oldLayer = theFeature->layer();
		Idx = theFeature->layer()->get(theFeature);
		oldLayer->remove(theFeature);
		theFeature->setDeleted(true);
		theLayer->add(theFeature);
		incDirtyLevel(oldLayer);
	}
	Command::redo();
}

void RemoveFeatureCommand::undo()
{
	Command::undo();
	if (theFeature->isUploaded() || postUploadCommand) {
		if (postUploadCommand)
			postUploadCommand->redo();
		else {
			theFeature->setId("");
			theFeature->setDeleted(false);
			postUploadCommand = new AddFeatureCommand(theLayer, theFeature, true);
		}
	} else {
		theLayer->remove(theFeature);
		if (oldLayer->size() < Idx)
			Idx = oldLayer->size();
		theFeature->setDeleted(false);
		oldLayer->add(theFeature,Idx);
		decDirtyLevel(oldLayer);
		if (CascadedCleanUp)
			CascadedCleanUp->undo();
	}
}

bool RemoveFeatureCommand::buildDirtyList(DirtyList &theList)
{
	//if (isUndone && postUploadCommand) {
	//	return postUploadCommand->buildDirtyList(theList);
	//}
	if (isUndone)
		return false;
	if (!oldLayer->isUploadable())
		return false;

	if (theFeature->lastUpdated() == MapFeature::OSMServerConflict)
		return false;

	//if (!theFeature->hasOSMId())
	//	return false;

	bool CascadedResult = true;
	if (CascadedCleanUp)
		CascadedResult = CascadedCleanUp->buildDirtyList(theList);

	if (!RemoveExecuted)
		RemoveExecuted = theList.erase(theFeature);
	return RemoveExecuted && CascadedResult;
}

bool RemoveFeatureCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RemoveFeatureCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("layer", oldLayer->id());
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("index", QString::number(Idx));

	if (CascadedCleanUp) {
		QDomElement casc = xParent.ownerDocument().createElement("Cascaded");
		e.appendChild(casc);

		CascadedCleanUp->toXML(casc);
	}
// 	if (theAlternatives.size() > 0) {
// 		QList<MapFeature*>::const_iterator myFeatIter;
// 		for(myFeatIter = theAlternatives.begin();
// 			myFeatIter != theAlternatives.end();
// 			myFeatIter++)
// 		{
// 			QDomElement alt = xParent.ownerDocument().createElement("Alternative");
// 			e.appendChild(alt);
//
// 			alt.setAttribute("xml:id", id());
// 		}
// 	}

	Command::toXML(e);

	return OK;
}

RemoveFeatureCommand * RemoveFeatureCommand::fromXML(MapDocument* d, QDomElement e)
{
	RemoveFeatureCommand* a = new RemoveFeatureCommand();

	a->setId(e.attribute("xml:id"));
	a->oldLayer = d->getLayer(e.attribute("layer"));
	a->theLayer = d->getDirtyOrOriginLayer();
	a->theFeature = d->getFeature(e.attribute("feature"), false);
	a->Idx = e.attribute("index").toInt();

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "Cascaded") {
			a->CascadedCleanUp = CommandList::fromXML(d, c.firstChildElement());
		}
		c = c.nextSiblingElement();
	}

	Command::fromXML(d, e, a);

	return a;
}


