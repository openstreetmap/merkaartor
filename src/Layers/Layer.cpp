#include "Layer.h"

#include "Features.h"

#include "Document.h"
#include "LayerWidget.h"

#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "WayCommands.h"

#include "Utils/LineF.h"

#include "ImportExport/ImportExportOsmBin.h"

#include <QApplication>
#include <QMultiMap>
#include <QProgressDialog>
#include <QUuid>
#include <QMap>
#include <QList>
#include <QMenu>

#include <algorithm>

/* Layer */

class LayerPrivate
{
public:
    LayerPrivate()
    {
        theDocument = NULL;
        selected = false;
        Enabled = true;
        Readonly = false;
        Uploadable = true;

        theRTree = new MyRTree(7, 2);
        IndexingBlocked = false;
        VirtualsUpdatesBlocked = false;

    }
    ~LayerPrivate()
    {
        delete theRTree;
    }
    QList<MapFeaturePtr> Features;
    MyRTree* theRTree;

    QHash<QString, MapFeaturePtr> IdMap;
    QString Name;
    QString Description;
    bool Visible;
    bool selected;
    bool Enabled;
    bool Readonly;
    bool Uploadable;
    bool IndexingBlocked;
    bool VirtualsUpdatesBlocked;
    qreal alpha;
    int dirtyLevel;

    Document* theDocument;
};

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
    SAFE_DELETE(p);
}

void Layer::get(const CoordBox& hz, QList<Feature*>& theFeatures)
{
    std::deque < MapFeaturePtr > ret = indexFind(hz);
    for (std::deque < MapFeaturePtr >::const_iterator it = ret.begin(); it < ret.end(); ++it) {
        theFeatures.push_back(*it);
    }
}

void Layer::getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, QSet<Way*>& theCoastlines, Document* /* theDocument */,
                   QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform)
{
    if (!isVisible() || !size())
        return;

    for (int i=0; i < invalidRects.size(); ++i) {
        std::deque < MapFeaturePtr > ret = indexFind(invalidRects[i]);
        for (std::deque < MapFeaturePtr >::const_iterator it = ret.begin(); it != ret.end(); ++it) {
            if (theFeatures[(*it)->renderPriority()].contains(*it))
                continue;

            if (Way * R = CAST_WAY(*it)) {
                R->buildPath(theProjection, theTransform, clipRect);
                theFeatures[(*it)->renderPriority()].insert(*it);

                if (R->isCoastline())
                    theCoastlines.insert(R);
            } else
            if (Relation * RR = CAST_RELATION(*it)) {
                RR->buildPath(theProjection, theTransform, clipRect);
                theFeatures[(*it)->renderPriority()].insert(*it);
            } else
            if (Node * pt = CAST_NODE(*it)) {
                if (arePointsDrawable())
                    theFeatures[(*it)->renderPriority()].insert(*it);
            } else
                theFeatures[(*it)->renderPriority()].insert(*it);
        }
    }
}


