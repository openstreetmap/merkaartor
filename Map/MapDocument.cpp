#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Command/Command.h"
#include "Map/ImportOSM.h"

#include "Map/MapDocument.h"

#include "ImportExport/ImportNMEA.h"

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
	MapDocumentPrivate()
	: History(new CommandHistory()), bgLayer(0), theDock(0), lastDownloadLayer(0) {};
	~MapDocumentPrivate()
	{
		History->cleanup();
		for (unsigned int i=0; i<Layers.size(); ++i) {
			if (theDock)
				theDock->deleteLayer(Layers[i]);
			delete Layers[i];
		}
	}
	CommandHistory*				History;
	std::vector<MapLayer*>		Layers;
	QMultiMap<QString, QString> tagList;
	ImageMapLayer*				bgLayer;
	LayerDock*					theDock;
	MapLayer*					lastDownloadLayer;

};

MapDocument::MapDocument()
	: p(new MapDocumentPrivate)
{
}

MapDocument::MapDocument(LayerDock* aDock)
: p(new MapDocumentPrivate)
{
	p->theDock = aDock;

	p->bgLayer = new ImageMapLayer(tr("Background imagery"));
	add(p->bgLayer);

	DrawingMapLayer* l = new DrawingMapLayer(tr("Generic layer"));
	l->setSelected(true);
	add(l);
}

MapDocument::MapDocument(const MapDocument&, LayerDock*)
: p(0)
{
}

MapDocument::~MapDocument()
{
	delete p;
}

bool MapDocument::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement mapDoc = xParent.namedItem("MapDocument").toElement();
	if (!mapDoc.isNull()) {
		xParent.removeChild(mapDoc);
	}
	mapDoc = xParent.ownerDocument().createElement("MapDocument");
	xParent.appendChild(mapDoc);

	for (unsigned int i=0; i<p->Layers.size(); ++i)
		p->Layers[i]->toXML(mapDoc);

	OK = history().toXML(mapDoc);

	return OK;
}

MapDocument* MapDocument::fromXML(const QDomElement e, LayerDock* aDock)
{
	MapDocument* NewDoc = new MapDocument();
	NewDoc->p->theDock = aDock;

	CommandHistory* h = 0;

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
/*		if (c.tagName() == "osm") {
			MapLayer* NewLayer = new DrawingMapLayer( "Document" );
			NewDoc->add(NewLayer);
			QByteArray ba;
			QTextStream out(&ba);
			c.save(out,2);

			bool importOK = importOSM(NULL, ba, NewDoc, NewLayer,NULL);
			if (importOK == false) {
				delete NewDoc;
 				delete NewLayer;
				return NULL;
			}
		} else*/
		if (c.tagName() == "DrawingMapLayer") {
			/* DrawingMapLayer* l = */ DrawingMapLayer::fromXML(NewDoc, c);
/*			if (l)
				NewDoc->add(l);				*/
		} else
		if (c.tagName() == "ImageMapLayer") {
			ImageMapLayer* l = ImageMapLayer::fromXML(NewDoc, c);
			if (l) {
				NewDoc->p->bgLayer = l;
				NewDoc->add(l);
			}
		} else
		if (c.tagName() == "TrackMapLayer") {
			/* TrackMapLayer* l = */ TrackMapLayer::fromXML(NewDoc, c);
/*			if (l)
			NewDoc->add(l);				*/
		} else
		if (c.tagName() == "CommandHistory") {
			h = CommandHistory::fromXML(NewDoc, c);
		}
		c = c.nextSiblingElement();
	}

	if (NewDoc)
		NewDoc->setHistory(h);

	return NewDoc;
}

void MapDocument::setLayerDock(LayerDock* aDock)
{
	p->theDock = aDock;
}

LayerDock* MapDocument::getLayerDock(void)
{
	return p->theDock;
}

void MapDocument::clear()
{
	delete p;
	p = new MapDocumentPrivate;
	p->bgLayer = new ImageMapLayer(tr("Background imagery"));
	p->bgLayer->setMapAdapter(MerkaartorPreferences::instance()->getBgType());
	add(p->bgLayer);

	DrawingMapLayer* l = new DrawingMapLayer(tr("Generic layer"));
	l->setSelected(true);
	add(l);
}

void MapDocument::setHistory(CommandHistory* h)
{
	delete p->History;
	p->History = h;
}

CommandHistory& MapDocument::history()
{
	return *(p->History);
}

const CommandHistory& MapDocument::history() const
{
	return *(p->History);
}

void MapDocument::add(MapLayer* aLayer)
{
	p->Layers.push_back(aLayer);
    aLayer->setDocument(this);
	if (p->theDock)
		p->theDock->addLayer(aLayer);
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
	if (i != p->Layers.end()) {
		p->Layers.erase(i);
	}
	if (aLayer == p->lastDownloadLayer)
		p->lastDownloadLayer = NULL;
	if (p->theDock)
		p->theDock->deleteLayer(aLayer);

	//delete (p->History);
	//p->History = new CommandHistory();
}

bool MapDocument::exists(MapLayer* L) const
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i] == L) return true;
	return false;
}

bool MapDocument::exists(MapFeature* F) const
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i]->exists(F)) return true;
	return false;
}

unsigned int MapDocument::layerSize() const
{
	return p->Layers.size();
}

MapLayer* MapDocument::getLayer(const QString& id)
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
	{
		if (p->Layers[i]->id() == id) return p->Layers[i];
	}
	return 0;
}

MapLayer* MapDocument::getLayer(unsigned int i)
{
	return p->Layers[i];
}

const MapLayer* MapDocument::getLayer(unsigned int i) const
{
	return p->Layers[i];
}

