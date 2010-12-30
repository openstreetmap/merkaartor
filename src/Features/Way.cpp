#include "Global.h"

#include "Way.h"
#include "Node.h"

#include "DocumentCommands.h"
#include "WayCommands.h"
#include "Maps/Coord.h"
#include "Maps/Painting.h"
#include "MapView.h"
#include "Utils/LineF.h"
#include "Utils/MDiscardableDialog.h"
#include "Utils/Utils.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QProgressDialog>

#include <algorithm>
#include <QList>

#ifndef _MOBILE
#if QT_VERSION < 0x040700
#include <ggl/ggl.hpp>
#include <ggl/geometries/cartesian2d.hpp>
#include <ggl/geometries/adapted/std_as_linestring.hpp>
#include <ggl/algorithms/intersects.hpp>
#endif
#endif

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

class WayPrivate
{
    public:
        WayPrivate(Way* aWay)
        : theWay(aWay), SmoothedUpToDate(false), BBoxUpToDate(false)
            , Area(0), Distance(0)
            , wasPathComplete(false), VirtualsUptodate(false)
            , ProjectionRevision(0)
            , BestSegment(-1)
            , Width(0)
        {
        }
        Way* theWay;

        std::vector<NodePtr> Nodes;
        std::vector<NodePtr> virtualNodes;
        QList<Coord> Smoothed;
        bool SmoothedUpToDate;

        mutable CoordBox BBox;
        bool BBoxUpToDate;

        double Area;
        double Distance;
        bool NotEverythingDownloaded;
        bool wasPathComplete;
        bool VirtualsUptodate;
        QRectF roadRect;
        QPainterPath theFullPath;
        QPainterPath thePath;
#ifndef _MOBILE
        int ProjectionRevision;
#endif
        int BestSegment;
        double Width;

        void CalculateWidth();
        void updateSmoothed(bool DoSmooth);
        void addSmoothedBezier(int i, int j, int k, int l);
        void doUpdateVirtuals();
        void removeVirtuals();
        void addVirtuals();
};

#define DEFAULTWIDTH 6
#define LANEWIDTH 4

void WayPrivate::CalculateWidth()
{
    QString s(theWay->tagValue("width",QString()));
    if (!s.isNull()) {
        Width = s.toDouble();
        return;
    }
    QString h = theWay->tagValue("highway",QString());
    if (s.isNull()) {
        Width = DEFAULTWIDTH;
        return;
    }

    if ( (h == "motorway") || (h=="motorway_link") )
        Width =  4*LANEWIDTH; // 3 lanes plus emergency
    else if ( (h == "trunk") || (h=="trunk_link") )
        Width =  3*LANEWIDTH; // 2 lanes plus emergency
    else if ( (h == "primary") || (h=="primary_link") )
        Width =  2*LANEWIDTH; // 2 lanes
    else if (h == "secondary")
        Width =  2*LANEWIDTH; // 2 lanes
    else if (h == "tertiary")
        Width =  1.5*LANEWIDTH; // shared middle lane
    else if (h == "cycleway")
        Width =  1.5;
    Width = DEFAULTWIDTH;
}

void WayPrivate::addSmoothedBezier(int i, int j, int k, int l)
{
    Coord A(Nodes[i]->position());
    Coord B(Nodes[j]->position());
    Coord C(Nodes[k]->position());
    Coord D(Nodes[l]->position());

    Coord Ctrl1(B+(C-A)*(1/6));
    Coord Ctrl2(C-(D-B)*(1/6));


    Smoothed.push_back(Ctrl1);
    Smoothed.push_back(Ctrl2);
    Smoothed.push_back(C);
}

void WayPrivate::updateSmoothed(bool DoSmooth)
{
    SmoothedUpToDate = true;
    Smoothed.clear();
    if ( (Nodes.size() < 3) || !DoSmooth )
        return;
    Smoothed.push_back(Nodes[0]->position());
    addSmoothedBezier(0,0,1,2);
    for (unsigned int i=1; i<Nodes.size()-2; ++i)
        addSmoothedBezier(i-1,i,i+1,i+2);
    int Last = Nodes.size()-1;
    addSmoothedBezier(Last-2,Last-1,Last,Last);
}

void WayPrivate::removeVirtuals()
{
    while (virtualNodes.size()) {
        virtualNodes[0]->unsetParentFeature(theWay);
        if (virtualNodes[0]->layer())
            virtualNodes[0]->layer()->deleteFeature(virtualNodes[0]);
//		delete p->virtualNodes[0];
        virtualNodes.erase(virtualNodes.begin());
    }
}

