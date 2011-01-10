#include "Layer.h"

#include "Features.h"

#include "Document.h"
#include "LayerWidget.h"

#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "WayCommands.h"

#include "Utils/LineF.h"

#include "Global.h"

#include <QApplication>
#include <QMultiMap>
#include <QProgressDialog>
#include <QUuid>
#include <QMap>
#include <QList>
#include <QMenu>

#include <algorithm>
#include "LayerPrivate.h"

/* Layer */

Layer::Layer()
:  p(new LayerPrivate), theWidget(0)
{
    p->alpha = 1.0;
    p->dirtyLevel = 0;
}

Layer::Layer(const QString& aName)
:  p(new LayerPrivate), theWidget(0)
{
    p->Name = aName;
    p->alpha = 1.0;
    p->dirtyLevel = 0;
}

Layer::Layer(const Layer&)
: QObject(), p(0), theWidget(0)
{
}

Layer::~Layer()
{
    clear();
    SAFE_DELETE(p);
}

void Layer::setName(const QString& s)
{
    p->Name = s;
    if (theWidget) {
        theWidget->setName(s);
        theWidget->getAssociatedMenu()->setTitle(s);
    }
}

const QString& Layer::name() const
{
    return p->Name;
}

void Layer::setDescription(const QString& s)
{
    p->Description = s;
}

const QString& Layer::description() const
{
    return p->Description;
}

void Layer::setVisible(bool b) {
    p->Visible = b;
    if (theWidget) {
        theWidget->setLayerVisible(p->Visible, false);
    }

    if (p->theDocument) {
        FeatureIterator it(p->theDocument);
        for(;!it.isEnd(); ++it) {
            it.get()->invalidateMeta();
        }
    }
}

bool Layer::isVisible() const
{
    return p->Visible;
}

void Layer::setSelected(bool b) {
    p->selected = b;
}

bool Layer::isSelected() const
{
    return p->selected;
}

void Layer::setEnabled(bool b) {
    p->Enabled = b;
    if (theWidget) {
        theWidget->setVisible(b);
        theWidget->getAssociatedMenu()->menuAction()->setVisible(b);
    }

    if (p->theDocument) {
        FeatureIterator it(p->theDocument);
        for(;!it.isEnd(); ++it) {
            it.get()->invalidateMeta();
        }
    }
}

bool Layer::isEnabled() const
{
    return p->Enabled;
}

void Layer::setReadonly(bool b) {
    p->Readonly = b;

    if (p->theDocument) {
        FeatureIterator it(p->theDocument);
        for(;!it.isEnd(); ++it) {
            it.get()->invalidateMeta();
        }
    }
}

bool Layer::isReadonly() const
{
    return p->Readonly;
}

void Layer::setUploadable(bool b) {
    p->Uploadable = b;
}

bool Layer::isUploadable() const
{
    return p->Uploadable;
}

void Layer::add(Feature* aFeature)
{
    aFeature->setLayer(this);
    p->Features.push_back(aFeature);
    aFeature->invalidateMeta();
    notifyIdUpdate(aFeature->id(),aFeature);
}

void Layer::remove(Feature* aFeature)
{
    if (p->Features.removeOne(aFeature))
    {
        aFeature->setLayer(0);
        notifyIdUpdate(aFeature->id(),0);
    }
}

void Layer::deleteFeature(Feature* aFeature)
{
    if (p->Features.removeOne(aFeature))
    {
        aFeature->setLayer(0);
        notifyIdUpdate(aFeature->id(),0);
    }

    g_backend.deallocFeature(this, aFeature);
}

void Layer::clear()
{
    while (p->Features.count())
    {
        p->Features[0]->setLayer(0);
        notifyIdUpdate(p->Features[0]->id(),0);
        g_backend.clearLayer(this);
        p->Features.removeAt(0);
    }
}

void Layer::notifyIdUpdate(const IFeature::FId& id, Feature* aFeature)
{
    QHash<qint64, MapFeaturePtr>::iterator i;

    if (!aFeature) {
        i = p->IdMap.find(id.numId);
        while (i != p->IdMap.end() && i.key() == id.numId) {
            if (i.value()->id().type & id.type)
                i = p->IdMap.erase(i);
            else
                ++i;
        }
    }
    else {
        if (!aFeature->isVirtual())
            p->IdMap.insertMulti(id.numId, aFeature);
    }
}

