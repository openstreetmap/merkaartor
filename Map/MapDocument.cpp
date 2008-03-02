#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Command/Command.h"

#include <QtCore/QString>
#include <QMultiMap>

#include <algorithm>
#include <map>
#include <vector>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

/* MAPDOCUMENT */

class MapDocumentPrivate
{
public:
	~MapDocumentPrivate()
	{
		History.cleanup();
		for (unsigned int i=0; i<Layers.size(); ++i)
			delete Layers[i];
	}
	CommandHistory History;
	std::vector<MapLayer*> Layers;
	QMultiMap<QString, QString> tagList;
	ImageMapLayer* bgLayer;

};

MapDocument::MapDocument()
: p(new MapDocumentPrivate)
{
	p->bgLayer = new ImageMapLayer("Background imagery");
	add(p->bgLayer);

	DrawingMapLayer* l = new DrawingMapLayer("Generic layer");
	l->setSelected(true);
	add(l);
}

MapDocument::MapDocument(const MapDocument&)
: p(0)
{
}

MapDocument::~MapDocument()
{
	delete p;
}

void MapDocument::clear()
{
	delete p;
	p = new MapDocumentPrivate;
	p->bgLayer = new ImageMapLayer("Background imagery");
	p->bgLayer->setMapAdapter(MerkaartorPreferences::instance()->getBgType());
	add(p->bgLayer);

	DrawingMapLayer* l = new DrawingMapLayer("Generic layer");
	l->setSelected(true);
	add(l);
}

CommandHistory& MapDocument::history()
{
	return p->History;
}

const CommandHistory& MapDocument::history() const
{
	return p->History;
}

void MapDocument::add(MapLayer* aLayer)
{
	p->Layers.push_back(aLayer);
    aLayer->setDocument(this);
}

void MapDocument::addToTagList(QString k, QString v)
{
	if (!p->tagList.contains(k, v)) {
    	p->tagList.insert(k, v);
	}
}

QStringList MapDocument::getTagList()
{
	return p->tagList.uniqueKeys();
}

QStringList MapDocument::getTagValueList(QString k)
{
	if (k == "*")
		return p->tagList.values();
	else
		return p->tagList.values(k);
}

void MapDocument::remove(MapLayer* aLayer)
{
	std::vector<MapLayer*>::iterator i = std::find(p->Layers.begin(),p->Layers.end(), aLayer);
	if (i != p->Layers.end())
		p->Layers.erase(i);
}

bool MapDocument::exists(MapFeature* F) const
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i]->exists(F)) return true;
	return false;
}

unsigned int MapDocument::numLayers() const
{
	return p->Layers.size();
}

MapLayer* MapDocument::layer(unsigned int i)
{
	return p->Layers[i];
}

const MapLayer* MapDocument::layer(unsigned int i) const
{
	return p->Layers[i];
}

MapFeature* MapDocument::get(const QString& id)
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
	{
		MapFeature* F = p->Layers[i]->get(id);
		if (F) return F;
	}
	return 0;
}

ImageMapLayer* MapDocument::getImageLayer() const
{
	return p->bgLayer;
}

void MapDocument::exportOSM(const QString& filename)
{
	QString theExport, coreExport;

	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		coreExport += i.get()->exportOSM();
	}

	std::pair<bool,CoordBox> bb = boundingBox(this);

	theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
	theExport += "<osm version='0.5' generator='Merkaartor'>\n";
	theExport += "<bound box='";
	theExport += QString().number(radToAng(bb.second.bottomLeft().lat())) + ",";
	theExport += QString().number(radToAng(bb.second.bottomLeft().lon())) + ",";
	theExport += QString().number(radToAng(bb.second.topRight().lat())) + ",";
	theExport += QString().number(radToAng(bb.second.topRight().lon()));
	theExport += "' origin='http://www.openstreetmap.org/api/0.5' />\n";
	theExport += coreExport;
	theExport += "</osm>";

	QFile file(filename);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QTextStream out(&file);
	out << theExport;
	file.close();
}


/* VISIBLEFEATUREITERATOR */

VisibleFeatureIterator::VisibleFeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Layer(0), Idx(0)
{
	while (Layer < theDocument->numLayers())
	{
		MapLayer* L = theDocument->layer(Layer);
		if (L->isVisible() && L->size())
			break;
		++Layer;
	}
}

MapFeature* VisibleFeatureIterator::get()
{
	return theDocument->layer(Layer)->get(Idx);
}

bool VisibleFeatureIterator::isEnd() const
{
	return Layer >= theDocument->numLayers();
}

VisibleFeatureIterator& VisibleFeatureIterator::operator++()
{
	++Idx;
	if (Idx >= theDocument->layer(Layer)->size())
	{
		Idx = 0;
		++Layer;
		while (Layer < theDocument->numLayers())
		{
			MapLayer* L = theDocument->layer(Layer);
			if (L->isVisible() && L->size())
				break;
			++Layer;
		}
	}
	return *this;
}

MapLayer* VisibleFeatureIterator::layer()
{
	return theDocument->layer(Layer);
}

unsigned int VisibleFeatureIterator::index()
{
	return Idx;
}


/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Layer(0), Idx(0)
{
	while (Layer < theDocument->numLayers())
	{
		if (theDocument->layer(Layer)->size())
			break;
		++Layer;
	}
}

MapFeature* FeatureIterator::get()
{
	return theDocument->layer(Layer)->get(Idx);
}

bool FeatureIterator::isEnd() const
{
	return Layer >= theDocument->numLayers();
}

FeatureIterator& FeatureIterator::operator++()
{
	++Idx;
	if (Idx >= theDocument->layer(Layer)->size())
	{
		Idx = 0;
		++Layer;
		while (Layer < theDocument->numLayers())
		{
			if (theDocument->layer(Layer)->size())
				break;
			++Layer;
		}
	}
	return *this;
}

MapLayer* FeatureIterator::layer()
{
	return theDocument->layer(Layer);
}

unsigned int FeatureIterator::index()
{
	return Idx;
}

/* RELATED */

static CoordBox boundingBox(const MapLayer* theLayer)
{
	CoordBox Box(theLayer->get(0)->boundingBox());
	for (unsigned int i=1; i<theLayer->size(); ++i)
		Box.merge(theLayer->get(i)->boundingBox());
	return Box;
}

std::pair<bool,CoordBox> boundingBox(const MapDocument* theDocument)
{
	unsigned int First;
	for (First = 0; First < theDocument->numLayers(); ++First)
		if (theDocument->layer(First)->size())
			break;
	if (First == theDocument->numLayers())
		return std::make_pair(false,CoordBox(Coord(0,0),Coord(0,0)));
	CoordBox BBox(boundingBox(theDocument->layer(First)));
	for (unsigned int i=First+1; i<theDocument->numLayers(); ++i)
		if (theDocument->layer(i)->size())
			BBox.merge(boundingBox(theDocument->layer(i)));
	return std::make_pair(true,BBox);
}

bool hasUnsavedChanges(const MapDocument& aDoc)
{
	return aDoc.history().index();
}
