#include "Node.h"

#include "MapView.h"
#include "Utils/LineF.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QProgressDialog>

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

class NodePrivate
{
    public:
        NodePrivate()
        : IsWaypoint(false), ProjectionRevision(0)
        , HasPhoto(false)
        , photoLocationBR(true)
        {
        }

        bool IsWaypoint;
#ifndef _MOBILE
        int ProjectionRevision;
#endif
        bool HasPhoto;
        QPixmap Photo;
        bool photoLocationBR;
};

Node::Node(const Coord& aCoord)
    : Position(aCoord)
    , Elevation(0.0)
    , Speed(0.0)
    , p(new NodePrivate)
{
    BBox = CoordBox(Position,Position);
    setRenderPriority(RenderPriority(RenderPriority::IsSingular,0., 0));
}

Node::Node(const Node& other)
    : Feature(other)
    , Position(other.Position)
    , Elevation(other.Elevation)
    , Speed(other.Speed)
    , Projected(other.Projected)
    , p(new NodePrivate)
{
    setTime(other.time());
    BBox = other.boundingBox();
    p->ProjectionRevision = other.projectionRevision();
    setRenderPriority(RenderPriority(RenderPriority::IsSingular,0., 0));
}

Node::~Node(void)
{
    delete p;
}

void Node::remove(int )
{
}

void Node::remove(Feature*)
{
}

int Node::size() const
{
    return 0;
}

int Node::find(Feature* ) const
{
    return 0;
}

Feature* Node::get(int )
{
    return NULL;
}

const Feature* Node::get(int ) const
{
    return NULL;
}

bool Node::isNull() const
{
    return Position.isNull();
}

bool Node::isInteresting() const
{
    // does its id look like one from osm
    if (id().left(5) == "node_")
        return true;
    // if the user has added special tags, that's fine also
    for (int i=0; i<tagSize(); ++i)
        if ((tagKey(i) != "created_by") && (tagKey(i) != "ele"))
            return true;
    // if it is part of a road, then too
    if (sizeParents())
        return true;

    return false;
}

bool Node::isPOI() const
{
    // if the user has added special tags, that's fine also
    for (int i=0; i<tagSize(); ++i)
        if ((tagKey(i) != "created_by") && (tagKey(i) != "ele"))
            return true;

    return false;
}

bool Node::isWaypoint()
{
    if (!MetaUpToDate)
        updateMeta();

    return p->IsWaypoint;
}

bool Node::isSelectable(MapView* theView) const
{
    // If Node has non-default tags -> POI -> always selectable
    if (isPOI())
        return true;

    bool Draw = false;
    if (TEST_RFLAGS(RendererOptions::NodesVisible) || (lastUpdated() == Feature::Log && !TEST_RFLAGS(RendererOptions::TrackSegmentVisible))) {
        Draw = (theView->nodeWidth() >= 1);
        // Do not draw GPX nodes when simple GPX track appearance is enabled
        if (M_PREFS->getSimpleGpxTrack() && layer()->isTrack())
            Draw = false;
        if (!Draw) {
            if (!sizeParents())
                Draw = true;
            else if (lastUpdated() == Feature::Log && !TEST_RFLAGS(RendererOptions::TrackSegmentVisible))
                Draw = true;
        }
    }
    return Draw;
}


const Coord& Node::position() const
{
    return Position;
}

void Node::setPosition(const Coord& aCoord)
{
    if (layer())
        layer()->indexRemove(BBox, this);
    Position = aCoord;
    BBox = CoordBox(Position,Position);
    p->ProjectionRevision = 0;
    if (layer() && !isDeleted() && isVisible()) {
        layer()->indexAdd(BBox, this);
    }
    notifyChanges();
}

const QPointF& Node::projection() const
{
    return Projected;
}

void Node::setProjection(const QPointF& aProjection)
{
    Projected = aProjection;
}

#ifndef _MOBILE
int Node::projectionRevision() const
{
    return p->ProjectionRevision;
}

void Node::setProjectionRevision(const int aProjectionRevision)
{
    p->ProjectionRevision = aProjectionRevision;
}
#endif

double Node::speed() const
{
    return Speed;
}

void Node::setSpeed(double aSpeed)
{
    Speed = aSpeed;
}

double Node::elevation() const
{
    return Elevation;
}

void Node::setElevation(double aElevation)
{
    Elevation = aElevation;
}

bool Node::hasPhoto() const
{
    return p->HasPhoto;
}