bool Layer::exists(Feature* F) const
{
    int i = p->Features.indexOf(F);
    return (i != -1);
}

int Layer::size() const
{
    return p->Features.size();
}

void Layer::setDocument(Document* aDocument)
{
    p->theDocument = aDocument;
}

Document* Layer::getDocument()
{
    return p->theDocument;
}

int Layer::get(Feature* aFeature)
{
    for (int i=0; i<p->Features.size(); ++i)
        if (p->Features[i] == aFeature)
            return i;

    return -1;
}

QList<Feature *> Layer::get()
{
    QList<Feature *> theList;
    for (int i=0; i<p->Features.size(); ++i)
        if (p->Features[i])
            theList.append(p->Features[i]);
    return theList;
}


Feature* Layer::get(int i)
{
    return p->Features.at(i);
}

Feature* Layer::get(const IFeature::FId& id)
{
    QHash<qint64, MapFeaturePtr>::const_iterator i;

    i = p->IdMap.find(id.numId);
    while (i != p->IdMap.end() && i.key() == id.numId) {
        if ((i.value()->id().type & id.type) != 0)
            return i.value();
        ++i;
    }
    return NULL;
}

const Feature* Layer::get(int i) const
{
    if((int)i>=p->Features.size()) return 0;
    return p->Features[i];
}

LayerWidget* Layer::getWidget(void)
{
    return theWidget;
}

void Layer::deleteWidget(void)
{
//	theWidget->deleteLater();
    delete theWidget;
    theWidget = NULL;
}

void Layer::setAlpha(const qreal a)
{
    p->alpha = a;

    if (p->theDocument) {
        FeatureIterator it(p->theDocument);
        for(;!it.isEnd(); ++it) {
            it.get()->invalidateMeta();
        }
    }
}

qreal Layer::getAlpha() const
{
    return p->alpha;
}

void Layer::setId(const QString& id)
{
    Id = id;
}

const QString& Layer::id() const
{
    if (Id.isEmpty())
        Id = QUuid::createUuid().toString();
    return Id;
}

CoordBox Layer::boundingBox()
{
    if(p->Features.size()==0) return CoordBox(Coord(0,0),Coord(0,0));
    CoordBox Box;
    bool haveFirst = false;
    for (int i=0; i<p->Features.size(); ++i) {
        if (p->Features.at(i)->isDeleted())
            continue;
        if (p->Features.at(i)->notEverythingDownloaded())
            continue;
        if (haveFirst)
            Box.merge(p->Features.at(i)->boundingBox());
        else {
            Box = p->Features.at(i)->boundingBox();
            haveFirst = true;
        }
    }
    return Box;
}

int Layer::incDirtyLevel(int inc)
{
    return p->dirtyLevel += inc;
}

int Layer::decDirtyLevel(int inc)
{
    return p->dirtyLevel -= inc;
}

int Layer::setDirtyLevel(int newLevel)
{
    return (p->dirtyLevel = newLevel);
}

int Layer::getDirtyLevel() const
{
    return p->dirtyLevel;
}

int Layer::getDisplaySize() const
{
    int objects = 0;

    QList<MapFeaturePtr>::const_iterator i;
    for (i = p->Features.constBegin(); i != p->Features.constEnd(); i++) {
        if ((*i)->isVirtual())
            continue;
        ++objects;
    }

    return objects;
}

int Layer::getDirtySize() const
{
    int dirtyObjects = 0;

    QList<MapFeaturePtr>::const_iterator i;
    for (i = p->Features.constBegin(); i != p->Features.constEnd(); i++) {
        Feature* F = (*i);
        if (F->isVirtual())
            continue;
        else if (F->isDirty() && (!(F->isDeleted()) || (F->isDeleted() && F->hasOSMId())))
            ++dirtyObjects;
    }

    return dirtyObjects;
}

bool Layer::canDelete() const
{
    return (p->dirtyLevel == 0);
}