void Layer::setName(const QString& s)
{
    p->Name = s;
    if (theWidget) {
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
}

bool Layer::isEnabled() const
{
    return p->Enabled;
}

void Layer::setReadonly(bool b) {
    p->Readonly = b;
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
    if (!aFeature->isDeleted())
        indexAdd(aFeature->boundingBox(), aFeature);
    p->Features.push_back(aFeature);
    notifyIdUpdate(aFeature->id(),aFeature);
}

void Layer::add(Feature* aFeature, int Idx)
{
    add(aFeature);
    std::rotate(p->Features.begin()+Idx,p->Features.end()-1,p->Features.end());
}

void Layer::notifyIdUpdate(const QString& id, Feature* aFeature)
{
    p->IdMap[id] = aFeature;
}

void Layer::remove(Feature* aFeature)
{
    if (p->Features.removeOne(aFeature))
    {
        aFeature->setLayer(0);
        if (!aFeature->isDeleted())
            indexRemove(aFeature->boundingBox(), aFeature);
        notifyIdUpdate(aFeature->id(),0);
    }
}

void Layer::deleteFeature(Feature* aFeature)
{
    if (p->Features.removeOne(aFeature))
    {
        aFeature->setLayer(0);
        if (!aFeature->isDeleted())
            indexRemove(aFeature->boundingBox(), aFeature);
        notifyIdUpdate(aFeature->id(),0);
    }

    delete aFeature;
}

void Layer::clear()
{
    QList<MapFeaturePtr>::iterator i;
    for (i=p->Features.begin(); i != p->Features.end();)
    {
        (*i)->setLayer(0);
        notifyIdUpdate((*i)->id(),0);
        i = p->Features.erase(i);
    }
    reIndex();
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

Feature* Layer::get(const QString& id, bool exact)
{
    QHash<QString, MapFeaturePtr>::const_iterator i;

    i = p->IdMap.find(id);
    if (i != p->IdMap.end())
        return i.value();

    if (!exact) {
        i = p->IdMap.find(QString("node_"+id));
        if (i != p->IdMap.end())
            return i.value();
        i = p->IdMap.find(QString("way_"+id));
        if (i != p->IdMap.end())
            return i.value();
        i = p->IdMap.find(QString("rel_"+id));
        if (i != p->IdMap.end())
            return i.value();
    }
    return 0;
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
    if (Id == "")
        Id = QUuid::createUuid().toString();
    return Id;
}

void Layer::blockIndexing(bool val)
{
    p->IndexingBlocked = val;
}

void Layer::blockVirtualUpdates(bool val)
{
    p->VirtualsUpdatesBlocked = val;
}

bool Layer::isVirtualUpdatesBlocked() const
{
    return p->VirtualsUpdatesBlocked;
}


void Layer::indexAdd(const CoordBox& bb, const MapFeaturePtr aFeat)
{
    if (bb.isNull())
        return;
    if (!p->IndexingBlocked)
        p->theRTree->insert(bb, aFeat);
}

void Layer::indexRemove(const CoordBox& bb, const MapFeaturePtr aFeat)
{
    if (bb.isNull())
        return;
    if (!p->IndexingBlocked)
        p->theRTree->remove(bb, aFeat);
}

std::deque<Feature*> Layer::indexFind(const CoordBox& vp)
{
    return p->theRTree->find(vp);
}

void Layer::reIndex()
{
    qDebug() << "Reindexing...";

    delete p->theRTree;
    p->theRTree = new MyRTree(7, 2);

    for (int i=0; i<p->Features.size(); ++i) {
        if (p->Features.at(i)->isDeleted())
            continue;
        Feature* f = p->Features.at(i);
        CoordBox bb = f->boundingBox();
        if (!bb.isNull()) {
            //Q_ASSERT((bb.bottomLeft().lon() <= bb.topRight().lon()) && (bb.bottomLeft().lat() <= bb.topRight().lat()));
            //Q_ASSERT((bb.bottomLeft().lon() < 100000000) && (bb.bottomLeft().lat() > 100000000));
            //Q_ASSERT((bb.topRight().lon() < 100000000) && (bb.topRight().lat() > 100000000));
            p->theRTree->insert(bb, f);
        }
    }
}

void Layer::reIndex(QProgressDialog & progress)
{
    qDebug() << "Reindexing...";

    delete p->theRTree;
    p->theRTree = new MyRTree(7, 2);

    progress.setLabelText("Indexing...");
    progress.setValue(0);
    progress.setMaximum(p->Features.size());
    for (int i=0; i<p->Features.size(); ++i) {
        if (!p->Features.at(i)->isDeleted()) {
            Feature* f = p->Features.at(i);
            CoordBox bb = f->boundingBox();
            if (!bb.isNull()) {
                p->theRTree->insert(bb, f);
            }
        }
        progress.setValue(i);
        qApp->processEvents();
    }
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

int Layer::getDirtyLevel()
{
    return p->dirtyLevel;
}

int Layer::getDirtySize()
{
    int dirtyObjects = 0;

    QList<MapFeaturePtr>::const_iterator i;
    for (i = p->Features.constBegin(); i != p->Features.constEnd(); i++)
        if ((*i)->isVirtual())
            continue;
        else if (!((*i)->isDeleted()) || ((*i)->isDeleted() && (*i)->hasOSMId()))
            ++dirtyObjects;

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
    S += "<i>"+QApplication::translate("Layer", "Size")+": </i>" + QApplication::translate("Layer", "%n features", "", QCoreApplication::CodecForTr, size())+"<br/>";
    S += "%1";
    S += "</body></html>";

    return S;
}

QString Layer::toHtml()
{
    return toMainHtml().arg("");
}


// DrawingLayer

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


bool DrawingLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
    xParent.appendChild(e);

    e.setAttribute("xml:id", id());
    e.setAttribute("name", p->Name);
    e.setAttribute("alpha", QString::number(p->alpha,'f',2));
    e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
    e.setAttribute("selected", QString((p->selected ? "true" : "false")));
    e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
    e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

    QDomElement o = xParent.ownerDocument().createElement("osm");
    e.appendChild(o);
    o.setAttribute("version", "0.5");
    o.setAttribute("generator", "Merkaartor");

    if (p->Features.size()) {
        QDomElement bb = xParent.ownerDocument().createElement("bound");
        o.appendChild(bb);
        CoordBox layBB = boundingBox();
        QString S = QString().number(intToAng(layBB.bottomLeft().lat()),'f',6) + ",";
        S += QString().number(intToAng(layBB.bottomLeft().lon()),'f',6) + ",";
        S += QString().number(intToAng(layBB.topRight().lat()),'f',6) + ",";
        S += QString().number(intToAng(layBB.topRight().lon()),'f',6);
        bb.setAttribute("box", S);
        bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
    }

    QList<MapFeaturePtr>::iterator it;
    for(it = p->Features.begin(); it != p->Features.end(); it++)
        (*it)->toXML(o, progress);

    return OK;
}

DrawingLayer * DrawingLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog & progress)
{
    DrawingLayer* l = new DrawingLayer(e.attribute("name"));
    d->add(l);
    if (!DrawingLayer::doFromXML(l, d, e, progress)) {
        d->remove(l);
        delete l;
        return NULL;
    }
    return l;
}

DrawingLayer * DrawingLayer::doFromXML(DrawingLayer* l, Document* d, const QDomElement e, QProgressDialog & progress)
{
    l->blockIndexing(true);
    l->blockVirtualUpdates(true);

    l->setId(e.attribute("xml:id"));
    l->setAlpha(e.attribute("alpha").toDouble());
    l->setVisible((e.attribute("visible") == "true" ? true : false));
    l->setSelected((e.attribute("selected") == "true" ? true : false));
    l->setEnabled((e.attribute("enabled") == "false" ? false : true));
    l->setReadonly((e.attribute("readonly") == "true" ? true : false));

    QDomElement c = e.firstChildElement();
    if (c.tagName() != "osm")
        return NULL;

    QSet<Way*> addedWays;
    int i=0;
    c = c.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "bound") {
        } else
        if (c.tagName() == "way") {
            Way* R = Way::fromXML(d, l, c);
            if (R)
                addedWays << R;
//			l->add(R);
            i++;
        } else
        if (c.tagName() == "relation") {
            /* Relation* r = */ Relation::fromXML(d, l, c);
//			l->add(r);
            i++;
        } else
        if (c.tagName() == "node") {
            /* Node* N = */ Node::fromXML(d, l, c);
//			l->add(N);
            i++;
        } else
        if (c.tagName() == "trkseg") {
            TrackSegment* T = TrackSegment::fromXML(d, l, c, progress);
            l->add(T);
            i++;
        }

        if (i >= progress.maximum()/100) {
            progress.setValue(progress.value()+i);
            i=0;
        }

        if (progress.wasCanceled())
            break;

        c = c.nextSiblingElement();
    }

    if (i > 0) progress.setValue(progress.value()+i);

    QString savlbl = progress.labelText();
    int savval = progress.value();
    int savmax = progress.maximum();

    l->blockVirtualUpdates(false);
    if (M_PREFS->getUseVirtualNodes()) {
        progress.setLabelText("Updating virtual...");
        progress.setMaximum(addedWays.size());
        progress.setValue(0);
        foreach (Way* value, addedWays) {
            value->updateVirtuals();
            progress.setValue(progress.value()+1);
            qApp->processEvents();
        }
    }
    l->blockIndexing(false);
    l->reIndex(progress);

    progress.setLabelText(savlbl);
    progress.setMaximum(savmax);
    progress.setValue(savval);
    qApp->processEvents();

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

    const double coordPer10M = (double(INT_MAX) * 2 / 40080000) * 2;

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

            P = new Node( S->getNode(0)->position() );
            if (M_PREFS->apiVersionNum() < 0.6)
                P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
            P->setTime(S->getNode(0)->time());
            P->setElevation(S->getNode(0)->elevation());
            P->setSpeed(S->getNode(0)->speed());
            //P->setTag("ele", QString::number(S->get(0)->elevation()));
            PL.append(P);
            int startP = 0;

            P = new Node( S->getNode(1)->position() );
            if (M_PREFS->apiVersionNum() < 0.6)
                P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
            P->setTime(S->getNode(1)->time());
            P->setElevation(S->getNode(1)->elevation());
            P->setSpeed(S->getNode(1)->speed());
            //P->setTag("ele", QString::number(S->get(1)->elevation()));
            PL.append(P);
            int endP = 1;

            for (int j=2; j < S->size(); j++) {
                P = new Node( S->getNode(j)->position() );
                if (M_PREFS->apiVersionNum() < 0.6)
                    P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
                P->setTime(S->getNode(j)->time());
                P->setElevation(S->getNode(j)->elevation());
                P->setSpeed(S->getNode(j)->speed());
                //P->setTag("ele", QString::number(S->get(j)->elevation()));
                PL.append(P);
                endP = PL.size()-1;

                LineF l(toQt(PL[startP]->position()), toQt(PL[endP]->position()));
                for (int k=startP+1; k < endP; k++) {
                    double d = l.distance(toQt(PL[k]->position()));
                    if (d < konstant) {
                        Node* P = PL[k];
                        PL.removeAt(k);
                        delete P;
                        endP--;
                    } else
                        startP = k;
                }
            }

            Way* R = new Way();
            R->setLastUpdated(Feature::OSMServer);
            if (M_PREFS->apiVersionNum() < 0.6)
                R->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
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

