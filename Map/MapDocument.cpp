#include "Map/MapDocument.h"
#include "Command/Command.h"
#include "Map/MapFeature.h"

#include <QtCore/QString>

#include <algorithm>
#include <map>
#include <vector>

/* MAPLAYER */

class MapLayerPrivate
{
public:
	~MapLayerPrivate()
	{
		for (unsigned int i=0; i<Features.size(); ++i)
			delete Features[i];
	}
	std::vector<MapFeature*> Features;
	std::map<QString, MapFeature*> IdMap;
	QString Name;
	bool Visible;
};

MapLayer::MapLayer(const QString& aName)
: p(new MapLayerPrivate)
{
	p->Name = aName;
	p->Visible = true;
}

MapLayer::MapLayer(const MapLayer&)
: p(0)
{
}

MapLayer::~MapLayer()
{
	delete p;
}

void MapLayer::setName(const QString& s)
{
	p->Name = s;
}

const QString& MapLayer::name() const
{
	return p->Name;
}

void MapLayer::setVisible(bool b)
{
	p->Visible = b;
}

bool MapLayer::isVisible() const
{
	return p->Visible;
}

void MapLayer::add(MapFeature* aFeature)
{
	p->Features.push_back(aFeature);
	notifyIdUpdate(aFeature->id(),aFeature);
	aFeature->addedToDocument();
}

void MapLayer::add(MapFeature* aFeature, unsigned int Idx)
{
	add(aFeature);
	std::rotate(p->Features.begin()+Idx,p->Features.end()-1,p->Features.end());
	aFeature->addedToDocument();
}

void MapLayer::notifyIdUpdate(const QString& id, MapFeature* aFeature)
{
	p->IdMap[id] = aFeature;
}

void MapLayer::remove(MapFeature* aFeature)
{
	std::vector<MapFeature*>::iterator i = std::find(p->Features.begin(),p->Features.end(), aFeature);
	if (i != p->Features.end())
	{
		p->Features.erase(i);
		aFeature->removedFromDocument();
		notifyIdUpdate(aFeature->id(),0);
	}
}

unsigned int MapLayer::size() const
{
	return p->Features.size();
}

MapFeature* MapLayer::get(unsigned int i)
{
	return p->Features[i];
}

MapFeature* MapLayer::get(const QString& id)
{
	std::map<QString, MapFeature*>::iterator i = p->IdMap.find(id);
	if (i != p->IdMap.end())
		return i->second;
	return 0;
}

const MapFeature* MapLayer::get(unsigned int i) const
{
	return p->Features[i];
}


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
};



MapDocument::MapDocument()
: p(new MapDocumentPrivate)
{
	add(new MapLayer("Generic layer"));
}

MapDocument::MapDocument(const MapDocument&)
: p(0)
{
}

MapDocument::~MapDocument(void)
{
	delete p;
}

void MapDocument::clear()
{
	delete p;
	p = new MapDocumentPrivate;
	add(new MapLayer("Generic layer"));
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
}

void MapDocument::remove(MapLayer* aLayer)
{
	std::vector<MapLayer*>::iterator i = std::find(p->Layers.begin(),p->Layers.end(), aLayer);
	if (i != p->Layers.end())
		p->Layers.erase(i);
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