#ifndef MEMORYBACKEND_H
#define MEMORYBACKEND_H

#include "Features.h"

struct IndexFindContext {
    QMap<RenderPriority, QSet <Feature*> >* theFeatures;
    QRectF* clipRect;
    Projection* theProjection;
    QTransform* theTransform;
    CoordBox bbox;
};

class MemoryBackendPrivate;
class MemoryBackend
{
public:
    MemoryBackend();
    ~MemoryBackend();

private:
    MemoryBackendPrivate* p;

public:
    virtual Node* allocNode(ILayer* l, const Node& other);
    virtual Node* allocNode(ILayer* l, const QPointF& aCoord);
    virtual TrackNode* allocTrackNode(ILayer* l, const QPointF& aCoord);
    virtual PhotoNode * allocPhotoNode(ILayer* l, const QPointF& aCoord);
    virtual PhotoNode * allocPhotoNode(ILayer* l, const Node& other);
    virtual PhotoNode * allocPhotoNode(ILayer* l, const TrackNode& other);
    virtual Node* allocVirtualNode(const QPointF& aCoord);

    virtual Way* allocWay(ILayer* l);
    virtual Way* allocWay(ILayer* l, const Way& other);

    virtual Relation* allocRelation(ILayer* l);
    virtual Relation* allocRelation(ILayer* l, const Relation& other);

    virtual TrackSegment* allocSegment(ILayer* l);

    virtual void deallocFeature(ILayer* l, Feature* f);
    virtual void deallocVirtualNode(Feature* f);

    virtual void sync(Feature* f);
    virtual void purge();
    virtual void delayDeletes();
    virtual void resumeDeletes();

    virtual const QList<Feature*>& indexFind(ILayer* l, const QRectF& vp);
    virtual void indexFind(ILayer* l, const QRectF& bb, const IndexFindContext& findResult);
    virtual void get(ILayer* l, const QRectF& bb, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(ILayer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                               const QList<CoordBox>& invalidRects, Projection& theProjection);
    virtual void getFeatureSet(ILayer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                               const CoordBox& invalidRect, Projection& theProjection);
    virtual void indexAdd(ILayer* l, const QRectF& bb, Feature* aFeat);
    virtual void indexRemove(ILayer* l, const QRectF& bb, Feature* aFeat);

};

#endif // MEMORYBACKEND_H
