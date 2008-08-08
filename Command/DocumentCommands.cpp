#include "DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Sync/DirtyList.h"

AddFeatureCommand::AddFeatureCommand(MapLayer* aDocument, MapFeature* aFeature, bool aUserAdded)
: theLayer(aDocument), theFeature(aFeature), UserAdded(aUserAdded)
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
	theLayer->remove(theFeature);
	if (oldLayer)
		oldLayer->add(theFeature);
	decDirtyLevel(theLayer);
}

void AddFeatureCommand::redo()
{
	oldLayer = theFeature->layer();
	if (oldLayer) 
		oldLayer->remove(theFeature);
	theLayer->add(theFeature);
	incDirtyLevel(theLayer);
}

bool AddFeatureCommand::buildDirtyList(DirtyList& theList)
{
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

	return a;
}

/* REMOVEFEATURECOMMAND */

RemoveFeatureCommand::RemoveFeatureCommand()
: theLayer(0), Idx(0), theFeature(0), CascadedCleanUp(0), RemoveExecuted(false)
{
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false)
{
	oldLayer = aFeature->layer();
	Idx = aFeature->layer()->get(aFeature);
	theLayer = theDocument->getTrashLayer();
	redo();
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature, const std::vector<MapFeature*>& Alternatives)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), theAlternatives(Alternatives)
{
	CascadedCleanUp  = new CommandList(MainWindow::tr("Cascaded cleanup"), NULL);
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		it.get()->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
	if (CascadedCleanUp->empty())
	{
		delete CascadedCleanUp;
		CascadedCleanUp = 0;
	}
	oldLayer = aFeature->layer();
	Idx = aFeature->layer()->get(aFeature);
//	redo();
	theLayer = theDocument->getTrashLayer();
	oldLayer->remove(theFeature);
	theLayer->add(theFeature);
	oldLayer->incDirtyLevel();
}

RemoveFeatureCommand::~RemoveFeatureCommand()
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
	delete CascadedCleanUp;
}

void RemoveFeatureCommand::redo()
{
	if (CascadedCleanUp)
		CascadedCleanUp->redo();
	oldLayer->remove(theFeature);
	theLayer->add(theFeature);
	incDirtyLevel(oldLayer);
}

void RemoveFeatureCommand::undo()
{
	theLayer->remove(theFeature);
	oldLayer->add(theFeature,Idx);
	decDirtyLevel(oldLayer);
	if (CascadedCleanUp)
		CascadedCleanUp->undo();
}

bool RemoveFeatureCommand::buildDirtyList(DirtyList &theList)
{
	if (!oldLayer->isUploadable())
		return false;

	if (theFeature->lastUpdated() == MapFeature::OSMServerConflict)
		return false;

	//if (!theFeature->hasOSMId())
	//	return false;

	if (CascadedCleanUp && CascadedCleanUp->buildDirtyList(theList))
	{
		delete CascadedCleanUp;
		CascadedCleanUp = 0;
	}
	if (!RemoveExecuted)
		RemoveExecuted = theList.erase(theFeature);
	return RemoveExecuted && (CascadedCleanUp == 0);
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
// 		std::vector<MapFeature*>::const_iterator myFeatIter;
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

	return OK;
}

RemoveFeatureCommand * RemoveFeatureCommand::fromXML(MapDocument* d, QDomElement e)
{
	RemoveFeatureCommand* a = new RemoveFeatureCommand();

	a->setId(e.attribute("xml:id"));
	a->oldLayer = d->getLayer(e.attribute("layer"));
	a->theLayer = d->getTrashLayer();
	a->theFeature = d->getFeature(e.attribute("feature"), false);
	a->Idx = e.attribute("index").toInt();

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "Cascaded") {
			a->CascadedCleanUp = CommandList::fromXML(d, c.firstChildElement());
		}
		c = c.nextSiblingElement();
	}

	return a;
}