QString Layer::toMainHtml()
{
    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(p->Name);
    if (!p->Description.isEmpty())
        desc += QString("<b>%1</b><br/>").arg(p->Description);
    desc += QString("<small>(%1)</small>").arg(id());

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "<hr/>";
    S += "<i>"+QApplication::translate("Layer", "Size")+": </i>" + QApplication::translate("Layer", "%n features", "", QCoreApplication::CodecForTr, getDisplaySize())+"<br/>";
    S += "%1";
    S += "</body></html>";

    return S;
}

QString Layer::toHtml()
{
    return toMainHtml().arg("");
}

QString Layer::toPropertiesHtml()
{
    QString h;

    h += "<u>" + p->Name + "</u><br/>";
    h += "<i>" + tr("Features") + ": </i>" + QString::number(getDirtySize());

    return h;
}

bool Layer::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress)
{
    Q_UNUSED(asTemplate);
    Q_UNUSED(progress);

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("name", p->Name);
    stream.writeAttribute("alpha", QString::number(p->alpha,'f',2));
    stream.writeAttribute("visible", QString((p->Visible ? "true" : "false")));
    stream.writeAttribute("selected", QString((p->selected ? "true" : "false")));
    stream.writeAttribute("enabled", QString((p->Enabled ? "true" : "false")));
    stream.writeAttribute("readonly", QString((p->Readonly ? "true" : "false")));
    stream.writeAttribute("uploadable", QString((p->Uploadable ? "true" : "false")));
    if (getDirtyLevel())
        stream.writeAttribute("dirtylevel", QString::number(getDirtyLevel()));

    return true;
}

Layer * Layer::fromXML(Layer* l, Document* /*d*/, QXmlStreamReader& stream, QProgressDialog * /*progress*/)
{
    l->setId(stream.attributes().value("xml:id").toString());
    l->setAlpha(stream.attributes().value("alpha").toString().toDouble());
    l->setVisible((stream.attributes().value("visible") == "true" ? true : false));
    l->setSelected((stream.attributes().value("selected") == "true" ? true : false));
    l->setEnabled((stream.attributes().value("enabled") == "false" ? false : true));
    l->setReadonly((stream.attributes().value("readonly") == "true" ? true : false));
    l->setUploadable((stream.attributes().value("uploadable") == "false" ? false : true));
    l->setDirtyLevel((stream.attributes().hasAttribute("dirtylevel") ? stream.attributes().value("dirtylevel").toString().toInt() : 0));

    return l;
}

// DrawingLayer

DrawingLayer::DrawingLayer()
    : Layer()
{
    p->Visible = true;
}

DrawingLayer::DrawingLayer(const QString & aName)
    : Layer(aName)
{
    p->Visible = true;
}

DrawingLayer::~ DrawingLayer()
{
}

LayerWidget* DrawingLayer::newWidget(void)
{
//	delete theWidget;
    theWidget = new DrawingLayerWidget(this);
    return theWidget;
}


bool DrawingLayer::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress)
{
    bool OK = true;

    stream.writeStartElement(metaObject()->className());
    Layer::toXML(stream, asTemplate, progress);

    if (!asTemplate) {
        stream.writeStartElement("osm");
        stream.writeAttribute("version", "0.6");
        stream.writeAttribute("generator", QString("Merkaartor %1").arg(STRINGIFY(VERSION)));

        if (p->Features.size()) {
            stream.writeStartElement("bound");
            CoordBox layBB = boundingBox();
            QString S = QString().number(layBB.bottomLeft().y(),'f',6) + ",";
            S += QString().number(layBB.bottomLeft().x(),'f',6) + ",";
            S += QString().number(layBB.topRight().y(),'f',6) + ",";
            S += QString().number(layBB.topRight().x(),'f',6);
            stream.writeAttribute("box", S);
            stream.writeAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
            stream.writeEndElement();
        }

        QList<MapFeaturePtr>::iterator it;
        for(it = p->Features.begin(); it != p->Features.end(); it++)
            (*it)->toXML(stream, progress);
        stream.writeEndElement();

        QList<CoordBox> downloadBoxes = p->theDocument->getDownloadBoxes(this);
        if (downloadBoxes.size() && p->theDocument->getLastDownloadLayerTime().secsTo(QDateTime::currentDateTime()) < 12*3600) { // Do not export downloaded areas if older than 12h
            stream.writeStartElement("DownloadedAreas");
            QListIterator<CoordBox>it(downloadBoxes);
            while(it.hasNext()) {
                it.next().toXML("DownloadedBoundingBox", stream);
            }
            stream.writeEndElement();
        }
    }
    stream.writeEndElement();

    return OK;
}

