#include "Node.h"

#include "MapView.h"
#include "MapRenderer.h"
#include "LineF.h"
#include "Global.h"

#include <QApplication>
#include <QtGui/QPainter>
#include <QProgressDialog>

#define TEST_RFLAGS(x) theView->renderOptions().options.testFlag(x)

Node::Node(const Coord& aCoord)
    : Feature()
    , ProjectionRevision(0)
{
    BBox = CoordBox(aCoord, aCoord);
//    qDebug() << "Node size:" << sizeof(Node) << sizeof(PhotoNode);

}

Node::Node(const Node& other)
    : Feature(other)
{
    BBox = other.BBox;
    Projected = other.Projected;
}

Node::~Node(void)
{
}

const QPointF& Node::projected() const
{
    return Projected;
}

const QPointF& Node::projected(const Projection& aProjection)
{
    buildPath(aProjection);
    return Projected;
}

void Node::buildPath(const Projection& aProjection)
{
    if (ProjectionRevision != aProjection.projectionRevision()) {
        Projected = aProjection.project(BBox.topLeft());
        ProjectionRevision = aProjection.projectionRevision();
    }
}

Coord Node::position() const
{
    return BBox.topLeft();
}

void Node::setPosition(const Coord& aCoord)
{
    BBox = CoordBox(aCoord, aCoord);
    ProjectionRevision = 0;
    g_backend.sync(this);

    notifyChanges();
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

    return IsPOI;
}

bool Node::isWaypoint()
{
    if (!MetaUpToDate)
        updateMeta();

    return IsWaypoint;
}

bool Node::isSelectable(qreal PixelPerM, RendererOptions options)
{
    // If Node has non-default tags -> POI -> always selectable
    if (isPOI())
        return true;

    bool Draw = false;
    if (options.options.testFlag(RendererOptions::NodesVisible) || (lastUpdated() == Feature::Log && !options.options.testFlag(RendererOptions::TrackSegmentVisible))) {
        Draw = (PixelPerM * M_PREFS->getNodeSize() >= 1);
        // Do not draw GPX nodes when simple GPX track appearance is enabled
        if (M_PREFS->getSimpleGpxTrack() && layer()->isTrack())
            Draw = false;
        if (!Draw) {
            if (!sizeParents())
                Draw = true;
            else if (lastUpdated() == Feature::Log && !options.options.testFlag(RendererOptions::TrackSegmentVisible))
                Draw = true;
        }
    }
    return Draw;
}


bool Node::notEverythingDownloaded()
{
    return lastUpdated() == Feature::NotYetDownloaded;
}

const CoordBox& Node::boundingBox(bool) const
{
    return BBox;
}

void Node::drawSimple(QPainter &P, MapView *theView)
{
//    if (!M_PREFS->getWireframeView() && !TEST_RFLAGS(RendererOptions::Interacting))
//        return;

    if (! ((isReadonly() || !isSelectable(theView->pixelPerM(), theView->renderOptions())) && (!isPOI() && !isWaypoint())))
        //        if (!Pt->isReadonly() && Pt->isSelectable(r))
    {
        if (!layer()) {
            qDebug() << "Node without layer:" << id().numId << xmlId();
            return;
        }
        qreal WW = theView->nodeWidth();
        if (WW >= 1) {
            QColor theColor = QColor(0,0,0,128);
            if (M_PREFS->getUseStyledWireframe() && hasPainter()) {
                const FeaturePainter* thePainter = getCurrentPainter();
                if (thePainter->DrawForeground)
                    theColor = thePainter->ForegroundColor;
                else if (thePainter->DrawBackground)
                    theColor = thePainter->BackgroundColor;
            }
            QPointF Pp(theView->toView(this));
            if (layer()->classGroups() & Layer::Special) {
                QRect R2(Pp.x()-(WW+4)/2, Pp.y()-(WW+4)/2, WW+4, WW+4);
                P.fillRect(R2,QColor(255,0,255,192));
            } else if (isWaypoint()) {
                QRect R2(Pp.x()-(WW+4)/2, Pp.y()-(WW+4)/2, WW+4, WW+4);
                P.fillRect(R2,QColor(255,0,0,192));
            }

            QRect R(Pp.x()-WW/2, Pp.y()-WW/2, WW, WW);
            P.fillRect(R,theColor);
        }
    }
}

void Node::drawTouchup(QPainter& thePainter, MapView* theView)
{
    if (isDirty() && isUploadable() && M_PREFS->getDirtyVisible()) {
        QPoint P = theView->toView(this);
        qreal theWidth = theView->nodeWidth();
        if (theWidth >= 1) {
            QRect R(P-QPoint(theWidth/2,theWidth/2),QSize(theWidth,theWidth));
            thePainter.fillRect(R,M_PREFS->getDirtyColor());
        }
    }
}

