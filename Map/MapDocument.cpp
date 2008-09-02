#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Command/Command.h"
#include "Map/ImportOSM.h"

#include "Map/MapDocument.h"

#include "ImportExport/ImportNMEA.h"
#include "ImportExport/ImportExportOsmBin.h"

#include <QtCore/QString>
#include <QMultiMap>
#include <QProgressDialog>

#include <algorithm>
#include <map>
#include <vector>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

/* MAPDOCUMENT */

class MapDocumentPrivate
{
public:
	MapDocumentPrivate()
	: History(new CommandHistory()), imageLayer(0), dirtyLayer(0), uploadedLayer(0), trashLayer(0), theDock(0), lastDownloadLayer(0)
	{
    	tagList.insert("created_by", QString("Merkaartor %1").arg(VERSION));
		tagKeys.append("created_by");
		tagValues.append(QString("Merkaartor %1").arg(VERSION));
	};
	~MapDocumentPrivate()
	{
		History->cleanup();
		delete History;
		for (unsigned int i=0; i<Layers.size(); ++i) {
			if (theDock)
				theDock->deleteLayer(Layers[i]);
			delete Layers[i];
		}
	}
	CommandHistory*				History;
	std::vector<MapLayer*>		Layers;
	ImageMapLayer*				imageLayer;
	DirtyMapLayer*				dirtyLayer;
	UploadedMapLayer*			uploadedLayer;
	DeletedMapLayer*			trashLayer;
	LayerDock*					theDock;
	MapLayer*					lastDownloadLayer;

	QList<CoordBox>				downloadBoxes;

	QMultiMap<QString, QString> tagList;
	QList<QString>				tagKeys;
	QList<QString>				tagValues;
};

MapDocument::MapDocument()
	: p(new MapDocumentPrivate)
{
	p->imageLayer = new ImageMapLayer(tr("Background imagery"));
	add(p->imageLayer);

	p->trashLayer = new DeletedMapLayer(tr("Trash layer"));
	add(p->trashLayer);

	p->dirtyLayer = new DirtyMapLayer(tr("Dirty layer"));
	add(p->dirtyLayer);

	p->uploadedLayer = new UploadedMapLayer(tr("Uploaded layer"));
	add(p->uploadedLayer);
}

MapDocument::MapDocument(LayerDock* aDock)
: p(new MapDocumentPrivate)
{
	p->theDock = aDock;

	p->imageLayer = new ImageMapLayer(tr("Background imagery"));
	add(p->imageLayer);

	p->trashLayer = new DeletedMapLayer(tr("Trash layer"));
	add(p->trashLayer);

	p->dirtyLayer = new DirtyMapLayer(tr("Dirty layer"));
	add(p->dirtyLayer);

	p->uploadedLayer = new UploadedMapLayer(tr("Uploaded layer"));
	add(p->uploadedLayer);
}

MapDocument::MapDocument(const MapDocument&, LayerDock*)
: p(0)
{
}

MapDocument::~MapDocument()
{
	delete p;
}

bool MapDocument::toXML(QDomElement xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement mapDoc = xParent.namedItem("MapDocument").toElement();
	if (!mapDoc.isNull()) {
		xParent.removeChild(mapDoc);
	}
	mapDoc = xParent.ownerDocument().createElement("MapDocument");
	xParent.appendChild(mapDoc);

	if (p->lastDownloadLayer)
		mapDoc.setAttribute("lastdownloadlayer", p->lastDownloadLayer->id());

	for (unsigned int i=0; i<p->Layers.size(); ++i) {
		progress.setMaximum(progress.maximum() + p->Layers[i]->size());
	}

	for (unsigned int i=0; i<p->Layers.size(); ++i) {
		p->Layers[i]->toXML(mapDoc, progress);
	}

	OK = history().toXML(mapDoc, progress);

	return OK;
}