DrawingLayer * DrawingLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    DrawingLayer* l = new DrawingLayer(stream.attributes().value("name").toString());
    Layer::fromXML(l, d, stream, progress);
    d->add(l);
    if (!DrawingLayer::doFromXML(l, d, stream, progress)) {
        d->remove(l);
        delete l;
        return NULL;
    }
    return l;
}

DrawingLayer * DrawingLayer::doFromXML(DrawingLayer* l, Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "osm") {
            QSet<Way*> addedWays;
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "way") {
                    Way* R = Way::fromXML(d, l, stream);
                    if (R)
                        addedWays << R;
                } else if (stream.name() == "relation") {
                    /* Relation* r = */ Relation::fromXML(d, l, stream);
                } else  if (stream.name() == "node") {
                    /* Node* N = */ Node::fromXML(d, l, stream);
                } else if (stream.name() == "trkseg") {
                    /*TrackSegment* T = */ TrackSegment::fromXML(d, l, stream, progress);
                } else if (stream.name() == "bound") {
                    stream.skipCurrentElement();
                } else if (!stream.isWhitespace()) {
                    qDebug() << "osm: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    QString el = stream.readElementText(QXmlStreamReader::IncludeChildElements);
                }

                progress->setValue(stream.characterOffset());

                if (progress->wasCanceled())
                    break;

                stream.readNext();
                qApp->processEvents();
            }
        } else if (stream.name() == "DownloadedAreas") {
            if (d->getLastDownloadLayerTime().secsTo(QDateTime::currentDateTime()) < 12*3600) {    // Do not import downloaded areas if older than 12h
                stream.readNext();
                while(!stream.atEnd() && !stream.isEndElement()) {
                    if (stream.name() == "DownloadedBoundingBox") {
                        d->addDownloadBox(l, CoordBox::fromXML(stream));
                        stream.readNext();
                    }
                    stream.readNext();
                }
            } else
                stream.skipCurrentElement();
        } else if (!stream.isWhitespace()) {
            qDebug() << "DrLayer: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }

        stream.readNext();
    }
    return l;
}

// TrackLayer

TrackLayer::TrackLayer(const QString & aName, const QString& filename)
    : Layer(aName), Filename(filename)
{
    p->Visible = true;
    p->Readonly = M_PREFS->getReadonlyTracksDefault();
}

TrackLayer::~ TrackLayer()
{
}

LayerWidget* TrackLayer::newWidget(void)
{
    theWidget = new TrackLayerWidget(this);
    return theWidget;
}

void TrackLayer::extractLayer()
{
    DrawingLayer* extL = new DrawingLayer(tr("Extract - %1").arg(name()));
    extL->setUploadable(false);

    Node* P;
    QList<Node*> PL;

    const double coordPer10M = (double(COORD_MAX) * 2 / 40080000) * 2;

    for (int i=0; i < size(); i++) {
        if (TrackSegment* S = dynamic_cast<TrackSegment*>(get(i))) {

            if (S->size() < 2)
                continue;

            // Cope with walking tracks
            double konstant = coordPer10M;
            double meanSpeed = S->distance() / S->duration() * 3600;
            if (meanSpeed < 10.)
                konstant /= 3.;


            PL.clear();

            P = g_backend.allocNode(extL, S->getNode(0)->position() );
            P->setTime(S->getNode(0)->time());
            P->setElevation(S->getNode(0)->elevation());
            P->setSpeed(S->getNode(0)->speed());
            PL.append(P);
            int startP = 0;

            P = g_backend.allocNode(extL, S->getNode(1)->position() );
            P->setTime(S->getNode(1)->time());
            P->setElevation(S->getNode(1)->elevation());
            P->setSpeed(S->getNode(1)->speed());
            PL.append(P);
            int endP = 1;

            for (int j=2; j < S->size(); j++) {
                P = g_backend.allocNode(extL, S->getNode(j)->position() );
                P->setTime(S->getNode(j)->time());
                P->setElevation(S->getNode(j)->elevation());
                P->setSpeed(S->getNode(j)->speed());
                PL.append(P);
                endP = PL.size()-1;

                LineF l(PL[startP]->position(), PL[endP]->position());
                for (int k=startP+1; k < endP; k++) {
                    double d = l.distance(PL[k]->position());
                    if (d < konstant) {
                        Node* P = PL[k];
                        PL.removeAt(k);
                        g_backend.deallocFeature(extL, P);
                        endP--;
                    } else
                        startP = k;
                }
            }

            Way* R = g_backend.allocWay(extL);
            R->setLastUpdated(Feature::OSMServer);
            extL->add(R);
            for (int i=0; i < PL.size(); i++) {
                extL->add(PL[i]);
                R->add(PL[i]);
            }
        }
    }

    p->theDocument->add(extL);
}

