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
class ImageMapLayer;
class TrackMapLayer;

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
	ImageMapLayer* getImageLayer() const;

	void exportOSM(const QString& filename);
	TrackMapLayer* importNMEA(const QString& filename);


private:
	MapDocumentPrivate* p;
	QStringList tagKeys;
	QStringList tagValues;
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


