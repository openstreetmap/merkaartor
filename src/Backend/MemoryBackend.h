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
    virtual Node* allocNode(Layer* l, const Node& other);
    virtual Node* allocNode(Layer* l, const QPointF& aCoord);
    virtual TrackNode* allocTrackNode(Layer* l, const QPointF& aCoord);
    virtual PhotoNode * allocPhotoNode(Layer* l, const QPointF& aCoord);
    virtual PhotoNode * allocPhotoNode(Layer* l, const Node& other);
    virtual PhotoNode * allocPhotoNode(Layer* l, const TrackNode& other);
    virtual Node* allocVirtualNode(const QPointF& aCoord);

    virtual Way* allocWay(Layer* l);
    virtual Way* allocWay(Layer* l, const Way& other);

    virtual Relation* allocRelation(Layer* l);
    virtual Relation* allocRelation(Layer* l, const Relation& other);

    virtual TrackSegment* allocSegment(Layer* l);

    virtual void deallocFeature(Layer* l, Feature* f);
    virtual void deallocVirtualNode(Feature* f);

    virtual void sync(Feature* f);

    virtual const QList<Feature*>& indexFind(Layer* l, const QRectF& vp);
    virtual void indexFind(Layer* l, const QRectF& bb, const IndexFindContext& findResult);
    virtual void get(Layer* l, const QRectF& bb, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(Layer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                               QList<CoordBox>& invalidRects, Projection& theProjection);
    virtual void getFeatureSet(Layer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                               CoordBox& invalidRect, Projection& theProjection);
    virtual void indexAdd(Layer* l, const QRectF& bb, Feature* aFeat);
    virtual void indexRemove(Layer* l, const QRectF& bb, Feature* aFeat);

};

#endif // MEMORYBACKEND_H
