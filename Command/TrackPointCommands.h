#ifndef MERKATOR_TRACKPOINTCOMMANDS_H_
#define MERKATOR_TRACKPOINTCOMMANDS_H_

#include "Command/Command.h"
#include "Map/Coord.h"

class TrackPoint;
class MapLayer;

class MoveTrackPointCommand : public Command
{
	public:
		MoveTrackPointCommand() : OldPos(Coord(0.0, 0.0)), NewPos(Coord(0.0, 0.0)) {};
		MoveTrackPointCommand(TrackPoint* aPt, const Coord& aPos, MapLayer* aLayer=NULL);
		void undo();
		void redo();
		bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static MoveTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		TrackPoint* thePoint;
		Coord OldPos, NewPos;
};

#endif


