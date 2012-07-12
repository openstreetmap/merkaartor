#include "Relation.h"

#include "Global.h"
#include "Features.h"
#include "MapView.h"
#include "MapRenderer.h"
#include "DocumentCommands.h"
#include "RelationCommands.h"
#include "Document.h"
#include "LineF.h"
#include "SvgCache.h"

#include <QApplication>
#include <QAbstractTableModel>
#include <QProgressDialog>
#include <QPainter>

#include <algorithm>
#include <utility>
#include <QList>

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

class RelationMemberModel : public QAbstractTableModel
{
public:
    RelationMemberModel(RelationPrivate* aParent);
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);

    RelationPrivate* Parent;
};

class RelationPrivate
{
public:
    RelationPrivate(Relation* R)
        : theRelation(R), theModel(0), ModelReferences(0)
        , PathUpToDate(false)
        , ProjectionRevision(0)
        , TurnRestrictionTypeIndex(-1)
        , IsTurnRestrictionOnly(false)
        , BBoxUpToDate(false)
        , Width(0)
    {
    }
    ~RelationPrivate()
    {
        delete theModel;
    }
    void CalculateWidth();

    Relation* theRelation;
    QList<QPair<QString, MapFeaturePtr> > Members;
    RelationMemberModel* theModel;
    int ModelReferences;
    QPainterPath thePath;
    QPainterPath theBoundingPath;
    bool PathUpToDate;
    int ProjectionRevision;
    int TurnRestrictionTypeIndex;
    bool IsTurnRestrictionOnly;

    bool BBoxUpToDate;

    RenderPriority theRenderPriority;

    qreal Width;
};

#define DEFAULTWIDTH 6
#define LANEWIDTH 4

