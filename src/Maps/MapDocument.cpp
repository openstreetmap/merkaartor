#include "Maps/MapFeature.h"
#include "Command/Command.h"
#include "Maps/ImportOSM.h"

#include "Maps/MapDocument.h"
#include "Maps/ImageMapLayer.h"

#include "ImportExport/ImportNMEA.h"
#include "ImportExport/ImportExportOsmBin.h"
#include "ImportExport/ImportExportKML.h"
#include "ImportExport/ImportExportSHP.h"

#include <QtCore/QString>
#include <QMultiMap>
#include <QProgressDialog>

#include <QMap>
#include <QList>

#include <QSet>

/* MAPDOCUMENT */

class MapDocumentPrivate
{
public:
	MapDocumentPrivate()
	: History(new CommandHistory()), imageLayer(0), dirtyLayer(0), uploadedLayer(0)/*, trashLayer(0)*/, theDock(0), lastDownloadLayer(0)
	{
	};
	~MapDocumentPrivate()
	{
		History->cleanup();
		delete History;
		for (int i=0; i<Layers.size(); ++i) {
			if (theDock)
				theDock->deleteLayer(Layers[i]);
			delete Layers[i];
		}
	}
	CommandHistory*				History;
	QList<MapLayer*>		Layers;
	ImageMapLayer*				imageLayer;
	DirtyMapLayer*				dirtyLayer;
	UploadedMapLayer*			uploadedLayer;
	//DeletedMapLayer*			trashLayer;
	LayerDock*					theDock;
	MapLayer*					lastDownloadLayer;

	QHash<MapLayer*, CoordBox>		downloadBoxes;

	QHash< QString, QSet<QString> * >		tagList;
	
};

MapDocument::MapDocument()
	: p(new MapDocumentPrivate)
{
	addToTagList("created_by", QString("Merkaartor %1").arg(VERSION));

	p->imageLayer = new ImageMapLayer(tr("Background imagery"));
	add(p->imageLayer);

	//p->trashLayer = new DeletedMapLayer(tr("Trash layer"));
	//add(p->trashLayer);

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

	//p->trashLayer = new DeletedMapLayer(tr("Trash layer"));
	//add(p->trashLayer);

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

	for (int i=0; i<p->Layers.size(); ++i) {
		progress.setMaximum(progress.maximum() + p->Layers[i]->size());
	}

	for (int i=0; i<p->Layers.size(); ++i) {
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
		if (c.tagName() == "OsbMapLayer") {
			/* OsbMapLayer* l = */ OsbMapLayer::fromXML(NewDoc, c, progress);
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
	p->imageLayer->setMapAdapter(MerkaartorPreferences::instance()->getBackgroundPlugin());
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
	if (p->tagList.contains(k)) {
		if (!p->tagList.value(k)->contains(v)) {
			//static_cast< QSet<QString> * >(p->tagList.value(k))->insert(v);
			p->tagList.value(k)->insert(v);
		}
	} else {
		QSet<QString> *values = new QSet<QString>;
		values->insert(v);
		p->tagList.insert(k, values);
	}
#endif
}

QList<QString> MapDocument::getTagKeys()
{
	return p->tagList.keys();
}

QList<QString> MapDocument::getTagValues()
{
	return getTagValueList("*");
}

QStringList MapDocument::getTagList()
{
	qDebug() << p->tagList.uniqueKeys() << endl;
	return p->tagList.uniqueKeys();
}

QStringList MapDocument::getTagValueList(QString k)
{
	if (k == "*") {
		QSet<QString> allValues;
		QSet<QString> *tagValues;
		foreach (tagValues, p->tagList) {
			allValues += *tagValues;
		}
		return allValues.toList();
	} else if (p->tagList.contains(k)) {
		return p->tagList.value(k)->toList();
	} else {
		return QStringList();
	}
}

void MapDocument::remove(MapLayer* aLayer)
{
	QList<MapLayer*>::iterator i = qFind(p->Layers.begin(),p->Layers.end(), aLayer);
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
	for (int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i] == L) return true;
	return false;
}

bool MapDocument::exists(MapFeature* F) const
{
	for (int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i]->exists(F)) return true;
	return false;
}

void MapDocument::deleteFeature(MapFeature* aFeature)
{
	for (int i=0; i<p->Layers.size(); ++i)
		if (p->Layers[i]->exists(aFeature)) {
			p->Layers[i]->deleteFeature(aFeature);
			return;
		}
}

int MapDocument::layerSize() const
{
	return p->Layers.size();
}

MapLayer* MapDocument::getLayer(const QString& id)
{
	for (int i=0; i<p->Layers.size(); ++i)
	{
		if (p->Layers[i]->id() == id) return p->Layers[i];
	}
	return 0;
}

MapLayer* MapDocument::getLayer(int i)
{
	return p->Layers[i];
}

