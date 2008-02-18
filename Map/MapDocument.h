#ifndef MAPDOCUMENT_H_
#define MAPDOCUMENT_H_

#include "Map/Coord.h"
#include "PaintStyle/PaintStyle.h"
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"

#include <utility>

class QString;
class CommandHistory;
class MapDocument;
class MapDocumentPrivate;
class MapFeature;
class MapLayerPrivate;
class MapAdapter;
class Layer;
class LayerManager;

class MapLayer
{
public:
	enum MapLayerType {
		ImageLayer,
		DrawingLayer
	};

public:
	MapLayer(const QString& aName, enum MapLayerType layertype);

private:
	MapLayer(const MapLayer& aLayer);

public:
	~MapLayer();

	void setName(const QString& aName);
	const QString& name() const;
	bool isVisible() const;
	void setVisible(bool b);

	void add(MapFeature* aFeature);
	void add(MapFeature* aFeature, unsigned int Idx);
	void remove(MapFeature* aFeature);
	bool exists(MapFeature* aFeature) const;
	unsigned int size() const;
	MapFeature* get(unsigned int i);
	const MapFeature* get(unsigned int i) const;
	MapFeature* get(const QString& id);
	void notifyIdUpdate(const QString& id, MapFeature* aFeature);
	void sortRenderingPriority(double PixelPerM);
	void invalidateRenderPriority();

	void setDocument(MapDocument* aDocument);
	MapDocument* getDocument();

	MapLayer::MapLayerType type();
	Layer* imageLayer();
	void setMapAdapter(ImageBackgroundType typ);
	void setMapAdapter(ImageBackgroundType typ, MainWindow* main);
	LayerManager* layermanager;

	void ExportOSM(void);

private:
	MapLayerPrivate* p;
};

class MapDocument
{

public:
	MapDocument();

private:
	MapDocument(const MapDocument&);

public:
	~MapDocument();

	void add(MapLayer* aLayer);
	void remove(MapLayer* aLayer);
	bool exists(MapFeature* aFeature) const;
	unsigned int numLayers() const;
	MapLayer* layer(unsigned int i);
	const MapLayer* layer(unsigned int i) const;

	MapFeature* get(const QString& id);
	CommandHistory& history();
	const CommandHistory& history() const;
	void clear();

	void addToTagList(QString k, QString v);
	QStringList getTagList() ;
	QStringList getTagValueList(QString k) ;
    MapLayer* getBgLayer() const;
	void exportOSM(const QString& filename);


private:
	MapDocumentPrivate* p;
	QStringList tagKeys;
	QStringList tagValues;
	MapLayer* bgLayer;
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


