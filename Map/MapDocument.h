#ifndef MAPDOCUMENT_H_
#define MAPDOCUMENT_H_

#include "Map/Coord.h"
#include "PaintStyle/PaintStyle.h"

#include <utility>

class QString;

class CommandHistory;
class MapDocument;
class MapDocumentPrivate;
class MapFeature;
class MapLayerPrivate;

class MapLayer
{
	public:
		MapLayer(const QString& Name);
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

	private:
		MapLayerPrivate* p;
};

class MapDocument
{
	public:
		MapDocument();
	private:
		MapDocument(const MapDocument& aDoc);
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
	private:
		MapDocumentPrivate* p;
		QStringList tagKeys;
		QStringList tagValues;
};

bool hasUnsavedChanges(const MapDocument& aDoc);
std::pair<bool,CoordBox> boundingBox(const MapDocument* theDocument);

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