void WayPrivate::addVirtuals()
{
    for (unsigned int i=1; i<Nodes.size(); ++i) {
        QLineF l(toQt(Nodes[i-1]->position()), toQt(Nodes[i]->position()));
        l.setLength(l.length()/2);
        Node* v = new Node(toCoord(l.p2()));
        v->setVirtual(true);
        v->setParentFeature(theWay);
        theWay->layer()->add(v);
        virtualNodes.push_back(v);
    }
}

void WayPrivate::doUpdateVirtuals()
{
//    Q_ASSERT(!(theWay->layer()->isIndexingBlocked()));

    if (VirtualsUptodate)
        return;

    removeVirtuals();
    if (M_PREFS->getUseVirtualNodes() && theWay->layer() && !(theWay->isReadonly()) && !(theWay->isDeleted()))
        addVirtuals();

    VirtualsUptodate = true;
}

/**************************/

Way::Way(void)
{
    p = new WayPrivate(this);
}

Way::Way(const Way& other)
: Feature(other)
{
    p = new WayPrivate(this);
}

Way::~Way(void)
{
    // TODO Those cleanup shouldn't be necessary and lead to crashes
    //      Check for side effect of supressing them.
//    while (p->virtualNodes.size()) {
//        p->virtualNodes[0]->unsetParentFeature(this);
//        delete p->virtualNodes[0];
//        p->virtualNodes.erase(p->virtualNodes.begin());
//    }
//    for (unsigned int i=0; i<p->Nodes.size(); ++i)
//        if (p->Nodes[i])
//            p->Nodes[i]->unsetParentFeature(this);

    delete p;
}

char Way::getType() const
{
    if (isClosed())
        return (IFeature::LineString | IFeature::Polygon);
    else
        return IFeature::LineString;
}


void Way::setDeleted(bool delState)
{
    Feature::setDeleted(delState);
    p->VirtualsUptodate = false;
    if (p->Nodes.size())
        p->doUpdateVirtuals();
}

void Way::setLayer(Layer* L)
{
    if (L) {
        for (unsigned int i=0; i<p->Nodes.size(); ++i) {
            if (p->Nodes[i])
                p->Nodes[i]->setParentFeature(this);
        }
    } else {
        for (unsigned int i=0; i<p->Nodes.size(); ++i) {
            if (p->Nodes[i])
                p->Nodes[i]->unsetParentFeature(this);
        }
    }
    Feature::setLayer(L);
    p->VirtualsUptodate = false;
}

void Way::partChanged(Feature* /*F*/, int ChangeId)
{
    if (isDeleted())
        return;

    p->BBoxUpToDate = false;
    p->wasPathComplete = false;
    MetaUpToDate = false;
    p->SmoothedUpToDate = false;
    p->wasPathComplete = false;
    p->VirtualsUptodate = false;

    notifyParents(ChangeId);
}

QString Way::description() const
{
    QString s(tagValue("name",""));
    if (!s.isEmpty())
        return QString("%1 (%2)").arg(s).arg(id().numId);
    return QString("%1").arg(id().numId);
}

void Way::add(Node* Pt)
{
    add(Pt, p->Nodes.size());
}

void Way::add(Node* Pt, int Idx)
{
    if (layer())
        layer()->indexRemove(p->BBox, this);
    p->Nodes.insert(p->Nodes.begin() + Idx, Pt);
//	p->Nodes.push_back(Pt);
//	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
    Pt->setParentFeature(this);
    p->BBoxUpToDate = false;
    p->wasPathComplete = false;
    MetaUpToDate = false;
    p->SmoothedUpToDate = false;
    p->wasPathComplete = false;
    p->VirtualsUptodate = false;
    if (layer() && !isDeleted()) {
        CoordBox bb = boundingBox();
        layer()->indexAdd(bb, this);
    }

    notifyChanges();
}

int Way::find(Feature* Pt) const
{
    for (unsigned int i=0; i<p->Nodes.size(); ++i)
        if (p->Nodes[i] == Pt)
            return i;
    return p->Nodes.size();
}

int Way::findVirtual(Feature* Pt) const
{
    for (unsigned int i=0; i<p->virtualNodes.size(); ++i)
        if (p->virtualNodes[i] == Pt)
            return i;
    return p->virtualNodes.size();
}

void Way::remove(int idx)
{
    if (layer())
        layer()->indexRemove(p->BBox, this);

    Node* Pt = p->Nodes[idx];
    // only remove as parent if the node is only included once
    p->Nodes.erase(p->Nodes.begin()+idx);
    if (Pt && (find(Pt) >= size()))
        Pt->unsetParentFeature(this);
    p->BBoxUpToDate = false;
    p->wasPathComplete = false;
    MetaUpToDate = false;
    p->SmoothedUpToDate = false;
    p->wasPathComplete = false;
    p->VirtualsUptodate = false;
    if (layer() && !isDeleted()) {
        CoordBox bb = boundingBox();
        layer()->indexAdd(bb, this);
    }

    notifyChanges();
}

