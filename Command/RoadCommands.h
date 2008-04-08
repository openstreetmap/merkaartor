#ifndef MERKAARTOR_ROADCOMMANDS_H_
#define MERKAARTOR_ROADCOMMANDS_H_

#include "Command/Command.h"

class Road;
class TrackPoint;

class RoadAddTrackPointCommand : public Command
{
	public:
		RoadAddTrackPointCommand() {};
		RoadAddTrackPointCommand(Road* R, TrackPoint* W);
		RoadAddTrackPointCommand(Road* R, TrackPoint* W, unsigned int Position);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RoadAddTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		Road* theRoad;
		TrackPoint* theTrackPoint;
		unsigned int Position;
};

class RoadRemoveTrackPointCommand : public Command
{
	public:
		RoadRemoveTrackPointCommand() {};
		RoadRemoveTrackPointCommand(Road* R, TrackPoint* W);
		RoadRemoveTrackPointCommand(Road* R, unsigned int anIdx);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static RoadRemoveTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		unsigned int Idx;
		Road* theRoad;
		TrackPoint* theTrackPoint;
};

#endif


