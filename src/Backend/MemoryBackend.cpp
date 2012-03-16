#include "MemoryBackend.h"
#include "RTree.h"

RenderPriority NodePri(RenderPriority::IsSingular,0., 0);
RenderPriority SegmentPri(RenderPriority::IsLinear,0.,99);

inline void* operator new (std::size_t sz)
{
  // fixme: throw bad_alloc on error
    void *p = (void *) malloc(sz);
    if (!p)
        qDebug() << "Alloc error";
    return p;
}

void operator delete (void *p)
{
    free(p);
}

typedef RTree<Feature*, qreal, 2, qreal, 32> CoordTree;

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
        if (!(F->isVirtual() && !M_PREFS->getVirtualNodesVisible()))
            (*(pCtxt->theFeatures))[NodePri].insert(F);
    } else {
        if (pCtxt->theFeatures->value(SegmentPri).contains(F))
            return true;
        (*(pCtxt->theFeatures))[SegmentPri].insert(F);
    }

    return true;
}

void MemoryBackend::indexAdd(Layer* l, const QRectF& bb, Feature* aFeat)
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

void MemoryBackend::indexRemove(Layer* l, const QRectF& bb, Feature* aFeat)
{
    if (!l)
        return;
    if (!p->theRTree.contains(l))
        return;

    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Remove(min, max, aFeat);
}

const QList<Feature*>& MemoryBackend::indexFind(Layer* l, const QRectF& bb)
{
    p->findResult.clear();
    if (p->theRTree.contains(l)) {
        qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
        qreal max[] = {bb.topRight().x(), bb.topRight().y()};
        p->theRTree[l]->Search(min, max, &indexFindCallbackList, (void*)&p->findResult);
    }

    return p->findResult;
}

void MemoryBackend::indexFind(Layer* l, const QRectF& bb, const IndexFindContext& ctxt)
{
    if (!p->theRTree.contains(l))
        return;
    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)&ctxt);
}


void MemoryBackend::get(Layer* l, const QRectF& bb, QList<Feature*>& theFeatures)
{
    if (!p->theRTree.contains(l))
        return;
    qreal min[] = {bb.bottomLeft().x(), bb.bottomLeft().y()};
    qreal max[] = {bb.topRight().x(), bb.topRight().y()};
    p->theRTree[l]->Search(min, max, &indexFindCallback, (void*)(&theFeatures));
}

void MemoryBackend::getFeatureSet(Layer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                                  QList<CoordBox>& invalidRects, Projection& theProjection)
{
    IndexFindContext ctxt;
    ctxt.theFeatures = &theFeatures;
    ctxt.theProjection = &theProjection;

    for (int i=0; i < invalidRects.size(); ++i) {
        ctxt.bbox = invalidRects[i];
        indexFind(l, invalidRects[i], ctxt);
    }
}

void MemoryBackend::getFeatureSet(Layer* l, QMap<RenderPriority, QSet <Feature*> >& theFeatures,
                                  CoordBox& invalidRect, Projection& theProjection)
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

Node * MemoryBackend::allocNode(Layer* l, const Node& other)
{
    Node* f;
    try {
        f = new Node(other);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocNode(Layer* l, const QPointF& aCoord)
{
    Node* f;
    try {
        f = new Node(aCoord);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

TrackNode * MemoryBackend::allocTrackNode(Layer* l, const QPointF& aCoord)
{
    TrackNode* f;
    try {
        f = new TrackNode(aCoord);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(Layer* l, const QPointF& aCoord)
{
    PhotoNode* f;
    try {
        f = new PhotoNode(aCoord);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(Layer* l, const Node& other)
{
    PhotoNode* f;
    try {
        f = new PhotoNode(other);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

PhotoNode * MemoryBackend::allocPhotoNode(Layer* l, const TrackNode& other)
{
    PhotoNode* f;
    try {
        f = new PhotoNode(other);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }

    p->AllocFeatures[f] = f->BBox;
    if (!f->BBox.isNull()) {
        indexAdd(l, f->BBox, f);
    }
    return f;
}

Node * MemoryBackend::allocVirtualNode(const QPointF& aCoord)
{
    Node* f;
    try {
        f = new Node(aCoord);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    return f;
}

Way * MemoryBackend::allocWay(Layer* l)
{
    Way* f;
    try {
        f = new Way();
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Way * MemoryBackend::allocWay(Layer* l, const Way& other)
{
    Way* f;
    try {
        f = new Way(other);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(Layer* l)
{
    Relation* f;
    try {
        f = new Relation();
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = CoordBox();
    return f;
}

Relation * MemoryBackend::allocRelation(Layer* l, const Relation& other)
{
    Relation* f;
    try {
        f = new Relation(other);
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = CoordBox();
    return f;
}

TrackSegment * MemoryBackend::allocSegment(Layer* l)
{
    TrackSegment* f;
    try {
        f = new TrackSegment();
        if (!f)
            return NULL;
    } catch (...) { // Out-of-memory?
        return NULL;
    }
    p->AllocFeatures[f] = CoordBox();
    return f;
}

void MemoryBackend::deallocFeature(Layer* l, Feature *f)
{
    if (p->AllocFeatures.contains(f)) {
        indexRemove(l, p->AllocFeatures[f], f);
        p->AllocFeatures.remove(f);
    }
}

void MemoryBackend::deallocVirtualNode(Feature *f)
{
    delete f;
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


