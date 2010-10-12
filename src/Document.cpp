#include "Command.h"

#include "Feature.h"
#include "Document.h"
#include "ImageMapLayer.h"

#include "ImportNMEA.h"
#include "ImportExportOsmBin.h"
#include "ImportExportKML.h"
#include "ImportExportSHP.h"
#include "ImportExportCSV.h"
#include "ImportExportOSC.h"

#include "LayerWidget.h"

#include "Utils/TagSelector.h"
#include "PaintStyle/IPaintStyle.h"
#include "PaintStyle/FeaturePainter.h"

#include "LayerIterator.h"
#include "IMapAdapter.h"

#include <QtCore/QString>
#include <QMultiMap>
#include <QProgressDialog>

#include <QMap>
#include <QList>
#include <QMenu>

#include <QSet>

/* MAPDOCUMENT */

class MapDocumentPrivate
{
public:
    MapDocumentPrivate()
        : History(new CommandHistory())
        , dirtyLayer(0)
        , uploadedLayer(0)
        /*, trashLayer(0)*/
        , theDock(0)
        , lastDownloadLayer(0)
        , tagFilter(0), FilterRevision(0)
        , layerNum(0)
    {
    };
    ~MapDocumentPrivate()
    {
        History->cleanup();
        delete History;
        for (int i=0; i<Layers.size(); ++i) {
            if (theDock)
                theDock->deleteLayer(Layers[i]);
            Layers[i]->blockIndexing(true);
            delete Layers[i];
        }
    }
    CommandHistory*				History;
    QList<Layer*>		Layers;
    DirtyLayer*				dirtyLayer;
    UploadedLayer*			uploadedLayer;
    LayerDock*					theDock;
    Layer*					lastDownloadLayer;
    QHash<Layer*, CoordBox>		downloadBoxes;

    QStringList tagKeys;
    QStringList tagValues;
    QHash< quint32, QList<quint32> >		tagList;
    TagSelector* tagFilter;
    int FilterRevision;
    QString title;
    int layerNum;
    mutable QString Id;

    QList<FeaturePainter> theFeaturePainters;

};

Document::Document()
    : p(new MapDocumentPrivate)
{
    setFilterType(M_PREFS->getCurrentFilter());
    p->title = tr("untitled");

    for (int i=0; i<M_STYLE->painterSize(); ++i) {
        p->theFeaturePainters.append(FeaturePainter(*M_STYLE->getPainter(i)));
    }
}

Document::Document(LayerDock* aDock)
    : p(new MapDocumentPrivate)
{
    p->theDock = aDock;
    setFilterType(M_PREFS->getCurrentFilter());
    p->title = tr("untitled");

    for (int i=0; i<M_STYLE->painterSize(); ++i) {
        p->theFeaturePainters.append(FeaturePainter(*M_STYLE->getPainter(i)));
    }
}

Document::Document(const Document&, LayerDock*)
: p(0)
{
    p->title = tr("untitled");
}

Document::~Document()
{
    delete p;
}

const QString& Document::id() const
{
    if (p->Id.isEmpty())
        p->Id = QUuid::createUuid().toString();
    return p->Id;
}

void Document::setPainters(QList<Painter> aPainters)
{
    p->theFeaturePainters.clear();
    for (int i=0; i<aPainters.size(); ++i) {
        FeaturePainter fp(aPainters[i]);
        p->theFeaturePainters.append(fp);
    }
}

int Document::getPaintersSize()
{
    return p->theFeaturePainters.size();
}

const Painter* Document::getPainter(int i)
{
    return &p->theFeaturePainters[i];
}

void Document::addDefaultLayers()
{
    ImageMapLayer*l = addImageLayer();
//    l->setMapAdapter(M_PREFS->getBackgroundPlugin(), M_PREFS->getSelectedServer());
//    if (M_PREFS->getBackgroundPlugin() != NONE_ADAPTER_UUID) {
//        l->setVisible(M_PREFS->getBgVisible());
//        // Sync the menu entry label & visible checkbox to the layer
//        QMenu *lMenu = l->getWidget()->getAssociatedMenu();
//        lMenu->menuAction()->menu()->actions().at(0)->setChecked(M_PREFS->getBgVisible());
//        lMenu->setTitle(l->name());
//    }

    if (g_Merk_Frisius) {
        DrawingLayer* aLayer = addDrawingLayer();
        setLastDownloadLayer(aLayer);
    } else {
        p->dirtyLayer = new DirtyLayer(tr("Dirty layer"));
        add(p->dirtyLayer);

        p->uploadedLayer = new UploadedLayer(tr("Uploaded layer"));
        add(p->uploadedLayer);
    }

    addFilterLayers();
}

