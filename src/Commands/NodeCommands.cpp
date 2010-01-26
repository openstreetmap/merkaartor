#include "NodeCommands.h"

#include "Node.h"
#include "Layer.h"
#include "DirtyList.h"

MoveNodeCommand::MoveNodeCommand() 
: Command(0), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
}

MoveNodeCommand::MoveNodeCommand(Node* aPt) 
: Command(aPt), theLayer(0), oldLayer(0), OldPos(Coord(0, 0)), NewPos(Coord(0, 0))
{
	if (!theLayer)
		theLayer = thePoint->layer();
	description = MainWindow::tr("Move node %1").arg(aPt->description());
}

MoveNodeCommand::MoveNodeCommand(Node* aPt, const Coord& aPos, Layer* aLayer)
: Command(aPt), theLayer(aLayer), oldLayer(0), thePoint(aPt), OldPos(aPt->position()), NewPos(aPos)
{
	if (!theLayer)
		theLayer = thePoint->layer();
	description = MainWindow::tr("Move node %1").arg(aPt->description());
	redo();
}

MoveNodeCommand::~MoveNodeCommand(void)
{
	if (oldLayer)
		oldLayer->decDirtyLevel(commandDirtyLevel);
}

void MoveNodeCommand::undo()
{
	Command::undo();
	thePoint->setPosition(OldPos);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		theLayer->remove(thePoint);
		oldLayer->add(thePoint);
		decDirtyLevel(oldLayer);
	}
}

void MoveNodeCommand::redo()
{
	oldLayer = thePoint->layer();
	thePoint->setPosition(NewPos);
    thePoint->setVirtual(false);
	if (theLayer && oldLayer && (theLayer != oldLayer)) {
		oldLayer->remove(thePoint);
		incDirtyLevel(oldLayer);
		theLayer->add(thePoint);
	}
	Command::redo();
}

bool MoveNodeCommand::buildDirtyList(DirtyList &theList)
{
	if (isUndone)
		return false;
	if (thePoint->lastUpdated() == Feature::NotYetDownloaded)
		return theList.noop(thePoint);
	if (!thePoint->layer())
		return theList.update(thePoint);
	if (thePoint->isUploadable())
		return theList.update(thePoint);

	return theList.noop(thePoint);
}

bool MoveNodeCommand::toXML(QDomElement& xParent) const
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

MoveNodeCommand * MoveNodeCommand::fromXML(Document * d, QDomElement e)
{
	MoveNodeCommand* a = new MoveNodeCommand();
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
	a->thePoint = Feature::getTrackPointOrCreatePlaceHolder(d, a->theLayer, e.attribute("trackpoint"));

	a->description = MainWindow::tr("Move node %1").arg(a->thePoint->description());

	Command::fromXML(d, e, a);

	return a;
}