bool TrackLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
    xParent.appendChild(e);

    e.setAttribute("xml:id", id());
    e.setAttribute("name", p->Name);
    e.setAttribute("alpha", QString::number(p->alpha,'f',2));
    e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
    e.setAttribute("selected", QString((p->selected ? "true" : "false")));
    e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
    e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

    QDomElement o = xParent.ownerDocument().createElement("gpx");
    e.appendChild(o);
    o.setAttribute("version", "1.1");
    o.setAttribute("creator", "Merkaartor");
    o.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1");

    QList<Node*>	waypoints;
    QList<TrackSegment*>	segments;
    QList<MapFeaturePtr>::iterator it;
    for(it = p->Features.begin(); it != p->Features.end(); it++) {
        if (TrackSegment* S = qobject_cast<TrackSegment*>(*it))
            segments.push_back(S);
        if (Node* P = qobject_cast<Node*>(*it))
            if (!P->tagValue("_waypoint_","").isEmpty())
                waypoints.push_back(P);
    }

    for (int i=0; i < waypoints.size(); ++i) {
        waypoints[i]->toGPX(o, progress);
    }

    QDomElement t = o.ownerDocument().createElement("trk");
    o.appendChild(t);

    for (int i=0; i < segments.size(); ++i)
        segments[i]->toXML(t, progress);

    return OK;
}