void Document::addFilterLayers()
{
    foreach (FilterItem it, *M_PREFS->getFiltersList()->getFilters()) {
        if (it.deleted)
            continue;
        FilterLayer* f = new FilterLayer(it.id.toString(), it.name, it.filter);
        addFilterLayer(f);
    }
}

bool Document::toXML(QDomElement xParent, bool asTemplate, QProgressDialog * progress)
{
    bool OK = true;

    QDomElement mapDoc = xParent.namedItem("MapDocument").toElement();
    if (!mapDoc.isNull()) {
        xParent.removeChild(mapDoc);
    }
    mapDoc = xParent.ownerDocument().createElement("MapDocument");
    xParent.appendChild(mapDoc);

    mapDoc.setAttribute("xml:id", id());
    mapDoc.setAttribute("layernum", p->layerNum);
    if (p->lastDownloadLayer)
        mapDoc.setAttribute("lastdownloadlayer", p->lastDownloadLayer->id());

    for (int i=0; i<p->Layers.size(); ++i) {
        progress->setMaximum(progress->maximum() + p->Layers[i]->getDisplaySize());
    }

    for (int i=0; i<p->Layers.size(); ++i) {
        if (p->Layers[i]->isEnabled()) {
            p->Layers[i]->toXML(mapDoc, asTemplate, progress);
        }
    }

    if (!asTemplate)
        OK = history().toXML(mapDoc, progress);

    return OK;
}

Document* Document::fromXML(QString title, const QDomElement e, double version, LayerDock* aDock, QProgressDialog * progress)
{
    Document* NewDoc = new Document(aDock);
    NewDoc->p->title = title;

    CommandHistory* h = 0;

    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "ImageMapLayer") {
            /*ImageMapLayer* l =*/ ImageMapLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "DeletedMapLayer") {
            /*DeletedMapLayer* l =*/ DeletedLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "DirtyLayer" || c.tagName() == "DirtyMapLayer") {
            /*DirtyMapLayer* l =*/ DirtyLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "UploadedLayer" || c.tagName() == "UploadedMapLayer") {
            /*UploadedMapLayer* l =*/ UploadedLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "DrawingLayer" || c.tagName() == "DrawingMapLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "TrackLayer" || c.tagName() == "TrackMapLayer") {
            /*TrackMapLayer* l =*/ TrackLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "ExtractedLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "OsbLayer" || c.tagName() == "OsbMapLayer") {
            /*OsbMapLayer* l =*/ OsbLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "FilterLayer") {
            /*OsbMapLayer* l =*/ FilterLayer::fromXML(NewDoc, c, progress);
        } else if (c.tagName() == "CommandHistory") {
            if (version > 1.0)
                h = CommandHistory::fromXML(NewDoc, c, progress);
        }

        if (progress->wasCanceled())
            break;

        c = c.nextSiblingElement();
    }

    if (progress->wasCanceled()) {
        delete NewDoc;
        NewDoc = NULL;
    }

    if (NewDoc) {
        if (e.hasAttribute("xml:id"))
            NewDoc->p->Id = e.attribute("xml:id");
        if (e.hasAttribute("layernum"))
            NewDoc->p->layerNum = e.attribute("layernum").toInt();
        else
            NewDoc->p->layerNum = 1;
        if (e.hasAttribute("lastdownloadlayer"))
            NewDoc->setLastDownloadLayer(NewDoc->getLayer(e.attribute("lastdownloadlayer")));

        if (h)
            NewDoc->setHistory(h);

        for (int i=0; i<NewDoc->layerSize(); ++i) {
            NewDoc->getLayer(i)->reIndex(progress);
        }
    }

    return NewDoc;
}

void Document::setLayerDock(LayerDock* aDock)
{
    p->theDock = aDock;
}

LayerDock* Document::getLayerDock(void)
{
    return p->theDock;
}

void Document::clear()
{
    delete p;
    p = new MapDocumentPrivate;
    addDefaultLayers();
}

void Document::setHistory(CommandHistory* h)
{
    delete p->History;
    p->History = h;
    emit(historyChanged());
}

