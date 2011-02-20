#include "Global.h"

#include "Command.h"

#include "Feature.h"
#include "Document.h"
#include "ImageMapLayer.h"

#include "ImportNMEA.h"
#include "ImportExportKML.h"
#include "ImportExportCSV.h"
#include "ImportExportOSC.h"
#include "ImportExportGdal.h"
#ifdef USE_PROTOBUF
#include "ImportExportPBF.h"
#endif

#include "LayerWidget.h"

#include "Utils/TagSelector.h"
#include "PaintStyle/IPaintStyle.h"
#include "PaintStyle/FeaturePainter.h"

#include "LayerIterator.h"
#include "IMapAdapter.h"


#include <QString>
#include <QMultiMap>
#include <QProgressDialog>
#include <QClipboard>
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
            delete Layers[i];
        }
    }
    CommandHistory*	History;
    QList<Layer*> Layers;
    DirtyLayer*	dirtyLayer;
    UploadedLayer* uploadedLayer;
    LayerDock*	theDock;
    Layer*	lastDownloadLayer;
    QDateTime lastDownloadTimestamp;
    QHash<Layer*, CoordBox>	downloadBoxes;

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
    /*ImageMapLayer*l = */addImageLayer();

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

bool Document::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress)
{
    bool OK = true;

    stream.writeStartElement("MapDocument");

    stream.writeAttribute("xml:id", id());
    if (!asTemplate)
        stream.writeAttribute("layernum", QString::number(p->layerNum));
    if (p->lastDownloadLayer) {
        stream.writeAttribute("lastdownloadlayer", p->lastDownloadLayer->id());
        stream.writeAttribute("lastdownloadtimestamp", p->lastDownloadTimestamp.toUTC().toString(Qt::ISODate)+"Z");
    }

    for (int i=0; i<p->Layers.size(); ++i) {
        progress->setMaximum(progress->maximum() + p->Layers[i]->getDisplaySize());
    }

    for (int i=0; i<p->Layers.size(); ++i) {
        if (p->Layers[i]->isEnabled()) {
            if (asTemplate && p->Layers[i]->classType() == Layer::DrawingLayerType)
                continue;
            p->Layers[i]->toXML(stream, asTemplate, progress);
        }
    }

    if (!asTemplate) {
        OK = history().toXML(stream, progress);
    }
    stream.writeEndElement();

    return OK;
}

