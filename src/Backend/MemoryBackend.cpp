#include "MemoryBackend.h"
#include "RTree.h"

#include <QReadWriteLock>

RenderPriority NodePri(RenderPriority::IsSingular,0., 0);
RenderPriority SegmentPri(RenderPriority::IsLinear,0.,99);

typedef RTree<Feature*, qreal, 2, qreal, 32> CoordTree;

class MemoryBackendPrivate
{
public:

    /* If locked, deletes are delayed. */
    QReadWriteLock delayedDeletesLock;

    /* Protects the toBeDeleted */
    QMutex toBeDeletedLock;
    QList<Feature*> toBeDeleted;

    QHash<Feature*, CoordBox> AllocFeatures;
    QHash<ILayer*, CoordTree*> theRTree;
    QList<Feature*> findResult;
};

bool indexFindCallbackList(Feature* F, void* ctxt)
{
    ((QList<Feature*>*)(ctxt))->append(F);
    return true;
}

bool indexFindCallback(Feature* F, void* ctxt)
{
    IndexFindContext* pCtxt = (IndexFindContext*)ctxt;

    if (!F->isVisible())
        return true;

    if (CHECK_WAY(F)) {
        Way * R = STATIC_CAST_WAY(F);
        if (pCtxt->theFeatures->value(R->renderPriority()).contains(F))
            return true;
        R->buildPath(*(pCtxt->theProjection));
        if (M_PREFS->getTrackPointsVisible()) {
            for (int i=0; i<R->size(); ++i) {
                if (pCtxt->bbox.contains(R->getNode(i)->boundingBox()))
                    if (!pCtxt->theFeatures->value(NodePri).contains(R->getNode(i)))
                        (*(pCtxt->theFeatures))[NodePri].insert(R->getNode(i));
            }
        }
        (*(pCtxt->theFeatures))[R->renderPriority()].insert(F);
    } else
    if (CHECK_RELATION(F)) {
        Relation * RR = STATIC_CAST_RELATION(F);
        if (pCtxt->theFeatures->value(RR->renderPriority()).contains(F))
            return true;
        RR->buildPath(*(pCtxt->theProjection));
        (*(pCtxt->theFeatures))[RR->renderPriority()].insert(F);
    } else
    if (CHECK_NODE(F)) {
        if (pCtxt->theFeatures->value(NodePri).contains(F))
            return true;
        if (!(F->isVirtual() && !M_PREFS->getVirtualNodesVisible())) {
            Node * N = STATIC_CAST_NODE(F);
            N->buildPath(*(pCtxt->theProjection));
            (*(pCtxt->theFeatures))[NodePri].insert(F);
        }
    } else {
        if (pCtxt->theFeatures->value(SegmentPri).contains(F))
            return true;
        (*(pCtxt->theFeatures))[SegmentPri].insert(F);
    }

    return true;
}

void MemoryBackend::indexAdd(ILayer* l, const QRectF& bb, Feature* aFeat)
{
    if (!l)
        return;
    if (!p->theRTree.contains(l))
        p->theRTree[l] = new CoordTree();

    p->AllocFeatures[aFeat] = bb;
    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Insert(min, max, aFeat);
}

void MemoryBackend::indexRemove(ILayer* l, const QRectF& bb, Feature* aFeat)
{
    if (!l)
        return;
    if (!p->theRTree.contains(l))
        return;

    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Remove(min, max, aFeat);
}

const QList<Feature*>& MemoryBackend::indexFind(ILayer* l, const QRectF& bb)
{
    p->findResult.clear();
    if (p->theRTree.contains(l)) {
        qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
        qreal max[] = {bb.topRight().x(), bb.topRight().y()};
        p->theRTree[l]->Search(min, max, &indexFindCallbackList, (void*)&p->findResult);
    }

    return p->findResult;
}

void MemoryBackend::indexFind(ILayer* l, const QRectF& bb, const IndexFindContext& ctxt)
{
    if (!p->theRTree.contains(l))
        return;
    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)&ctxt);
}


void MemoryBackend::get(ILayer* l, const QRectF& bb, QList<Feature*>& theFeatures)
{
    if (!p->theRTree.contains(l))
        return;
    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)(&theFeatures));
}

void MemoryBackend::getFeatureSet(ILayer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                                  const QList<CoordBox>& invalidRects, Projection& theProjection)
{
    IndexFindContext ctxt;
    ctxt.theFeatures = &theFeatures;
    ctxt.theProjection = &theProjection;

    for (int i=0; i < invalidRects.size(); ++i) {
        ctxt.bbox = invalidRects[i];
        indexFind(l, invalidRects[i], ctxt);
    }
}

