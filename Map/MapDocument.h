#ifndef MAPDOCUMENT_H_
#define MAPDOCUMENT_H_

#include <QtXml>

#include "Map/Coord.h"
#include "PaintStyle/PaintStyle.h"
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "LayerDock.h"

#include <utility>

class QString;
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

	void setLayerDock(LayerDock* aDock);
	LayerDock* getLayerDock(void);

	void add(MapLayer* aLayer);
	void remove(MapLayer* aLayer);
	bool exists(MapLayer* aLayer) const;
	bool exists(MapFeature* aFeature) const;
	unsigned int layerSize() const;
	MapLayer* getLayer(const QString& id);
	MapLayer* getLayer(unsigned int i);
	const MapLayer* getLayer(unsigned int i) const;

	MapFeature* getFeature(const QString& id, bool exact=true);
	QVector<MapFeature*> getFeatures();
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
	void setTagKeys(QList<QString> list);
	QString getTagKey(int idx);
	int getTagKeyIndex(QString k);
	int getTagValueIndex(QString v);
	QList<QString> getTagValues();
	void setTagValues(QList<QString> list);
	QString getTagValue(int idx);

	ImageMapLayer* getImageLayer() const;
	DirtyMapLayer* getDirtyLayer() const;
	MapLayer* getDirtyOrOriginLayer(MapLayer* aLayer);
	UploadedMapLayer* getUploadedLayer() const;
	DeletedMapLayer* getTrashLayer() const;

	QString exportOSM(const CoordBox& aCoordBox = WORLD_COORDBOX);
	QString exportOSM(QVector<MapFeature*> aFeatures);
	bool toXML(QDomElement xParent);
	static MapDocument* fromXML(const QDomElement e, double version, LayerDock* aDock);

	bool importNMEA(const QString& filename, TrackMapLayer* NewLayer);
	bool importOSB(const QString& filename, DrawingMapLayer* NewLayer);

	MapLayer* getLastDownloadLayer();
	void setLastDownloadLayer(MapLayer * aLayer);
	void addDownloadBox(CoordBox aBox);
	QList<CoordBox> *getDownloadBoxes();

private:
	MapDocumentPrivate* p;
	QStringList tagKeys;
	QStringList tagValues;

signals:
	void historyChanged();

};

bool hasUnsavedChanges(const MapDocument& aDoc);
std::pair<bool, CoordBox> boundingBox(const MapDocument* theDocument);

class VisibleFeatureIterator
{

public:
	VisibleFeatureIterator(MapDocument* aDoc);

	bool isEnd() const;
	VisibleFeatureIterator& operator ++();
	MapFeature* get();
	MapLayer* layer();
	unsigned int index();

private:
	MapDocument* theDocument;
	unsigned int Layer;
	unsigned int Idx;
};

class FeatureIterator
{

public:
	FeatureIterator(MapDocument* aDoc);

	bool isEnd() const;
	FeatureIterator& operator ++();
	MapFeature* get();
	MapLayer* layer();
	unsigned int index();

private:
	MapDocument* theDocument;
	unsigned int Layer;
	unsigned int Idx;
};

#endif