CommandHistory& Document::history()
{
    return *(p->History);
}

const CommandHistory& Document::history() const
{
    return *(p->History);
}

void Document::addHistory(Command* aCommand)
{
    p->History->add(aCommand);
    emit(historyChanged());
}

void Document::redoHistory()
{
    p->History->redo();
    emit(historyChanged());
}

void Document::undoHistory()
{
    p->History->undo();
    emit(historyChanged());
}

void Document::add(Layer* aLayer)
{
    p->Layers.push_back(aLayer);
    aLayer->setDocument(this);
    if (p->theDock)
        p->theDock->addLayer(aLayer);
}

void Document::moveLayer(Layer* aLayer, int pos)
{
    p->Layers.move(p->Layers.indexOf(aLayer), pos);
}

ImageMapLayer* Document::addImageLayer(ImageMapLayer* aLayer)
{
    ImageMapLayer* theLayer = aLayer;
    if (!theLayer)
        theLayer = new ImageMapLayer(tr("Background imagery"));
    add(theLayer);

    connect(theLayer, SIGNAL(imageRequested(ImageMapLayer*)),
        this, SLOT(on_imageRequested(ImageMapLayer*)), Qt::QueuedConnection);
    connect(theLayer, SIGNAL(imageReceived(ImageMapLayer*)),
        this, SLOT(on_imageReceived(ImageMapLayer*)), Qt::QueuedConnection);
    connect(theLayer, SIGNAL(loadingFinished(ImageMapLayer*)),
        this, SLOT(on_loadingFinished(ImageMapLayer*)), Qt::QueuedConnection);

    return theLayer;
}

DrawingLayer* Document::addDrawingLayer(DrawingLayer *aLayer)
{
    DrawingLayer* theLayer = aLayer;
    if (!theLayer)
        theLayer = new DrawingLayer(tr("Drawing layer #%1").arg(++p->layerNum));
    add(theLayer);
    return theLayer;
}

FilterLayer* Document::addFilterLayer(FilterLayer *aLayer)
{
    FilterLayer* theLayer = aLayer;
    if (!theLayer)
        theLayer = new FilterLayer(QUuid::createUuid(), tr("Filter layer #%1").arg(++p->layerNum), "false");
    add(theLayer);

    FeatureIterator it(this);
    for(;!it.isEnd(); ++it) {
        it.get()->updateFilters();
    }

    return theLayer;
}

QPair<quint32, quint32> Document::addToTagList(QString k, QString v)
{
    qint32 ik, iv;

    ik = p->tagKeys.indexOf(k);
    if (ik == -1) {
        p->tagKeys.append(k);
        ik = p->tagKeys.size()-1;
    }
    iv = p->tagValues.indexOf(v);
    if (iv == -1) {
        p->tagValues.append(v);
        iv = p->tagValues.size()-1;
    }

    if (!k.isEmpty() && !v.isEmpty())
        p->tagList[ik].append(iv);

    return qMakePair((quint32)ik, (quint32)iv);
}

void Document::removeFromTagList(quint32 k, quint32 v)
{
    p->tagList[k].removeOne(v);
    if (p->tagList[k].isEmpty())
        p->tagList.remove(k);
}

QList<QString> Document::getTagKeys()
{
    return p->tagKeys;
}

QList<QString> Document::getTagValues()
{
    return p->tagValues;
}

QStringList Document::getTagValueList(QString k)
{
    QSet<quint32> retList;
    if (k == "*") {
        foreach (QList<quint32> list, p->tagList)
            retList.unite(list.toSet());
    } else
        retList = p->tagList[p->tagKeys.indexOf(k)].toSet();

    QStringList res;
    foreach (quint32 i, retList)
        res << getTagValue(i);

    return res;
}

QString Document::getTagKey(int idx)
{
    return p->tagKeys.at(idx);
}

quint32 Document::getTagKeyIndex(const QString& s)
{
    return p->tagKeys.indexOf(s);
}

QStringList Document::getTagKeyList()
{
    return p->tagKeys.toSet().toList();
}

QString Document::getTagValue(int idx)
{
    return p->tagValues.at(idx);
}

quint32 Document::getTagValueIndex(const QString& s)
{
    return p->tagValues.indexOf(s);
}

