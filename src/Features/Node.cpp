#include "Node.h"

#include "MapView.h"
#include "Utils/LineF.h"
#include "Global.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QProgressDialog>

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

class NodePrivate
{
    public:
        NodePrivate()
        : IsWaypoint(false)
        , ProjectionRevision(0)
        , HasPhoto(false)
        , Photo(0)
        , photoLocationBR(true)
        {
        }

        bool IsWaypoint;
        bool IsPOI;
#ifndef _MOBILE
        int ProjectionRevision;
        QPointF Projected;
#endif
        bool HasPhoto;
        QPixmap* Photo;
        bool photoLocationBR;
};

Node::Node(const Coord& aCoord)
    : Elevation(0.0)
    , Speed(0.0)
    , p(new NodePrivate)
{
    BBox = CoordBox(aCoord, aCoord);
    setRenderPriority(RenderPriority(RenderPriority::IsSingular,0., 0));
}

Node::Node(const Node& other)
    : Feature(other)
    , Elevation(other.Elevation)
    , Speed(other.Speed)
    , p(new NodePrivate)
{
    BBox = other.BBox;
    p->Projected = other.p->Projected;
    p->ProjectionRevision = other.projectionRevision();
    setRenderPriority(RenderPriority(RenderPriority::IsSingular,0., 0));
}

Node::~Node(void)
{
    SAFE_DELETE(p->Photo)
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
    return BBox.isNull();
}

bool Node::isInteresting() const
{
    // does its id look like one from osm
    if (hasOSMId())
        return true;
    // if the user has added special tags, that's fine also
    if (tagSize())
        return true;
    // if it is part of a road, then too
    if (sizeParents())
        return true;

    return false;
}

bool Node::isPOI()
{
    if (!MetaUpToDate)
        updateMeta();

    return p->IsPOI;
}

bool Node::isWaypoint()
{
    if (!MetaUpToDate)
        updateMeta();

    return p->IsWaypoint;
}

bool Node::isSelectable(MapView* theView)
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


Coord Node::position() const
{
    return BBox.topLeft();
}

void Node::setPosition(const Coord& aCoord)
{
    BBox = CoordBox(aCoord, aCoord);
    p->ProjectionRevision = 0;
    g_backend.sync(this);

    notifyChanges();
}

const QPointF& Node::projection() const
{
    return p->Projected;
}

void Node::setProjection(const QPointF& aProjection)
{
    p->Projected = aProjection;
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
    if (p->Photo)
        return *(p->Photo);
    else
        return QPixmap();
}
void Node::setPhoto(QPixmap thePhoto)
{
    SAFE_DELETE(p->Photo)
    p->Photo = new QPixmap(thePhoto.scaled(M_PREFS->getMaxGeoPicWidth(), M_PREFS->getMaxGeoPicWidth(), Qt::KeepAspectRatio));
    p->HasPhoto = true;
}

bool Node::notEverythingDownloaded()
{
    return lastUpdated() == Feature::NotYetDownloaded;
}

const CoordBox& Node::boundingBox(bool) const
{
    return BBox;
}

void Node::draw(QPainter& thePainter , MapView* theView)
{
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
                 double phRt = 1. * p->Photo->width() / p->Photo->height();
                 phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
             }
             thePainter.drawPixmap(phPt, p->Photo->scaledToWidth(M_PREFS->getMaxGeoPicWidth()*rt));
         }
     }
#endif
    if (isDirty() && isUploadable() && M_PREFS->getDirtyVisible()) {
        QPoint P = theView->toView(this);
        double theWidth = theView->nodeWidth();
        if (theWidth >= 1) {
            QRect R(P-QPoint(theWidth/2,theWidth/2),QSize(theWidth,theWidth));
            thePainter.fillRect(R,M_PREFS->getDirtyColor());
        }
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
            double phRt = 1. * p->Photo->width() / p->Photo->height();
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
            double phRt = 1. * p->Photo->width() / p->Photo->height();
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
        return QString("%1 (%2)").arg(s).arg(id().numId);
    return
        QString("%1").arg(id().numId);
}

void Node::partChanged(Feature*, int)
{
}

void Node::updateMeta()
{
    Feature::updateMeta();
    MetaUpToDate = true;

    p->IsWaypoint = (findKey("_waypoint_") != -1);
    p->IsPOI = false;
    for (int i=0; i<tagSize(); ++i) {
        if (!M_PREFS->getTechnicalTags().contains(tagKey(i))) {
            p->IsPOI = true;
            break;
        }
    }

    if (!p->IsPOI && !p->IsWaypoint) {
        int i=0;
        int prtReadonly=0, prtWritable=0;
        for (; i<sizeParents(); ++i) {
            if (getParent(i)->isReadonly())
                ++prtReadonly;
            else
                ++prtWritable;
        }
        if (!isReadonly()) {
            if (prtReadonly && !prtWritable)
                setReadonly(true);
        }
    }
}

