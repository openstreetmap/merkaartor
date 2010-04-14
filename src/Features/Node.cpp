#include "Node.h"

#include "MapView.h"
#include "Utils/LineF.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QProgressDialog>

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

#ifdef GEOIMAGE
void Node::draw(QPainter& thePainter , MapView* theView )
{
    if (p->HasPhoto) {
        QPoint me = theView->toView(this);
        thePainter.setPen(QPen(QColor(0, 0, 0), 2));
        QRect box(me - QPoint(5, 3), QSize(10, 6));
        thePainter.drawRect(box);

        if (M_PREFS->getPhotosVisible() && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
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
}
#else
void Node::draw(QPainter& /* thePainter */, MapView* /*theView*/ )
{
}
#endif

void Node::drawFocus(QPainter& thePainter, MapView* theView, bool solid)
{
    thePainter.setPen(MerkaartorPreferences::instance()->getFocusColor());
    QPoint me(theView->toView(this));
    QRect R(me-QPoint(3,3),QSize(6,6));
    thePainter.drawRect(R);
    R.adjust(-7, -7, 7, 7);
    thePainter.drawEllipse(R);

    if (M_PREFS->getShowParents() && solid) {
        for (int i=0; i<sizeParents(); ++i)
            if (!getParent(i)->isDeleted())
                getParent(i)->drawFocus(thePainter, theView, false);
    }
}

void Node::drawHover(QPainter& thePainter, MapView* theView, bool solid)
{
    thePainter.setPen(MerkaartorPreferences::instance()->getHoverColor());
    QPoint me(theView->toView(this));
    QRect R(me-QPoint(3,3),QSize(6,6));
    thePainter.drawRect(R);
    R.adjust(-7, -7, 7, 7);
    thePainter.drawEllipse(R);

#ifdef GEOIMAGE
    if (p->HasPhoto) {
        if (M_PREFS->getPhotosVisible() && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
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
#endif

    if (M_PREFS->getShowParents() && solid) {
        for (int i=0; i<sizeParents(); ++i)
            if (!getParent(i)->isDeleted())
                getParent(i)->drawHover(thePainter, theView, false);
    }
}

void Node::drawHighlight(QPainter& thePainter, MapView* theView, bool /*solid*/)
{
    thePainter.setPen(MerkaartorPreferences::instance()->getHighlightColor());
    QPoint me(theView->toView(this));
    QRect R(me-QPoint(3,3),QSize(6,6));
    thePainter.drawRect(R);
    R.adjust(-7, -7, 7, 7);
    thePainter.drawEllipse(R);

//	if (M_PREFS->getShowParents() && solid) {
//		for (int i=0; i<sizeParents(); ++i)
//			if (!getParent(i)->isDeleted())
//				getParent(i)->drawHover(thePainter, theView, false);
//	}
}

double Node::pixelDistance(const QPointF& Target, double, bool, MapView* theView) const
{
    double Best = 1000000;

    QPoint me = theView->toView(const_cast<Node*>(this));

    Best = distance(Target, me);
#ifdef GEOIMAGE
    if (p->HasPhoto) {
        if (M_PREFS->getPhotosVisible() && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
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
    p->IsWaypoint = (findKey("_waypoint_") != tagSize());
    MetaUpToDate = true;
}

QString Node::toXML(int lvl, QProgressDialog * progress)
{
    if (progress)
        progress->setValue(progress->value()+1);

    if (isVirtual())
        return QString();

    QString S(lvl*2, ' ');
    S += "<node id=\"%1\" lat=\"%2\" lon=\"%3\">\n";
    S += tagsToXML(lvl+1);
    S += QString(lvl*2, ' ') + "</node>\n";
    return S.arg(stripToOSMId(id())).arg(intToAng(position().lat()),0,'f',8).arg(intToAng(position().lon()),0,'f',8);
}

bool Node::toXML(QDomElement xParent, QProgressDialog & progress, bool strict)
{
    bool OK = true;
    progress.setValue(progress.value()+1);

    if (isVirtual())
        return OK;

    QDomElement e = xParent.ownerDocument().createElement("node");
    xParent.appendChild(e);

    e.setAttribute("id", xmlId());
    e.setAttribute("lon",QString::number(intToAng(Position.lon()),'f',8));
    e.setAttribute("lat", QString::number(intToAng(Position.lat()),'f',8));
    e.setAttribute("timestamp", time().toString(Qt::ISODate)+"Z");
    e.setAttribute("version", versionNumber());
    e.setAttribute("user", user());
    if (!strict) {
        e.setAttribute("actor", (int)lastUpdated());
        if (isDeleted())
            e.setAttribute("deleted","true");
    }

    tagsToXML(e);

    return OK;
}

bool Node::toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport)
{
    bool OK = true;
    progress.setValue(progress.value()+1);

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
    e.setAttribute("lon",QString::number(intToAng(Position.lon()),'f',8));
    e.setAttribute("lat", QString::number(intToAng(Position.lat()),'f',8));

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

    return OK;
}

Node * Node::fromXML(Document* d, Layer* L, const QDomElement e)
{
    double Lat = e.attribute("lat").toDouble();
    double Lon = e.attribute("lon").toDouble();
    bool Deleted = (e.attribute("deleted") == "true");

    QDateTime time;
    time = QDateTime::fromString(e.attribute("timestamp").left(19), Qt::ISODate);
    QString user = e.attribute("user");
    int Version = e.attribute("version").toInt();
    Feature::ActorType A = (Feature::ActorType)(e.attribute("actor", "2").toInt());

    QString id = (e.hasAttribute("id") ? e.attribute("id") : e.attribute("xml:id"));
    if (!id.startsWith('{') && !id.startsWith('-'))
        id = "node_" + id;
    Node* Pt = dynamic_cast<Node*>(d->getFeature(id));
    if (!Pt) {
        Pt = new Node(Coord(angToInt(Lat),angToInt(Lon)));
        Pt->setId(id);
        Pt->setLastUpdated(A);
        L->add(Pt);
    } else {
        if (Pt->layer() != L) {
            Pt->layer()->remove(Pt);
            L->add(Pt);
        }
        Pt->setPosition(Coord(angToInt(Lat), angToInt(Lon)));
        if (Pt->lastUpdated() == Feature::NotYetDownloaded)
            Pt->setLastUpdated(A);
    }
    Pt->setDeleted(Deleted);
    Pt->setTime(time);
    Pt->setUser(user);
    Pt->setVersionNumber(Version);

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
        Pt = new Node(Coord(angToInt(Lat),angToInt(Lon)));
        Pt->setId(id);
        Pt->setLastUpdated(Feature::Log);
        L->add(Pt);
    } else {
        Pt->setPosition(Coord(angToInt(Lat), angToInt(Lon)));
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

        c = c.nextSiblingElement();
    }
    Pt->setTime(time);

    return Pt;
}

QString Node::toHtml()
{
    QString D;
    int i;


    D += "<i>"+QApplication::translate("MapFeature", "timestamp")+": </i>" + time().toString(Qt::ISODate) + "<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "coord")+": </i>" + QString::number(intToAng(position().lat()), 'f', 4) + " / " + QString::number(intToAng(position().lon()), 'f', 4) + "<br/>";

    if (elevation())
        D += "<i>"+QApplication::translate("MapFeature", "elevation")+": </i>" + QString::number(elevation(), 'f', 4) + "<br/>";
    if (speed())
        D += "<i>"+QApplication::translate("MapFeature", "speed")+": </i>" + QString::number(speed(), 'f', 4) + "<br/>";
    if ((i = findKey("_description_")) < tagSize())
        D += "<i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i) + "<br/>";
    if ((i = findKey("_comment_")) < tagSize())
        D += "<i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i) + "<br/>";

    if ((i = findKey("_waypoint_")) < tagSize()) {
        D += "<p><b>"+QApplication::translate("MapFeature", "Waypoint")+"</b><br/>";

        if ((i = findKey("_description_")) < tagSize())
            D += "<i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i) + "<br/>";

        if ((i = findKey("_comment_")) < tagSize())
            D += "<i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i) + "<br/>";
    }

    D += "<i>"+QApplication::translate("MapFeature", "layer")+": </i>";
    if (layer())
        D += layer()->name();
    D += "<br/>";

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
