#ifndef DOCUMENT_H_
#define DOCUMENT_H_

#include <QtXml>

#include "Layer.h"
#include "Maps/Coord.h"
#include "PaintStyle/PaintStyle.h"
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "LayerDock.h"

#include <utility>

class QString;
class QProgressDialog;

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

class Document : public QObject
{
Q_OBJECT
public:
    Document();
    Document(LayerDock* aDock);

private:
    Document(const Document&, LayerDock* aDock);

public:
    ~Document();

    void addDefaultLayers();

    void setLayerDock(LayerDock* aDock);
    LayerDock* getLayerDock(void);

    void add(Layer* aLayer);
    void moveLayer(Layer* aLayer, int pos);
    ImageMapLayer* addImageLayer(ImageMapLayer* aLayer = NULL);
    void remove(Layer* aLayer);
    bool exists(Layer* aLayer) const;
    bool exists(Feature* aFeature) const;
    void deleteFeature(Feature* aFeature);
    int layerSize() const;
    Layer* getLayer(const QString& id);
    Layer* getLayer(int i);
    const Layer* getLayer(int i) const;

    Feature* getFeature(const QString& id, bool exact=true);
    QList<Feature*> getFeatures(Layer::LayerType layerType = Layer::UndefinedType);
    void setHistory(CommandHistory* h);
    CommandHistory& history();
    const CommandHistory& history() const;
    void addHistory(Command* aCommand);
    void redoHistory();
    void undoHistory();
    void clear();

    void addToTagList(QString k, QString v);
    QStringList getTagList() ;
    QStringList getTagValueList(QString k) ;
    QList<QString> getTagKeys();
    QList<QString> getTagValues();
    QString getTagValue(int idx);

    void setDirtyLayer(DirtyLayer* aLayer);
    //DirtyMapLayer* getDirtyLayer() const;
    Layer* getDirtyOrOriginLayer(Layer* aLayer = NULL) const;
    Layer* getDirtyOrOriginLayer(Feature* F) const;
    void setUploadedLayer(UploadedLayer* aLayer);
    UploadedLayer* getUploadedLayer() const;
    //DeletedMapLayer* getTrashLayer() const;

    QString exportOSM(const CoordBox& aCoordBox = WORLD_COORDBOX, bool renderBounds=false);
    QString exportOSM(QList<Feature*> aFeatures);
    QList<Feature*> exportCoreOSM(QList<Feature*> aFeatures);
    bool toXML(QDomElement xParent, QProgressDialog & progress);
    static Document* fromXML(const QDomElement e, double version, LayerDock* aDock, QProgressDialog & progress);

    bool importNMEA(const QString& filename, TrackLayer* NewLayer);
    bool importOSB(const QString& filename, DrawingLayer* NewLayer);
    bool importKML(const QString& filename, TrackLayer* NewLayer);
    bool importSHP(const QString& filename, DrawingLayer* NewLayer);
    bool importCSV(const QString& filename, DrawingLayer* NewLayer);
    bool importOSC(const QString& filename, DirtyLayer* NewLayer);

    Layer* getLastDownloadLayer();
    void setLastDownloadLayer(Layer * aLayer);
    void addDownloadBox(Layer*l, CoordBox aBox);
    void removeDownloadBox(Layer*l);
    const QList<CoordBox> getDownloadBoxes() const;

    bool hasUnsavedChanges();
    QPair<bool, CoordBox> boundingBox();

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
    virtual ~VisibleFeatureIterator();

protected:
    virtual bool check();
};


#endif