const QString TrackLayer::getFilename()
{
    return Filename;
}

QString TrackLayer::toHtml()
{
    QString S;

    int totSegment = 0;
    int totSec = 0;
    double totDistance = 0;
    for (int i=0; i < size(); ++i) {
        if (TrackSegment* S = CAST_SEGMENT(get(i))) {
            totSegment++;
            totSec += S->duration();
            totDistance += S->distance();
        }
    }

    S += "<i>"+QApplication::translate("TrackLayer", "# of track segments")+": </i>" + QApplication::translate("TrackLayer", "%1").arg(QLocale().toString(totSegment))+"<br/>";
    S += "<i>"+QApplication::translate("TrackLayer", "Total distance")+": </i>" + QApplication::translate("TrackLayer", "%1 km").arg(QLocale().toString(totDistance, 'g', 3))+"<br/>";
    S += "<i>"+QApplication::translate("TrackLayer", "Total duration")+": </i>" + QApplication::translate("TrackLayer", "%1h %2m").arg(QLocale().toString(totSec/3600)).arg(QLocale().toString((totSec%3600)/60))+"<br/>";

    return toMainHtml().arg(S);
}

bool TrackLayer::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress)
{
    bool OK = true;
    if (asTemplate)
        return OK;

    stream.writeStartElement(metaObject()->className());
    Layer::toXML(stream, asTemplate, progress);

    stream.writeStartElement("gpx");
    stream.writeAttribute("version", "1.1");
    stream.writeAttribute("creator", "Merkaartor");
    stream.writeAttribute("xmlns", "http://www.topografix.com/GPX/1/1");

    QList<Node*>	waypoints;
    QList<TrackSegment*>	segments;
    QList<MapFeaturePtr>::iterator it;
    for(it = p->Features.begin(); it != p->Features.end(); it++) {
        if (TrackSegment* S = CAST_SEGMENT(*it))
            segments.push_back(S);
        if (Node* P = CAST_NODE(*it))
            if (!P->tagValue("_waypoint_","").isEmpty())
                waypoints.push_back(P);
    }

    for (int i=0; i < waypoints.size(); ++i) {
        waypoints[i]->toGPX(stream, progress, "wpt");
    }

    stream.writeStartElement("trk");
    for (int i=0; i < segments.size(); ++i)
        segments[i]->toXML(stream, progress);
    stream.writeEndElement();

    stream.writeEndElement(); //gpx
    stream.writeEndElement();

    return OK;
}

TrackLayer * TrackLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    TrackLayer* l = new TrackLayer(stream.attributes().value("name").toString());
    Layer::fromXML(l, d, stream, progress);
    d->add(l);

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "gpx") {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == "trk") {
                    stream.readNext();
                    while(!stream.atEnd() && !stream.isEndElement()) {
                        if (stream.name() == "trkseg") {
                            TrackSegment::fromXML(d, l, stream, progress);
                        }
                        stream.readNext();
                    }
                } else if (stream.name() == "wpt") {
                    /* Node* N = */ Node::fromGPX(d, l, stream);
                    //l->add(N);
                    progress->setValue(progress->value()+1);
                } else if (!stream.isWhitespace()) {
                    qDebug() << "gpx: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }

                progress->setValue(stream.characterOffset());

                if (progress->wasCanceled())
                    break;

                stream.readNext();
                qApp->processEvents();
            }
        } else if (!stream.isWhitespace()) {
            qDebug() << "TrLayer: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }

        stream.readNext();
    }
    return l;
}

