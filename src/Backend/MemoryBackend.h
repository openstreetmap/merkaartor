#ifndef MEMORYBACKEND_H
#define MEMORYBACKEND_H

#include "IBackend.h"
#include "Features.h"

struct IndexFindContext {
    QMap<RenderPriority, QSet <Feature*> >* theFeatures;
    QRectF* clipRect;
    Projection* theProjection;
    QTransform* theTransform;
};

class MemoryBackendPrivate;
class MemoryBackend : public IBackend
{
public:
    MemoryBackend();
    ~MemoryBackend();

private:
    MemoryBackendPrivate* p;

public:
    virtual Node* allocNode(const Node& other);
    virtual Node* allocNode(const QPointF& aCoord);

    virtual Way* allocWay();
    virtual Way* allocWay(const Way& other);

    virtual Relation* allocRelation();
    virtual Relation* allocRelation(const Relation& other);

    virtual TrackSegment* allocSegment();

    virtual void deallocFeature(Feature* f);

    virtual void sync(Feature* f);

    virtual const QList<Feature*>& indexFind(const QRectF& vp);
    virtual void indexFind(const QRectF& bb, const IndexFindContext& findResult);
    virtual void get(const QRectF& bb, QList<Feature*>& theFeatures);
    virtual void getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                       QList<QRectF>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform);

private:
    virtual void indexAdd(const QRectF& bb, Feature* aFeat);
    virtual void indexRemove(const QRectF& bb, Feature* aFeat);

};

#endif // MEMORYBACKEND_H