void Way::remove(Feature* F)
{
    for (int i=p->Nodes.size(); i; --i)
        if (p->Nodes[i-1] == F)
            remove(i-1);
}

int Way::size() const
{
    return p->Nodes.size();
}

Node* Way::getNode(int idx)
{
    return p->Nodes.at(idx);
}

const Node* Way::getNode(int idx) const
{
    return p->Nodes.at(idx);
}

const std::vector<NodePtr>& Way::getNodes() const
{
    return p->Nodes;
}

const std::vector<NodePtr>& Way::getVirtuals() const
{
    return p->virtualNodes;
}


Feature* Way::get(int idx)
{
    return p->Nodes.at(idx);
}

const Feature* Way::get(int idx) const
{
    return p->Nodes.at(idx);
}

bool Way::isNull() const
{
    return (p->Nodes.size() == 0);
}

bool Way::notEverythingDownloaded()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->NotEverythingDownloaded;
}

const CoordBox& Way::boundingBox(bool update) const
{
    if (!p->BBoxUpToDate && update)
    {
        if (p->Nodes.size())
        {
            p->BBox = CoordBox(p->Nodes[0]->position(),p->Nodes[0]->position());
            for (unsigned int i=1; i<p->Nodes.size(); ++i)
                p->BBox.merge(p->Nodes[i]->position());
        }
        else
            p->BBox = CoordBox(Coord(0,0),Coord(0,0));
        p->BBoxUpToDate = true;
    }
    return p->BBox;
}

void Way::updateMeta()
{
    Feature::updateMeta();
    MetaUpToDate = true;

    p->Area = 0;
    p->Distance = 0;
    p->CalculateWidth();

    p->NotEverythingDownloaded = false;
    if (lastUpdated() == Feature::NotYetDownloaded)
        p->NotEverythingDownloaded = true;
    else
        for (unsigned int i=0; i<p->Nodes.size(); ++i)
            if (p->Nodes.at(i) && p->Nodes.at(i)->notEverythingDownloaded()) {
                p->NotEverythingDownloaded = true;
                break;
            }

    if (p->Nodes.size() == 0)
        return;

    bool isArea = false;
    if (tagValue("junction", "") != "roundabout")
        isArea = (p->Nodes[0] == p->Nodes[p->Nodes.size()-1]);

    for (unsigned int i=0; (i+1)<p->Nodes.size(); ++i)
    {
        if (p->Nodes[i] && p->Nodes[i+1]) {
            const Coord & here = p->Nodes[i]->position();
            const Coord & next = p->Nodes[i+1]->position();

            p->Distance += next.distanceFrom(here);
            //if (isArea)
                //p->Area += here.lat() * next.lon() - next.lat() * here.lon();
        }
    }

    if (isArea) {
        p->Area = p->Distance;
        setRenderPriority(RenderPriority(RenderPriority::IsArea,-fabs(p->Area), 0));
    } else {
        qreal Priority = tagValue("layer","0").toInt();
        if (Priority >= 0)
            Priority++;
        int layer = Priority;
        // dummy number to get a deterministic feature sort
//		Priority += sin(intToRad(boundingBox().lonDiff()));
        Priority += p->Distance / INT_MAX;
        setRenderPriority(RenderPriority(RenderPriority::IsLinear,Priority, layer));
    }

    p->doUpdateVirtuals();
}

double Way::distance()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->Distance;
}

bool Way::isClosed() const
{
    // minimal closed way is a triangle, with 4 nodes (3 distinct)
    return (p->Nodes.size() > 3 && p->Nodes[0] == p->Nodes[p->Nodes.size()-1]);
}

double Way::area()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->Area;
}

double Way::widthOf()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->Width;
}

void Way::draw(QPainter& P, MapView* theView)
{
    if (isDirty() && isUploadable() && M_PREFS->getDirtyVisible()) {
        QPen thePen(M_PREFS->getDirtyColor(),M_PREFS->getDirtyWidth());
        P.setPen(thePen);
        P.drawPath(theView->transform().map(getPath()));
    }

    double theWidth = theView->nodeWidth();
    bool Draw = (theWidth >= 1);
    if (!Draw || !TEST_RFLAGS(RendererOptions::VirtualNodesVisible) || !TEST_RFLAGS(RendererOptions::NodesVisible) || isReadonly())
        return;

    theWidth /= 2;
    P.setPen(QColor(0,0,0));
    foreach (NodePtr N, p->virtualNodes) {
        if (theView->viewport().contains(N->position())) {
            QPoint p =  theView->toView(N);
            P.drawLine(p+QPoint(-theWidth, -theWidth), p+QPoint(theWidth, theWidth));
            P.drawLine(p+QPoint(theWidth, -theWidth), p+QPoint(-theWidth, theWidth));
        }
    }
}