void RelationPrivate::CalculateWidth()
{
    QString s(theRelation->tagValue("width",QString()));
    if (!s.isNull()) {
        Width = s.toDouble();
        return;
    }
    QString h = theRelation->tagValue("highway",QString());
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


Relation::Relation()
    : Feature()
{
    p = new RelationPrivate(this);
}

Relation::Relation(const Relation& other)
    : Feature(other)
{
    p = new RelationPrivate(this);
}

Relation::~Relation()
{
    // TODO Those cleanup shouldn't be necessary and lead to crashes
    //      Check for side effect of supressing them.
    //	for (int i=0; i<p->Members.size(); ++i)
    //		if (p->Members[i].second)
    //			p->Members[i].second->unsetParentFeature(this);
    delete p;
}

void Relation::setLayer(Layer* L)
{
    if (L) {
        for (int i=0; i<p->Members.size(); ++i)
            if (p->Members[i].second)
                p->Members[i].second->setParentFeature(this);
    } else {
        for (int i=0; i<p->Members.size(); ++i)
            if (p->Members[i].second)
                p->Members[i].second->unsetParentFeature(this);
    }
    Feature::setLayer(L);
}

void Relation::partChanged(Feature*, int ChangeId)
{
    if (isDeleted())
        return;

    p->PathUpToDate = false;
    p->BBoxUpToDate = false;
    MetaUpToDate = false;
    g_backend.sync(this);

    notifyParents(ChangeId);
}

QString Relation::description() const
{
    QString s(tagValue("name",""));
    if (!s.isEmpty())
        return QString("%1 (%2)").arg(s).arg(id().numId);
    return QString("%1").arg(id().numId);
}

const CoordBox& Relation::boundingBox(bool update) const
{
    if (!p->BBoxUpToDate && update)
    {
        if (p->Members.size() == 0)
            BBox = CoordBox(Coord(0,0),Coord(0,0));
        else
        {
            CoordBox Clip;
            bool haveFirst = false;
            for (int i=0; i<p->Members.size(); ++i) {
                if (p->Members[i].second && !p->Members[i].second->boundingBox().isNull()/* && !CAST_RELATION(p->Members[i].second)*/) {
                    if (!haveFirst) {
                        Clip = p->Members[i].second->boundingBox();
                        haveFirst = true;
                    } else
                        Clip.merge(p->Members[i].second->boundingBox());
                }
            }
            BBox = Clip;
            p->BBoxUpToDate = true;
        }
    }
    return BBox;
}

void Relation::drawSimple(QPainter &P, MapView *theView)
{
    Q_UNUSED(P)
    Q_UNUSED(theView)
}

static const QColor onlyTurnColor(0, 85, 255, 127);
static const QColor noTurnColor(255, 32, 0, 127);
static const QColor errorTurnColor(255, 0, 0, 0);

void Relation::drawTouchup(QPainter& P, MapView* theView)
{
    P.save();

    if (M_PREFS->getViewTurnRestrictions() && p->TurnRestrictionTypeIndex >= 0) {
        int FromViaNode = -1;
        int ToViaNode = -1;
        Way* ToWay;
        qreal FromAng = 0;
        QPointF FromToF;
        QList<Node*> viaNodes;

        for (int i=0; i<p->Members.size(); ++i) {
            if (p->Members[i].first == "via") {
                if (Node* N = CAST_NODE(p->Members[i].second))
                    viaNodes << N;
                else if (Way* R = CAST_WAY(p->Members[i].second)) {
                    for (int j=0; j<R->size(); ++j)
                        viaNodes << R->getNode(j);
                }
            }
        }
        for (int i=0; i<p->Members.size(); ++i) {
            Way* R = CAST_WAY(p->Members[i].second);
            if (R && R->size() < 2)
                continue;
            if (R && (p->Members[i].first == "from" || (p->Members[i].first == "to"))) {
                qreal WW = theView->pixelPerM()*3+2;
                QColor c;
                if (p->IsTurnRestrictionOnly)
                    c = onlyTurnColor;
                else
                    c = noTurnColor;

                QPen thePen(QBrush(c),WW);
                thePen.setCapStyle(Qt::RoundCap);
                thePen.setJoinStyle(Qt::RoundJoin);
                P.setPen(thePen);
                P.setBrush(Qt::NoBrush);
                P.drawPath(theView->transform().map(p->Members[i].second->getPath()));

                if (p->Members[i].first == "from") {
                    foreach (Node* N, viaNodes) {
                        if (R->getNode(0) == N ) {
                            FromViaNode = 0;
                            break;
                        } else if (R->getNode(R->size()-1) == N) {
                            FromViaNode = R->size()-1;
                            break;
                        }
                    }
                    if (FromViaNode != -1) {

                        int j, jmin1;
                        if (FromViaNode == 0) {
                            j = 0;
                            jmin1 = 1;
                        } else {
                            j = R->size()-1;
                            jmin1 = R->size()-2;
                        }
                        QPointF FromF(theView->transform().map(R->getNode(jmin1)->projected()));
                        FromToF = QPointF(theView->transform().map(R->getNode(j)->projected()));
                        FromAng = angle(FromF-FromToF);
                    }
                } else if (p->Members[i].first == "to") {
                    ToWay = R;
                    qreal WW = theView->pixelPerM()*5+2;
                    foreach (Node* N, viaNodes) {
                        if (R->getNode(0) == N ) {
                            ToViaNode = 0;
                            break;
                        } else if (R->getNode(R->size()-1) == N) {
                            ToViaNode = R->size()-1;
                            break;
                        }
                    }
                    if (ToViaNode != -1) {

                        int j, jmin1;
                        if (ToViaNode == 0) {
                            j = R->size()-1;
                            jmin1 = R->size()-2;
                        } else {
                            j = 0;
                            jmin1 = 1;
                        }
                        QPointF FromF(theView->transform().map(R->getNode(jmin1)->projected()));
                        QPointF ToF(theView->transform().map(R->getNode(j)->projected()));
                        QPoint H(ToF.toPoint());
                        qreal ToAng = angle(FromF-ToF);
                        if (p->IsTurnRestrictionOnly) {
                            QPoint V1(qRound(WW*cos(ToAng+M_PI/4)),qRound(WW*sin(ToAng+M_PI/4)));
                            QPoint V2(qRound(WW*cos(ToAng-M_PI/4)),qRound(WW*sin(ToAng-M_PI/4)));
                            QPoint T(V1.x()+(V2.x()-V1.x())/2, V1.y()+(V2.y()-V1.y())/2);

                            const QPoint points[3] = {
                                H-T,
                                H-T+V1,
                                H-T+V2
                            };

                            P.setPen(Qt::NoPen);
                            P.setBrush(c);
                            P.drawPolygon(points, 3);
                        } else {
                            P.save();
                            P.setPen(Qt::NoPen);
                            P.setBrush(c);
                            P.translate(ToF);
                            P.rotate(radToAng(ToAng-M_PI/2));
                            P.drawRect(-WW*0.75, -WW/2, WW*1.5, WW/2);
                            P.restore();
                        }
                    }
                }
            } else if (p->Members[i].first == "location_hint") {
            }
        }
        if (FromViaNode != -1) {
            if (ToViaNode != -1) {

                int j, jmin1;
                if (ToViaNode == 0) {
                    j = 1;
                    jmin1 = 0;
                } else {
                    j = ToWay->size()-2;
                    jmin1 = ToWay->size()-1;
                }
                QPointF FromF(theView->transform().map(ToWay->getNode(jmin1)->projected()));
                QPointF ToF(theView->transform().map(ToWay->getNode(j)->projected()));
                qreal ToAng = angle(FromF-ToF);

                qreal WW = theView->pixelPerM()*3+2;
                QImage* pm = getSVGImageFromFile(":/TurnRestrictions/" + tagValue(p->TurnRestrictionTypeIndex) + ".png",int(WW));

                if (pm && !pm->isNull()) {
                    qreal A = ToAng - ((ToAng-FromAng)/2);
                    qDebug() << radToAng( ToAng) << radToAng(FromAng) << radToAng(A);
                    P.save();
                    P.translate(FromToF);
//                    P.rotate(radToAng(A-M_PI/2));
                    P.rotate(radToAng(FromAng-M_PI/2));
                    P.translate(QPoint(WW, 0));
                    P.drawImage( int(-pm->width()/2), int(-pm->height()/2) , *pm);
                    P.restore();
                }
            }
        }
    }

    if (theView->renderOptions().options.testFlag(RendererOptions::RelationsVisible)) {
        P.setBrush(Qt::NoBrush);
        if (notEverythingDownloaded())
            P.setPen(QPen(Qt::red,M_PREFS->getRelationsWidth(),Qt::DashLine));
        else {
            if (isDirty() && isUploadable() && M_PREFS->getDirtyVisible())
                P.setPen(QPen(M_PREFS->getDirtyColor(),M_PREFS->getDirtyWidth()));
            else
                P.setPen(QPen(M_PREFS->getRelationsColor(),M_PREFS->getRelationsWidth(),Qt::DashLine));
        }
        P.drawPath(theView->transform().map(p->theBoundingPath));
    }
    P.restore();
}

void Relation::drawSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    QPen TP(Pen);

    /* draw relation itself now solid
    if (Pen.style() == Qt::SolidLine)
    {
        TP.setStyle(Qt::DashLine);
    } */

    thePainter.setPen(TP);
    buildPath(theView->projection());
    thePainter.drawPath(theView->transform().map(p->theBoundingPath));
}

