#ifndef MERKAARTOR_TRACKSEGMENTCOMMANDS_H_
#define MERKAARTOR_TRACKSEGMENTCOMMANDS_H_

#include "Command/Command.h"

class TrackSegment;
class TrackPoint;
class MapLayer;

class TrackSegmentAddTrackPointCommand : public Command
{
	public:
		TrackSegmentAddTrackPointCommand(TrackSegment* R = NULL);
		TrackSegmentAddTrackPointCommand(TrackSegment* R, TrackPoint* W, MapLayer* aLayer=NULL);
		TrackSegmentAddTrackPointCommand(TrackSegment* R, TrackPoint* W, int Position, MapLayer* aLayer=NULL);
		~TrackSegmentAddTrackPointCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static TrackSegmentAddTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		TrackSegment* theTrackSegment;
		TrackPoint* theTrackPoint;
		int Position;
};

class TrackSegmentRemoveTrackPointCommand : public Command
{
	public:
		TrackSegmentRemoveTrackPointCommand(TrackSegment* R = NULL);
		TrackSegmentRemoveTrackPointCommand(TrackSegment* R, TrackPoint* W, MapLayer* aLayer=NULL);
		TrackSegmentRemoveTrackPointCommand(TrackSegment* R, int anIdx, MapLayer* aLayer=NULL);
		~TrackSegmentRemoveTrackPointCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static TrackSegmentRemoveTrackPointCommand* fromXML(MapDocument* d,QDomElement e);

	private:
		MapLayer* theLayer;
		MapLayer* oldLayer;
		int Idx;
		TrackSegment* theTrackSegment;
		TrackPoint* theTrackPoint;
};

#endif