void Way::drawSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    thePainter.setPen(Pen);
    if (p->BestSegment != -1)
        thePainter.drawLine(theView->transform().map(theView->projection().project(getSegment(p->BestSegment))));
    else
        thePainter.drawPath(theView->transform().map(p->thePath));
}

void Way::drawParentsSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    for (int i=0; i<sizeParents(); ++i) {
        if (!getParent(i)->isDeleted()) {
            Feature* f = CAST_FEATURE(getParent(i));
            if (f)
                f->drawSpecial(thePainter, Pen, theView);
        }
    }
}

void Way::drawChildrenSpecial(QPainter& thePainter, QPen& Pen, MapView *theView, int depth)
{
    Q_UNUSED(depth);

    QPen TP(Pen);
    TP.setWidth(TP.width()*3);
    TP.setCapStyle(Qt::RoundCap);
    thePainter.setPen(TP);

    QPolygonF Pl;
    if (p->BestSegment != -1) {
        for (int i=p->BestSegment; i<=p->BestSegment+1; ++i)
            if (getNode(i)->isVisible() && !getNode(i)->isVirtual())
                Pl.append(theView->projection().project(getNode(i)));
    } else
        buildPolygonFromRoad(this,theView->projection(),Pl);

    thePainter.drawPoints(theView->transform().map(Pl));
}


double Way::pixelDistance(const QPointF& Target, double ClearEndDistance, bool selectNodes, MapView* theView) const
{
    double Best = 1000000;
    p->BestSegment = -1;

    if (selectNodes) {
        for (unsigned int i=0; i<p->Nodes.size(); ++i)
        {
            if (p->Nodes.at(i)) {
                double x = ::distance(Target, theView->toView(p->Nodes.at(i)));
                if (x<ClearEndDistance)
                    return Best;
            }
        }
    }
    if (smoothed().size())
        for (int i=3; i <p->Smoothed.size(); i += 3)
        {
            BezierF F(
                theView->toView(p->Smoothed[i-3]),
                theView->toView(p->Smoothed[i-2]),
                theView->toView(p->Smoothed[i-1]),
                theView->toView(p->Smoothed[i]));
            double D = F.distance(Target);
            if (D < ClearEndDistance)
                Best = D;
        }
    else
        for (unsigned int i=0; i<p->Nodes.size()-1; ++i)
        {
            if (p->Nodes.at(i) && p->Nodes.at(i+1)) {
                LineF F(theView->toView(p->Nodes.at(i)),theView->toView(p->Nodes.at(i+1)));
                double D = F.capDistance(Target);
                if (D < ClearEndDistance && D < Best) {
                    Best = D;
                    if (g_Merk_Segment_Mode)
                        p->BestSegment = i;
                }
            }
        }
    return Best;
}

Node* Way::pixelDistanceVirtual(const QPointF& Target, double ClearEndDistance, MapView* theView) const
{
    for (unsigned int i=0; i<p->virtualNodes.size(); ++i)
    {
        if (p->virtualNodes.at(i)) {
            double x = ::distance(Target,theView->toView(p->virtualNodes.at(i)));
            if (x<ClearEndDistance)
                return p->virtualNodes.at(i);
        }
    }
    return NULL;
}

void Way::cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Proposals)
{
    for (unsigned int i=0; i<p->Nodes.size();) {
        if (p->Nodes[i] == aFeature)
        {
            QList<Node*> Alternatives;
            for (int j=0; j<Proposals.size(); ++j)
            {
                Node* Pt = dynamic_cast<Node*>(Proposals[j]);
                if (Pt)
                    Alternatives.push_back(Pt);
            }
            for (int j=0; j<Alternatives.size(); ++j)
            {
                if (i < p->Nodes.size())
                {
                    if (p->Nodes[i+j] != Alternatives[j])
                    {
                        if ((i+j) == 0)
                            theList->add(new WayAddNodeCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(layer())));
                        else if (p->Nodes[i+j-1] != Alternatives[j] && p->Nodes[i+j+1] != Alternatives[j])
                            theList->add(new WayAddNodeCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(layer())));
                    }
                }
            }
            theList->add(new WayRemoveNodeCommand(this, (Node*)aFeature,theDocument->getDirtyOrOriginLayer(layer())));
        }
        ++i;
    }
    if (p->Nodes.size() == 1) {
        if (!isDeleted())
            theList->add(new RemoveFeatureCommand(theDocument,this));
    }
}

