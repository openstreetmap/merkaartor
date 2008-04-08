#include "RoadCommands.h"

#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Sync/DirtyList.h"

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W)
: theRoad(R), theTrackPoint(W), Position(theRoad->size())
{
	redo();
}

RoadAddTrackPointCommand::RoadAddTrackPointCommand(Road* R, TrackPoint* W, unsigned int aPos)
: theRoad(R), theTrackPoint(W), Position(aPos)
{
	redo();
}

void RoadAddTrackPointCommand::undo()
{
	theRoad->remove(Position);
}

void RoadAddTrackPointCommand::redo()
{
	theRoad->add(theTrackPoint, Position);
}

bool RoadAddTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}

bool RoadAddTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RoadAddTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("id", id());
	e.setAttribute("road", theRoad->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("pos", QString::number(Position));

	return OK;
}

RoadAddTrackPointCommand * RoadAddTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	RoadAddTrackPointCommand* a = new RoadAddTrackPointCommand();
	a->setId(e.attribute("id"));
	a->theRoad = dynamic_cast<Road*>(d->getFeature("way_"+e.attribute("road")));
	a->theTrackPoint = dynamic_cast<TrackPoint*>(d->getFeature("node_"+e.attribute("trackpoint")));
	a->Position = e.attribute("pos").toUInt();

	return a;
}

/* ROADREMOVETRACKPOINTCOMMAND */

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, TrackPoint* W)
: Idx(R->find(W)), theRoad(R), theTrackPoint(W)
{
	redo();
}

RoadRemoveTrackPointCommand::RoadRemoveTrackPointCommand(Road* R, unsigned int anIdx)
: Idx(anIdx), theRoad(R), theTrackPoint(R->get(anIdx))
{
	redo();
}


void RoadRemoveTrackPointCommand::undo()
{
	theRoad->add(theTrackPoint,Idx);
}

void RoadRemoveTrackPointCommand::redo()
{
	theRoad->remove(Idx);
}

bool RoadRemoveTrackPointCommand::buildDirtyList(DirtyList& theList)
{
	return theList.update(theRoad);
}

bool RoadRemoveTrackPointCommand::toXML(QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("RoadRemoveTrackPointCommand");
	xParent.appendChild(e);

	e.setAttribute("id", id());
	e.setAttribute("road", theRoad->xmlId());
	e.setAttribute("trackpoint", theTrackPoint->xmlId());
	e.setAttribute("index", QString::number(Idx));

	return OK;
}

RoadRemoveTrackPointCommand * RoadRemoveTrackPointCommand::fromXML(MapDocument * d, QDomElement e)
{
	RoadRemoveTrackPointCommand* a = new RoadRemoveTrackPointCommand();
	a->setId(e.attribute("id"));
	a->theRoad = dynamic_cast<Road*>(d->getFeature("way_"+e.attribute("road")));
	a->theTrackPoint = dynamic_cast<TrackPoint*>(d->getFeature("node_"+e.attribute("trackpoint")));
	a->Idx = e.attribute("index").toUInt();

	return a;
}