void Relation::drawParentsSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    for (int i=0; i<sizeParents(); ++i) {
        if (!getParent(i)->isDeleted()) {
            Feature* f = CAST_FEATURE(getParent(i));
            if (f)
                f->drawSpecial(thePainter, Pen, theView);
        }
    }
}

void Relation::drawChildrenSpecial(QPainter& thePainter, QPen& Pen, MapView* theView, int depth)
{
    QPen TP(Pen);
    TP.setStyle(Qt::DashLine);
    for (int i=0; i<p->Members.size(); ++i)
        if (p->Members[i].second && !p->Members[i].second->isDeleted())
            if (p->Members[i].second->boundingBox().intersects(theView->viewport()))
            {
                p->Members[i].second->drawSpecial(thePainter, TP, theView);
                if (--depth > 0)
                    p->Members[i].second->drawChildrenSpecial(thePainter, TP, theView, depth);
            }
}


qreal Relation::pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const
{
    Q_UNUSED(NoSnap)

    qreal Best = 1000000;
    if (!TEST_RFLAGS(RendererOptions::RelationsVisible) && !M_PREFS->getRelationsSelectableWhenHidden())
        return Best;

    //for (int i=0; i<p->Members.size(); ++i)
    //{
    //	if (p->Members[i].second) {
    //		qreal Dist = p->Members[i].second->pixelDistance(Target, ClearEndDistance, theProjection);
    //		if (Dist < Best)
    //			Best = Dist;
    //	}
    //}


    qreal D;
    //QRectF bb = QRectF(theView->toView(boundingBox().bottomLeft()),theView->toView(boundingBox().topRight()));
    //bb.adjust(-10, -10, 10, 10);

    LineF F(theView->toView(boundingBox().topLeft()), theView->toView(boundingBox().topRight()));
    D = F.capDistance(Target);
    if ((D < ClearEndDistance) && (D<Best)) Best = D;
    F = LineF(theView->toView(boundingBox().topLeft()), theView->toView(boundingBox().bottomLeft()));
    D = F.capDistance(Target);
    if ((D < ClearEndDistance) && (D<Best)) Best = D;
    F = LineF(theView->toView(boundingBox().bottomRight()), theView->toView(boundingBox().bottomLeft()));
    D = F.capDistance(Target);
    if ((D < ClearEndDistance) && (D<Best)) Best = D;
    F = LineF(theView->toView(boundingBox().bottomRight()), theView->toView(boundingBox().topRight()));
    D = F.capDistance(Target);
    if ((D < ClearEndDistance) && (D<Best)) Best = D;

    return Best + 0.1; // Make sure we select simple elements first
}

