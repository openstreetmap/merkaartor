#include "DocumentCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Sync/DirtyList.h"

AddFeatureCommand::AddFeatureCommand(MapLayer* aDocument, MapFeature* aFeature, bool aUserAdded)
: theLayer(aDocument), theFeature(aFeature), UserAdded(aUserAdded), RemoveOnDelete(false)
{
	redo();
}

AddFeatureCommand::~AddFeatureCommand()
{
	if (theLayer)
		theLayer->decDirtyLevel(commandDirtyLevel);
	if (RemoveOnDelete)
		delete theFeature;
}

void AddFeatureCommand::undo()
{
	theLayer->remove(theFeature);
	RemoveOnDelete = true;
	decDirtyLevel(theLayer);
}

void AddFeatureCommand::redo()
{
	theLayer->add(theFeature);
	RemoveOnDelete = false;
	incDirtyLevel(theLayer);
}

bool AddFeatureCommand::buildDirtyList(DirtyList& theList)
{
	if (UserAdded)
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
	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("useradded", QString(UserAdded ? "true" : "false"));
	return OK;
}

AddFeatureCommand * AddFeatureCommand::fromXML(MapDocument* d, QDomElement e)
{
	AddFeatureCommand* a = new AddFeatureCommand();

	a->setId(e.attribute("xml:id"));
	a->theLayer = d->getLayer(e.attribute("layer"));
	MapFeature* F;
	if (!(F = d->getFeature("node_"+e.attribute("feature"))))
		if (!(F = d->getFeature("way_"+e.attribute("feature"))))
			if (!(F = d->getFeature("rel_"+e.attribute("feature"))))
				return NULL;
	a->theFeature = F;
	a->UserAdded = (e.attribute("useradded") == "true" ? true : false);
	a->	RemoveOnDelete = false;

	return a;
}

/* REMOVEFEATURECOMMAND */

RemoveFeatureCommand::RemoveFeatureCommand()
: theLayer(0), Idx(0), theFeature(0), CascadedCleanUp(0), RemoveExecuted(false), RemoveOnDelete(true)
{
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), RemoveOnDelete(true)
{
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
	{
		if (it.get() == aFeature)
		{
			theLayer = it.layer();
			Idx = it.index();
			break;
		}
	}
	redo();
}

RemoveFeatureCommand::RemoveFeatureCommand(MapDocument *theDocument, MapFeature *aFeature, const std::vector<MapFeature*>& Alternatives)
: theLayer(0), Idx(0), theFeature(aFeature), CascadedCleanUp(0), RemoveExecuted(false), RemoveOnDelete(true), theAlternatives(Alternatives)
{
	CascadedCleanUp  = new CommandList(MainWindow::tr("Cascaded cleanup"), NULL);
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		it.get()->cascadedRemoveIfUsing(theDocument, aFeature, CascadedCleanUp, Alternatives);
	if (CascadedCleanUp->empty())
	{
		delete CascadedCleanUp;
		CascadedCleanUp = 0;
	}
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
	{
		if (it.get() == aFeature)
		{
			theLayer = it.layer();
			Idx = it.index();
			break;
		}
	}
//	redo();
	theLayer->remove(theFeature);
	theLayer->incDirtyLevel();
}

RemoveFeatureCommand::~RemoveFeatureCommand()
{
	if (theLayer)
		theLayer->decDirtyLevel(commandDirtyLevel);
	delete CascadedCleanUp;
	if (RemoveOnDelete)
		delete theFeature;
}

void RemoveFeatureCommand::redo()
{
	if (CascadedCleanUp)
		CascadedCleanUp->redo();
	theLayer->remove(theFeature);
	incDirtyLevel(theLayer);
	RemoveOnDelete = true;
}

void RemoveFeatureCommand::undo()
{
	theLayer->add(theFeature,Idx);
	decDirtyLevel(theLayer);
	if (CascadedCleanUp)
		CascadedCleanUp->undo();
	RemoveOnDelete = false;
}

bool RemoveFeatureCommand::buildDirtyList(DirtyList &theList)
{
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
	e.setAttribute("layer", theLayer->id());
//	e.setAttribute("feature", theFeature->xmlId());
	e.setAttribute("index", QString::number(Idx));

	QString S = theFeature->toXML();

	QDomDocument xd;
	xd.setContent(S);

	QDomNode n = xParent.ownerDocument().importNode(xd.documentElement(), true);
	e.appendChild(n);

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
	a->theLayer = d->getLayer(e.attribute("layer"));
//	a->theFeature = d->getFeature(e.attribute("feature"));
	a->Idx = e.attribute("index").toInt();
	a->	RemoveOnDelete = true;

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "way") {
			a->theFeature = Road::fromXML(d, a->theLayer, c);
		} else
		if (c.tagName() == "relation") {
			a->theFeature =  Relation::fromXML(d, a->theLayer, c);
		} else
		if (c.tagName() == "node") {
			a->theFeature = TrackPoint::fromXML(d, a->theLayer, c);
		} else
		if (c.tagName() == "Cascaded") {
			a->CascadedCleanUp = CommandList::fromXML(d, c.firstChildElement());
		}
		c = c.nextSiblingElement();
	}
	if (a->theFeature)
		a->theLayer->remove(a->theFeature);

	return a;
}