void Document::remove(Layer* aLayer)
{
    QList<Layer*>::iterator i = qFind(p->Layers.begin(),p->Layers.end(), aLayer);
    if (i != p->Layers.end()) {
        p->Layers.erase(i);
    }
    if (aLayer == p->lastDownloadLayer)
        p->lastDownloadLayer = NULL;
    if (p->theDock)
        p->theDock->deleteLayer(aLayer);
}

bool Document::exists(Layer* L) const
{
    for (int i=0; i<p->Layers.size(); ++i)
        if (p->Layers[i] == L) return true;
    return false;
}

bool Document::exists(Feature* F) const
{
    for (int i=0; i<p->Layers.size(); ++i)
        if (p->Layers[i]->exists(F)) return true;
    return false;
}

void Document::deleteFeature(Feature* aFeature)
{
    for (int i=0; i<p->Layers.size(); ++i)
        if (p->Layers[i]->exists(aFeature)) {
            p->Layers[i]->deleteFeature(aFeature);
            return;
        }
}

int Document::layerSize() const
{
    return p->Layers.size();
}

Layer* Document::getLayer(const QString& id)
{
    for (int i=0; i<p->Layers.size(); ++i)
    {
        if (p->Layers[i]->id() == id) return p->Layers[i];
    }
    return 0;
}

Layer* Document::getLayer(int i)
{
    return p->Layers.at(i);
}

const Layer* Document::getLayer(int i) const
{
    return p->Layers[i];
}

QList<Feature*> Document::getFeatures(Layer::LayerType layerType)
{
    QList<Feature*> theFeatures;
    for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
        if (!layerType)
            theFeatures.append(i.get());
        else
            if (i.get()->layer()->classType() == layerType)
                theFeatures.append(i.get());
    }
    return theFeatures;
}

Feature* Document::getFeature(const QString& id, bool exact)
{
    for (int i=0; i<p->Layers.size(); ++i)
    {
        Feature* F = p->Layers[i]->get(id);
        if (F)
            return F;
        if (!exact) {
            if ((F = p->Layers[i]->get("node_"+id)))
                return F;
            if ((F = p->Layers[i]->get("way_"+id)))
                return F;
            if ((F = p->Layers[i]->get("rel_"+id)))
                return F;
        }
    }
    return 0;
}

void Document::setDirtyLayer(DirtyLayer* aLayer)
{
    p->dirtyLayer = aLayer;
}

Layer* Document::getDirtyLayer()
{
    if (!p->dirtyLayer) {
        p->dirtyLayer = new DirtyLayer(tr("Dirty layer"));
        add(p->dirtyLayer);
    }
    return p->dirtyLayer;
}

Layer* Document::getDirtyOrOriginLayer(Layer* aLayer)
{
    if (g_Merk_Frisius) {
        if (aLayer)
            return aLayer;
        else {
            DrawingLayer* firstDrLayer = NULL;
            for (int i=0; i<layerSize(); ++i) {
                if (getLayer(i)->classType() == Layer::DrawingLayerType) {
                    if (!firstDrLayer)
                        firstDrLayer = dynamic_cast<DrawingLayer*>(getLayer(i));
                    if (getLayer(i)->isSelected())
                        return (Layer*)getLayer(i);
                }
            }
            if (firstDrLayer)
                return firstDrLayer;
            else
                return addDrawingLayer();
        }
    }

    if (!aLayer || aLayer->isUploadable())
        return p->dirtyLayer;
    else
        return aLayer;
}

Layer* Document::getDirtyOrOriginLayer(Feature* F)
{
    if (g_Merk_Frisius) {
        return getDirtyOrOriginLayer(F->layer());
    }

    if (!F || !F->layer() || F->layer()->isUploadable())
        return p->dirtyLayer;
    else
        return F->layer();
}

int Document::getDirtySize() const
{
    int dirtyObjects = 0;
    for (int i=0; i<layerSize(); ++i) {
        dirtyObjects += getLayer(i)->getDirtySize();
    }
    return dirtyObjects;
}

void Document::setUploadedLayer(UploadedLayer* aLayer)
{
    p->uploadedLayer = aLayer;
}


UploadedLayer* Document::getUploadedLayer() const
{
    return p->uploadedLayer;
}