void Relation::cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives)
{
    for (int i=0; i<p->Members.size();) {
        if (p->Members[i].second && p->Members[i].second == aFeature)
        {
            QString Role = p->Members[i].first;
            theList->add(new RelationRemoveFeatureCommand(this, i, theDocument->getDirtyOrOriginLayer(layer())));
            for (int j=0; j<Alternatives.size(); ++j)
                if (i+j >= p->Members.size() || p->Members[i+j].second != Alternatives[j]) {
                    if ((i+j) == 0)
                        theList->add(new RelationAddFeatureCommand(this, Role, Alternatives[j], 0, theDocument->getDirtyOrOriginLayer(Alternatives[j]->layer())));
                    else if (p->Members[i+j-1].second != Alternatives[j])
                        theList->add(new RelationAddFeatureCommand(this, Role, Alternatives[j], i+j, theDocument->getDirtyOrOriginLayer(Alternatives[j]->layer())));
                }
            continue;
        }
        ++i;
    }
    if (p->Members.size() == 0) {
        if (!isDeleted()) {
            QList<Feature*> alt;
            theList->add(new RemoveFeatureCommand(theDocument,this,alt));
        }
    }
}

bool Relation::notEverythingDownloaded()
{
    if (lastUpdated() == Feature::NotYetDownloaded)
        return true;
    for (int i=0; i<p->Members.size(); ++i)
        if (p->Members.at(i).second && !CAST_RELATION(p->Members[i].second))
            if (p->Members.at(i).second->notEverythingDownloaded())
                return true;
    return false;
}


void Relation::add(const QString& Role, Feature* F)
{
    p->Members.push_back(qMakePair(Role,F));
    F->setParentFeature(this);
    p->PathUpToDate = false;
    p->BBoxUpToDate = false;
    MetaUpToDate = false;
    g_backend.sync(this);

    notifyChanges();
}

void Relation::add(const QString& Role, Feature* F, int Idx)
{
    p->Members.push_back(qMakePair(Role,F));
    std::rotate(p->Members.begin()+Idx,p->Members.end()-1,p->Members.end());
    F->setParentFeature(this);
    p->PathUpToDate = false;
    p->BBoxUpToDate = false;
    MetaUpToDate = false;
    g_backend.sync(this);

    notifyChanges();
}

void Relation::remove(int Idx)
{
    Feature* F = p->Members[Idx].second;
    // only remove as parent if the feature is only a member once
    p->Members.erase(p->Members.begin()+Idx);
    if (F && find(F) == p->Members.size())
        F->unsetParentFeature(this);
    p->PathUpToDate = false;
    p->BBoxUpToDate = false;
    MetaUpToDate = false;
    g_backend.sync(this);

    notifyChanges();
}

void Relation::remove(Feature* F)
{
    for (int i=p->Members.size(); i; --i)
        if (F == p->Members[i-1].second)
            remove(i-1);
}

int Relation::size() const
{
    return p->Members.size();
}

int Relation::find(Feature* Pt) const
{
    for (int i=0; i<p->Members.size(); ++i)
        if (Pt == p->Members[i].second)
            return i;
    return p->Members.size();
}

