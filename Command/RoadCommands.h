#ifndef MERKAARTOR_ROADCOMMANDS_H_
#define MERKAARTOR_ROADCOMMANDS_H_

#include "Command/Command.h"

class Road;
class TrackPoint;

class RoadAddTrackPointCommand : public Command
{
	public:
		RoadAddTrackPointCommand(Road* R, TrackPoint* W);
		RoadAddTrackPointCommand(Road* R, TrackPoint* W, unsigned int Position);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		Road* theRoad;
		TrackPoint* theTrackPoint;
		unsigned int Position;
};

class RoadRemoveTrackPointCommand : public Command
{
	public:
		RoadRemoveTrackPointCommand(Road* R, TrackPoint* W);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

	private:
		unsigned int Idx;
		Road* theRoad;
		TrackPoint* theTrackPoint;
};

#endif


