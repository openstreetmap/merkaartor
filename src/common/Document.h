#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <QtXml>

#include "IDocument.h"
#include "Layer.h"
#include "Coord.h"
#include "MerkaartorPreferences.h"
#include "LayerDock.h"

#include <utility>

class QString;
class QProgressDialog;
class QMainWindow;

class Command;
class CommandHistory;
class Document;
class MapDocumentPrivate;
class ImageMapLayer;
class TrackLayer;
class DrawingLayer;
class DirtyLayer;
class UploadedLayer;
class DeletedLayer;
class FeaturePainter;
class SpatialiteBackend;

class Document : public QObject, public IDocument
{
Q_OBJECT
public:
    Document();
    Document(LayerDock* aDock);

private:
    Document(const Document&, LayerDock* aDock);

public:
    ~Document();

    const QString& id() const;

    void addDefaultLayers();
    void addFilterLayers();

    void setLayerDock(LayerDock* aDock);
    LayerDock* getLayerDock(void);

    void add(Layer* aLayer);
    void moveLayer(Layer* aLayer, int pos);
    ImageMapLayer* addImageLayer(ImageMapLayer* aLayer = NULL);
    DrawingLayer* addDrawingLayer(DrawingLayer* aLayer = NULL);
    FilterLayer* addFilterLayer(FilterLayer* aLayer = NULL);
    void remove(Layer* aLayer);
    bool exists(Layer* aLayer) const;
    bool exists(Feature* aFeature) const;
    void deleteFeature(Feature* aFeature);
    int layerSize() const;
    Layer* getLayer(const QString& id);
    Layer* getLayer(int i);
    const Layer* getLayer(int i) const;
    int size() const;

    Feature* getFeature(const IFeature::FId& id);
    QList<Feature*> getFeatures(Layer::LayerType layerType = Layer::UndefinedType);
    void setHistory(CommandHistory* h);
    CommandHistory& history();
    const CommandHistory& history() const;
    void addHistory(Command* aCommand);
    void redoHistory();
    void undoHistory();
    void rebuildHistory();
    void clear();

    void setDirtyLayer(DirtyLayer* aLayer);
    Layer* getDirtyLayer();
    Layer* getDirtyOrOriginLayer(Layer* aLayer = NULL);
    Layer* getDirtyOrOriginLayer(Feature* F);
    int getDirtySize() const;

    void setUploadedLayer(UploadedLayer* aLayer);
    UploadedLayer* getUploadedLayer() const;

    void exportOSM(QWidget* main, QIODevice* device, QList<Feature*> aFeatures);
    QList<Feature*> exportCoreOSM(QList<Feature*> aFeatures, bool forCopyPaste=false, QProgressDialog * progress=NULL);
    bool toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * progress);
    static Document* fromXML(QString title, QXmlStreamReader& stream, qreal version, LayerDock* aDock, QProgressDialog * progress);

    bool importNMEA(const QString& filename, TrackLayer* NewLayer);
    bool importKML(const QString& filename, TrackLayer* NewLayer);
    bool importCSV(const QString& filename, DrawingLayer* NewLayer);
    bool importOSC(const QString& filename, DrawingLayer* NewLayer);
#ifndef _MOBILE
    bool importGDAL(const QString& filename, DrawingLayer* NewLayer);
#endif
#ifdef USE_PROTOBUF
    bool importPBF(const QString& filename, DrawingLayer* NewLayer);
#endif

    QDateTime getLastDownloadLayerTime() const;
    Layer* getLastDownloadLayer() const;
    void setLastDownloadLayer(Layer * aLayer);

    void addDownloadBox(Layer*l, CoordBox aBox);
    void removeDownloadBox(Layer*l);
    const QList<CoordBox> getDownloadBoxes() const;
    const QList<CoordBox> getDownloadBoxes(Layer* l) const;
    bool isDownloadedSafe(const CoordBox& bb) const;

    QPair<bool, CoordBox> boundingBox();

    bool setFilterType(FilterType aFilter);
    TagSelector* getTagFilter();
    int filterRevision() const;

    QString title() const;
    void setTitle(const QString aTitle);

    QString toPropertiesHtml();

    virtual void setPainters(QList<Painter> aPainters);
    virtual int getPaintersSize();
    void lockPainters();
    void lockPaintersForWrite();
    void unlockPainters();
    virtual const Painter* getPainter(int i);

    QStringList getCurrentSourceTags();

    static Document* getDocumentFromXml(QDomDocument* theXmlDoc);
    static Document* getDocumentFromClipboard();

    QList<Feature*> mergeDocument(Document *otherDoc, Layer* layer, CommandList* theList=NULL);
private:
    MapDocumentPrivate* p;

protected slots:
    void on_imageRequested(ImageMapLayer* anImageLayer);
    void on_imageReceived(ImageMapLayer* anImageLayer);
    void on_loadingFinished(ImageMapLayer* anImageLayer);

signals:
    void imageRequested(ImageMapLayer*);
    void imageReceived(ImageMapLayer*);
    void loadingFinished(ImageMapLayer*);
    void historyChanged();

};

class FeatureIterator
{

public:
    FeatureIterator(Document* aDoc);
    virtual ~FeatureIterator();

    bool isEnd() const;
    FeatureIterator& operator ++();
    Feature* get();
    int index();

protected:
    virtual bool check();
    Document* theDocument;
    int curLayerIdx;
    int curFeatureIdx;
    bool isAtEnd;
    int docSize;
    int curLayerSize;
};

class VisibleFeatureIterator: public FeatureIterator
{
public:
    VisibleFeatureIterator(Document* aDoc);
    ~VisibleFeatureIterator() override;

protected:
    bool check() override;
};


#endif


