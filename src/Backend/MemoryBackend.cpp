#include "MemoryBackend.h"
#include "RTree.h"

typedef RTree<Feature*, double, 2, double> CoordTree;

class MemoryBackendPrivate
{
public:
    QHash<Feature*, CoordBox> AllocFeatures;
    QHash<Layer*, CoordTree*> theRTree;
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
    if (CHECK_NODE(F)) {
        if (!(F->isVirtual() && !M_PREFS->getVirtualNodesVisible()))
            (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);
    } else
        (*(pCtxt->theFeatures))[F->renderPriority()].insert(F);

    return true;
}

void MemoryBackend::indexAdd(Layer* l, const QRectF& bb, Feature* aFeat)
{
    if (!l)
        return;
    if (!p->theRTree.contains(l))
        p->theRTree[l] = new CoordTree();

    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Insert(min, max, aFeat);
}

void MemoryBackend::indexRemove(Layer* l, const QRectF& bb, Feature* aFeat)
{
    if (!l)
        return;
    if (!p->theRTree.contains(l))
        return;

    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Remove(min, max, aFeat);
}

const QList<Feature*>& MemoryBackend::indexFind(Layer* l, const QRectF& bb)
{
    p->findResult.clear();
    if (p->theRTree.contains(l)) {
        double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
        double max[] = {bb.topRight().x(), bb.topRight().y()};
        p->theRTree[l]->Search(min, max, &indexFindCallbackList, (void*)&p->findResult);
    }

    return p->findResult;
}

void MemoryBackend::indexFind(Layer* l, const QRectF& bb, const IndexFindContext& ctxt)
{
    if (!p->theRTree.contains(l))
        return;
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)&ctxt);
}


void MemoryBackend::get(Layer* l, const QRectF& bb, QList<Feature*>& theFeatures)
{
    if (!p->theRTree.contains(l))
        return;
    double min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    double max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)(&theFeatures));
}

void MemoryBackend::getFeatureSet(Layer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                   QList<QRectF>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform)
{
    IndexFindContext ctxt;
    ctxt.theFeatures = &theFeatures;
    ctxt.clipRect = &clipRect;
    ctxt.theProjection = &theProjection;
    ctxt.theTransform = &theTransform;

    for (int i=0; i < invalidRects.size(); ++i) {
        indexFind(l, invalidRects[i], ctxt);
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

    QHash<Feature *, CoordBox>::const_iterator i = p->AllocFeatures.constBegin();
    while (i != p->AllocFeatures.constEnd()) {
        delete i.key();
        ++i;
    }

    delete p;
}

Node * MemoryBackend::allocNode(Layer* l, const Node& other)
{
    Node* f = new Node(other);
    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocNode(Layer* l, const QPointF& aCoord)
{
    Node* f = new Node(aCoord);
    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocVirtualNode(const QPointF& aCoord)
{
    Node* f = new Node(aCoord);
    return f;
}

Way * MemoryBackend::allocWay(Layer* l)
{
    Way* f = new Way();
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Way * MemoryBackend::allocWay(Layer* l, const Way& other)
{
    Way* f = new Way(other);
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(Layer* l)
{
    Relation* f = new Relation();
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(Layer* l, const Relation& other)
{
    Relation* f = new Relation(other);
    p->AllocFeatures[f] = CoordBox();
    return f;
}

TrackSegment * MemoryBackend::allocSegment(Layer* l)
{
    TrackSegment* f = new TrackSegment();
    p->AllocFeatures[f] = CoordBox();
    return f;
}

void MemoryBackend::clearLayer(Layer *l)
{
    if (p->theRTree.contains(l))
        p->theRTree.remove(l);
}

void MemoryBackend::deallocFeature(Layer* l, Feature *f)
{
    if (p->AllocFeatures.contains(f))
        indexRemove(l, p->AllocFeatures[f], f);
}

void MemoryBackend::deallocVirtualNode(Feature *f)
{
    delete f;
}

void MemoryBackend::move(Layer *oldL, Layer *newL, Feature *f)
{
    if (oldL)
        indexRemove(oldL, p->AllocFeatures[f], f);
    if (newL)
        indexAdd(newL, p->AllocFeatures[f], f);
}

void MemoryBackend::sync(Feature *f)
{
    if (p->AllocFeatures.contains(f) && !p->AllocFeatures[f].isNull())
        indexRemove(f->layer(), p->AllocFeatures[f], f);
    if (!f->isDeleted()) {
        CoordBox bb = f->boundingBox();
        p->AllocFeatures[f] = bb;
        if (!bb.isNull()) {
            indexAdd(f->layer(), bb, f);
        }
    }
}


