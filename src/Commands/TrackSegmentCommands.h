#ifndef MERKAARTOR_TRACKSEGMENTCOMMANDS_H_
#define MERKAARTOR_TRACKSEGMENTCOMMANDS_H_

#include "Command.h"

class TrackSegment;
class Node;
class Layer;

class TrackSegmentAddNodeCommand : public Command
{
	public:
		TrackSegmentAddNodeCommand(TrackSegment* R = NULL);
		TrackSegmentAddNodeCommand(TrackSegment* R, Node* W, Layer* aLayer=NULL);
		TrackSegmentAddNodeCommand(TrackSegment* R, Node* W, int Position, Layer* aLayer=NULL);
		~TrackSegmentAddNodeCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static TrackSegmentAddNodeCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		TrackSegment* theTrackSegment;
		Node* theNode;
		int Position;
};

class TrackSegmentRemoveNodeCommand : public Command
{
	public:
		TrackSegmentRemoveNodeCommand(TrackSegment* R = NULL);
		TrackSegmentRemoveNodeCommand(TrackSegment* R, Node* W, Layer* aLayer=NULL);
		TrackSegmentRemoveNodeCommand(TrackSegment* R, int anIdx, Layer* aLayer=NULL);
		~TrackSegmentRemoveNodeCommand(void);

		virtual void undo();
		virtual void redo();
		virtual bool buildDirtyList(DirtyList& theList);

		virtual bool toXML(QDomElement& xParent) const;
		static TrackSegmentRemoveNodeCommand* fromXML(Document* d,QDomElement e);

	private:
		Layer* theLayer;
		Layer* oldLayer;
		int Idx;
		TrackSegment* theTrackSegment;
		Node* theTrackPoint;
};

#endif