const QPainterPath& Way::getPath() const
{
    return p->thePath;
}

void Way::buildPath(const Projection &theProjection, const QTransform& theTransform, const QRectF& cr)
{
#if QT_VERSION < 0x040700
    using namespace ggl;
#endif

    Q_UNUSED(theTransform);
    Q_UNUSED(cr);

    if (p->Nodes.size() < 2)
        return;

    if (!p->wasPathComplete || p->ProjectionRevision != theProjection.projectionRevision()) {
        p->theFullPath = QPainterPath();

        p->theFullPath.moveTo(theProjection.project(p->Nodes.at(0)));
        for (unsigned int i=1; i<p->Nodes.size(); ++i) {
            p->theFullPath.lineTo(theProjection.project(p->Nodes.at(i)));
        }
        p->ProjectionRevision = theProjection.projectionRevision();
        p->wasPathComplete = true;

        QPointF pbl = theProjection.project(p->BBox.bottomLeft());
        QPointF ptr = theProjection.project(p->BBox.topRight());
        p->roadRect = QRectF(pbl, ptr);
    }
#if QT_VERSION >= 0x040700
    p->thePath = p->theFullPath;
#else
    if (!g_Merk_SelfClip) {
        p->thePath = p->theFullPath;
    } else {
        bool toClip = !cr.contains(p->roadRect);
    //    bool toClip = false;
        if (!toClip) {
            p->thePath = p->theFullPath;
        } else {
            if (area() > 0) {
                QPainterPath clipPath;
                clipPath.addRect(cr);

                p->thePath = clipPath.intersected(p->theFullPath);
            } else {
                std::vector<linestring_2d> clipped;

                // Ensure nodes' projection is up-to-date
                for (unsigned int i=0; i<p->Nodes.size(); ++i)
                    theProjection.project(p->Nodes[i]);
                box_2d clipRect (make<point_2d>(cr.bottomLeft().x(), cr.topRight().y()), make<point_2d>(cr.topRight().x(), cr.bottomLeft().y()));

                intersection <linestring_2d, box_2d, /*linestring_2d*/ std::vector<NodePtr>, std::back_insert_iterator <std::vector<linestring_2d> > >
                        (clipRect, /*in*/ p->Nodes, std::back_inserter(clipped));

                p->thePath = QPainterPath();
                for (std::vector<linestring_2d>::const_iterator it = clipped.begin(); it != clipped.end(); it++)
                {
                    if (!(*it).empty()) {
                        p->thePath.moveTo(QPointF((*it)[0].x(), (*it)[0].y()));
                    }
                    for (linestring_2d::const_iterator itl = (*it).begin()+1; itl != (*it).end(); itl++)
                    {
                        p->thePath.lineTo(QPointF((*itl).x(), (*itl).y()));
                    }
                }

    //            bool lastPointVisible = true;
    //            QPointF lastPoint = p->Nodes.at(0)->projection();
    //            QPointF aP = lastPoint;
    //            if (!cr.contains(aP)) {
    //                aP.setX(qMax(cr.left(), aP.x()));
    //                aP.setX(qMin(cr.right(), aP.x()));
    //                aP.setY(qMax(cr.top(), aP.y()));
    //                aP.setY(qMin(cr.bottom(), aP.y()));
    //                lastPointVisible = false;
    //            }
    //            p->thePath = QPainterPath();
    //            p->thePath.moveTo(aP);
    //            for (int j=1; j<size(); ++j) {
    //                aP = p->Nodes.at(j)->projection();
    //                QLineF l(lastPoint, aP);
    //                if (!cr.contains(aP)) {
    //                    if (!lastPointVisible) {
    //                        QPointF a, b;
    //                        if (Utils::QRectInterstects(cr, l, a, b)) {
    //                            p->thePath.lineTo(a);
    //                            lastPoint = aP;
    //                            aP = b;
    //                        } else {
    //                            lastPoint = aP;
    //                            aP.setX(qMax(cr.left(), aP.x()));
    //                            aP.setX(qMin(cr.right(), aP.x()));
    //                            aP.setY(qMax(cr.top(), aP.y()));
    //                            aP.setY(qMin(cr.bottom(), aP.y()));
    //                        }
    //                    } else {
    //                        QPointF a, b;
    //                        Utils::QRectInterstects(cr, l, a, b);
    //                        lastPoint = aP;
    //                        aP = a;
    //                    }
    //                    lastPointVisible = false;
    //                } else {
    //                    if (!lastPointVisible) {
    //                        QPointF a, b;
    //                        Utils::QRectInterstects(cr, l, a, b);
    //                        p->thePath.lineTo(a);
    //                    }
    //                    lastPoint = aP;
    //                    lastPointVisible = true;
    //                }
    //                p->thePath.lineTo(aP);
    //            }
            }
        }
    }
#endif
}