MapFeature* MapDocument::getFeature(const QString& id)
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

QString MapDocument::exportOSM(const CoordBox& aCoordBox)
{
	QString theExport, coreExport;

	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		if (TrackPoint* P = dynamic_cast<TrackPoint*>(i.get())) {
			if (aCoordBox.contains(P->position())) {
				coreExport += P->toXML(1) + "\n";
			}
		} else
			if (Road* G = dynamic_cast<Road*>(i.get())) {
				if (aCoordBox.intersects(G->boundingBox())) {
					for (unsigned int j=0; j < G->size(); j++) {
						if (TrackPoint* P = dynamic_cast<TrackPoint*>(G->get(j)))
							if (!aCoordBox.contains(P->position()))
								coreExport += P->toXML(1);
					}
					coreExport += G->toXML(1) + "\n";
				}
			} else
				//FIXME Not working for relation (not made of point?)
				if (Relation* G = dynamic_cast<Relation*>(i.get())) {
					if (aCoordBox.intersects(G->boundingBox())) {
						for (unsigned int j=0; j < G->size(); j++) {
							if (Road* R = dynamic_cast<Road*>(G->get(j))) {
								if (!aCoordBox.contains(R->boundingBox())) {
									for (unsigned int k=0; k < R->size(); k++) {
										if (TrackPoint* P = dynamic_cast<TrackPoint*>(R->get(k)))
											if (!aCoordBox.contains(P->position()))
												coreExport += P->toXML(1) + "\n";
									}
									coreExport += R->toXML(1) + "\n";
								}
							}
						}
						coreExport += G->toXML(1) + "\n";
					}
				}
	}

	theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
	theExport += "<osm version='0.5' generator='Merkaartor'>\n";
	theExport += "<bound box='";
	theExport += QString().number(radToAng(aCoordBox.bottomLeft().lat())) + ",";
	theExport += QString().number(radToAng(aCoordBox.bottomLeft().lon())) + ",";
	theExport += QString().number(radToAng(aCoordBox.topRight().lat())) + ",";
	theExport += QString().number(radToAng(aCoordBox.topRight().lon()));
	theExport += "' origin='http://www.openstreetmap.org/api/0.5' />\n";
	theExport += coreExport;
	theExport += "</osm>";

	return theExport;
}

bool MapDocument::importNMEA(const QString& filename, TrackMapLayer* NewLayer)
{
	ImportNMEA imp;
	if (!imp.loadFile(filename))
		return false;
	imp.import(NewLayer);

	if (NewLayer->size())
		return true;
	else
		return false;
}


/* VISIBLEFEATUREITERATOR */

VisibleFeatureIterator::VisibleFeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Layer(0), Idx(0)
{
	while (Layer < theDocument->layerSize())
	{
		MapLayer* L = theDocument->getLayer(Layer);
		if (L->isVisible() && L->size())
			break;
		++Layer;
	}
}

MapFeature* VisibleFeatureIterator::get()
{
	return theDocument->getLayer(Layer)->get(Idx);
}

bool VisibleFeatureIterator::isEnd() const
{
	return Layer >= theDocument->layerSize();
}

VisibleFeatureIterator& VisibleFeatureIterator::operator++()
{
	++Idx;
	if (Idx >= theDocument->getLayer(Layer)->size())
	{
		Idx = 0;
		++Layer;
		while (Layer < theDocument->layerSize())
		{
			MapLayer* L = theDocument->getLayer(Layer);
			if (L->isVisible() && L->size())
				break;
			++Layer;
		}
	}
	return *this;
}

MapLayer* VisibleFeatureIterator::layer()
{
	return theDocument->getLayer(Layer);
}

unsigned int VisibleFeatureIterator::index()
{
	return Idx;
}


/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Layer(0), Idx(0)
{
	while (Layer < theDocument->layerSize())
	{
		if (theDocument->getLayer(Layer)->size())
			break;
		++Layer;
	}
}

MapFeature* FeatureIterator::get()
{
	return theDocument->getLayer(Layer)->get(Idx);
}

bool FeatureIterator::isEnd() const
{
	return Layer >= theDocument->layerSize();
}

FeatureIterator& FeatureIterator::operator++()
{
	++Idx;
	if (Idx >= theDocument->getLayer(Layer)->size())
	{
		Idx = 0;
		++Layer;
		while (Layer < theDocument->layerSize())
		{
			if (theDocument->getLayer(Layer)->size())
				break;
			++Layer;
		}
	}
	return *this;
}

MapLayer* FeatureIterator::layer()
{
	return theDocument->getLayer(Layer);
}

unsigned int FeatureIterator::index()
{
	return Idx;
}

/* RELATED */

std::pair<bool,CoordBox> boundingBox(const MapDocument* theDocument)
{
	unsigned int First;
	for (First = 0; First < theDocument->layerSize(); ++First)
		if (theDocument->getLayer(First)->size())
			break;
	if (First == theDocument->layerSize())
		return std::make_pair(false,CoordBox(Coord(0,0),Coord(0,0)));
	CoordBox BBox(MapLayer::boundingBox(theDocument->getLayer(First)));
	for (unsigned int i=First+1; i<theDocument->layerSize(); ++i)
		if (theDocument->getLayer(i)->size())
			BBox.merge(MapLayer::boundingBox(theDocument->getLayer(i)));
	return std::make_pair(true,BBox);
}

bool hasUnsavedChanges(const MapDocument& aDoc)
{
	return aDoc.history().index();
}

MapLayer * MapDocument::getLastDownloadLayer()
{
	return p->lastDownloadLayer;
}

void MapDocument::setLastDownloadLayer(MapLayer * aLayer)
{
	p->lastDownloadLayer = aLayer;
}