void Node::drawSpecial(QPainter& thePainter, QPen& Pen, MapView* theView)
{
    QPen TP(Pen);
    TP.setWidth(TP.width() / 2);

    buildPath(theView->projection());
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


qreal Node::pixelDistance(const QPointF& Target, qreal, const QList<Feature*>& /* NoSnap */, MapView* theView) const
{
    qreal Best = 1000000;

    QPoint me = theView->toView(const_cast<Node*>(this));

    Best = distance(Target, me);

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
    QMutexLocker mutlock(&featMutex);
    if (MetaUpToDate)
        return;

    Feature::updateMeta();

    IsWaypoint = (findKey("_waypoint_") != -1);
    IsPOI = false;
    for (int i=0; i<tagSize(); ++i) {
        if (!M_PREFS->getTechnicalTags().contains(tagKey(i))) {
            IsPOI = true;
            break;
        }
    }

    if (!IsPOI && !IsWaypoint) {
        int i=0;
        int prtReadonly=0, prtWritable=0;
        for (; i<sizeParents(); ++i) {
            if (getParent(i)->isReadonly())
                ++prtReadonly;
            else
                ++prtWritable;
        }
        if (!ReadOnly) {
            if (prtReadonly && !prtWritable)
                setReadonly(true);
        }
    }
    MetaUpToDate = true;
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

Node * Node::fromXML(Document* d, Layer* L, QXmlStreamReader& stream)
{
    qreal Lat = stream.attributes().value("lat").toString().toDouble();
    qreal Lon = stream.attributes().value("lon").toString().toDouble();

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

QString Node::toHtml()
{
    QString D;
    int i;


    if ((i = findKey("_waypoint_")) != -1)
        D += "<p><b>"+QApplication::translate("MapFeature", "Waypoint")+"</b><br/>";
    D += "<i>"+QApplication::translate("MapFeature", "WGS84 coordinates")+": </i>" + COORD2STRING(position().y()) + " (" + Coord2Sexa(position().y()) + ") / " + COORD2STRING(position().x()) + " (" + Coord2Sexa(position().x()) + ")<br/>";
    D += "<i>"+QApplication::translate("MapFeature", "Projected coordinates")+": </i>" + COORD2STRING(projected().y()) + " / " + COORD2STRING(projected().x());

    if ((i = findKey("_description_")) != -1)
        D += "<br/><i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i);
    if ((i = findKey("_comment_")) != -1)
        D += "<br/><i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i);

    return Feature::toMainHtml(QApplication::translate("MapFeature", "Node"), "node").arg(D);
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
#ifndef FRISIUS_BUILD
    stream.writeTextElement("time", time().toString(Qt::ISODate)+"Z");
#else
    stream.writeTextElement("time", QDateTime::currentDateTime().toString(Qt::ISODate)+"Z");
#endif

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

    // OpenStreetBug
    s = tagValue("_special_","");
    if (!s.isEmpty() && id().type & IFeature::Special) {
        stream.writeStartElement("extensions");
        QString sid = stripToOSMId(id());
        stream.writeTextElement("id", sid);
        stream.writeEndElement();
    }
    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

/*********************************************/

TrackNode::TrackNode(const Coord &aCoord)
    : Node(aCoord)
    , Elevation(0.0)
    , Speed(0.0)
{
}

TrackNode::TrackNode(const Node& other)
    : Node(other)
    , Elevation(0.0)
    , Speed(0.0)
{
}

TrackNode::TrackNode(const TrackNode& other)
    : Node(other)
    , Elevation(other.Elevation)
    , Speed(other.Speed)
{
}

qreal TrackNode::speed() const
{
    return Speed;
}

void TrackNode::setSpeed(qreal aSpeed)
{
    Speed = aSpeed;
}

qreal TrackNode::elevation() const
{
    return Elevation;
}

void TrackNode::setElevation(qreal aElevation)
{
    Elevation = aElevation;
}

#ifdef FRISIUS_BUILD

const QDateTime& TrackNode::time() const
{
    return QDateTime::fromTime_t(Time);
}

void TrackNode::setTime(const QDateTime& time)
{
    Time = time.toTime_t();
}

void TrackNode::setTime(uint epoch)
{
    Time = epoch;
}
#endif

TrackNode * TrackNode::fromGPX(Document* d, Layer* L, QXmlStreamReader& stream)
{
    qreal Lat = stream.attributes().value("lat").toString().toDouble();
    qreal Lon = stream.attributes().value("lon").toString().toDouble();

    QString sid = (stream.attributes().hasAttribute("id") ? stream.attributes().value("id").toString() : stream.attributes().value("xml:id").toString());
    IFeature::FId id(IFeature::Point, sid.toLongLong());
    TrackNode* Pt = CAST_TRACKNODE(d->getFeature(id));
    if (!Pt) {
        Pt = g_backend.allocTrackNode(L, Coord(Lon,Lat));
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
        } else if (stream.name() == "speed") {
            stream.readNext();
            Pt->setSpeed(stream.text().toString().toFloat());
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

bool TrackNode::toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, QString element, bool forExport)
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
    if (speed()) {
        stream.writeTextElement("speed", QString::number(speed(),'f',6));
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
    stream.writeEndElement();

    if (progress)
        progress->setValue(progress->value()+1);

    return OK;
}

QString TrackNode::toHtml()
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

/*********************************/

PhotoNode::PhotoNode(const Coord& aCoord)
    : TrackNode(aCoord)
    , Photo(0)
    , photoLocationBR(true)
{

}

PhotoNode::PhotoNode(const Node& other)
    : TrackNode(other)
    , Photo(0)
    , photoLocationBR(true)
{
}

PhotoNode::PhotoNode(const TrackNode& other)
    : TrackNode(other)
    , Photo(0)
    , photoLocationBR(true)
{
}

PhotoNode::~PhotoNode(void)
{
    delete Photo;
}

QPixmap PhotoNode::photo() const
{
    if (Photo)
        return *(Photo);
    else
        return QPixmap();
}

void PhotoNode::setPhoto(QPixmap thePhoto)
{
    delete Photo;
    Photo = new QPixmap(thePhoto.scaled(M_PREFS->getMaxGeoPicWidth(), M_PREFS->getMaxGeoPicWidth(), Qt::KeepAspectRatio));
}

void PhotoNode::drawTouchup(QPainter& thePainter , MapView* theView)
{
#ifdef GEOIMAGE
    QPoint me = theView->toView(this);
    thePainter.setPen(QPen(QColor(0, 0, 0), 2));
    QRect box(me - QPoint(5, 3), QSize(10, 6));
    thePainter.drawRect(box);
    if (theView->renderOptions().options.testFlag(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
        qreal rt = qBound(0.2, (double)theView->pixelPerM(), 1.0);
        QPoint phPt;

        if (photoLocationBR) {
            phPt = me + QPoint(10*rt, 10*rt);
        } else {
            qreal rt = qBound(0.2, (double)theView->pixelPerM(), 1.0);
            qreal phRt = 1. * Photo->width() / Photo->height();
            phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
        }
        thePainter.drawPixmap(phPt, Photo->scaledToWidth(M_PREFS->getMaxGeoPicWidth()*rt));
    }
#endif
    Node::drawTouchup(thePainter, theView);
}

#ifdef GEOIMAGE
void PhotoNode::drawHover(QPainter& thePainter, MapView* theView)
{
    /* call the parent function */
    Feature::drawHover(thePainter, theView);

    /* and then the image */
    if (TEST_RFLAGS(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
        QPoint me(theView->toView(this));

        qreal rt = qBound(0.2, (double)theView->pixelPerM(), 1.0);
        qreal phRt = 1. * Photo->width() / Photo->height();
        QPoint phPt;
        if (photoLocationBR) {
            phPt = me + QPoint(10*rt, 10*rt);
        } else {
            phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
        }
        QRect box(phPt, QSize(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt));
        thePainter.drawRect(box);
    }
}
#endif

qreal PhotoNode::pixelDistance(const QPointF& Target, qreal ClearDistance, const QList<Feature*>& NoSnap, MapView* theView) const
{
#ifdef GEOIMAGE
    QPoint me = theView->toView(const_cast<PhotoNode*>(this));
    if (TEST_RFLAGS(RendererOptions::PhotosVisible) && theView->pixelPerM() > M_PREFS->getRegionalZoom()) {
        qreal rt = qBound(0.2, (double)theView->pixelPerM(), 1.0);
        qreal phRt = 1. * Photo->width() / Photo->height();
        QPoint phPt;
        if (photoLocationBR) {
        } else {
            phPt = me - QPoint(10*rt, 10*rt) - QPoint(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt);
        }
        QRect box(phPt, QSize(M_PREFS->getMaxGeoPicWidth()*rt, M_PREFS->getMaxGeoPicWidth()*rt/phRt));
        if (box.contains(Target.toPoint())) {
            photoLocationBR = !photoLocationBR;
            theView->invalidate(true, false, false);
        }
    }
#endif

    return Node::pixelDistance(Target, ClearDistance, NoSnap, theView );
}