MapDocument* MapDocument::fromXML(const QDomElement e, double version, LayerDock* aDock, QProgressDialog & progress)
{
	MapDocument* NewDoc = new MapDocument(aDock);

	CommandHistory* h = 0;

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "ImageMapLayer") {
			/* ImageMapLayer* l = */ ImageMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "DeletedMapLayer") {
			/* TrashMapLayer* l = */ DeletedMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "DirtyMapLayer") {
			/* DirtyMapLayer* l = */ DirtyMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "UploadedMapLayer") {
			/* UploadedMapLayer* l = */ UploadedMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "DrawingMapLayer") {
			/* DrawingMapLayer* l = */ DrawingMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "TrackMapLayer") {
			/* TrackMapLayer* l = */ TrackMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "ExtractedMapLayer") {
			/* ExtractedMapLayer* l = */ ExtractedMapLayer::fromXML(NewDoc, c, progress);
		} else
		if (c.tagName() == "CommandHistory") {
			if (version > 1.0)
				h = CommandHistory::fromXML(NewDoc, c, progress);
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}

	if (progress.wasCanceled()) {
		delete NewDoc;
		NewDoc = NULL;
	}

	if (NewDoc) {
		if (e.hasAttribute("lastdownloadlayer"))
			NewDoc->setLastDownloadLayer(NewDoc->getLayer(e.attribute("lastdownloadlayer")));

		if (h)
			NewDoc->setHistory(h);
	}

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
	p->imageLayer = new ImageMapLayer(tr("Background imagery"));
	p->imageLayer->setMapAdapter(MerkaartorPreferences::instance()->getBgType());
	add(p->imageLayer);

	p->dirtyLayer = new DirtyMapLayer(tr("Dirty layer"));
	add(p->dirtyLayer);

	p->uploadedLayer = new UploadedMapLayer(tr("Uploaded layer"));
	add(p->uploadedLayer);
}

void MapDocument::setHistory(CommandHistory* h)
{
	delete p->History;
	p->History = h;
	emit(historyChanged());
}

CommandHistory& MapDocument::history()
{
	return *(p->History);
}

const CommandHistory& MapDocument::history() const
{
	return *(p->History);
}

void MapDocument::addHistory(Command* aCommand)
{
	p->History->add(aCommand);
	emit(historyChanged());
}

void MapDocument::redoHistory()
{
	p->History->redo();
	emit(historyChanged());
}

void MapDocument::undoHistory()
{
	p->History->undo();
	emit(historyChanged());
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
#ifndef _MOBILE
	if (!p->tagList.contains(k, v)) {
    	p->tagList.insert(k, v);
	}
	if (!p->tagKeys.contains(k)) {
		p->tagKeys.append(k);
	}
	if (!p->tagValues.contains(v)) {
		p->tagValues.append(v);
	}
#endif
}

QList<QString> MapDocument::getTagKeys()
{
	return p->tagKeys;
}

void MapDocument::setTagKeys(QList<QString> list)
{
	p->tagKeys = list;
}

QString MapDocument::getTagKey(int idx)
{
	return p->tagKeys[idx];
}

int MapDocument::getTagKeyIndex(QString k)
{
	return p->tagKeys.indexOf(k);
}

QList<QString> MapDocument::getTagValues()
{
	return p->tagValues;
}

void MapDocument::setTagValues(QList<QString> list)
{
	p->tagValues = list;
}

QString MapDocument::getTagValue(int idx)
{
	return p->tagValues[idx];
}

int MapDocument::getTagValueIndex(QString v)
{
	return p->tagValues.indexOf(v);
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

QVector<MapFeature*> MapDocument::getFeatures(QString* layerType)
{
	QVector<MapFeature*> theFeatures;
	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		if (!layerType)
			theFeatures.append(i.get());
		else
			if (i.get()->layer()->className() == *layerType)
				theFeatures.append(i.get());
	}
	return theFeatures;
}

MapFeature* MapDocument::getFeature(const QString& id, bool exact)
{
	for (unsigned int i=0; i<p->Layers.size(); ++i)
	{
		MapFeature* F = p->Layers[i]->get(id);
		if (F) 
			return F;
		if (!exact) {
			if ((F = p->Layers[i]->get("node_"+id)))
				return F;
			if ((F = p->Layers[i]->get("way_"+id)))
				return F;
			if ((F = p->Layers[i]->get("rel_"+id)))
				return F;
		}
	}
	return 0;
}

ImageMapLayer* MapDocument::getImageLayer() const
{
	return p->imageLayer;
}

DirtyMapLayer* MapDocument::getDirtyLayer() const
{
	return p->dirtyLayer;
}

