#ifndef MERKAARTOR_TRACKSEGMENTCOMMANDS_H_
#define MERKAARTOR_TRACKSEGMENTCOMMANDS_H_

#include "Command.h"

class TrackSegment;
class TrackNode;
class Layer;

class TrackSegmentAddNodeCommand : public Command
{
    public:
        TrackSegmentAddNodeCommand(TrackSegment* R = NULL);
        TrackSegmentAddNodeCommand(TrackSegment* R, TrackNode* W, Layer* aLayer=NULL);
        TrackSegmentAddNodeCommand(TrackSegment* R, TrackNode* W, int Position, Layer* aLayer=NULL);
        ~TrackSegmentAddNodeCommand(void);

        virtual void undo();
        virtual void redo();
        virtual bool buildDirtyList(DirtyList& theList);

        virtual bool toXML(QXmlStreamWriter& stream) const;
        static TrackSegmentAddNodeCommand* fromXML(Document* d, QXmlStreamReader& stream);

    private:
        Layer* theLayer;
        Layer* oldLayer;
        TrackSegment* theTrackSegment;
        TrackNode* theNode;
        int Position;
};

class TrackSegmentRemoveNodeCommand : public Command
{
    public:
        TrackSegmentRemoveNodeCommand(TrackSegment* R = NULL);
        TrackSegmentRemoveNodeCommand(TrackSegment* R, TrackNode* W, Layer* aLayer=NULL);
        TrackSegmentRemoveNodeCommand(TrackSegment* R, int anIdx, Layer* aLayer=NULL);
        ~TrackSegmentRemoveNodeCommand(void);

        virtual void undo();
        virtual void redo();
        virtual bool buildDirtyList(DirtyList& theList);

        virtual bool toXML(QXmlStreamWriter& stream) const;
        static TrackSegmentRemoveNodeCommand* fromXML(Document* d, QXmlStreamReader& stream);

    private:
        Layer* theLayer;
        Layer* oldLayer;
        int Idx;
        TrackSegment* theTrackSegment;
        TrackNode* theTrackPoint;
};

#endif


