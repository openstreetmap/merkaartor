#ifndef MERKATOR_TRACKPOINTCOMMANDS_H_
#define MERKATOR_TRACKPOINTCOMMANDS_H_

#include "Command/Command.h"
#include "Map/Coord.h"

class TrackPoint;

class MoveTrackPointCommand : public Command
{
	public:
		MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos);
		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

	private:
		TrackPoint* thePoint;
		Coord OldPos, NewPos;
};

#endif