TrackLayer * TrackLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog & progress)
{
    TrackLayer* l = new TrackLayer(e.attribute("name"));
    l->blockIndexing(true);

    l->setId(e.attribute("xml:id"));
    l->setAlpha(e.attribute("alpha").toDouble());
    l->setVisible((e.attribute("visible") == "true" ? true : false));
    l->setSelected((e.attribute("selected") == "true" ? true : false));
    l->setEnabled((e.attribute("enabled") == "false" ? false : true));
    l->setReadonly((e.attribute("readonly") == "true" ? true : false));

    d->add(l);

    QDomElement c = e.firstChildElement();
    if (c.tagName() != "gpx")
        return NULL;

    c = c.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "trk") {
            QDomElement t = c.firstChildElement();
            while(!t.isNull()) {
                if (t.tagName() == "trkseg") {
                    TrackSegment* N = TrackSegment::fromXML(d, l, t, progress);
                    l->add(N);
                }

                t = t.nextSiblingElement();
            }
        }
        if (c.tagName() == "wpt") {
            /* Node* N = */ Node::fromGPX(d, l, c);
            //l->add(N);
            progress.setValue(progress.value()+1);
        }

        if (progress.wasCanceled())
            break;

        c = c.nextSiblingElement();
    }

    l->blockIndexing(false);
    l->reIndex();

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