DeletedMapLayer* MapDocument::getTrashLayer() const
{
	return p->trashLayer;
}

MapLayer* MapDocument::getDirtyOrOriginLayer(MapLayer* aLayer) 
{
	if (aLayer->isUploadable())
		return p->dirtyLayer;
	else
		return aLayer;
}

UploadedMapLayer* MapDocument::getUploadedLayer() const
{
	return p->uploadedLayer;
}

QString MapDocument::exportOSM(const CoordBox& aCoordBox, bool renderBounds)
{
	QString theExport, coreExport;
	QVector<MapFeature*> theFeatures;

	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		if (TrackPoint* P = dynamic_cast<TrackPoint*>(i.get())) {
			if (aCoordBox.contains(P->position())) {
				theFeatures.append(P);
			}
		} else
			if (Road* G = dynamic_cast<Road*>(i.get())) {
				if (aCoordBox.intersects(G->boundingBox())) {
					for (unsigned int j=0; j < G->size(); j++) {
						if (TrackPoint* P = dynamic_cast<TrackPoint*>(G->get(j)))
							if (!aCoordBox.contains(P->position()))
								theFeatures.append(P);
					}
					theFeatures.append(G);
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
												theFeatures.append(P);
									}
									theFeatures.append(R);
								}
							}
						}
						theFeatures.append(G);
					}
				}
	}

	QVector<MapFeature*> exportedFeatures = exportCoreOSM(theFeatures);

	if (exportedFeatures.size()) {
		for (int i=0; i < exportedFeatures.size(); i++) {
			coreExport += exportedFeatures[i]->toXML(1) + "\n";
		}
	}
	theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
	theExport += "<osm version='0.5' generator='Merkaartor'>\n";
	theExport += "<bound box='";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
	theExport += "' origin='http://www.openstreetmap.org/api/0.5' />\n";
	if (renderBounds) {
		theExport += "<bounds ";
		theExport += "minlat=\"" + QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + "\" ";
		theExport += "minlon=\"" + QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + "\" ";
		theExport += "maxlat=\"" + QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + "\" ";
		theExport += "maxlon=\"" + QString().number(intToAng(aCoordBox.topRight().lon()),'f',6) + "\" ";
		theExport += "/>\n";
	}
	theExport += coreExport;
	theExport += "</osm>";

	return theExport;
}

QString MapDocument::exportOSM(QVector<MapFeature*> aFeatures)
{
	QString theExport, coreExport;
	QVector<MapFeature*> exportedFeatures = exportCoreOSM(aFeatures);
	CoordBox aCoordBox;

	if (exportedFeatures.size()) {
		aCoordBox = exportedFeatures[0]->boundingBox();
		coreExport += exportedFeatures[0]->toXML(1) + "\n";
		for (int i=1; i < exportedFeatures.size(); i++) {
			aCoordBox.merge(exportedFeatures[i]->boundingBox());
			coreExport += exportedFeatures[i]->toXML(1) + "\n";
		}
	}
	theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
	theExport += "<osm version='0.5' generator='Merkaartor'>\n";
	theExport += "<bound box='";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
	theExport += "' origin='http://www.openstreetmap.org/api/0.5' />\n";
	theExport += coreExport;
	theExport += "</osm>";

	return theExport;
}

QVector<MapFeature*> MapDocument::exportCoreOSM(QVector<MapFeature*> aFeatures)
{
	QString coreExport;
	QVector<MapFeature*> exportedFeatures;
	QVector<MapFeature*>::Iterator i;

	for (i = aFeatures.begin(); i != aFeatures.end(); ++i) {
		if (/*TrackPoint* P = */dynamic_cast<TrackPoint*>(*i)) {
			if (!exportedFeatures.contains(*i))
				exportedFeatures.append(*i);
		} else {
			if (Road* G = dynamic_cast<Road*>(*i)) {
				for (unsigned int j=0; j < G->size(); j++) {
					if (TrackPoint* P = dynamic_cast<TrackPoint*>(G->get(j))) {
						if (!exportedFeatures.contains(P))
							exportedFeatures.append(P);
					}
					if (!exportedFeatures.contains(G))
						exportedFeatures.append(G);
				}
			} else {
				//FIXME Not working for relation (not made of point?)
				if (Relation* G = dynamic_cast<Relation*>(*i)) {
					for (unsigned int j=0; j < G->size(); j++) {
						if (Road* R = dynamic_cast<Road*>(G->get(j))) {
							for (unsigned int k=0; k < R->size(); k++) {
								if (TrackPoint* P = dynamic_cast<TrackPoint*>(R->get(k))) {
									if (!exportedFeatures.contains(P))
										exportedFeatures.append(P);
								}
							}
						if (!exportedFeatures.contains(R))
							exportedFeatures.append(R);
						}
					}
					if (!exportedFeatures.contains(G))
						exportedFeatures.append(G);
				}
			}
		}
	}

	return exportedFeatures;
}