bool Way::deleteChildren(Document* theDocument, CommandList* theList)
{
    if (lastUpdated() == Feature::OSMServerConflict)
        return true;

    QList<Feature*> Alternatives;
    QMap<Feature*, int> ToDelete;
    for (int i=(int)p->Nodes.size()-1; i>=0; --i) {
        Node* N = p->Nodes[i];
        if (!theDocument->isDownloadedSafe(N->boundingBox()) && N->hasOSMId())
            continue;
        int sizeValidParents = 0;
        for (int j=0; j<N->sizeParents(); ++j)
            if (!N->getParent(j)->isDeleted())
                sizeValidParents++;
        if (sizeValidParents == 1) {
            ToDelete[N] = i;
        }
    }
    QList<Feature*> ToDeleteKeys = ToDelete.uniqueKeys();
    for (int i=0; i<ToDeleteKeys.size(); ++i) {
        if (!ToDeleteKeys[i]->isDeleted())
            theList->add(new RemoveFeatureCommand(theDocument, ToDeleteKeys[i], Alternatives));
    }
    return true;
}

void Way::setTag(const QString& key, const QString& value)
{
    Feature::setTag(key, value);
    MetaUpToDate = false;
}

void Way::setTag(int index, const QString& key, const QString& value)
{
    Feature::setTag(index, key, value);
    MetaUpToDate = false;
}

void Way::clearTags()
{
    Feature::clearTags();
    MetaUpToDate = false;
}

void Way::clearTag(const QString& k)
{
    Feature::clearTag(k);
    MetaUpToDate = false;
}

bool Way::toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, bool forExport)
{
    bool OK = true;

    stream.writeStartElement("rte");

    if (!forExport)
        stream.writeAttribute("xml:id", xmlId());
    QString s = tagValue("name","");
    if (!s.isEmpty()) {
        stream.writeTextElement("name", s);
    }
    s = tagValue("_comment_","");
    if (!s.isEmpty()) {
        stream.writeTextElement("cmt", s);
    }
    s = tagValue("_description_","");
    if (!s.isEmpty()) {
        stream.writeTextElement("desc", s);
    }

    for (int i=0; i<size(); ++i) {
        if (!getNode(i)->isVirtual())
            getNode(i)->toGPX(stream, progress, "rtept", forExport);
    }
    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

bool Way::toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict, QString changetsetid)
{
    bool OK = true;

    stream.writeStartElement("way");

    Feature::toXML(stream, strict, changetsetid);

    if (size()) {
        stream.writeStartElement("nd");
        stream.writeAttribute("ref", get(0)->xmlId());
        stream.writeEndElement();

        for (int i=1; i<size(); ++i) {
            if (!getNode(i)->isVirtual())
                if (get(i)->xmlId() != get(i-1)->xmlId()) {
                    stream.writeStartElement("nd");
                    stream.writeAttribute("ref", get(i)->xmlId());
                    stream.writeEndElement();
                }
        }
    }

    tagsToXML(stream, strict);
    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

Way * Way::fromXML(Document* d, Layer * L, QXmlStreamReader& stream)
{
    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id(IFeature::LineString, sid.toLongLong());
    Way* R = CAST_WAY(d->getFeature(id));

    if (!R) {
        R = new Way();
        R->setId(id);
        Feature::fromXML(stream, R);
        L->add(R);
    } else {
        Feature::fromXML(stream, R);
        if (R->layer() != L) {
            R->layer()->remove(R);
            L->add(R);
        }
        while (R->p->Nodes.size())
            R->remove(0);
    }

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "nd") {
            QString sId = stream.attributes().value("ref").toString();
            IFeature::FId nId(IFeature::Point, sId.toLongLong());
            Node* Part = CAST_NODE(d->getFeature(nId));
            if (!Part)
            {
                Part = new Node(Coord(0,0));
                Part->setId(nId);
                Part->setLastUpdated(Feature::NotYetDownloaded);
                L->add(Part);
            }
            R->add(Part);
            stream.readNext();
        } else if (stream.name() == "tag") {
            R->setTag(stream.attributes().value("k").toString(), stream.attributes().value("v").toString());
            stream.readNext();
        }

       stream.readNext();
    }

    return R;
}

