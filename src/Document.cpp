#include "Command.h"

#include "Feature.h"
#include "ImportOSM.h"
#include "Document.h"
#include "ImageMapLayer.h"

#include "ImportNMEA.h"
#include "ImportExportOsmBin.h"
#include "ImportExportKML.h"
#include "ImportExportSHP.h"

#include "LayerWidget.h"

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
    : History(new CommandHistory()), dirtyLayer(0), uploadedLayer(0)/*, trashLayer(0)*/, theDock(0), lastDownloadLayer(0)
    {
    };
    ~MapDocumentPrivate()
    {
        History->cleanup();
        delete History;
        for (int i=0; i<Layers.size(); ++i) {
            if (theDock)
                theDock->deleteLayer(Layers[i]);
            Layers[i]->	blockIndexing(true);
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

    QHash< QString, QSet<QString> * >		tagList;

};

Document::Document()
    : p(new MapDocumentPrivate)
{
    if (!(M_PREFS->apiVersionNum() > 0.5))
        addToTagList("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
}

Document::Document(LayerDock* aDock)
: p(new MapDocumentPrivate)
{
    p->theDock = aDock;
}

Document::Document(const Document&, LayerDock*)
: p(0)
{
}

Document::~Document()
{
    delete p;
}

void Document::addDefaultLayers()
{
    ImageMapLayer*l = addImageLayer();
    l->setMapAdapter(M_PREFS->getBackgroundPlugin(), M_PREFS->getSelectedServer());
    if (M_PREFS->getBackgroundPlugin() != NONE_ADAPTER_UUID) {
        l->setVisible(M_PREFS->getBgVisible());
        // Sync the menu entry label & visible checkbox to the layer
        QMenu *lMenu = l->getWidget()->getAssociatedMenu();
        lMenu->menuAction()->menu()->actions().at(0)->setChecked(M_PREFS->getBgVisible());
        lMenu->setTitle(l->name());
    }

    p->dirtyLayer = new DirtyLayer(tr("Dirty layer"));
    add(p->dirtyLayer);

    p->uploadedLayer = new UploadedLayer(tr("Uploaded layer"));
    add(p->uploadedLayer);
}

bool Document::toXML(QDomElement xParent, QProgressDialog & progress)
{
    bool OK = true;

    QDomElement mapDoc = xParent.namedItem("MapDocument").toElement();
    if (!mapDoc.isNull()) {
        xParent.removeChild(mapDoc);
    }
    mapDoc = xParent.ownerDocument().createElement("MapDocument");
    xParent.appendChild(mapDoc);

    if (p->lastDownloadLayer)
        mapDoc.setAttribute("lastdownloadlayer", p->lastDownloadLayer->id());

    for (int i=0; i<p->Layers.size(); ++i) {
        progress.setMaximum(progress.maximum() + p->Layers[i]->size());
    }

    for (int i=0; i<p->Layers.size(); ++i) {
        p->Layers[i]->toXML(mapDoc, progress);
    }

    OK = history().toXML(mapDoc, progress);

    return OK;
}

Document* Document::fromXML(const QDomElement e, double version, LayerDock* aDock, QProgressDialog & progress)
{
    Document* NewDoc = new Document(aDock);

    CommandHistory* h = 0;

    QDomElement c = e.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "ImageMapLayer") {
            /*ImageMapLayer* l =*/ ImageMapLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "DeletedMapLayer") {
            /*DeletedMapLayer* l =*/ DeletedLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "DirtyLayer" || c.tagName() == "DirtyMapLayer") {
            /*DirtyMapLayer* l =*/ DirtyLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "UploadedLayer" || c.tagName() == "UploadedMapLayer") {
            /*UploadedMapLayer* l =*/ UploadedLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "DrawingLayer" || c.tagName() == "DrawingMapLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "TrackLayer" || c.tagName() == "TrackMapLayer") {
            /*TrackMapLayer* l =*/ TrackLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "ExtractedLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "OsbLayer" || c.tagName() == "OsbMapLayer") {
            /*OsbMapLayer* l =*/ OsbLayer::fromXML(NewDoc, c, progress);
        } else
        if (c.tagName() == "CommandHistory") {
            if (version > 1.0)
                h = CommandHistory::fromXML(NewDoc, c, progress);
        }

        if (progress.wasCanceled())
            break;

        c = c.nextSiblingElement();
    }

    if (progress.wasCanceled()) {
        delete NewDoc;
        NewDoc = NULL;
    }

    if (NewDoc) {
        if (e.hasAttribute("lastdownloadlayer"))
            NewDoc->setLastDownloadLayer(NewDoc->getLayer(e.attribute("lastdownloadlayer")));

        if (h)
            NewDoc->setHistory(h);
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

void Document::addToTagList(QString k, QString v)
{
#ifndef _MOBILE
    if (p->tagList.contains(k)) {
        if (!p->tagList.value(k)->contains(v)) {
            //static_cast< QSet<QString> * >(p->tagList.value(k))->insert(v);
            p->tagList.value(k)->insert(v);
        }
    } else {
        QSet<QString> *values = new QSet<QString>;
        values->insert(v);
        p->tagList.insert(k, values);
    }
#endif
}

QList<QString> Document::getTagKeys()
{
    return p->tagList.keys();
}

QList<QString> Document::getTagValues()
{
    return getTagValueList("*");
}

QStringList Document::getTagList()
{
    qDebug() << p->tagList.uniqueKeys() << endl;
    return p->tagList.uniqueKeys();
}

QStringList Document::getTagValueList(QString k)
{
    if (k == "*") {
        QSet<QString> allValues;
        QSet<QString> *tagValues;
        foreach (tagValues, p->tagList) {
            allValues += *tagValues;
        }
        return allValues.toList();
    } else if (p->tagList.contains(k)) {
        return p->tagList.value(k)->toList();
    } else {
        return QStringList();
    }
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

Layer* Document::getDirtyOrOriginLayer(Layer* aLayer) const
{
    if (!aLayer || aLayer->isUploadable())
        return p->dirtyLayer;
    else
        return aLayer;
}

Layer* Document::getDirtyOrOriginLayer(Feature* F) const
{
    if (!F || !F->layer() || F->layer()->isUploadable())
        return p->dirtyLayer;
    else
        return F->layer();
}

void Document::setUploadedLayer(UploadedLayer* aLayer)
{
    p->uploadedLayer = aLayer;
}


UploadedLayer* Document::getUploadedLayer() const
{
    return p->uploadedLayer;
}

QString Document::exportOSM(const CoordBox& aCoordBox, bool renderBounds)
{
    QString theExport, coreExport;
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

    if (exportedFeatures.size()) {
        for (int i=0; i < exportedFeatures.size(); i++) {
            coreExport += exportedFeatures[i]->toXML(1) + "\n";
        }
    }
    theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
    theExport += QString("<osm version='%1' generator='Merkaartor'>\n").arg(M_PREFS->apiVersion());
    theExport += "<bound box='";
    theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
    theExport += QString("' origin='http://www.openstreetmap.org/api/%1' />\n").arg(M_PREFS->apiVersion());
    if (renderBounds) {
        theExport += "<bounds ";
        theExport += "minlat=\"" + QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + "\" ";
        theExport += "minlon=\"" + QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + "\" ";
        theExport += "maxlat=\"" + QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + "\" ";
        theExport += "maxlon=\"" + QString().number(intToAng(aCoordBox.topRight().lon()),'f',6) + "\" ";
        theExport += "/>\n";
    }
    theExport += coreExport;
    theExport += "</osm>";

    return theExport;
}

QString Document::exportOSM(QList<Feature*> aFeatures)
{
    QString theExport, coreExport;
    QList<Feature*> exportedFeatures = exportCoreOSM(aFeatures);
    CoordBox aCoordBox;

    if (exportedFeatures.size()) {
        aCoordBox = exportedFeatures[0]->boundingBox();
        coreExport += exportedFeatures[0]->toXML(1) + "\n";
        for (int i=1; i < exportedFeatures.size(); i++) {
            aCoordBox.merge(exportedFeatures[i]->boundingBox());
            coreExport += exportedFeatures[i]->toXML(1) + "\n";
        }
    }
    theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
    theExport += QString("<osm version='%1' generator='Merkaartor'>\n").arg(M_PREFS->apiVersion());
    theExport += "<bound box='";
    theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
    theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
    theExport += QString("' origin='http://www.openstreetmap.org/api/%1' />\n").arg(M_PREFS->apiVersion());
    theExport += coreExport;
    theExport += "</osm>";

    return theExport;
}

QList<Feature*> Document::exportCoreOSM(QList<Feature*> aFeatures)
{
    QString coreExport;
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

Layer * Document::getLastDownloadLayer()
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
    return (getDirtyOrOriginLayer()->getDirtySize() > 0);
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
    else {
        if (theDocument->getLayer(curLayerIdx)->isVisible()) {
            if (CAST_NODE(theDocument->getLayer(curLayerIdx)->get(curFeatureIdx))
                    && !(theDocument->getLayer(curLayerIdx)->arePointsDrawable()))
                return false;
        } else
            return false;
    }

    return true;
}


/* RELATED */