DirtyLayer* DirtyLayer::fromXML(Document* d, const QDomElement e, QProgressDialog & progress)
{
    DirtyLayer* l = new DirtyLayer(e.attribute("name"));
    d->add(l);
    d->setDirtyLayer(l);
    DrawingLayer::doFromXML(l, d, e, progress);
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

UploadedLayer* UploadedLayer::fromXML(Document* d, const QDomElement e, QProgressDialog & progress)
{
    UploadedLayer* l = new UploadedLayer(e.attribute("name"));
    d->add(l);
    d->setUploadedLayer(l);
    DrawingLayer::doFromXML(l, d, e, progress);
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

bool DeletedLayer::toXML(QDomElement& , QProgressDialog & )
{
    return true;
}

DeletedLayer* DeletedLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog & progress)
{
    /* Only keep DeletedLayer for backward compatibility with MDC */
    DrawingLayer::doFromXML(dynamic_cast<DrawingLayer*>(d->getDirtyOrOriginLayer()), d, e, progress);
    return NULL;
}

LayerWidget* DeletedLayer::newWidget(void)
{
    return NULL;
}

// OsbLayer

class OsbLayerPrivate
{
public:
    OsbLayerPrivate()
        : theImp(0), IsWorld(false)
    {
    }
    OsbLayer* theLayer;
    ImportExportOsmBin* theImp;
    QList<qint32> loadedTiles;
    QList<qint32> loadedRegions;

    int rl;
    QMap< qint32, quint64 >::const_iterator ri;
    void loadRegion(Document* d, int rt);
    void clearRegion(Document* d, int rt);
    void handleTile(Document* d, int rt);

    bool IsWorld;
    CoordBox theVP;
};

class OsbFeatureIterator
{
public:
    OsbLayer* theLayer;

    QMap< qint32, OsbRegion* >::const_iterator itR;
    QHash< qint32, OsbTile* >::const_iterator itT;
    QList<Feature_ptr>::const_iterator itF;

    bool isAtEnd;

    Feature* get()
    {
        if (!isAtEnd)
            return (*itF).get();
        else
            return NULL;
    }

    OsbFeatureIterator(OsbLayer* aLayer)
            : theLayer(aLayer), isAtEnd(false)
    {
        itR = theLayer->pp->theImp->theRegionList.constBegin()+1;
        while (true) {
            if (itR != theLayer->pp->theImp->theRegionList.constEnd()) {
                itT = itR.value()->getTileIndex().constBegin();
                while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                    ++itT;
                }
                if (itT == itR.value()->getTileIndex().constEnd()) {
                    ++itR;
                    continue;
                }
                itF = itT.value()->theIndex.constBegin();
                break;
            } else {
                isAtEnd = true;
                break;
            }
        }
    }

    OsbFeatureIterator& operator++()
    {
        if (isAtEnd)
            return *this;

        while (true) {
            ++itF;
            while (itF == itT.value()->theIndex.constEnd())
            {
                ++itT;
                while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                    ++itT;
                }
                if (itT != itR.value()->getTileIndex().constEnd())
                    itF = itT.value()->theIndex.constBegin();
                else {
                    while (true) {
                        ++itR;
                        if (itR != theLayer->pp->theImp->theRegionList.constEnd()) {
                            itT = itR.value()->getTileIndex().constBegin();
                            while (itT != itR.value()->getTileIndex().constEnd() && (!itT.value() || itT.value()->isDeleted)) {
                                ++itT;
                            }
                            if (itT == itR.value()->getTileIndex().constEnd())
                                continue;
                            itF = itT.value()->theIndex.constBegin();
                            break;
                        } else {
                            isAtEnd = true;
                            return *this;
                        }
                    }
                }
            }

            if ((*itF)->isDeleted() || (*itF)->isHidden() || !(*itF)->hasEditPainter())
                continue;

            break;
        }
        return *this;
    }

    bool isEnd() const
    {
        return isAtEnd;
    }

};

