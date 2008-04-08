#ifndef MERKATOR_TRACKPOINTCOMMANDS_H_
#define MERKATOR_TRACKPOINTCOMMANDS_H_

#include "Command/Command.h"
#include "Map/Coord.h"

class TrackPoint;

class MoveTrackPointCommand : public Command
{
	public:
		MoveTrackPointCommand() : OldPos(Coord(0.0, 0.0)), NewPos(Coord(0.0, 0.0)) {};
		MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos);
		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static MoveTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		TrackPoint* thePoint;
		Coord OldPos, NewPos;
};

#endif