Document* Document::fromXML(QString title, QXmlStreamReader& stream, double version, LayerDock* aDock, QProgressDialog * progress)
{
    Document* NewDoc = new Document(aDock);
    NewDoc->p->title = title;

    CommandHistory* h = 0;

    if (stream.attributes().hasAttribute("xml:id"))
        NewDoc->p->Id = stream.attributes().value("xml:id").toString();
    if (stream.attributes().hasAttribute("layernum"))
        NewDoc->p->layerNum = stream.attributes().value("layernum").string()->toInt();
    else
        NewDoc->p->layerNum = 1;
    QString lastdownloadlayerId;
    if (stream.attributes().hasAttribute("lastdownloadlayer")) {
        NewDoc->p->lastDownloadTimestamp = QDateTime::fromString(stream.attributes().value("lastdownloadtimestamp").toString().left(19), Qt::ISODate);
        NewDoc->p->lastDownloadTimestamp.setTimeSpec(Qt::UTC);
        lastdownloadlayerId = stream.attributes().value("lastdownloadlayer").toString();
    }

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "ImageMapLayer") {
            /*ImageMapLayer* l =*/ ImageMapLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "DeletedMapLayer") {
            /*DeletedMapLayer* l =*/ DeletedLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "DirtyLayer" || stream.name() == "DirtyMapLayer") {
            /*DirtyMapLayer* l =*/ DirtyLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "UploadedLayer" || stream.name() == "UploadedMapLayer") {
            /*UploadedMapLayer* l =*/ UploadedLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "DrawingLayer" || stream.name() == "DrawingMapLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "TrackLayer" || stream.name() == "TrackMapLayer") {
            /*TrackMapLayer* l =*/ TrackLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "ExtractedLayer") {
            /*DrawingMapLayer* l =*/ DrawingLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "FilterLayer") {
            /*FilterLayer* l =*/ FilterLayer::fromXML(NewDoc, stream, progress);
        } else if (stream.name() == "CommandHistory") {
            if (version > 1.0)
                h = CommandHistory::fromXML(NewDoc, stream, progress);
        } else if (!stream.isWhitespace()) {
            qDebug() << "Doc: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }

        if (progress->wasCanceled())
            break;

        stream.readNext();
    }

    if (progress->wasCanceled()) {
        delete NewDoc;
        NewDoc = NULL;
    }

    if (NewDoc) {
        if (!lastdownloadlayerId.isEmpty())
            NewDoc->p->lastDownloadLayer = NewDoc->getLayer(lastdownloadlayerId);

        if (h)
            NewDoc->setHistory(h);
        else
            h = &NewDoc->history();

        if (!h->size() && NewDoc->getDirtySize()) {
            progress->setLabelText("History was corrupted. Rebuilding it...");
            qDebug() << "History was corrupted. Rebuilding it...";
            NewDoc->rebuildHistory();
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
        theLayer = new DrawingLayer(tr("Drawing layer #%1").arg(p->layerNum++));
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

Feature* Document::getFeature(const IFeature::FId& id)
{
    for (int i=0; i<p->Layers.size(); ++i)
    {
        Feature* F = p->Layers[i]->get(id);
        if (F)
            return F;
    }
    return NULL;
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
        else if (p->lastDownloadLayer)
            return p->lastDownloadLayer;
        else {
            DrawingLayer* firstDrLayer = NULL;
            for (int i=0; i<layerSize(); ++i) {
                if (!getLayer(i)->isEnabled())
                    continue;
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
    } else {
        if (!aLayer || (aLayer && !aLayer->isUploadable()))
            return p->dirtyLayer;
        else
            return aLayer;
    }
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

int Document::size() const
{
    int sz = 0;
    for (int i=0; i<layerSize(); ++i) {
        sz += getLayer(i)->size();
    }
    return sz;
}


void Document::setUploadedLayer(UploadedLayer* aLayer)
{
    p->uploadedLayer = aLayer;
}


UploadedLayer* Document::getUploadedLayer() const
{
    return p->uploadedLayer;
}

void Document::exportOSM(QMainWindow* main, QIODevice* device, QList<Feature*> aFeatures)
{
    if (aFeatures.isEmpty())
        return;

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(main);
    if (!aProgressWindow)
        return;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    if (dlg)
        dlg->setWindowTitle(tr("OSM Export"));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    if (Bar) {
        Bar->setTextVisible(false);
        Bar->setMaximum(aFeatures.size());
    }

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    if (Lbl)
        Lbl->setText(tr("Exporting OSM..."));

    if (dlg)
        dlg->show();

    QXmlStreamWriter stream(device);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);
    stream.writeStartDocument();

    stream.writeStartElement("osm");
    stream.writeAttribute("version", "0.6");
    stream.writeAttribute("generator", QString("%1 %2").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)));

    CoordBox aCoordBox = aFeatures[0]->boundingBox(true);
    aFeatures[0]->toXML(stream, dlg);
    for (int i=1; i < aFeatures.size(); i++) {
        aCoordBox.merge(aFeatures[i]->boundingBox(true));
        aFeatures[i]->toXML(stream, dlg);
    }

    stream.writeStartElement("bound");
    QString S = QString().number(aCoordBox.bottom(),'f',6) + ",";
    S += QString().number(aCoordBox.left(),'f',6) + ",";
    S += QString().number(aCoordBox.top(),'f',6) + ",";
    S += QString().number(aCoordBox.right(),'f',6);
    stream.writeAttribute("box", S);
    stream.writeAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
    stream.writeEndElement();

    stream.writeEndElement();
    stream.writeEndDocument();
}

QList<Feature*> Document::exportCoreOSM(QList<Feature*> aFeatures, bool forCopyPaste, QProgressDialog * progress)
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
                    if (!forCopyPaste) {
                        for (int j=0; j < G->size(); j++) {
                            if (Way* R = CAST_WAY(G->get(j))) {
                                for (int k=0; k < R->size(); k++) {
                                    if (Node* P = dynamic_cast<Node*>(R->get(k))) {
                                        if (!exportedFeatures.contains(P))
                                            exportedFeatures.append(P);
                                    }
                                }
                                if (!exportedFeatures.contains(R))
                                    exportedFeatures.append(R);
                            } else
                            if (Node* P = CAST_NODE(G->get(j))) {
                                if (!exportedFeatures.contains(P))
                                    exportedFeatures.append(P);
                            }

                        }
                    }
                    if (!exportedFeatures.contains(G))
                        exportedFeatures.append(G);
                }
            }
        }
        if (progress) {
            if (progress->wasCanceled()) {
                exportedFeatures.clear();
                return exportedFeatures;
            }
            progress->setValue(progress->value()+1);
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

bool Document::importGDAL(const QString& filename, DrawingLayer* NewLayer)
{
    Q_UNUSED(filename)
    Q_UNUSED(NewLayer)

    ImportExportGdal imp(this);
    if (!imp.loadFile(filename))
        return false;
    bool ret = imp.import(NewLayer);

    if (ret && NewLayer->size())
        return true;
    else
        return false;
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

#ifdef USE_PROTOBUF
bool Document::importPBF(const QString& filename, DrawingLayer* NewLayer)
{
    ImportExportPBF imp(this);
    if (!imp.loadFile(filename))
        return false;
    imp.import(NewLayer);

    if (NewLayer->size())
        return true;
    else
        return false;
}
#endif

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

const QList<CoordBox> Document::getDownloadBoxes(Layer* l) const
{
    return p->downloadBoxes.values(l);
}

bool Document::isDownloadedSafe(const CoordBox& bb) const
{
    QHashIterator<Layer*, CoordBox>it(p->downloadBoxes);
    while(it.hasNext()) {
        it.next();
        if (it.value().intersects(bb))
            return true;
    }

    return false;
}

QDateTime Document::getLastDownloadLayerTime() const
{
    return p->lastDownloadTimestamp;
}

Layer * Document::getLastDownloadLayer() const
{
    return p->lastDownloadLayer;
}

void Document::setLastDownloadLayer(Layer * aLayer)
{
    p->lastDownloadLayer = aLayer;
    p->lastDownloadTimestamp = QDateTime::currentDateTime();
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
        if (getLayer(First)->size() && !getLayer(First)->boundingBox().isNull())
            break;
    if (First == layerSize())
        return qMakePair(false,CoordBox(Coord(0,0),Coord(0,0)));
    Layer* aLayer = getLayer(First);
    CoordBox BBox = aLayer->boundingBox();
    for (int i=First+1; i<layerSize(); ++i)
        aLayer = getLayer(i);
        if (aLayer->size() && !aLayer->boundingBox().isNull())
            BBox.merge(aLayer->boundingBox());
    return qMakePair(true,BBox);
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

Document* Document::getDocumentFromXml(QDomDocument* theXmlDoc)
{
    QDomElement c;
    c = theXmlDoc->documentElement();
//    if (c.tagName().isNull())
//        c = c.firstChildElement();
//    while (!c.isNull() && c.tagName().isNull()) {
//        c =c.nextSiblingElement();
//    }
//    if (c.isNull())
//        return NULL;

    if (c.tagName() == "osm") {
        QString xml;
        QTextStream tstr(&xml, QIODevice::ReadOnly);
        c.save(tstr, 2);

        QXmlStreamReader stream(xml);

        Document* NewDoc = new Document(NULL);
        DrawingLayer* l = new DrawingLayer("Dummy");
        NewDoc->add(l);

        stream.readNext();
        while(!stream.atEnd() && !stream.isEndElement()) {
            if (stream.name() == "osm") {
                stream.readNext();
                while(!stream.atEnd() && !stream.isEndElement()) {
                    if (stream.name() == "way") {
                        Way::fromXML(NewDoc, l, stream);
                    } else if (stream.name() == "relation") {
                        Relation::fromXML(NewDoc, l, stream);
                    } else if (stream.name() == "node") {
                        Node::fromXML(NewDoc, l, stream);
                    } else if (!stream.isWhitespace()) {
                        qDebug() << "Doc::clipboard logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                        stream.skipCurrentElement();
                    }
                    stream.readNext();
                }
            }

            stream.readNext();
        }

        return NewDoc;
    } else
    if (c.tagName() == "kml") {
        Document* NewDoc = new Document(NULL);
        DrawingLayer* l = new DrawingLayer("Dummy");
        NewDoc->add(l);

        ImportExportKML imp(NewDoc);
        QByteArray ba = theXmlDoc->toByteArray();
        QBuffer kmlBuf(&ba);
        kmlBuf.open(QIODevice::ReadOnly);
        if (imp.setDevice(&kmlBuf))
            imp.import(l);

        return NewDoc;
    } else
        if (c.tagName() == "osmChange" || c.tagName() == "osmchange") {
            Document* NewDoc = new Document(NULL);
            DrawingLayer* l = new DrawingLayer("Dummy");
            NewDoc->add(l);

            ImportExportOSC imp(NewDoc);
            QByteArray ba = theXmlDoc->toByteArray();
            QBuffer buf(&ba);
            buf.open(QIODevice::ReadOnly);
            if (imp.setDevice(&buf))
                imp.import(l);

            return NewDoc;
        } else
    if (c.tagName() == "gpx") {
    }
    return NULL;
}

QList<Feature*> Document::mergeDocument(Document* otherDoc, Layer* layer, CommandList* theList)
{
    QList<Feature*> theFeats;
    for (int i=0; i<otherDoc->layerSize(); ++i)
        for (int j=0; j<otherDoc->getLayer(i)->size(); ++j)
            if (!otherDoc->getLayer(i)->get(j)->isNull())
                theFeats.push_back(otherDoc->getLayer(i)->get(j));
    for (int i=0; i<theFeats.size(); ++i) {
        Feature*F = theFeats.at(i);
        if (getFeature(F->id()))
            F->resetId();

        // Re-link null features to the ones in the current document
        for (int j=0; j<F->size(); ++j) {
            Feature* C = F->get(j);
            if (C->isNull()) {
                if (Feature* CC = getFeature(C->id())) {
                    if (Relation* R = CAST_RELATION(F)) {
                        QString role = R->getRole(j);
                        R->remove(j);
                        R->add(role, CC, j);
                    } else if (Way* W = CAST_WAY(F)) {
                        Node* N = CAST_NODE(CC);
                        W->remove(j);
                        W->add(N, j);
                    }
                } else
                    theFeats.push_back(C);
            }
        }
        F->layer()->remove(F);
        if (theList)
            theList->add(new AddFeatureCommand(layer, F, true));
        else {
            layer->add(F);
        }
    }
    return theFeats;
}

Document* Document::getDocumentFromClipboard()
{
    QClipboard *clipboard = QApplication::clipboard();
    QDomDocument* theXmlDoc = new QDomDocument();

    if (clipboard->mimeData()->hasFormat("application/x-openstreetmap+xml")) {
        if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/x-openstreetmap+xml"))) {
            delete theXmlDoc;
            return NULL;
        }
    } else
    if (clipboard->mimeData()->hasFormat("application/vnd.google-earth.kml+xml")) {
        if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/vnd.google-earth.kml+xml"))) {
            delete theXmlDoc;
            return NULL;
        }
    } else
    if (clipboard->mimeData()->hasText()) {
        if (!theXmlDoc->setContent(clipboard->text())) {
            delete theXmlDoc;
            return NULL;
        }
    } else {
        delete theXmlDoc;
        return NULL;
    }
    Document* doc = Document::getDocumentFromXml(theXmlDoc);
    delete theXmlDoc;

    return doc;
}

/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(Document *aDoc)
: theDocument(aDoc), curLayerIdx(0), curFeatureIdx(0), isAtEnd(false)
{
    docSize = theDocument->layerSize();
    if (!docSize)
        isAtEnd = true;
    else  {
        curLayerSize = theDocument->getLayer(curLayerIdx)->size();

        if(!check() && !isAtEnd)
            ++(*this);
    }
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

    return true;
}

void Document::rebuildHistory()
{
    delete p->History;
    p->History = new CommandHistory();
    CommandHistory* h = p->History;

    // Identify changes
    QList<Node*> newNode;
    QList<Node*> updNode;
    QList<Node*> delNode;
    QList<Way*> newWay;
    QList<Way*> updWay;
    QList<Way*> delWay;
    QList<Relation*> newRelation;
    QList<Relation*> updRelation;
    QList<Relation*> delRelation;
    for (FeatureIterator it(this); !it.isEnd(); ++it) {
        if (!it.get()->isDirty())
            continue;

        if (CHECK_NODE(it.get())) {
            Node* N = STATIC_CAST_NODE(it.get());
            if (N->hasOSMId()) {
                if (!N->isDeleted())
                    updNode << N;
                else
                    delNode << N;
            } else {
                if (!N->isDeleted())
                    newNode << N;
            }
        } else if (CHECK_WAY(it.get())) {
            Way* W = STATIC_CAST_WAY(it.get());
            if (W->hasOSMId()) {
                if (!W->isDeleted())
                    updWay << W;
                else
                    delWay << W;
            } else {
                if (!W->isDeleted())
                    newWay << W;
            }
        } else if (CHECK_RELATION(it.get())) {
            Relation* R = STATIC_CAST_RELATION(it.get());
            if (R->hasOSMId()) {
                if (!R->isDeleted())
                    updRelation << R;
                else
                    delRelation << R;
            } else {
                if (!R->isDeleted())
                    newRelation << R;
            }
        }
    }

    // Recreate history
    foreach (Node* N, newNode) {
        CommandList* CL = new CommandList(tr("History rebuild: Create node %1").arg(N->description()), N);
        Command* C = new AddFeatureCommand(N->layer(), N, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Node* N, updNode) {
        CommandList* CL = new CommandList(tr("History rebuild: Update node %1").arg(N->description()), N);;
        Command* C = new AddFeatureCommand(N->layer(), N, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Node* N, delNode) {
        CommandList* CL = new CommandList(tr("History rebuild: Delete node %1").arg(N->description()), N);
        Command* C = new RemoveFeatureCommand(this, N);
        CL->add(C);
        h->add(CL);
    }
    foreach (Way* W, newWay) {
        CommandList* CL = new CommandList(tr("History rebuild: Create way %1").arg(W->description()), W);
        Command* C = new AddFeatureCommand(W->layer(), W, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Way* W, updWay) {
        CommandList* CL = new CommandList(tr("History rebuild: Update way %1").arg(W->description()), W);
        Command* C = new AddFeatureCommand(W->layer(), W, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Way* W, delWay) {
        CommandList* CL = new CommandList(tr("History rebuild: Delete way %1").arg(W->description()), W);
        Command* C = new RemoveFeatureCommand(this, W);
        CL->add(C);
        h->add(CL);
    }
    foreach (Relation* R, newRelation) {
        CommandList* CL = new CommandList(tr("History rebuild: Create relation %1").arg(R->description()), R);
        Command* C = new AddFeatureCommand(R->layer(), R, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Relation* R, updRelation) {
        CommandList* CL = new CommandList(tr("History rebuild: Update relation %1").arg(R->description()), R);
        Command* C = new AddFeatureCommand(R->layer(), R, true);
        CL->add(C);
        h->add(CL);
    }
    foreach (Relation* R, delRelation) {
        CommandList* CL = new CommandList(tr("History rebuild: Delete relation %1").arg(R->description()), R);
        Command* C = new RemoveFeatureCommand(this, R);
        CL->add(C);
        h->add(CL);
    }
}

/* RELATED */