QString Document::exportOSM(QMainWindow* main, const CoordBox& aCoordBox, bool renderBounds)
{
    QList<Feature*> theFeatures;

    for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
        if (Node* P = dynamic_cast<Node*>(i.get())) {
            if (aCoordBox.contains(P->position())) {
                theFeatures.append(P);
            }
        } else
            if (Way* G = dynamic_cast<Way*>(i.get())) {
                if (aCoordBox.intersects(G->boundingBox())) {
                    for (int j=0; j < G->size(); j++) {
                        if (Node* P = dynamic_cast<Node*>(G->get(j)))
                            if (!aCoordBox.contains(P->position()))
                                theFeatures.append(P);
                    }
                    theFeatures.append(G);
                }
            } else
                //FIXME Not working for relation (not made of point?)
                if (Relation* G = dynamic_cast<Relation*>(i.get())) {
                    if (aCoordBox.intersects(G->boundingBox())) {
                        for (int j=0; j < G->size(); j++) {
                            if (Way* R = dynamic_cast<Way*>(G->get(j))) {
                                if (!aCoordBox.contains(R->boundingBox())) {
                                    for (int k=0; k < R->size(); k++) {
                                        if (Node* P = dynamic_cast<Node*>(R->get(k)))
                                            if (!aCoordBox.contains(P->position()))
                                                theFeatures.append(P);
                                    }
                                    theFeatures.append(R);
                                }
                            }
                        }
                        theFeatures.append(G);
                    }
                }
    }

    QList<Feature*> exportedFeatures = exportCoreOSM(theFeatures);

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(main);
    if (!aProgressWindow)
        return "";

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    dlg->setWindowTitle(tr("OSM Export"));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    Bar->setTextVisible(false);
    Bar->setMaximum(exportedFeatures.size());

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    Lbl->setText(tr("Exporting OSM..."));

    if (dlg)
        dlg->show();

    QDomDocument theXmlDoc;
    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement o = theXmlDoc.createElement("osm");
    theXmlDoc.appendChild(o);
    o.setAttribute("version", "0.6");
    o.setAttribute("generator", QString("%1 %2").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)));

    QDomElement bb = theXmlDoc.createElement("bound");
    o.appendChild(bb);
    QString S = QString().number(coordToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.topRight().lat()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.topRight().lon()),'f',6);
    bb.setAttribute("box", S);
    bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));

    if (renderBounds) {
        QDomElement bnds = theXmlDoc.createElement("bounds");
        o.appendChild(bnds);

        bnds.setAttribute("minlat", COORD2STRING(coordToAng(aCoordBox.bottomLeft().lat())));
        bnds.setAttribute("minlon", COORD2STRING(coordToAng(aCoordBox.bottomLeft().lon())));
        bnds.setAttribute("maxlat", COORD2STRING(coordToAng(aCoordBox.topRight().lat())));
        bnds.setAttribute("maxlon", COORD2STRING(coordToAng(aCoordBox.topRight().lon())));
    }

    for (int i=0; i < exportedFeatures.size(); i++) {
        exportedFeatures[i]->toXML(o, dlg);
    }

    return theXmlDoc.toString(2);
}

QString Document::exportOSM(QMainWindow* main, QList<Feature*> aFeatures)
{
    QList<Feature*> exportedFeatures = exportCoreOSM(aFeatures);
    CoordBox aCoordBox;

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(main);
    if (!aProgressWindow)
        return "";

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    if (dlg)
        dlg->setWindowTitle(tr("OSM Export"));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    if (Bar) {
        Bar->setTextVisible(false);
        Bar->setMaximum(exportedFeatures.size());
    }

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    if (Lbl)
        Lbl->setText(tr("Exporting OSM..."));

    if (dlg)
        dlg->show();

    QDomDocument theXmlDoc;
    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement o = theXmlDoc.createElement("osm");
    theXmlDoc.appendChild(o);
    o.setAttribute("version", "0.6");
    o.setAttribute("generator", QString("%1 %2").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)));

    if (exportedFeatures.size()) {
        aCoordBox = exportedFeatures[0]->boundingBox();
        exportedFeatures[0]->toXML(o, dlg);
        for (int i=1; i < exportedFeatures.size(); i++) {
            aCoordBox.merge(exportedFeatures[i]->boundingBox());
            exportedFeatures[i]->toXML(o, dlg);
        }
    }

    QDomElement bb = theXmlDoc.createElement("bound");
    o.appendChild(bb);
    QString S = QString().number(coordToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.topRight().lat()),'f',6) + ",";
    S += QString().number(coordToAng(aCoordBox.topRight().lon()),'f',6);
    bb.setAttribute("box", S);
    bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));

    return theXmlDoc.toString(2);
}