const MapLayer* MapDocument::getLayer(int i) const
{
	return p->Layers[i];
}

QList<MapFeature*> MapDocument::getFeatures(MapLayer::LayerType layerType)
{
	QList<MapFeature*> theFeatures;
	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		if (!layerType)
			theFeatures.append(i.get());
		else
			if (i.get()->layer()->classType() == layerType)
				theFeatures.append(i.get());
	}
	return theFeatures;
}

MapFeature* MapDocument::getFeature(const QString& id, bool exact)
{
	for (int i=0; i<p->Layers.size(); ++i)
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

//DirtyMapLayer* MapDocument::getDirtyLayer() const
//{
//	return p->dirtyLayer;
//}

//DeletedMapLayer* MapDocument::getTrashLayer() const
//{
//	return p->trashLayer;
//}

MapLayer* MapDocument::getDirtyOrOriginLayer(MapLayer* aLayer) const
{
	if (!aLayer || aLayer->isUploadable())
		return p->dirtyLayer;
	else
		return aLayer;
}

MapLayer* MapDocument::getDirtyOrOriginLayer(MapFeature* F) const
{
	if (!F || !F->layer() || F->layer()->isUploadable())
		return p->dirtyLayer;
	else
		return F->layer();
}

UploadedMapLayer* MapDocument::getUploadedLayer() const
{
	return p->uploadedLayer;
}

QString MapDocument::exportOSM(const CoordBox& aCoordBox, bool renderBounds)
{
	QString theExport, coreExport;
	QList<MapFeature*> theFeatures;

	for (VisibleFeatureIterator i(this); !i.isEnd(); ++i) {
		if (TrackPoint* P = dynamic_cast<TrackPoint*>(i.get())) {
			if (aCoordBox.contains(P->position())) {
				theFeatures.append(P);
			}
		} else
			if (Road* G = dynamic_cast<Road*>(i.get())) {
				if (aCoordBox.intersects(G->boundingBox())) {
					for (int j=0; j < G->size(); j++) {
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
						for (int j=0; j < G->size(); j++) {
							if (Road* R = dynamic_cast<Road*>(G->get(j))) {
								if (!aCoordBox.contains(R->boundingBox())) {
									for (int k=0; k < R->size(); k++) {
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

	QList<MapFeature*> exportedFeatures = exportCoreOSM(theFeatures);

	if (exportedFeatures.size()) {
		for (int i=0; i < exportedFeatures.size(); i++) {
			coreExport += exportedFeatures[i]->toXML(1) + "\n";
		}
	}
	theExport += "<?xml version='1.0' encoding='UTF-8'?>\n";
	theExport += QString("<osm version='%1' generator='Merkaartor'>\n").arg(M_PREFS->apiVersion());
	theExport += "<bound box='";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
	theExport += QString("' origin='http://www.openstreetmap.org/api/%1' />\n").arg(M_PREFS->apiVersion());
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

QString MapDocument::exportOSM(QList<MapFeature*> aFeatures)
{
	QString theExport, coreExport;
	QList<MapFeature*> exportedFeatures = exportCoreOSM(aFeatures);
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
	theExport += QString("<osm version='%1' generator='Merkaartor'>\n").arg(M_PREFS->apiVersion());
	theExport += "<bound box='";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.bottomLeft().lon()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lat()),'f',6) + ",";
	theExport += QString().number(intToAng(aCoordBox.topRight().lon()),'f',6);
	theExport += QString("' origin='http://www.openstreetmap.org/api/%1' />\n").arg(M_PREFS->apiVersion());
	theExport += coreExport;
	theExport += "</osm>";

	return theExport;
}

QList<MapFeature*> MapDocument::exportCoreOSM(QList<MapFeature*> aFeatures)
{
	QString coreExport;
	QList<MapFeature*> exportedFeatures;
	QList<MapFeature*>::Iterator i;

	for (i = aFeatures.begin(); i != aFeatures.end(); ++i) {
		if (/*TrackPoint* P = */dynamic_cast<TrackPoint*>(*i)) {
			if (!exportedFeatures.contains(*i))
				exportedFeatures.append(*i);
		} else {
			if (Road* G = dynamic_cast<Road*>(*i)) {
				for (int j=0; j < G->size(); j++) {
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
					for (int j=0; j < G->size(); j++) {
						if (Road* R = dynamic_cast<Road*>(G->get(j))) {
							for (int k=0; k < R->size(); k++) {
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

bool MapDocument::importKML(const QString& filename, TrackMapLayer* NewLayer)
{
	ImportExportKML imp(this);
	if (!imp.loadFile(filename))
		return false;
	imp.import(NewLayer);

	if (NewLayer->size())
		return true;
	else
		return false;
}

bool MapDocument::importSHP(const QString& filename, DrawingMapLayer* NewLayer)
{
	Q_UNUSED(filename)
	Q_UNUSED(NewLayer)

#ifdef USE_GDAL
	ImportExportSHP imp(this);
	if (!imp.loadFile(filename))
		return false;
	imp.import(NewLayer);

	if (NewLayer->size())
		return true;
	else
		return false;
#else
	return false;
#endif
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

void MapDocument::addDownloadBox(MapLayer* l, CoordBox aBox)
{
	p->downloadBoxes.insertMulti(l, aBox);
}

void MapDocument::removeDownloadBox(MapLayer* l)
{
	p->downloadBoxes.remove(l);
}

const QList<CoordBox> MapDocument::getDownloadBoxes() const
{
	return p->downloadBoxes.values();
}

MapLayer * MapDocument::getLastDownloadLayer()
{
	return p->lastDownloadLayer;
}

void MapDocument::setLastDownloadLayer(MapLayer * aLayer)
{
	p->lastDownloadLayer = aLayer;
}

/* FEATUREITERATOR */

FeatureIterator::FeatureIterator(MapDocument *aDoc)
: theDocument(aDoc), curLayerIdx(0), curFeatureIdx(0), isAtEnd(false)
{
	docSize = theDocument->layerSize();
	curLayerSize = theDocument->getLayer(curLayerIdx)->size();

	if(!check() && !isAtEnd)
		++(*this);
}

FeatureIterator::~FeatureIterator()
{
}

MapFeature* FeatureIterator::get()
{
	return theDocument->getLayer(curLayerIdx)->get(curFeatureIdx);
}

bool FeatureIterator::isEnd() const
{
	return isAtEnd;
}

FeatureIterator& FeatureIterator::operator++()
{
	docSize = theDocument->layerSize();
	curLayerSize = theDocument->getLayer(curLayerIdx)->size();

	if (curFeatureIdx < curLayerSize-1)
		curFeatureIdx++;
	else
		if (curLayerIdx < docSize-1) {
			curLayerIdx++;
			curLayerSize = theDocument->getLayer(curLayerIdx)->size();
			curFeatureIdx = 0;
		} else
			isAtEnd = true;

	while(!isAtEnd && !check()) {
		if (curFeatureIdx < curLayerSize-1)
			curFeatureIdx++;
		else
			if (curLayerIdx < docSize-1) {
				curLayerIdx++;
				curLayerSize = theDocument->getLayer(curLayerIdx)->size();
				curFeatureIdx = 0;
			} else
				isAtEnd = true;
	}

	return *this;
}

int FeatureIterator::index()
{
	return (curLayerIdx*10000000)+curFeatureIdx;
}

bool FeatureIterator::check()
{
	if (curLayerIdx >= docSize) {
		isAtEnd = true;
		return false;
	}
	if (curFeatureIdx >= curLayerSize)
		return false;

	MapFeature* curFeature = theDocument->getLayer(curLayerIdx)->get(curFeatureIdx);
	if (curFeature->lastUpdated() == MapFeature::NotYetDownloaded 
			|| curFeature->isDeleted())
		return false;

	return true;
}


/* VISIBLEFEATUREITERATOR */

VisibleFeatureIterator::VisibleFeatureIterator(MapDocument *aDoc)
: FeatureIterator(aDoc)
{
	if(!check() && !isAtEnd)
		++(*this);
}

VisibleFeatureIterator::~VisibleFeatureIterator()
{
}

bool VisibleFeatureIterator::check()
{
	if (!FeatureIterator::check())
		return false;
	else {
		if (theDocument->getLayer(curLayerIdx)->isVisible()) {
			if (CAST_NODE(theDocument->getLayer(curLayerIdx)->get(curFeatureIdx)) 
					&& !(theDocument->getLayer(curLayerIdx)->arePointsDrawable()))
				return false;
		} else
			return false;
	}

	return true;
}


/* RELATED */

QPair<bool,CoordBox> boundingBox(const MapDocument* theDocument)
{
	int First;
	for (First = 0; First < theDocument->layerSize(); ++First)
		if (theDocument->getLayer(First)->size())
			break;
	if (First == theDocument->layerSize())
		return qMakePair(false,CoordBox(Coord(0,0),Coord(0,0)));
	CoordBox BBox(MapLayer::boundingBox(theDocument->getLayer(First)));
	for (int i=First+1; i<theDocument->layerSize(); ++i)
		if (theDocument->getLayer(i)->size())
			BBox.merge(MapLayer::boundingBox(theDocument->getLayer(i)));
	return qMakePair(true,BBox);
}

bool hasUnsavedChanges(const MapDocument& aDoc)
{
//	return aDoc.history().index();
	return (aDoc.getDirtyOrOriginLayer()->getDirtySize() > 0);
}