OsbLayer::OsbLayer(const QString & aName)
    : Layer(aName)
{
    p->Visible = true;
    pp = new OsbLayerPrivate();
    pp->theLayer = this;
    pp->theImp = new ImportExportOsmBin(NULL);
}

OsbLayer::OsbLayer(const QString & aName, const QString & filename, bool isWorld)
    : Layer(aName)
{
    p->Visible = true;
    pp = new OsbLayerPrivate();
    pp->theLayer = this;
    pp->IsWorld = isWorld;
    pp->theImp = new ImportExportOsmBin(NULL);
    if (pp->theImp->loadFile(filename))
        pp->theImp->import(this);
}

OsbLayer::~ OsbLayer()
{
    delete pp->theImp;
    delete pp;
}

bool OsbLayer::arePointsDrawable()
{
    return true;
}

void OsbLayer::setFilename(const QString& filename)
{
    delete pp->theImp;

    pp->theImp = new ImportExportOsmBin(NULL);
    if (pp->theImp->loadFile(filename))
        pp->theImp->import(this);
}

LayerWidget* OsbLayer::newWidget(void)
{
    theWidget = new OsbLayerWidget(this);
    return theWidget;
}

void OsbLayerPrivate::loadRegion(Document* d, int rt)
{
    while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
        if (loadedRegions.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
            ++rl;
        }
    }
    while(ri != theImp->theRegionToc.constEnd() && ri.key() < rt) {
        ++ri;
    }
    if (ri != theImp->theRegionToc.constEnd() && ri.key() == rt) {
        if (theImp->loadRegion(rt, d, theLayer)) {
            loadedRegions.insert(rl, rt);
            ++rl;
            ++ri;
        }
    }
}

void OsbLayerPrivate::handleTile(Document* d, int rt)
{
    while (rl < loadedTiles.size() && loadedTiles.at(rl) <= rt) {
        if (loadedTiles.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedTiles.size() && loadedTiles.at(rl) < rt ) {
            if (theImp->clearTile(loadedTiles.at(rl), d, theLayer))
                loadedTiles.removeAt(rl);
            else
                ++rl;
        }
    }
    if (theImp->loadTile(rt, d, theLayer)) {
        loadedTiles.insert(rl, rt);
        ++rl;
    }
}

void OsbLayerPrivate::clearRegion(Document* d, int rt)
{
    while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
        if (loadedRegions.at(rl) == rt) {
            ++rl;
            return;
        }
        while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
            if (theImp->clearRegion(loadedRegions.at(rl), d, theLayer))
                loadedRegions.removeAt(rl);
            else
                ++rl;
        }
    }
}


void OsbLayer::preload()
{
    pp->rl = 0;
    pp->ri = pp->theImp->theRegionToc.constBegin();

    pp->loadRegion(NULL, 0);
}