QList<Feature*> Document::exportCoreOSM(QList<Feature*> aFeatures)
{
    QList<Feature*> exportedFeatures;
    QList<Feature*>::Iterator i;

    for (i = aFeatures.begin(); i != aFeatures.end(); ++i) {
        if (/*Node* n = */dynamic_cast<Node*>(*i)) {
            if (!exportedFeatures.contains(*i))
                exportedFeatures.append(*i);
        } else {
            if (Way* G = dynamic_cast<Way*>(*i)) {
                for (int j=0; j < G->size(); j++) {
                    if (Node* P = dynamic_cast<Node*>(G->get(j))) {
                        if (!exportedFeatures.contains(P))
                            exportedFeatures.append(P);
                    }
                    if (!exportedFeatures.contains(G))
                        exportedFeatures.append(G);
                }
            } else {
                //FIXME Not working for relation (not made of point?)
                if (Relation* G = dynamic_cast<Relation*>(*i)) {
                    for (int j=0; j < G->size(); j++) {
                        if (Way* R = dynamic_cast<Way*>(G->get(j))) {
                            for (int k=0; k < R->size(); k++) {
                                if (Node* P = dynamic_cast<Node*>(R->get(k))) {
                                    if (!exportedFeatures.contains(P))
                                        exportedFeatures.append(P);
                                }
                            }
                        if (!exportedFeatures.contains(R))
                            exportedFeatures.append(R);
                        }
                    }
                    if (!exportedFeatures.contains(G))
                        exportedFeatures.append(G);
                }
            }
        }
    }

    return exportedFeatures;
}

bool Document::importNMEA(const QString& filename, TrackLayer* NewLayer)
{
    ImportNMEA imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
}

bool Document::importOSC(const QString& filename, DrawingLayer* NewLayer)
{
    ImportExportOSC imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
}

bool Document::importKML(const QString& filename, TrackLayer* NewLayer)
{
    ImportExportKML imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
}

bool Document::importSHP(const QString& filename, DrawingLayer* NewLayer)
{
    Q_UNUSED(filename)
    Q_UNUSED(NewLayer)

#ifdef USE_GDAL
    ImportExportSHP imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
#else
    return false;
#endif
}

bool Document::importCSV(const QString& filename, DrawingLayer* NewLayer)
{
    ImportExportCSV imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
}

bool Document::importOSB(const QString& filename, DrawingLayer* NewLayer)
{
    Q_UNUSED(filename)
    Q_UNUSED(NewLayer)
    //ImportExportOsmBin imp(this);
    //if (!imp.loadFile(filename))
    //	return false;
    //imp.import(NewLayer);

    //if (NewLayer->size())
    //	return true;
    //else
    //	return false;
    return true;
}

void Document::addDownloadBox(Layer* l, CoordBox aBox)
{
    p->downloadBoxes.insertMulti(l, aBox);
}

void Document::removeDownloadBox(Layer* l)
{
    p->downloadBoxes.remove(l);
}

const QList<CoordBox> Document::getDownloadBoxes() const
{
    return p->downloadBoxes.values();
}

Layer * Document::getLastDownloadLayer() const
{
    return p->lastDownloadLayer;
}

void Document::setLastDownloadLayer(Layer * aLayer)
{
    p->lastDownloadLayer = aLayer;
}

void Document::on_imageRequested(ImageMapLayer* anImageLayer)
{
    emit imageRequested(anImageLayer);
}

void Document::on_imageReceived(ImageMapLayer* anImageLayer)
{
    emit imageReceived(anImageLayer);
}

void Document::on_loadingFinished(ImageMapLayer* anImageLayer)
{
    emit loadingFinished(anImageLayer);
}

QPair<bool,CoordBox> Document::boundingBox()
{
    int First;
    for (First = 0; First < layerSize(); ++First)
        if (getLayer(First)->size())
            break;
    if (First == layerSize())
        return qMakePair(false,CoordBox(Coord(0,0),Coord(0,0)));
    Layer* aLayer = getLayer(First);
    CoordBox BBox = aLayer->boundingBox();
    for (int i=First+1; i<layerSize(); ++i)
        aLayer = getLayer(i);
        if (aLayer->size())
            BBox.merge(aLayer->boundingBox());
    return qMakePair(true,BBox);
}

bool Document::hasUnsavedChanges()
{
//	return aDoc.history().index();
    return (getDirtySize() > 0);
}