QPixmap Node::photo() const
{
    return p->Photo;
}
void Node::setPhoto(QPixmap thePhoto)
{
    p->Photo = thePhoto.scaled(M_PREFS->getMaxGeoPicWidth(), M_PREFS->getMaxGeoPicWidth(), Qt::KeepAspectRatio);
    p->HasPhoto = true;
}

bool Node::notEverythingDownloaded()
{
    return lastUpdated() == Feature::NotYetDownloaded;
}

CoordBox Node::boundingBox() const
{
    return BBox;
}

void Node::draw(QPainter& thePainter , MapView* theView)
{
    Q_UNUSED(thePainter)
    Q_UNUSED(theView)
#ifdef GEOIMAGE
    if (p->HasPhoto) {
         QPoint me = theView->toView(this);
         thePainter.setPen(QPen(QColor(0, 0, 0), 2));
         QRect box(me - QPoint(5, 3), QSize(10, 6));
         thePainter.drawRect(box);
         if (TEST_RFLAGS(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
             qreal rt = qBound(0.2, theView->pixelPerM(), 1.0);
             QPoint phPt;

             if (p->photoLocationBR) {
                 phPt = me + QPoint(10*rt, 10*rt);
             } else {
                 qreal rt = qBound(0.2, theView->pixelPerM(), 1.0);
                 double phRt = 1. * p->Photo.width() / p->Photo.height();
                 phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
             }
             thePainter.drawPixmap(phPt, p->Photo.scaledToWidth(M_PREFS->getMaxGeoPicWidth()*rt));
         }
     }
#endif
    if (isDirty() && M_PREFS->getDirtyVisible()) {
        QPointF P(theView->toView(this));
        QRectF R(P-QPointF(2,2),QSize(4,4));
        thePainter.fillRect(R,M_PREFS->getDirtyColor());
    }

}

#ifdef GEOIMAGE
void Node::drawHover(QPainter& thePainter, MapView* theView)
{
    /* call the parent function */
    Feature::drawHover(thePainter, theView);

    /* and then the image */
    if (p->HasPhoto) {
        if (TEST_RFLAGS(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
            QPoint me(theView->toView(this));

            qreal rt = qBound(0.2, theView->pixelPerM(), 1.0);
            double phRt = 1. * p->Photo.width() / p->Photo.height();
            QPoint phPt;
            if (p->photoLocationBR) {
                phPt = me + QPoint(10*rt, 10*rt);
            } else {
                phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
            }
            QRect box(phPt, QSize(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt));
            thePainter.drawRect(box);
        }
    }
}
#endif

void Node::drawSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{

    QPen TP(Pen);
    TP.setWidth(TP.width() / 2);

    QPoint me(theView->toView(this));
    QRect R(me-QPoint(3,3),QSize(6,6));

    thePainter.setPen(TP);
    thePainter.drawRect(R);
    R.adjust(-7, -7, 7, 7);
    thePainter.drawEllipse(R);
}

void Node::drawParentsSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    for (int i=0; i<sizeParents(); ++i) {
        if (!getParent(i)->isDeleted()) {
            Feature* f = CAST_FEATURE(getParent(i));
            if (f)
                f->drawSpecial(thePainter, Pen, theView);
        }
    }
}

void Node::drawChildrenSpecial(QPainter& thePainter, QPen& Pen, MapView* theView, int depth)
{
    Q_UNUSED(thePainter);
    Q_UNUSED(Pen);
    Q_UNUSED(theView);
    Q_UNUSED(depth);
    // Node has no children
}


double Node::pixelDistance(const QPointF& Target, double, bool, MapView* theView) const
{
    double Best = 1000000;

    QPoint me = theView->toView(const_cast<Node*>(this));

    Best = distance(Target, me);

#ifdef GEOIMAGE
    if (p->HasPhoto) {
        if (TEST_RFLAGS(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
            qreal rt = qBound(0.2, theView->pixelPerM(), 1.0);
            double phRt = 1. * p->Photo.width() / p->Photo.height();
            QPoint phPt;
            if (p->photoLocationBR) {
                phPt = me + QPoint(10*rt, 10*rt);
            } else {
                phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
            }
            QRect box(phPt, QSize(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt));
            if (box.contains(Target.toPoint())) {
                p->photoLocationBR = !p->photoLocationBR;
                theView->invalidate(true, false);
            }
        }
    }
#endif

    return Best;
}

void Node::cascadedRemoveIfUsing(Document*, Feature*, CommandList*, const QList<Feature*>&)
{
}

QString Node::description() const
{
    QString s(tagValue("name",""));
    if (!s.isEmpty())
        return QString("%1 (%2)").arg(s).arg(id());
    return
        QString("%1").arg(id());
}

void Node::partChanged(Feature*, int)
{
}

void Node::updateMeta()
{
    Feature::updateMeta();

    p->IsWaypoint = (findKey("_waypoint_") != tagSize());

    MetaUpToDate = true;
}

bool Node::toXML(QDomElement xParent, QProgressDialog * progress, bool strict)
{
    bool OK = true;

    if (isVirtual())
        return OK;

    QDomElement e = xParent.ownerDocument().createElement("node");
    xParent.appendChild(e);

    Feature::toXML(e, strict);
    e.setAttribute("lon",COORD2STRING(coordToAng(Position.lon())));
    e.setAttribute("lat", COORD2STRING(coordToAng(Position.lat())));

    tagsToXML(e, strict);

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

bool Node::toGPX(QDomElement xParent, QProgressDialog * progress, bool forExport)
{
    bool OK = true;

    if (isVirtual())
        return OK;

    QDomElement e;
    if (!tagValue("_waypoint_","").isEmpty() ||!sizeParents())
        e = xParent.ownerDocument().createElement("wpt");
    else
        if (xParent.tagName() == "trkseg")
            e = xParent.ownerDocument().createElement("trkpt");
        else
            if (xParent.tagName() == "rte")
                e = xParent.ownerDocument().createElement("rtept");
    xParent.appendChild(e);

    if (!forExport)
        e.setAttribute("xml:id", xmlId());
    e.setAttribute("lon",COORD2STRING(coordToAng(Position.lon())));
    e.setAttribute("lat", COORD2STRING(coordToAng(Position.lat())));

    QDomElement c = xParent.ownerDocument().createElement("time");
    e.appendChild(c);
    QDomText v = c.ownerDocument().createTextNode(time().toString(Qt::ISODate)+"Z");
    c.appendChild(v);

    QString s = tagValue("name","");
    if (!s.isEmpty()) {
        QDomElement c = xParent.ownerDocument().createElement("name");
        e.appendChild(c);
        QDomText v = c.ownerDocument().createTextNode(s);
        c.appendChild(v);
    }
    if (elevation()) {
        QDomElement c = xParent.ownerDocument().createElement("ele");
        e.appendChild(c);
        QDomText v = c.ownerDocument().createTextNode(QString::number(elevation(),'f',6));
        c.appendChild(v);
    }
    s = tagValue("_comment_","");
    if (!s.isEmpty()) {
        QDomElement c = xParent.ownerDocument().createElement("cmt");
        e.appendChild(c);
        QDomText v = c.ownerDocument().createTextNode(s);
        c.appendChild(v);
    }
    s = tagValue("_description_","");
    if (!s.isEmpty()) {
        QDomElement c = xParent.ownerDocument().createElement("desc");
        e.appendChild(c);
        QDomText v = c.ownerDocument().createTextNode(s);
        c.appendChild(v);
    }

    // OpenStreetBug
    s = tagValue("_special_","");
    if (!s.isEmpty() && id().startsWith("osb_")) {
        QDomElement c = xParent.ownerDocument().createElement("extensions");
        e.appendChild(c);
        QDomElement osbId = xParent.ownerDocument().createElement("id");
        c.appendChild(osbId);
        QString sid = stripToOSMId(id());
        QDomText v = c.ownerDocument().createTextNode(sid);
        osbId.appendChild(v);
    }

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

Node * Node::fromXML(Document* d, Layer* L, const QDomElement e)
{
    double Lat = e.attribute("lat").toDouble();
    double Lon = e.attribute("lon").toDouble();

    QString id = (e.hasAttribute("id") ? e.attribute("id") : e.attribute("xml:id"));
    if (!id.startsWith('{') && !id.startsWith('-'))
        id = "node_" + id;

    Node* Pt = dynamic_cast<Node*>(d->getFeature(id));
    if (!Pt) {
        Pt = new Node(Coord(angToCoord(Lat),angToCoord(Lon)));
        Feature::fromXML(e, Pt);
        L->add(Pt);
    } else {
        Feature::fromXML(e, Pt);
        if (Pt->layer() != L) {
            Pt->layer()->remove(Pt);
            L->add(Pt);
        }
        Pt->setPosition(Coord(angToCoord(Lat), angToCoord(Lon)));
        if (Pt->lastUpdated() == Feature::NotYetDownloaded)
            Pt->setLastUpdated(A);
    }

    Feature::tagsFromXML(d, Pt, e);

    return Pt;
}

Node * Node::fromGPX(Document* d, Layer* L, const QDomElement e)
{
    double Lat = e.attribute("lat").toDouble();
    double Lon = e.attribute("lon").toDouble();

    QString id = (e.hasAttribute("id") ? e.attribute("id") : e.attribute("xml:id"));
    if (!id.startsWith('{') && !id.startsWith('-'))
        id = "node_" + id;

    Node* Pt = dynamic_cast<Node*>(d->getFeature(id));
    if (!Pt) {
        Pt = new Node(Coord(angToCoord(Lat),angToCoord(Lon)));
        Pt->setId(id);
        Pt->setLastUpdated(Feature::Log);
        L->add(Pt);
    } else {
        Pt->setPosition(Coord(angToCoord(Lat), angToCoord(Lon)));
        if (Pt->lastUpdated() == Feature::NotYetDownloaded)
            Pt->setLastUpdated(Feature::OSMServer);
    }

    if (e.tagName() == "wpt")
        Pt->setTag("_waypoint_", "yes");

    QDateTime time = QDateTime::currentDateTime();
    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "time") {
            QString dtm = c.text();
            time = QDateTime::fromString(dtm.left(19), Qt::ISODate);
        } else
        if (c.tagName() == "ele") {
            Pt->setElevation(c.text().toFloat());
        } else
        if (c.tagName() == "name") {
            Pt->setTag("name", c.text(), false);
        } else
        if (c.tagName() == "cmt") {
            Pt->setTag("_comment_", c.text(), false);
        } else
        if (c.tagName() == "desc") {
            Pt->setTag("_description_", c.text(), false);
        }
        else if (c.tagName() == "cmt")
        {
            Pt->setTag("_comment_", c.text(), false);
        }
        else if (c.tagName() == "extensions") // for OpenStreetBugs
        {
            QDomNodeList li = c.elementsByTagName("id");
            if (li.size()) {
                QString id = li.at(0).toElement().text();
                Pt->setId("osb_" + id);
                Pt->setTag("_special_", "yes"); // Assumed to be OpenstreetBugs as they don't use their own namesoace
                Pt->setSpecial(true);
            }
        }

        c = c.nextSiblingElement();
    }
    Pt->setTime(time);

    return Pt;
}

QString Node::toHtml()
{
    QString D;
    int i;


    if ((i = findKey("_waypoint_")) < tagSize())
        D += "<p><b>"+QApplication::translate("MapFeature", "Waypoint")+"</b><br/>";
    D += "<i>"+QApplication::translate("MapFeature", "coord")+": </i>" + COORD2STRING(coordToAng(position().lat())) + " (" + Coord2Sexa(position().lat()) + ") / " + COORD2STRING(coordToAng(position().lon())) + " (" + Coord2Sexa(position().lon()) + ")";

    if (elevation())
        D += "<br/><i>"+QApplication::translate("MapFeature", "elevation")+": </i>" + QString::number(elevation(), 'f', 4);
    if (speed())
        D += "<br/><i>"+QApplication::translate("MapFeature", "speed")+": </i>" + QString::number(speed(), 'f', 4);
    if ((i = findKey("_description_")) < tagSize())
        D += "<br/><i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i);
    if ((i = findKey("_comment_")) < tagSize())
        D += "<br/><i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i);

    return Feature::toMainHtml(QApplication::translate("MapFeature", "Node"), "node").arg(D);
}

void Node::toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex)
{
    Q_UNUSED(theIndex);

    theIndex["N" + QString::number(idToLong())] = ds.device()->pos();
    ds << (qint8)'N' << idToLong() << (qint32)(Position.lon()) << (qint32)(Position.lat());
}

Node* Node::fromBinary(Document* d, OsbLayer* L, QDataStream& ds, qint8 c, qint64 id)
{
    Q_UNUSED(c);

    qint32	lon;
    qint32	lat;
    QString strId;

    ds >> lon;
    ds >> lat;

    if (!L)
        return NULL;

    Coord cd( lat, lon );
    if (id < 1)
        strId = QString::number(id);
    else
        strId = QString("node_%1").arg(QString::number(id));

    Node* Pt = CAST_NODE(d->getFeature(strId));
    if (!Pt) {
        Pt = new Node(cd);
        Pt->setId(strId);
        Pt->setLastUpdated(Feature::OSMServer);
        L->add(Pt);
    } else {
        L->remove(Pt);
        Pt->setPosition(cd);
        L->add(Pt);
        if (Pt->lastUpdated() == Feature::NotYetDownloaded)
            Pt->setLastUpdated(Feature::OSMServer);
    }

    return Pt;
}