Feature::TrafficDirectionType trafficDirection(const Way* R)
{
    // TODO some duplication with Way trafficDirection
    QString d;
    int idx=R->findKey("oneway");
    if (idx != -1)
    {
        d = R->tagValue(idx);
        if ( (d == "yes") || (d == "1") || (d == "true")) return Feature::OneWay;
        if (d == "-1") return Feature::OtherWay;
        if ((d == "no") || (d == "false") || (d == "0")) return Feature::BothWays;
    }

    idx=R->findKey("junction");
    if (idx != -1)
    {
        d = R->tagValue(idx);
        if(d=="roundabout") return Feature::OneWay;
        //TODO For motorway and motorway_link, this is still discussed on the wiki.
    }
    return Feature::UnknownDirection;
}

int findSnapPointIndex(const Way* R, Coord& P)
{
    if (R->smoothed().size())
    {
        BezierF L(R->smoothed()[0],R->smoothed()[1],R->smoothed()[2],R->smoothed()[3]);
        int BestIdx = 3;
        double BestDistance = L.distance(toQt(P));
        for (int i=3; i<R->smoothed().size(); i+=3)
        {
            BezierF L(R->smoothed()[i-3],R->smoothed()[i-2],R->smoothed()[i-1],R->smoothed()[i]);
            double Distance = L.distance(toQt(P));
            if (Distance < BestDistance)
            {
                BestIdx = i;
                BestDistance = Distance;
            }
        }
        BezierF B(R->smoothed()[BestIdx-3],R->smoothed()[BestIdx-2],R->smoothed()[BestIdx-1],R->smoothed()[BestIdx]);
        P = toCoord(B.project(toQt(P)));
        return BestIdx/3;
    }
    LineF L(R->getNode(0)->position(),R->getNode(1)->position());
    int BestIdx = 1;
    double BestDistance = L.capDistance(P);
    for (int i = 2; i<R->size(); ++i)
    {
        LineF L(R->getNode(i-1)->position(),R->getNode(i)->position());
        double Distance = L.capDistance(P);
        if (Distance < BestDistance)
        {
            BestIdx = i;
            BestDistance = Distance;
        }
    }
    LineF F(R->getNode(BestIdx-1)->position(),R->getNode(BestIdx)->position());
    P = F.project(Coord(P));
    return BestIdx;
}

const QList<Coord>& Way::smoothed() const
{
    if (!p->SmoothedUpToDate)
        p->updateSmoothed(tagValue("smooth","") == "yes");
    return p->Smoothed;
}

QString Way::toHtml()
{
    QString distanceLabel;
    if (distance() < 1.0)
        distanceLabel = QString("%1 m").arg(int(distance() * 1000));
    else
        distanceLabel = QString("%1 km").arg(distance(), 0, 'f', 3);

    QString D;

    if (isClosed()) {
        D += "<i>"+QApplication::translate("MapFeature", "Closed way")+"</i>";
        D += "<br/>";
    }
    D += "<i>"+QApplication::translate("MapFeature", "Length")+": </i>" + distanceLabel;
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Size")+": </i>" + QApplication::translate("MapFeature", "%1 nodes").arg(size());
    CoordBox bb = boundingBox();
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + COORD2STRING(coordToAng(bb.topLeft().lat())) + " / " + COORD2STRING(coordToAng(bb.topLeft().lon()));
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + COORD2STRING(coordToAng(bb.bottomRight().lat())) + " / " + COORD2STRING(coordToAng(bb.bottomRight().lon()));

    QString type = isClosed() ? QApplication::translate("MapFeature", "Area") : QApplication::translate("MapFeature", "Way");

    return Feature::toMainHtml(type, "way").arg(D);
}

void Way::toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex)
{
    char type = 'R';
    qint32 sz = size();
    if (isClosed()) {
        type = 'A';
        sz--;
    }
    theIndex[type  + QString::number(id().numId)] = ds.device()->pos();
    ds << (qint8)type;
    ds << id().numId;
    ds << (qint32)sz;
    for (int i=0; i < sz; ++i) {
        Node* N = CAST_NODE(get(i));
        if (N->sizeParents() > 1)
            ds << (qint8)'N' << (qint64)(N->id().numId);
        else
            ds << (qint8)'C' << (qint32)(N->position().lat()/180.*INT_MAX) << (qint32)(N->position().lon()/180.*INT_MAX);
    }
}