bool Document::setFilterType(FilterType aFilter)
{
    p->FilterRevision++;
    QString theFilter = M_PREFS->getFilter(aFilter).filter;
    if (theFilter.isEmpty()) {
        if (p->tagFilter)
            SAFE_DELETE(p->tagFilter);
        return true;
    }
    p->tagFilter = TagSelector::parse(M_PREFS->getFilter(aFilter).filter);
    return (p->tagFilter != NULL);
}

TagSelector* Document::getTagFilter()
{
    return p->tagFilter;
}

int Document::filterRevision() const
{
    return p->FilterRevision;
}

QString Document::title() const
{
    return p->title;
}

void Document::setTitle(const QString aTitle)
{
    p->title = aTitle;
}

QString Document::toPropertiesHtml()
{
    QString h;

    h += "<big><strong>" + tr("Document") + "</strong></big><hr/>";
    for (int i=0; i<p->Layers.size(); ++i) {
        h += p->Layers[i]->toPropertiesHtml() + "<br/>";
    }
    h += "";

    return h;
}

QStringList Document::getCurrentSourceTags()
{
    QStringList theSrc;
    for (LayerIterator<ImageMapLayer*> ImgIt(this); !ImgIt.isEnd(); ++ImgIt) {
        if (ImgIt.get()->isVisible()) {
            QString s = ImgIt.get()->getMapAdapter()->getSourceTag();
            if (!s.isEmpty())
                theSrc << ImgIt.get()->getMapAdapter()->getSourceTag();
        }
    }
    return theSrc;
}

/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(Document *aDoc)
: theDocument(aDoc), curLayerIdx(0), curFeatureIdx(0), isAtEnd(false)
{
    docSize = theDocument->layerSize();
    curLayerSize = theDocument->getLayer(curLayerIdx)->size();

    if(!check() && !isAtEnd)
        ++(*this);
}

FeatureIterator::~FeatureIterator()
{
}

Feature* FeatureIterator::get()
{
    return theDocument->getLayer(curLayerIdx)->get(curFeatureIdx);
}

bool FeatureIterator::isEnd() const
{
    return isAtEnd;
}

FeatureIterator& FeatureIterator::operator++()
{
    docSize = theDocument->layerSize();
    curLayerSize = theDocument->getLayer(curLayerIdx)->size();

    if (curFeatureIdx < curLayerSize-1)
        curFeatureIdx++;
    else
        if (curLayerIdx < docSize-1) {
            curLayerIdx++;
            curLayerSize = theDocument->getLayer(curLayerIdx)->size();
            curFeatureIdx = 0;
        } else
            isAtEnd = true;

    while(!isAtEnd && !check()) {
        if (curFeatureIdx < curLayerSize-1)
            curFeatureIdx++;
        else
            if (curLayerIdx < docSize-1) {
                curLayerIdx++;
                curLayerSize = theDocument->getLayer(curLayerIdx)->size();
                curFeatureIdx = 0;
            } else
                isAtEnd = true;
    }

    return *this;
}

int FeatureIterator::index()
{
    return (curLayerIdx*10000000)+curFeatureIdx;
}

bool FeatureIterator::check()
{
    if (curLayerIdx >= docSize) {
        isAtEnd = true;
        return false;
    }
    if (curFeatureIdx >= curLayerSize)
        return false;

    Feature* curFeature = theDocument->getLayer(curLayerIdx)->get(curFeatureIdx);
    if (curFeature->lastUpdated() == Feature::NotYetDownloaded
            || curFeature->isDeleted() || curFeature->isVirtual())
        return false;

    return true;
}


/* VISIBLEFEATUREITERATOR */

VisibleFeatureIterator::VisibleFeatureIterator(Document *aDoc)
: FeatureIterator(aDoc)
{
    if(!check() && !isAtEnd)
        ++(*this);
}

VisibleFeatureIterator::~VisibleFeatureIterator()
{
}

bool VisibleFeatureIterator::check()
{
    if (!FeatureIterator::check())
        return false;
    else if (theDocument->getLayer(curLayerIdx)->get(curFeatureIdx)->isHidden())
        return false;
    else if (CAST_NODE(theDocument->getLayer(curLayerIdx)->get(curFeatureIdx))
            && !(theDocument->getLayer(curLayerIdx)->arePointsDrawable()))
                return false;

    return true;
}


/* RELATED */


