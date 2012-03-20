#include "Global.h"

#include "Way.h"
#include "Node.h"

#include "DocumentCommands.h"
#include "WayCommands.h"
#include "Coord.h"
#include "Painting.h"
#include "MapView.h"
#include "LineF.h"
#include "MDiscardableDialog.h"
#include "Utils.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QProgressDialog>

#include <algorithm>
#include <QList>

#if QT_VERSION < 0x040700 && !defined(_MOBILE)
#include <ggl/ggl.hpp>
#include <ggl/geometries/cartesian2d.hpp>
#include <ggl/geometries/adapted/std_as_linestring.hpp>
#include <ggl/algorithms/intersects.hpp>
#endif

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

class WayPrivate
{
    public:
        WayPrivate(Way* aWay)
        : theWay(aWay), BBoxUpToDate(false)
            , Area(0), Distance(0)
            , PathUpToDate(false), VirtualsUptodate(false)
            , ProjectionRevision(0)
            , BestSegment(-1)
            , Width(0)
        {
        }
        Way* theWay;

        QList<Node*> Nodes;
        QList<Node*> virtualNodes;

        bool BBoxUpToDate;

        qreal Area;
        qreal Distance;
        bool NotEverythingDownloaded;
        bool PathUpToDate;
        bool VirtualsUptodate;
        QPainterPath thePath;
        int ProjectionRevision;
        int BestSegment;
        qreal Width;

        RenderPriority theRenderPriority; // 10 (24)

        void CalculateWidth();
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

void WayPrivate::removeVirtuals()
{
    while (virtualNodes.size()) {
        virtualNodes[0]->unsetParentFeature(theWay);
        g_backend.deallocVirtualNode(virtualNodes[0]);
//        if (virtualNodes[0]->layer())
//            virtualNodes[0]->layer()->deleteFeature(virtualNodes[0]);
//		delete p->virtualNodes[0];
        virtualNodes.erase(virtualNodes.begin());
    }
}

void WayPrivate::addVirtuals()
{
    for (unsigned int i=1; i<Nodes.size(); ++i) {
        QLineF l(Nodes[i-1]->position(), Nodes[i]->position());
        l.setLength(l.length()/2);
        Node* v = g_backend.allocVirtualNode(l.p2());
        v->setVirtual(true);
        v->setParentFeature(theWay);
//        theWay->layer()->add(v);
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
    : Feature()
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
    p->PathUpToDate = false;
    MetaUpToDate = false;
    p->VirtualsUptodate = false;
    g_backend.sync(this);

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
    p->Nodes.insert(p->Nodes.begin() + Idx, Pt);
//	p->Nodes.push_back(Pt);
//	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
    Pt->setParentFeature(this);
    g_backend.sync(Pt);
    p->BBoxUpToDate = false;
    p->PathUpToDate = false;
    MetaUpToDate = false;
    p->VirtualsUptodate = false;
    g_backend.sync(this);

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
    Node* Pt = p->Nodes[idx];
    // only remove as parent if the node is only included once
    p->Nodes.erase(p->Nodes.begin()+idx);
    if (Pt && (find(Pt) >= size()))
        Pt->unsetParentFeature(this);
    g_backend.sync(Pt);
    p->BBoxUpToDate = false;
    p->PathUpToDate = false;
    MetaUpToDate = false;
    p->VirtualsUptodate = false;
    g_backend.sync(this);

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

const QList<NodePtr>& Way::getNodes() const
{
    return p->Nodes;
}

const QList<NodePtr>& Way::getVirtuals() const
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
            BBox = CoordBox(p->Nodes[0]->position(),p->Nodes[0]->position());
            for (unsigned int i=1; i<p->Nodes.size(); ++i)
                BBox.merge(p->Nodes[i]->position());
        }
        else
            BBox = CoordBox(Coord(0,0),Coord(0,0));
        p->BBoxUpToDate = true;
    }
    return BBox;
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
    if (tagValue("highway", "") == "" || tagValue("area", "") != "")
        isArea = (p->Nodes[0] == p->Nodes[p->Nodes.size()-1]);