bool MapDocument::importNMEA(const QString& filename, TrackMapLayer* NewLayer)
{
	ImportNMEA imp(this);
	if (!imp.loadFile(filename))
		return false;
	imp.import(NewLayer);

	if (NewLayer->size())
		return true;
	else
		return false;
}

bool MapDocument::importOSB(const QString& filename, DrawingMapLayer* NewLayer)
{
	Q_UNUSED(filename)
	Q_UNUSED(NewLayer)
	//ImportExportOsmBin imp(this);
	//if (!imp.loadFile(filename))
	//	return false;
	//imp.import(NewLayer);

	//if (NewLayer->size())
	//	return true;
	//else
	//	return false;
	return true;
}

void MapDocument::addDownloadBox(CoordBox aBox)
{
	p->downloadBoxes.append(aBox);
}

QList<CoordBox> *MapDocument::getDownloadBoxes()
{
	return &(p->downloadBoxes);
}

MapLayer * MapDocument::getLastDownloadLayer()
{
	return p->lastDownloadLayer;
}

void MapDocument::setLastDownloadLayer(MapLayer * aLayer)
{
	p->lastDownloadLayer = aLayer;
}

/* VISIBLEFEATUREITERATOR */

VisibleFeatureIterator::VisibleFeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Idx(0)
{
	for (unsigned int i=0; i<theDocument->layerSize(); ++i) {
		if (!theDocument->getLayer(i)->isVisible())
			continue;
		for (unsigned int j=0; j<theDocument->getLayer(i)->size(); ++j)
			theFeatures.push_back(theDocument->getLayer(i)->get(j));
	}
}

MapFeature* VisibleFeatureIterator::get()
{
	return theFeatures[Idx];
}

bool VisibleFeatureIterator::isEnd() const
{
	return Idx >= (unsigned int)theFeatures.size();
}

VisibleFeatureIterator& VisibleFeatureIterator::operator++()
{
	++Idx;
	return *this;
}

unsigned int VisibleFeatureIterator::index()
{
	return Idx;
}


/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), Idx(0)
{
	for (unsigned int i=0; i<theDocument->layerSize(); ++i)
		for (unsigned int j=0; j<theDocument->getLayer(i)->size(); ++j)
			theFeatures.push_back(theDocument->getLayer(i)->get(j));
}

MapFeature* FeatureIterator::get()
{
	return theFeatures[Idx];
}

bool FeatureIterator::isEnd() const
{
	return Idx >= (unsigned int)theFeatures.size();
}

FeatureIterator& FeatureIterator::operator++()
{
	++Idx;
	return *this;
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
		if (theDocument->getLayer(First)->size() && theDocument->getLayer(First) != theDocument->getTrashLayer())
			break;
	if (First == theDocument->layerSize())
		return std::make_pair(false,CoordBox(Coord(0,0),Coord(0,0)));
	CoordBox BBox(MapLayer::boundingBox(theDocument->getLayer(First)));
	for (unsigned int i=First+1; i<theDocument->layerSize(); ++i)
		if (theDocument->getLayer(i)->size() && theDocument->getLayer(First) != theDocument->getTrashLayer())
			BBox.merge(MapLayer::boundingBox(theDocument->getLayer(i)));
	return std::make_pair(true,BBox);
}

bool hasUnsavedChanges(const MapDocument& aDoc)
{
//	return aDoc.history().index();
	return (aDoc.getDirtyLayer()->size() > 0);
}

