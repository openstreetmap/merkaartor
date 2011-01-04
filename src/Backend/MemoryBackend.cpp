#include "MemoryBackend.h"
#include "RTree.h"

typedef RTree<Feature*, double, 2, double> CoordTree;

class MemoryBackendPrivate
{
public:
    QSet<Feature*> AllocFeatures;
    CoordTree theRTree;
    QList<Feature*> findResult;
};

bool __cdecl indexFindCallbackList(Feature* F, void* ctxt)
{
    ((QList<Feature*>*)(ctxt))->append(F);
    return true;
}

bool __cdecl indexFindCallback(Feature* F, void* ctxt)
{
    IndexFindContext* pCtxt = (IndexFindContext*)ctxt;

    if (!F->isVisible())
        return true;

    if (pCtxt->theFeatures->value(F->renderPriority()).contains(F))
        return true;

    if (CHECK_WAY(F)) {
        Way * R = STATIC_CAST_WAY(F);
        R->buildPath(*(pCtxt->theProjection), *(pCtxt->theTransform), *(pCtxt->clipRect));
        (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);
    } else
    if (CHECK_RELATION(F)) {
        Relation * RR = STATIC_CAST_RELATION(F);
        RR->buildPath(*(pCtxt->theProjection), *(pCtxt->theTransform), *(pCtxt->clipRect));
        (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);
    } else
    if (F->getType() == IFeature::Point) {
        if (!(F->isVirtual() && !M_PREFS->getVirtualNodesVisible()))
            (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);
    } else
        (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);

    return true;
}

void MemoryBackend::indexAdd(const QRectF& bb, Feature* aFeat)
{
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree.Insert(min, max, aFeat);
}

void MemoryBackend::indexRemove(const QRectF& bb, Feature* aFeat)
{
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree.Remove(min, max, aFeat);
}

const QList<Feature*>& MemoryBackend::indexFind(const QRectF& bb)
{
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->findResult.clear();
    p->theRTree.Search(min, max, &indexFindCallbackList, (void*)&p->findResult);

    return p->findResult;
}

void MemoryBackend::indexFind(const QRectF& bb, const IndexFindContext& ctxt)
{
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree.Search(min, max, &indexFindCallback, (void*)&ctxt);
}


void MemoryBackend::get(const QRectF& bb, QList<Feature*>& theFeatures)
{
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree.Search(min, max, &indexFindCallback, (void*)(&theFeatures));
}

void MemoryBackend::getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                   QList<QRectF>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform)
{
    IndexFindContext ctxt;
    ctxt.theFeatures = &theFeatures;
    ctxt.clipRect = &clipRect;
    ctxt.theProjection = &theProjection;
    ctxt.theTransform = &theTransform;

    for (int i=0; i < invalidRects.size(); ++i) {
        indexFind(invalidRects[i], ctxt);
    }
}

/******************************/

MemoryBackend::MemoryBackend()
{
    p = new MemoryBackendPrivate;
}

MemoryBackend::~MemoryBackend()
{
//    CoordTree::Iterator it;
//    p->theRTree.GetFirst(it);
//    while (!p->theRTree.IsNull(it)) {
//        delete *it;
//        p->theRTree.GetNext(it);
//    }

    QSet<Feature *>::const_iterator i = p->AllocFeatures.constBegin();
    while (i != p->AllocFeatures.constEnd()) {
        delete *i;
    }

    delete p;
}

Node * MemoryBackend::allocNode(const Node& other)
{
    Node* f = new Node(other);
    p->AllocFeatures.insert(f);
    return f;
}

Node * MemoryBackend::allocNode(const QPointF& aCoord)
{
    Node* f = new Node(aCoord);
    p->AllocFeatures.insert(f);
    return f;
}

Way * MemoryBackend::allocWay()
{
    Way* f = new Way();
    p->AllocFeatures.insert(f);
    return f;
}

Way * MemoryBackend::allocWay(const Way& other)
{
    Way* f = new Way(other);
    p->AllocFeatures.insert(f);
    return f;
}

Relation * MemoryBackend::allocRelation()
{
    Relation* f = new Relation();
    p->AllocFeatures.insert(f);
    return f;
}

Relation * MemoryBackend::allocRelation(const Relation& other)
{
    Relation* f = new Relation(other);
    p->AllocFeatures.insert(f);
    return f;
}

TrackSegment * MemoryBackend::allocSegment()
{
    TrackSegment* f = new TrackSegment();
    p->AllocFeatures.insert(f);
    return f;
}

void MemoryBackend::deallocFeature(Feature *f)
{
    indexRemove(f->boundingBox(), f);
}

void MemoryBackend::sync(Feature *f)
{
//    CoordBox bb = f->boundingBox();
//    if (!bb.isNull()) {
//        double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
//        double max[] = {bb.topRight().x(), bb.topRight().y()};
//        p->theRTree.Insert(min, max, f);
//    }
}