void OsbLayer::get(const CoordBox& hz, QList<Feature*>& theFeatures)
{
    if (intToAng(pp->theVP.lonDiff()) > M_PREFS->getRegionTo0Threshold()) {
//		Layer::get(hz, theFeatures);
        return;
    } else {
        OsbFeatureIterator oit(this);
        while (!oit.isEnd()) {
            Feature* F = oit.get();

            if (!theFeatures.contains(F)) {
                if (hz.intersects(F->boundingBox())) {
                    if (Way * R = CAST_WAY(F)) {
                        theFeatures.push_back(R);
                    } else
                    if (Relation * RR = CAST_RELATION(F)) {
//						theFeatures.push_back(RR);
                    } else
                    if (Node * pt = CAST_NODE(F)) {
                        if (arePointsDrawable())
                            theFeatures.push_back(pt);
                    } else
                        theFeatures.push_back(F);
                    break;
                }
            }

            ++oit;
        }
    }
}

void OsbLayer::getFeatureSet(QMap<RenderPriority, QSet <Feature*> >& theFeatures, QSet<Way*>& theCoastlines, Document* theDocument,
                   QList<CoordBox>& invalidRects, QRectF& clipRect, Projection& theProjection, QTransform& theTransform)
{
    if (!isVisible())
        return;

    pp->theVP = CoordBox();
    for (int i=0; i < invalidRects.size(); ++i) {
        if (pp->theVP.isNull())
            pp->theVP = invalidRects[i];
        else
            pp->theVP.merge(invalidRects[i]);
    }

    QRectF r(pp->theVP.toQRectF());

    int xr1 = int((r.topLeft().x() + INT_MAX) / REGION_WIDTH);
    int yr1 = int((r.topLeft().y() + INT_MAX) / REGION_WIDTH);
    int xr2 = int((r.bottomRight().x() + INT_MAX) / REGION_WIDTH);
    int yr2 = int((r.bottomRight().y() + INT_MAX) / REGION_WIDTH);

    int xt1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH);
    int yt1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
    int xt2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
    int yt2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);

    pp->rl = 0;
    pp->ri = pp->theImp->theRegionToc.constBegin();


    pp->loadRegion(theDocument, 0);

    blockIndexing(true);

    if (intToAng(pp->theVP.lonDiff()) <= M_PREFS->getRegionTo0Threshold()) {
        for (int j=yr1; j <= yr2; ++j)
            for (int i=xr1; i <= xr2; ++i) {
                pp->loadRegion(theDocument, j*NUM_REGIONS+i);
            }
    }

    pp->rl = 0;
    if (intToAng(pp->theVP.lonDiff()) <= M_PREFS->getTileToRegionThreshold()) {
        for (int j=yt1; j <= yt2; ++j)
            for (int i=xt1; i <= xt2; ++i)
                pp->handleTile(theDocument, j*NUM_TILES+i);
    }
    while (pp->rl < pp->loadedTiles.size() ) {
        if (pp->theImp->clearTile(pp->loadedTiles.at(pp->rl), theDocument, this))
            pp->loadedTiles.removeAt(pp->rl);
        else
            ++pp->rl;
    }

    pp->rl = 1;
    if (intToAng(pp->theVP.lonDiff()) <= M_PREFS->getRegionTo0Threshold()) {
        for (int j=yr1; j <= yr2; ++j)
            for (int i=xr1; i <= xr2; ++i) {
                pp->clearRegion(theDocument, j*NUM_REGIONS+i);
            }
    }

    while (pp->rl < pp->loadedRegions.size()) {
        if (pp->theImp->clearRegion(pp->loadedRegions.at(pp->rl), theDocument, this))
            pp->loadedRegions.removeAt(pp->rl);
        else
            ++pp->rl;
    }

    blockIndexing(false);

    if (intToAng(pp->theVP.lonDiff()) > M_PREFS->getRegionTo0Threshold()) {
        Layer::getFeatureSet(theFeatures, theCoastlines, theDocument,
                   invalidRects, clipRect, theProjection, theTransform);
    } else {
        OsbFeatureIterator oit(this);
        while (!oit.isEnd()) {
            Feature* F = oit.get();
            if (theFeatures[F->renderPriority()].contains(F))
                continue;
            for (int i=0; i < invalidRects.size(); ++i) {
                if (invalidRects[i].intersects(F->boundingBox())) {
                    if (Way * R = CAST_WAY(F)) {
                        R->buildPath(theProjection, theTransform, clipRect);
                        theFeatures[F->renderPriority()].insert(F);
                        if (R->isCoastline())
                            theCoastlines.insert(R);
                    } else
                    if (Relation * RR = CAST_RELATION(F)) {
                        RR->buildPath(theProjection, theTransform, clipRect);
                        theFeatures[F->renderPriority()].insert(F);
                    } else
                    if (Node * pt = CAST_NODE(F)) {
                        if (arePointsDrawable())
                            theFeatures[F->renderPriority()].insert(F);
                    } else
                        theFeatures[F->renderPriority()].insert(F);
                    break;
                }
            }
            ++oit;
        }
    }
}