Way* Way::fromBinary(Document* d, OsbLayer* L, QDataStream& ds, qint8 c, qint64 id)
{
    char type = c;

    qint32	fSize;
    qint32 lat, lon;

    quint8 nodeType;
    qint64 refId;

    ds >> fSize;

    if (!L) {
        for (int i=0; i < fSize; ++i) {
            ds >> nodeType;
            ds >> refId;
        }
        return NULL;
    }

    IFeature::FId strId(IFeature::LineString, id);
    Way* R = CAST_WAY(d->getFeature(strId));
    if (!R) {
        R = new Way();
        R->setId(strId);
        R->setLastUpdated(Feature::OSMServer);
    } else {
        if (R->lastUpdated() == Feature::NotYetDownloaded) {
            R->setLastUpdated(Feature::OSMServer);
            L->remove(R);
        }
        else  {
            for (int i=0; i < fSize; ++i) {
                ds >> nodeType;
                ds >> refId;
            }
            return R;
        }
    }

    Node* N = NULL;
    Node* firstPoint = NULL;
    for (int i=0; i < fSize; ++i) {
        ds >> nodeType;
        switch (nodeType) {
            case 'C':
                ds >> lat;
                ds >> lon;

                N = new Node(Coord( (double)lat/INT_MAX*180., (double)lon/INT_MAX*180. ));
//                N->setParent(R);
                break;
            case 'N':
                ds >> refId;
                N = CAST_NODE(d->getFeature(IFeature::FId(IFeature::Point, refId)));
                Q_ASSERT(N);

        }
        R->add(N);
        if (!firstPoint)
            firstPoint = N;
    }
    if (type == 'A' && firstPoint)
        R->add(firstPoint);

    L->add(R);

    return R;
}

bool Way::isExtrimity(Node* node)
{
    if (p->Nodes[0] == node)
        return true;

    if (p->Nodes[p->Nodes.size()-1] == node)
        return true;

    return false;
}

Way * Way::GetSingleParentRoad(Feature * mapFeature)
{
    Way * parentRoad = NULL;

    for (int i=0; i<mapFeature->sizeParents(); i++)
    {
        Way * road = CAST_WAY(mapFeature->getParent(i));

        if (!road || road->isDeleted())
            continue;

        if (parentRoad && road != parentRoad)
            return NULL;

        //FIXME This test shouldn't be necessary, but there is at least a case where the road has a NULL layer. The root cause must be found.
        if (!(road->isDeleted()) && road->layer() && road->layer()->isEnabled())
            parentRoad = road;
    }

    return parentRoad;
}

Way * Way::GetSingleParentRoadInner(Feature * mapFeature)
{
    Way * parentRoad = NULL;
    Node* trackPoint = CAST_NODE(mapFeature);

    for (int i=0; i<mapFeature->sizeParents(); i++)
    {
        Way * road = CAST_WAY(mapFeature->getParent(i));

        if (!road || road->isDeleted())
            continue;

        if (road->isExtrimity(trackPoint) && !road->isClosed())
            continue;

        if (parentRoad && road != parentRoad)
            return NULL;

        //FIXME This test shouldn't be necessary, but there is at least a case where the road has a NULL layer. The root cause must be found.
        if (!(road->isDeleted()) && road->layer() && road->layer()->isEnabled())
            parentRoad = road;
    }

    return parentRoad;
}

int Way::createJunction(Document* theDocument, CommandList* theList, Way* R1, Way* R2, bool doIt)
{
    int numInter = 0;

    //TODO test that the junction do not already exists!
    for (int i=0; i<R1->size()-1; ++i) {
        QLineF S1(R1->getNode(i)->position().toQPointF(), R1->getNode(i+1)->position().toQPointF());

        for (int j=0; j<R2->size()-1; ++j) {
            QLineF S2(R2->getNode(j)->position().toQPointF(), R2->getNode(j+1)->position().toQPointF());
            QPointF intPoint;
            if (S1.intersect(S2, &intPoint) == QLineF::BoundedIntersection) {
                numInter++;
                if (doIt) {
                    Node* pt = new Node(Coord::fromQPointF(intPoint));
                    theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(R1->layer()),pt,true));
                    theList->add(new WayAddNodeCommand(R1,pt,i+1,theDocument->getDirtyOrOriginLayer(R1->layer())));
                    theList->add(new WayAddNodeCommand(R2,pt,j+1,theDocument->getDirtyOrOriginLayer(R2->layer())));
                }
                ++i; ++j;
            }
        }
    }

    return numInter;
}

int Way::segmentCount()
{
    return p->Nodes.size()-1;
}

QLineF Way::getSegment(int i)
{
    return QLineF(p->Nodes[i]->position().toQPointF(), p->Nodes[i+1]->position().toQPointF());
}

int Way::bestSegment()
{
    return p->BestSegment;
}