bool Node::toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict, QString changesetid)
{
    bool OK = true;

    if (isVirtual())
        return OK;

    stream.writeStartElement("node");

    Feature::toXML(stream, strict, changesetid);
    stream.writeAttribute("lon",COORD2STRING(BBox.topRight().x()));
    stream.writeAttribute("lat", COORD2STRING(BBox.topRight().y()));

    tagsToXML(stream, strict);
    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

bool Node::toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, QString element, bool forExport)
{
    bool OK = true;

    if (isVirtual())
        return OK;

    if (!tagValue("_waypoint_","").isEmpty() ||!sizeParents())
        stream.writeStartElement("wpt");
    else
        stream.writeStartElement(element);

    if (!forExport)
        stream.writeAttribute("xml:id", xmlId());
    stream.writeAttribute("lon",COORD2STRING(BBox.topRight().x()));
    stream.writeAttribute("lat", COORD2STRING(BBox.topRight().y()));

    stream.writeTextElement("time", time().toString(Qt::ISODate)+"Z");

    QString s = tagValue("name","");
    if (!s.isEmpty()) {
        stream.writeTextElement("name", s);
    }
    if (elevation()) {
        stream.writeTextElement("ele", QString::number(elevation(),'f',6));
    }
    s = tagValue("_comment_","");
    if (!s.isEmpty()) {
        stream.writeTextElement("cmt", s);
    }
    s = tagValue("_description_","");
    if (!s.isEmpty()) {
        stream.writeTextElement("desc", s);
    }

    // OpenStreetBug
    s = tagValue("_special_","");
    if (!s.isEmpty() && id().type & IFeature::Special) {
        stream.writeStartElement("extensions");
        QString sid = stripToOSMId(id());
        stream.writeTextElement("id", sid);
        stream.writeEndElement();
    }

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

Node * Node::fromXML(Document* d, Layer* L, QXmlStreamReader& stream)
{
    double Lat = stream.attributes().value("lat").toString().toDouble();
    double Lon = stream.attributes().value("lon").toString().toDouble();

    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id(IFeature::Point, sid.toLongLong());
    Node* Pt = CAST_NODE(d->getFeature(id));
    if (!Pt) {
        Pt = g_backend.allocNode(L, Coord(Lon,Lat));
        Pt->setId(id);
        L->add(Pt);
        Feature::fromXML(stream, Pt);
    } else {
        Feature::fromXML(stream, Pt);
        if (Pt->layer() != L) {
            Pt->layer()->remove(Pt);
            L->add(Pt);
        }
        Pt->setPosition(Coord(Lon, Lat));
    }

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "tag") {
            Pt->setTag(stream.attributes().value("k").toString(), stream.attributes().value("v").toString());
            stream.readNext();
        }

       stream.readNext();
    }

    return Pt;
}

Node * Node::fromGPX(Document* d, Layer* L, QXmlStreamReader& stream)
{
    double Lat = stream.attributes().value("lat").toString().toDouble();
    double Lon = stream.attributes().value("lon").toString().toDouble();

    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id(IFeature::Point, sid.toLongLong());
    Node* Pt = CAST_NODE(d->getFeature(id));
    if (!Pt) {
        Pt = g_backend.allocNode(L, Coord(Lon,Lat));
        Pt->setId(id);
        Pt->setLastUpdated(Feature::Log);
        L->add(Pt);
    } else {
        Pt->setPosition(Coord(Lon,Lat));
        if (Pt->lastUpdated() == Feature::NotYetDownloaded)
            Pt->setLastUpdated(Feature::OSMServer);
    }

    if (stream.name() == "wpt")
        Pt->setTag("_waypoint_", "yes");

    QDateTime time = QDateTime::currentDateTime();
    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "time") {
            stream.readNext();
            QString dtm = stream.text().toString();
            time = QDateTime::fromString(dtm.left(19), Qt::ISODate);
            stream.readNext();
        } else if (stream.name() == "ele") {
            stream.readNext();
            Pt->setElevation(stream.text().toString().toFloat());
            stream.readNext();
        } else if (stream.name() == "name") {
            stream.readNext();
            Pt->setTag("name", stream.text().toString());
            stream.readNext();
        } else if (stream.name() == "cmt") {
            stream.readNext();
            Pt->setTag("_comment_", stream.text().toString());
            stream.readNext();
        } else if (stream.name() == "desc") {
            stream.readNext();
            Pt->setTag("_description_", stream.text().toString());
            stream.readNext();
        }
        else if (stream.name() == "cmt")
        {
            stream.readNext();
            Pt->setTag("_comment_", stream.text().toString());
            stream.readNext();
        }
        else if (stream.name() == "extensions") // for OpenStreetBugs
        {
            QString str = stream.readElementText(QXmlStreamReader::IncludeChildElements);
            QDomDocument doc;
            doc.setContent(str);

            QDomNodeList li = doc.elementsByTagName("id");
            if (li.size()) {
                QString id = li.at(0).toElement().text();
                Pt->setId(IFeature::FId(IFeature::Point | IFeature::Special, id.toLongLong()));
                Pt->setTag("_special_", "yes"); // Assumed to be OpenstreetBugs as they don't use their own namesoace
                Pt->setSpecial(true);
            }
        }

        stream.readNext();
    }
    Pt->setTime(time);

    return Pt;
}

QString Node::toHtml()
{
    QString D;
    int i;


    if ((i = findKey("_waypoint_")) != -1)
        D += "<p><b>"+QApplication::translate("MapFeature", "Waypoint")+"</b><br/>";
    D += "<i>"+QApplication::translate("MapFeature", "coord")+": </i>" + COORD2STRING(position().y()) + " (" + Coord2Sexa(position().y()) + ") / " + COORD2STRING(position().x()) + " (" + Coord2Sexa(position().x()) + ")";

    if (elevation())
        D += "<br/><i>"+QApplication::translate("MapFeature", "elevation")+": </i>" + QString::number(elevation(), 'f', 4);
    if (speed())
        D += "<br/><i>"+QApplication::translate("MapFeature", "speed")+": </i>" + QString::number(speed(), 'f', 4);
    if ((i = findKey("_description_")) != -1)
        D += "<br/><i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i);
    if ((i = findKey("_comment_")) != -1)
        D += "<br/><i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i);

    return Feature::toMainHtml(QApplication::translate("MapFeature", "Node"), "node").arg(D);
}