    for (unsigned int i=0; (i+1)<p->Nodes.size(); ++i)
    {
        if (p->Nodes[i] && p->Nodes[i+1]) {
            const Coord & here = p->Nodes[i]->position();
            const Coord & next = p->Nodes[i+1]->position();

            p->Distance += next.distanceFrom(here);
        }
    }

    if (isArea) {
        p->Area = p->Distance;
        p->theRenderPriority = RenderPriority(RenderPriority::IsArea,-fabs(p->Area), 0);
    } else {
        qreal Priority = tagValue("layer","0").toInt();
        if (Priority >= 0)
            Priority++;
        int layer = Priority;
        // dummy number to get a deterministic feature sort
//		Priority += sin(intToRad(boundingBox().lonDiff()));
        Priority += p->Distance / INT_MAX;
        p->theRenderPriority = RenderPriority(RenderPriority::IsLinear,Priority, layer);
    }

    p->doUpdateVirtuals();
}

qreal Way::distance()
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

qreal Way::area()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->Area;
}

qreal Way::widthOf()
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

    qreal theWidth = theView->nodeWidth();
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


qreal Way::pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const
{
    qreal Best = 1000000;
    p->BestSegment = -1;

//    if (selectNodes) {
//        for (unsigned int i=0; i<p->Nodes.size(); ++i)
//        {
//            if (p->Nodes.at(i)) {
//                qreal x = ::distance(Target, theView->toView(p->Nodes.at(i)));
//                if (x<ClearEndDistance)
//                    return Best;
//            }
//        }
//    }
    for (unsigned int i=0; i<p->Nodes.size()-1; ++i)
    {
        if (NoSnap.contains(p->Nodes.at(i)) || NoSnap.contains(p->Nodes.at(i+1)))
            continue;

        if (p->Nodes.at(i) && p->Nodes.at(i+1)) {
            LineF F(theView->toView(p->Nodes.at(i)),theView->toView(p->Nodes.at(i+1)));
            qreal D = F.capDistance(Target);
            if (D < ClearEndDistance && D < Best) {
                Best = D;
                if (g_Merk_Segment_Mode)
                    p->BestSegment = i;
            }
        }
    }
    return Best;
}

Node* Way::pixelDistanceNode(const QPointF& Target, qreal ClearEndDistance, MapView* theView, const QList<Feature*>& NoSnap, bool NoSelectVirtuals) const
{
    qreal Best = 1000000;
    Node* ret = NULL;

    for (unsigned int i=0; i<p->Nodes.size(); ++i)
    {
        if (p->Nodes.at(i) && !NoSnap.contains(p->Nodes.at(i))) {
            qreal D = ::distance(Target,theView->toView(p->Nodes.at(i)));
            if (D < ClearEndDistance && D < Best) {
                Best = D;
                ret = p->Nodes.at(i);
            }
        }
    }
    if (!NoSelectVirtuals && M_PREFS->getVirtualNodesVisible()) {
        for (unsigned int i=0; i<p->virtualNodes.size(); ++i)
        {
            if (p->virtualNodes.at(i)) {
                qreal D = ::distance(Target,theView->toView(p->virtualNodes.at(i)));
                if (D < ClearEndDistance && D < Best) {
                    Best = D;
                    return p->virtualNodes.at(i);
                }
            }
        }
    }
    return ret;
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
                if (p->Nodes[i+j] != Alternatives[j])
                {
                    if ((i+j) == 0)
                        theList->add(new WayAddNodeCommand(this, Alternatives[j], 0,theDocument->getDirtyOrOriginLayer(layer())));
                    else if (p->Nodes[i+j-1] != Alternatives[j] && (i+j+1 >= p->Nodes.size() || p->Nodes[i+j+1] != Alternatives[j]))
                        theList->add(new WayAddNodeCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(layer())));
                }
            }
            theList->add(new WayRemoveNodeCommand(this, (Node*)aFeature,theDocument->getDirtyOrOriginLayer(layer())));
        }
        ++i;
    }
    if (p->Nodes.size() == 1) {
        if (!isDeleted()) {
            QList<Feature*> alt;
            theList->add(new WayRemoveNodeCommand(this, p->Nodes[0], theDocument->getDirtyOrOriginLayer(layer())));
            theList->add(new RemoveFeatureCommand(theDocument,this,alt));
        }
    }
}