Feature* Relation::get(int idx)
{
    return p->Members[idx].second;
}

const Feature* Relation::get(int idx) const
{
    return p->Members[idx].second;
}

bool Relation::isNull() const
{
    return (p->Members.size() == 0);
}

const QString& Relation::getRole(int idx) const
{
    return p->Members[idx].first;
}

QAbstractTableModel* Relation::referenceMemberModel()
{
    ++p->ModelReferences;
    if (!p->theModel)
        p->theModel = new RelationMemberModel(p);
    return p->theModel;
}

void Relation::releaseMemberModel()
{
    --p->ModelReferences;
    if (p->ModelReferences == 0)
    {
        delete p->theModel;
        p->theModel = 0;
    }
}

void Relation::buildPath(Projection const &theProjection)
{
    //    QPainterPath clipPath;
    //    clipPath.addRect(cr);

    QMutexLocker mutlock(&featMutex);
    p->theBoundingPath = QPainterPath();

    if (!p->Members.size())
        return;

    QPolygonF theVector;
    theVector.append(theProjection.project(boundingBox().bottomLeft()));
    theVector.append(theProjection.project(boundingBox().topLeft()));
    theVector.append(theProjection.project(boundingBox().topRight()));
    theVector.append(theProjection.project(boundingBox().bottomRight()));
    theVector.append(theProjection.project(boundingBox().bottomLeft()));

    //QRectF bb = QPolygonF(theVector).boundingRect();
    //p->theBoundingPath.addRect(bb);

    p->theBoundingPath.addPolygon(theVector);
    //    p->theBoundingPath = p->theBoundingPath.intersected(clipPath);

    if (!p->PathUpToDate || p->ProjectionRevision != theProjection.projectionRevision()) {
        p->thePath = QPainterPath();

        Way* outerWay = NULL;
        int numOuter = 0;
        bool isMultipolygon = false;
        if (tagValue("type", "") == "multipolygon")
            isMultipolygon = true;


        // Handle polygons made of scattered ways
        QList< QPair<QString,QPainterPath> > memberPaths;
        for (int i=0; i<size(); ++i) {
            if (CHECK_WAY(p->Members[i].second)) {
                Way* M = STATIC_CAST_WAY(p->Members[i].second);
                M->buildPath(theProjection);
                if (M->getPath().elementCount() > 1) {
                    memberPaths << qMakePair(p->Members[i].first, M->getPath());
                    if (isMultipolygon && (p->Members[i].first == "outer" || p->Members[i].first.isEmpty())) {
                        if (!numOuter)
                            outerWay = M;
                        else
                            outerWay = NULL;
                        ++numOuter;
                    }
                }
            }
        }

        QList<QPainterPath> innerPaths;
        QList<QPainterPath> outerPaths;

        while (memberPaths.size()) {
            // handle the start...
            QPointF curPoint;
            QPainterPath curPath;
            QString curRole;

            curRole = memberPaths[0].first;
            curPath.moveTo(memberPaths[0].second.elementAt(0));
            for (int j=1; j<memberPaths[0].second.elementCount(); ++j) {
                curPoint = memberPaths[0].second.elementAt(j);
                curPath.lineTo(curPoint);
            }
            // ... and remove it
            memberPaths.removeAt(0);
            // Check if any remaining path starts or ends at the current point
            for (int k=0; k<memberPaths.size(); ++k) {
                if (memberPaths[k].second.elementAt(0) == curPoint && memberPaths[k].first == curRole) { // Check start
                    for (int l=1; l<memberPaths[k].second.elementCount(); ++l) {
                        curPoint = memberPaths[k].second.elementAt(l);
                        curPath.lineTo(curPoint);
                    }
                    memberPaths.removeAt(k);
                    k=0;
                } else if (memberPaths[k].second.elementAt(memberPaths[k].second.elementCount()-1) == curPoint  && memberPaths[k].first == curRole) { // Check end
                    for (int l=memberPaths[k].second.elementCount()-2; l>=0; --l) {
                        curPoint = memberPaths[k].second.elementAt(l);
                        curPath.lineTo(curPoint);
                    }
                    memberPaths.removeAt(k);
                    k=0;
                }
            }
            if (curRole == "inner" && isMultipolygon)
                innerPaths << curPath;
            else
                outerPaths << curPath;
        }

        if (outerWay && tagSize() == 1) {
            outerWay->rebuildPath(theProjection);
            for (int i=0; i<innerPaths.size(); ++i) {
                outerWay->addPathHole(innerPaths[i]);
            }
        } else {
            for (int i=0; i<outerPaths.size(); ++i) {
                p->thePath.addPath(outerPaths[i]);
            }
            for (int i=0; i<innerPaths.size(); ++i) {
                p->thePath = p->thePath.subtracted(innerPaths[i]);
            }
        }

        p->ProjectionRevision = theProjection.projectionRevision();
        p->PathUpToDate = true;
    }
}