void MemoryBackend::getFeatureSet(ILayer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                                  const CoordBox& invalidRect, Projection& theProjection)
{
    IndexFindContext ctxt;
    ctxt.theFeatures = &theFeatures;
    ctxt.theProjection = &theProjection;

    ctxt.bbox = invalidRect;
    indexFind(l, invalidRect, ctxt);
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

Node * MemoryBackend::allocNode(ILayer* l, const Node& other)
{
    Node* f;
    f = new (std::nothrow) Node(other);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocNode(ILayer* l, const QPointF& aCoord)
{
    Node* f;
    f = new (std::nothrow) Node(aCoord);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

TrackNode * MemoryBackend::allocTrackNode(ILayer* l, const QPointF& aCoord)
{
    TrackNode* f;
    f = new (std::nothrow) TrackNode(aCoord);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(ILayer* l, const QPointF& aCoord)
{
    PhotoNode* f;
    f = new (std::nothrow) PhotoNode(aCoord);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(ILayer* l, const Node& other)
{
    PhotoNode* f;
    f = new (std::nothrow) PhotoNode(other);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(ILayer* l, const TrackNode& other)
{
    PhotoNode* f;
    f = new (std::nothrow) PhotoNode(other);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocVirtualNode(const QPointF& aCoord)
{
    return new (std::nothrow) Node(aCoord);
}

Way * MemoryBackend::allocWay(ILayer* /*l*/)
{
    Way* f;
    f = new (std::nothrow) Way();
    if (!f)
        return NULL;

    p->AllocFeatures[f] = CoordBox();
    return f;
}

Way * MemoryBackend::allocWay(ILayer* /*l*/, const Way& other)
{
    Way* f;
    f = new (std::nothrow) Way(other);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(ILayer* /*l*/)
{
    Relation* f;
    f = new (std::nothrow) Relation();
    if (!f)
        return NULL;

    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(ILayer* /*l*/, const Relation& other)
{
    Relation* f;
    f = new (std::nothrow) Relation(other);
    if (!f)
        return NULL;

    p->AllocFeatures[f] = CoordBox();
    return f;
}

TrackSegment * MemoryBackend::allocSegment(ILayer* /*l*/)
{
    TrackSegment* f;
    f = new (std::nothrow) TrackSegment();
    if (!f)
        return NULL;

    p->AllocFeatures[f] = CoordBox();
    return f;
}

void MemoryBackend::deallocFeature(ILayer* l, Feature *f)
{
    p->delayedDeletesLock.lockForRead();
    p->toBeDeletedLock.lock();
    if (p->AllocFeatures.contains(f)) {
        indexRemove(l, p->AllocFeatures[f], f);
        if (!p->AllocFeatures.remove(f)) {
            qWarning() << "Feature, that is not in a list is being removed.";
        } else {
            p->toBeDeleted.append(f);
        }
    }
    p->toBeDeletedLock.unlock();
    p->delayedDeletesLock.unlock();
}

void MemoryBackend::purge()
{
    if (p->toBeDeleted.empty()) return; /* Don't bother if there is nothing to delete */
    if (!p->delayedDeletesLock.tryLockForWrite()) return; /* If locked, deletes need to be delayed */
    p->toBeDeletedLock.lock();
    QList<Feature*>::iterator it = p->toBeDeleted.begin();
    while (it != p->toBeDeleted.end()) {
        delete *(it++);
    }
    p->toBeDeleted.clear();
    p->toBeDeletedLock.unlock();
    p->delayedDeletesLock.unlock();
}

void MemoryBackend::delayDeletes() {
    p->delayedDeletesLock.lockForRead();
}

void MemoryBackend::resumeDeletes() {
    p->delayedDeletesLock.unlock();
    purge();
}

void MemoryBackend::deallocVirtualNode(Feature *f)
{
    p->delayedDeletesLock.tryLockForRead();
    p->toBeDeletedLock.lock();
    p->toBeDeleted.append(f);
    p->toBeDeletedLock.unlock();
    p->delayedDeletesLock.unlock();
}

void MemoryBackend::sync(Feature *f)
{
    if (p->AllocFeatures.contains(f) && !p->AllocFeatures[f].isNull())
        indexRemove(f->layer(), p->AllocFeatures[f], f);
    if (CHECK_NODE(f)) {
        Node* N = STATIC_CAST_NODE(f);
        if (!N->tagSize())
            for (int i=0; i<N->sizeParents(); ++i)
                if (CHECK_WAY(N->getParent(i)))
                    return;
    }
    if (!f->isDeleted() && f->layer()) {
        CoordBox bb = f->boundingBox();
        if (!bb.isNull()) {
            indexAdd(f->layer(), bb, f);
        }
    }
}