// DirtyLayer

DirtyLayer::DirtyLayer(const QString & aName)
    : DrawingLayer(aName)
{
    p->Visible = true;
}

DirtyLayer::~ DirtyLayer()
{
}

DirtyLayer* DirtyLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    DirtyLayer* l = new DirtyLayer(stream.attributes().value("name").toString());
    Layer::fromXML(l, d, stream, progress);
    d->add(l);
    d->setDirtyLayer(l);
    DrawingLayer::doFromXML(l, d, stream, progress);
    return l;
}

LayerWidget* DirtyLayer::newWidget(void)
{
    theWidget = new DirtyLayerWidget(this);
    return theWidget;
}

// UploadedLayer

UploadedLayer::UploadedLayer(const QString & aName)
    : DrawingLayer(aName)
{
    p->Visible = true;
}

UploadedLayer::~ UploadedLayer()
{
}

UploadedLayer* UploadedLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    UploadedLayer* l = new UploadedLayer(stream.attributes().value("name").toString());
    Layer::fromXML(l, d, stream, progress);
    d->add(l);
    d->setUploadedLayer(l);
    DrawingLayer::doFromXML(l, d, stream, progress);
    return l;
}

LayerWidget* UploadedLayer::newWidget(void)
{
    theWidget = new UploadedLayerWidget(this);
    return theWidget;
}

// DeletedLayer

DeletedLayer::DeletedLayer(const QString & aName)
    : DrawingLayer(aName)
{
    p->Visible = false;
    p->Enabled = false;
}

DeletedLayer::~ DeletedLayer()
{
}

bool DeletedLayer::toXML(QXmlStreamWriter& , bool, QProgressDialog * )
{
    return true;
}

DeletedLayer* DeletedLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    /* Only keep DeletedLayer for backward compatibility with MDC */
    Layer::fromXML(dynamic_cast<DrawingLayer*>(d->getDirtyOrOriginLayer()), d, stream, progress);
    DrawingLayer::doFromXML(dynamic_cast<DrawingLayer*>(d->getDirtyOrOriginLayer()), d, stream, progress);
    return NULL;
}

LayerWidget* DeletedLayer::newWidget(void)
{
    return NULL;
}

// FilterLayer

FilterLayer::FilterLayer(const QString& aId, const QString & aName, const QString& aFilter)
    : Layer(aName)
    , theSelectorString(aFilter)
{
    setId(aId);
    p->Visible = true;
    theSelector = TagSelector::parse(theSelectorString);
}

FilterLayer::~ FilterLayer()
{
}

void FilterLayer::setFilter(const QString& aFilter)
{
    theSelectorString = aFilter;
    delete theSelector;
    theSelector = TagSelector::parse(theSelectorString);

    FeatureIterator it(p->theDocument);
    for(;!it.isEnd(); ++it) {
        it.get()->updateFilters();
    }
}

bool FilterLayer::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress)
{
    stream.writeStartElement(metaObject()->className());
    Layer::toXML(stream, asTemplate, progress);
    stream.writeAttribute("filter", theSelectorString);
    stream.writeEndElement();

    return true;
}

FilterLayer* FilterLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * progress)
{
    QString id;
    if (stream.attributes().hasAttribute("xml:id"))
        id = stream.attributes().value("xml:id").toString();
    else
        id = QUuid::createUuid().toString();
    FilterLayer* l = new FilterLayer(id, stream.attributes().value("name").toString(), stream.attributes().value("filter").toString());
    Layer::fromXML(l, d, stream, progress);
    stream.readNext();

    d->add(l);
    return l;
}

LayerWidget* FilterLayer::newWidget(void)
{
    theWidget = new FilterLayerWidget(this);
    return theWidget;
}