const QPainterPath& Relation::getPath() const
{
    return p->thePath;
}

const RenderPriority& Relation::renderPriority()
{
    if (!MetaUpToDate)
        updateMeta();
    return p->theRenderPriority;
}

void Relation::updateMeta()
{
    QMutexLocker mutlock(&featMutex);
    if (MetaUpToDate)
        return;

    Feature::updateMeta();

    p->PathUpToDate = false;
    p->CalculateWidth();

    p->theRenderPriority = RenderPriority(RenderPriority::IsSingular, 0., 0);
    for (int i=0; i<p->Members.size(); ++i) {
        if (Way* W = CAST_WAY(p->Members.at(i).second)) {
            if (W->renderPriority() < p->theRenderPriority)
                p->theRenderPriority = W->renderPriority();
        } else if (Relation* R = CAST_RELATION(p->Members.at(i).second)) {
            if (R->renderPriority() < p->theRenderPriority)
                p->theRenderPriority = R->renderPriority();
        }
    }

    p->TurnRestrictionTypeIndex = -1;
    p->IsTurnRestrictionOnly = false;
    for (int i=0; i<tagSize(); ++i) {
        if (tagKey(i).startsWith("restriction"))
            p->TurnRestrictionTypeIndex = i;
        if (tagValue(i).startsWith("only"))
            p->IsTurnRestrictionOnly = true;
    }

    MetaUpToDate = true;
}

bool Relation::toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict, QString changetsetid)
{
    bool OK = true;

    stream.writeStartElement("relation");
    Feature::toXML(stream, strict, changetsetid);

    // Has to be first to be picked up when reading back
    if (!strict)
        boundingBox().toXML("BoundingBox", stream);

    for (int i=0; i<size(); ++i) {
        QString Type("node");
        if (CHECK_WAY(get(i)))
            Type="way";
        else if (CHECK_RELATION(get(i)))
            Type="relation";

        stream.writeStartElement("member");
        stream.writeAttribute("type", Type);
        stream.writeAttribute("ref", get(i)->xmlId());
        stream.writeAttribute("role", getRole(i));
        stream.writeEndElement();
    }

    tagsToXML(stream, strict);

    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);
    return OK;
}

Relation * Relation::fromXML(Document * d, Layer * L, QXmlStreamReader& stream)
{
    bool hasBbox = false;

    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id;
    id.type = IFeature::OsmRelation;
    id.numId = sid.toLongLong();

    Relation* R = CAST_RELATION(d->getFeature(id));
    if (!R) {
        R = g_backend.allocRelation(L);
        R->setId(id);
        L->add(R);
        Feature::fromXML(stream, R);
    } else {
        Feature::fromXML(stream, R);
        if (R->layer() != L) {
            R->layer()->remove(R);
            L->add(R);
        }
        while (R->p->Members.size())
            R->remove(0);
    }

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "member") {
            QString Type = stream.attributes().value("type").toString();
            QString sId = stream.attributes().value("ref").toString();
            QString role = stream.attributes().value("role").toString();
            Feature* F = 0;
            if (Type == "node") {
                IFeature::FId nId(IFeature::Point, sId.toLongLong());
                Node* Part = CAST_NODE(d->getFeature(nId));
                if (!Part)
                {
                    Part = g_backend.allocNode(L, Coord(0,0));
                    Part->setId(nId);
                    Part->setLastUpdated(Feature::NotYetDownloaded);
                    L->add(Part);
                }
                F = Part;
            } else if (Type == "way") {
                IFeature::FId rId(IFeature::LineString, sId.toLongLong());
                Way* Part = CAST_WAY(d->getFeature(rId));
                if (!Part)
                {
                    Part = g_backend.allocWay(L);
                    Part->setId(rId);
                    Part->setLastUpdated(Feature::NotYetDownloaded);
                    L->add(Part);
                }
                F = Part;
            } else if (Type == "relation") {
                IFeature::FId RId(IFeature::OsmRelation, sId.toLongLong());
                Relation* Part = dynamic_cast<Relation*>(d->getFeature(RId));
                if (!Part)
                {
                    Part = g_backend.allocRelation(L);
                    Part->setId(RId);
                    Part->setLastUpdated(Feature::NotYetDownloaded);
                    L->add(Part);
                }
                F = Part;
            }
            if (F) {
                if (!hasBbox) {
                    R->add(role, F);
                } else {
                    R->p->Members.push_back(qMakePair(role,F));
                    F->setParentFeature(R);
                }
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
            qDebug() << "Relation: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }
        stream.readNext();
    }

    if (hasBbox && !R->isDeleted())
        g_backend.indexAdd(L, R->BBox, R);
    return R;
}