//Feature*  OsbLayer::getFeatureByRef(Document* d, quint64 ref)
//{
//	return pp->theImp->getFeature(d, this, ref);
//}

bool OsbLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
    Q_UNUSED(progress);

    bool OK = true;

    if (pp->IsWorld)
        return OK;

    QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
    xParent.appendChild(e);

    e.setAttribute("xml:id", id());
    e.setAttribute("name", p->Name);
    e.setAttribute("alpha", QString::number(p->alpha,'f',2));
    e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
    e.setAttribute("selected", QString((p->selected ? "true" : "false")));
    e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
    e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

    e.setAttribute("filename", pp->theImp->getFilename());

    return OK;

//	bool OK = true;
//
//	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
//	xParent.appendChild(e);
//
//	e.setAttribute("xml:id", id());
//	e.setAttribute("name", p->Name);
//	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
//	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
//	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
//	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
//	e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));
//
//	QDomElement o = xParent.ownerDocument().createElement("osm");
//	e.appendChild(o);
//	o.setAttribute("version", "0.5");
//	o.setAttribute("generator", "Merkaartor");
//
//	if (p->Features.size()) {
//		QDomElement bb = xParent.ownerDocument().createElement("bound");
//		o.appendChild(bb);
//		CoordBox layBB = boundingBox();
//		QString S = QString().number(intToAng(layBB.bottomLeft().lat()),'f',6) + ",";
//		S += QString().number(intToAng(layBB.bottomLeft().lon()),'f',6) + ",";
//		S += QString().number(intToAng(layBB.topRight().lat()),'f',6) + ",";
//		S += QString().number(intToAng(layBB.topRight().lon()),'f',6);
//		bb.setAttribute("box", S);
//		bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
//	}
//
//	QList<MapFeaturePtr>::iterator it;
//	for(it = p->Features.begin(); it != p->Features.end(); it++)
//		(*it)->toXML(o, progress);
//
//	return OK;

}

OsbLayer * OsbLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog & progress)
{
    Q_UNUSED(progress);

    OsbLayer* l = new OsbLayer(e.attribute("name"));

    l->setId(e.attribute("xml:id"));
    l->setAlpha(e.attribute("alpha").toDouble());
    l->setVisible((e.attribute("visible") == "true" ? true : false));
    l->setSelected((e.attribute("selected") == "true" ? true : false));
    l->setEnabled((e.attribute("enabled") == "false" ? false : true));
    l->setReadonly((e.attribute("readonly") == "true" ? true : false));

    if (l->pp->theImp->loadFile(e.attribute("filename")))
        l->pp->theImp->import(l);
    else {
        delete l;
        return NULL;
    }

    d->add(l);
    return l;
}

QString OsbLayer::toHtml()
{
    QString S;

    S += "<i>"+QApplication::translate("OsbLayer", "# of loaded Regions")+": </i>" + QApplication::translate("OsbLayer", "%1").arg(QLocale().toString(pp->loadedRegions.size()))+"<br/>";
    S += "<i>"+QApplication::translate("OsbLayer", "# of loaded Tiles")+": </i>" + QApplication::translate("OsbLayer", "%1").arg(QLocale().toString(pp->loadedTiles.size()))+"<br/>";

    return toMainHtml().arg(S);
}
