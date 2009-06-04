#include "Command/TrackPointCommands.h"
#include "Maps/TrackPoint.h"
#include "Maps/MapLayer.h"
#include "Sync/DirtyList.h"

MoveTrackPointCommand::MoveTrackPointCommand() 
: Command(0), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
}

MoveTrackPointCommand::MoveTrackPointCommand(TrackPoint* aPt) 
: Command(aPt), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
	if (!theLayer)
		theLayer = thePoint->layer();
	description = MainWindow::tr("Move node %1").arg(aPt->description());
}

MoveTrackPointCommand::MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos, MapLayer* aLayer)
: Command(aPt), theLayer(aLayer), oldLayer(0), thePoint(aPt), OldPos(aPt->position()), NewPos(aPos)
{
	if (!theLayer)
		theLayer = thePoint->layer();
	description = MainWindow::tr("Move node %1").arg(aPt->description());
	redo();
}

MoveTrackPointCommand::~MoveTrackPointCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void MoveTrackPointCommand::undo()
{
	Command::undo();
	CoordBox bb = thePoint->boundingBox();
	theLayer->getRTree()->remove(geometry::box < Coord > (bb.bottomLeft(), bb.topRight() ), thePoint);
	thePoint->setPosition(OldPos);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(thePoint);
		oldLayer->add(thePoint);
		decDirtyLevel(oldLayer);
	}
	bb = thePoint->boundingBox();
	oldLayer->getRTree()->insert(geometry::box < Coord > (bb.bottomLeft(), bb.topRight() ), thePoint);
}

void MoveTrackPointCommand::redo()
{
	oldLayer = thePoint->layer();
	CoordBox bb = thePoint->boundingBox();
	oldLayer->getRTree()->remove(geometry::box < Coord > (bb.bottomLeft(), bb.topRight() ), thePoint);
	thePoint->setPosition(NewPos);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(thePoint);
		incDirtyLevel(oldLayer);
		theLayer->add(thePoint);
	}
	bb = thePoint->boundingBox();
	theLayer->getRTree()->insert(geometry::box < Coord > (bb.bottomLeft(), bb.topRight() ), thePoint);
	Command::redo();
}

bool MoveTrackPointCommand::buildDirtyList(DirtyList &theList)
{
	if (isUndone)
		return false;
	if (thePoint->lastUpdated() == MapFeature::NotYetDownloaded)
		return theList.noop(thePoint);
	if (!thePoint->layer())
		return theList.update(thePoint);
	if (thePoint->isUploadable())
		return theList.update(thePoint);

	return theList.noop(thePoint);
}

bool MoveTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("MoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("trackpoint", thePoint->xmlId());
	OldPos.toXML("oldpos", e);
	NewPos.toXML("newpos", e);
	if (theLayer)
		e.setAttribute("layer", theLayer->id());
	if (oldLayer)
		e.setAttribute("oldlayer", oldLayer->id());

	Command::toXML(e);

	return OK;
}

MoveTrackPointCommand * MoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	MoveTrackPointCommand* a = new MoveTrackPointCommand();
	a->setId(e.attribute("xml:id"));
	a->OldPos = Coord::fromXML(e.firstChildElement("oldpos"));
	a->NewPos = Coord::fromXML(e.firstChildElement("newpos"));
	if (e.hasAttribute("layer"))
		a->theLayer = d->getLayer(e.attribute("layer"));
	else
		a->theLayer = NULL;
	if (e.hasAttribute("oldlayer"))
		a->oldLayer = d->getLayer(e.attribute("oldlayer"));
	else
		a->oldLayer = NULL;
	a->thePoint = MapFeature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));

	a->description = MainWindow::tr("Move node %1").arg(a->thePoint->description());

	Command::fromXML(d, e, a);

	return a;
}