QString Relation::toHtml()
{
    QString D;

    D += "<i>"+QApplication::translate("MapFeature", "size")+": </i>" + QString::number(size()) + " " + QApplication::translate("MapFeature", "members");
    CoordBox bb = boundingBox();
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + COORD2STRING(bb.topLeft().y()) + " / " + COORD2STRING(bb.topLeft().x());
    D += "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + COORD2STRING(bb.bottomRight().y()) + " / " + COORD2STRING(bb.bottomRight().x());

    return Feature::toMainHtml(QApplication::translate("MapFeature", "Relation"),"relation").arg(D);
}

qreal Relation::widthOf()
{
    if (MetaUpToDate == false)
        updateMeta();

    return p->Width;
}

/* RELATIONMODEL */

RelationMemberModel::RelationMemberModel(RelationPrivate *aParent)
    : Parent(aParent)
{
}

int RelationMemberModel::rowCount(const QModelIndex &) const
{
    return Parent->Members.size();
}

int RelationMemberModel::columnCount(const QModelIndex &) const
{
    return 2;
}

QVariant RelationMemberModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (index.row() >= (int)Parent->Members.size())
        return QVariant();
    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
            return Parent->Members[index.row()].first;
        else
            return Parent->Members[index.row()].second->description();
    }
    else if (role == Qt::EditRole)
    {
        if ( (index.column() == 0) )
            return Parent->Members[index.row()].first;
        else
            return Parent->Members[index.row()].second->description();
    }
    else if (role == Qt::UserRole)
    {
        QVariant v;
        v.setValue((Feature *)(Parent->Members[index.row()].second));
        return v;
    }
    return QVariant();
}

QVariant RelationMemberModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
        return QVariant();
    if (orientation == Qt::Horizontal)
    {
        if (section == 0)
            return QApplication::translate("MapFeature", "Role");
        else
            return QApplication::translate("MapFeature", "Member");
    }
    return QVariant();
}

Qt::ItemFlags RelationMemberModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::ItemIsEnabled;
    return QAbstractTableModel::flags(index) | Qt::ItemIsEditable  | Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool RelationMemberModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (index.isValid() && role == Qt::EditRole)
    {
        QString Role;
        Feature* Member;
        if (index.column() == 0) {
            Member = Parent->Members[index.row()].second;
            Role = value.toString();
        } else {
            Member = (Feature *) value.value<void *>();
            if (Member == NULL)
                return false;
            Role = Parent->Members[index.row()].first;
        }
        CommandList* L = new CommandList(MainWindow::tr("Relation Modified %1").arg(Parent->theRelation->id().numId), Parent->theRelation);
        L->add(new RelationRemoveFeatureCommand(Parent->theRelation, index.row(), CUR_DOCUMENT->getDirtyOrOriginLayer(Parent->theRelation->layer())));
        L->add(new RelationAddFeatureCommand(Parent->theRelation,Role,Member,index.row(), CUR_DOCUMENT->getDirtyOrOriginLayer(Parent->theRelation->layer())));
        CUR_DOCUMENT->addHistory(L);
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

