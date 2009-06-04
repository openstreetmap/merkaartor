#ifndef MAPDOCUMENT_H_
#define MAPDOCUMENT_H_

#include <QtXml>

#include "Maps/MapLayer.h"
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
class MapDocument;
class MapDocumentPrivate;
class ImageMapLayer;
class TrackMapLayer;
class DrawingMapLayer;
class DirtyMapLayer;
class UploadedMapLayer;
class DeletedMapLayer;

class MapDocument : public QObject
{
Q_OBJECT
public:
	MapDocument();
	MapDocument(LayerDock* aDock);

private:
	MapDocument(const MapDocument&, LayerDock* aDock);

public:
	~MapDocument();

	void addDefaultLayers();

	void setLayerDock(LayerDock* aDock);
	LayerDock* getLayerDock(void);

	void add(MapLayer* aLayer);
	void moveLayer(MapLayer* aLayer, int pos);
	ImageMapLayer* addImageLayer(ImageMapLayer* aLayer = NULL);
	void remove(MapLayer* aLayer);
	bool exists(MapLayer* aLayer) const;
	bool exists(MapFeature* aFeature) const;
	void deleteFeature(MapFeature* aFeature);
	int layerSize() const;
	MapLayer* getLayer(const QString& id);
	MapLayer* getLayer(int i);
	const MapLayer* getLayer(int i) const;

	MapFeature* getFeature(const QString& id, bool exact=true);
	QList<MapFeature*> getFeatures(MapLayer::LayerType layerType = MapLayer::UndefinedType);
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

	void setDirtyLayer(DirtyMapLayer* aLayer);
	//DirtyMapLayer* getDirtyLayer() const;
	MapLayer* getDirtyOrOriginLayer(MapLayer* aLayer = NULL) const;
	MapLayer* getDirtyOrOriginLayer(MapFeature* F) const;
	void setUploadedLayer(UploadedMapLayer* aLayer);
	UploadedMapLayer* getUploadedLayer() const;
	//DeletedMapLayer* getTrashLayer() const;

	QString exportOSM(const CoordBox& aCoordBox = WORLD_COORDBOX, bool renderBounds=false);
	QString exportOSM(QList<MapFeature*> aFeatures);
	QList<MapFeature*> exportCoreOSM(QList<MapFeature*> aFeatures);
	bool toXML(QDomElement xParent, QProgressDialog & progress);
	static MapDocument* fromXML(const QDomElement e, double version, LayerDock* aDock, QProgressDialog & progress);

	bool importNMEA(const QString& filename, TrackMapLayer* NewLayer);
	bool importOSB(const QString& filename, DrawingMapLayer* NewLayer);
	bool importKML(const QString& filename, TrackMapLayer* NewLayer);
	bool importSHP(const QString& filename, DrawingMapLayer* NewLayer);

	MapLayer* getLastDownloadLayer();
	void setLastDownloadLayer(MapLayer * aLayer);
	void addDownloadBox(MapLayer*l, CoordBox aBox);
	void removeDownloadBox(MapLayer*l);
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
	FeatureIterator(MapDocument* aDoc);
	virtual ~FeatureIterator();

	bool isEnd() const;
	FeatureIterator& operator ++();
	MapFeature* get();
	int index();

protected:
	virtual bool check();
	MapDocument* theDocument;
	int curLayerIdx;
	int curFeatureIdx;
	bool isAtEnd;
	int docSize;
	int curLayerSize;
};

class VisibleFeatureIterator: public FeatureIterator
{

public:
	VisibleFeatureIterator(MapDocument* aDoc);
	virtual ~VisibleFeatureIterator();

protected:
	virtual bool check();
};


#endif