const QPainterPath& Way::getPath() const
{
    return p->thePath;
}

void Way::addPathHole(const QPainterPath& pth)
{
    if (!p->PathUpToDate)
        return;

    p->thePath = p->thePath.subtracted(pth);
}

void Way::rebuildPath(const Projection &theProjection)
{
    p->PathUpToDate = false;
    buildPath(theProjection);
}

void Way::buildPath(const Projection &theProjection)
{
    if (p->PathUpToDate && p->ProjectionRevision == theProjection.projectionRevision())
        return;
    else {
        p->thePath = QPainterPath();
        if (p->Nodes.size() < 2) {
            p->PathUpToDate = true;
            return;
        }

        p->thePath.moveTo(theProjection.project(p->Nodes.at(0)));
        for (unsigned int i=1; i<p->Nodes.size(); ++i) {
            p->thePath.lineTo(theProjection.project(p->Nodes.at(i)));
        }
        p->ProjectionRevision = theProjection.projectionRevision();
        p->PathUpToDate = true;
    }
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

    // Has to be first to be picked up when reading back
    if (!strict)
        boundingBox().toXML("BoundingBox", stream);

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
    bool hasBbox = false;

    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id(IFeature::LineString, sid.toLongLong());
    Way* R = CAST_WAY(d->getFeature(id));

    if (!R) {
        R = g_backend.allocWay(L);
        R->setId(id);
        L->add(R);
        Feature::fromXML(stream, R);
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
                Part = g_backend.allocNode(L, Coord(0,0));
                Part->setId(nId);
                Part->setLastUpdated(Feature::NotYetDownloaded);
                L->add(Part);
            }
            if (!hasBbox) {
                R->add(Part);
            } else {
                R->p->Nodes.push_back(Part);
                Part->setParentFeature(R);
            }
            stream.readNext();
        } else if (stream.name() == "tag") {
            R->setTag(stream.attributes().value("k").toString(), stream.attributes().value("v").toString());
            stream.readNext();
        } else if (stream.name() == "BoundingBox") {
            R->BBox = CoordBox::fromXML(stream);
            R->p->BBoxUpToDate = true;
            hasBbox = true;
            stream.readNext();
        } else if (!stream.isWhitespace()) {
            qDebug() << "Way: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }

       stream.readNext();
    }

    if (hasBbox && !R->isDeleted())
        g_backend.indexAdd(L, R->BBox, R);
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
    LineF L(R->getNode(0)->position(),R->getNode(1)->position());
    int BestIdx = 1;
    qreal BestDistance = L.capDistance(P);
    for (int i = 2; i<R->size(); ++i)
    {
        LineF L(R->getNode(i-1)->position(),R->getNode(i)->position());
        qreal Distance = L.capDistance(P);
        if (Distance < BestDistance)
        {
            BestIdx = i;
            BestDistance = Distance;
        }
    }
    LineF F(R->getNode(BestIdx-1)->position(),R->getNode(BestIdx)->position());
    P = F.project(P);
    return BestIdx;
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
    D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + COORD2STRING(bb.topLeft().y()) + " / " + COORD2STRING(bb.topLeft().x());
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + COORD2STRING(bb.bottomRight().y()) + " / " + COORD2STRING(bb.bottomRight().x());

    QString type = isClosed() ? QApplication::translate("MapFeature", "Area") : QApplication::translate("MapFeature", "Way");

    return Feature::toMainHtml(type, "way").arg(D);
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
        QLineF S1(R1->getNode(i)->position(), R1->getNode(i+1)->position());

        for (int j=0; j<R2->size()-1; ++j) {
            QLineF S2(R2->getNode(j)->position(), R2->getNode(j+1)->position());
            QPointF intPoint;
            if (S1.intersect(S2, &intPoint) == QLineF::BoundedIntersection) {
                numInter++;
                if (doIt) {
                    Node* pt = g_backend.allocNode(theDocument->getDirtyOrOriginLayer(R1->layer()), intPoint);
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
    return QLineF(p->Nodes[i]->position(), p->Nodes[i+1]->position());
}

int Way::bestSegment()
{
    return p->BestSegment;
}

const RenderPriority& Way::renderPriority()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->theRenderPriority;
}

