#include "Map/MapLayer.h"

#include "Map/MapFeature.h"

#include "Map/MapDocument.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"
#include "LayerWidget.h"
#include "Map/ImportOSM.h"

#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"

#include "Utils/LineF.h"

#include "IMapAdapter.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
#include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/wmsmapadapter.h"
#include "QMapControl/layer.h"
#include "QMapControl/layermanager.h"

#include "ImportExport/ImportExportOsmBin.h"

#include <QtCore/QString>
#include <QMultiMap>
#include <QProgressDialog>
#include <QUuid>

#include <algorithm>
#include <map>
#include <vector>

/* MAPLAYER */

class MapLayerPrivate
{
public:
	MapLayerPrivate()
		: RenderPriorityUpToDate(false)
	{
		theDocument = NULL;
		layer_bg = NULL;
		theWidget = NULL;
		selected = false;
		Enabled = true;
		Readonly = false;
	}
	~MapLayerPrivate()
	{
		//for (int i=0; i<Features.size(); ++i)
		//	if (Features[i])
		//		delete Features[i];
	}
	QList<MapFeaturePtr> Features;
	QHash<QString, MapFeaturePtr> IdMap;
	QString Name;
	QString Description;
	bool Visible;
	bool selected;
	bool Enabled;
	bool Readonly;
	LayerWidget* theWidget;
	qreal alpha;
	unsigned int dirtyLevel;

	QUuid bgType;
	Layer* layer_bg;

	bool RenderPriorityUpToDate;
	MapDocument* theDocument;
 	double RenderPriorityForPixelPerM;

 	void sortRenderingPriority(double PixelPerM);
};

class SortAccordingToRenderingPriority
{
	public:
 		SortAccordingToRenderingPriority(double aPixelPerM)
 			: PixelPerM(aPixelPerM)
		{
		}
		bool operator()(MapFeature* A, MapFeature* B)
		{
 			return A->renderPriority(PixelPerM) < B->renderPriority(PixelPerM);
		}

 		double PixelPerM;
};

 void MapLayerPrivate::sortRenderingPriority(double aPixelPerM)
{
 	std::sort(Features.begin(),Features.end(),SortAccordingToRenderingPriority(aPixelPerM));
  	RenderPriorityUpToDate = true;
 	RenderPriorityForPixelPerM = aPixelPerM;
}

MapLayer::MapLayer()
:  p(new MapLayerPrivate)
{
	p->alpha = 1.0;
	p->dirtyLevel = 0;
}

MapLayer::MapLayer(const QString& aName)
:  p(new MapLayerPrivate)
{
	p->Name = aName;
	p->alpha = 1.0;
	p->dirtyLevel = 0;
}

MapLayer::MapLayer(const MapLayer&)
: QObject(), p(0)
{
}

MapLayer::~MapLayer()
{
	SAFE_DELETE(p);
}

void MapLayer::sortRenderingPriority(double aPixelPerM)
{
	// No need to resort after a zoom, is there?
	// if (!p->RenderPriorityUpToDate || (aPixelPerM != p->RenderPriorityForPixelPerM) )
	if (!p->RenderPriorityUpToDate)
		p->sortRenderingPriority(aPixelPerM);
}

void MapLayer::invalidateRenderPriority()
{
	p->RenderPriorityUpToDate = false;
}

void MapLayer::setName(const QString& s)
{
	p->Name = s;
}

const QString& MapLayer::name() const
{
	return p->Name;
}

void MapLayer::setDescription(const QString& s)
{
	p->Description = s;
}

const QString& MapLayer::description() const
{
	return p->Description;
}

bool MapLayer::isVisible() const
{
	return p->Visible;
}

void MapLayer::setSelected(bool b) {
	p->selected = b;
}

bool MapLayer::isSelected() const
{
	return p->selected;
}

void MapLayer::setEnabled(bool b) {
	p->Enabled = b;
	if (p->theWidget) {
		p->theWidget->setVisible(b);
		p->theWidget->getAssociatedMenu()->menuAction()->setVisible(b);
	}
}

bool MapLayer::isEnabled() const
{
	return p->Enabled;
}

void MapLayer::setReadonly(bool b) {
	p->Readonly = b;
}

bool MapLayer::isReadonly() const
{
	return p->Readonly;
}

void MapLayer::add(MapFeature* aFeature)
{
	aFeature->setLayer(this);
	p->Features.push_back(aFeature);
	notifyIdUpdate(aFeature->id(),aFeature);
	p->RenderPriorityUpToDate = false;
}

void MapLayer::add(MapFeature* aFeature, unsigned int Idx)
{
	add(aFeature);
	std::rotate(p->Features.begin()+Idx,p->Features.end()-1,p->Features.end());
}

void MapLayer::notifyIdUpdate(const QString& id, MapFeature* aFeature)
{
	if(p)
		p->IdMap[id] = aFeature;
}

void MapLayer::remove(MapFeature* aFeature)
{
	if (p->Features.removeOne(aFeature))
	{
		aFeature->setLayer(0);
		notifyIdUpdate(aFeature->id(),0);
		p->RenderPriorityUpToDate = false;
	}
}

void MapLayer::deleteFeature(MapFeature* aFeature)
{
	if (p->Features.removeOne(aFeature))
	{
		aFeature->setLayer(0);
		notifyIdUpdate(aFeature->id(),0);
		p->RenderPriorityUpToDate = false;
	}
	delete aFeature;
}

void MapLayer::clear()
{
	QList<MapFeaturePtr>::iterator i;
	for (i=p->Features.begin(); i != p->Features.end();)
	{
		(*i)->setLayer(0);
		notifyIdUpdate((*i)->id(),0);
		p->RenderPriorityUpToDate = false;
		i = p->Features.erase(i);
	}
}

bool MapLayer::exists(MapFeature* F) const
{
	int i = p->Features.indexOf(F);
	return (i != -1);
}

unsigned int MapLayer::size() const
{
	return p->Features.size();
}

void MapLayer::setDocument(MapDocument* aDocument)
{
    p->theDocument = aDocument;
}

MapDocument* MapLayer::getDocument()
{
    return p->theDocument;
}

int MapLayer::get(MapFeature* aFeature)
{
	for (int i=0; i<p->Features.size(); ++i)
		if (p->Features[i] == aFeature)
			return i;

	return -1;
}

QList<MapFeature *> MapLayer::get()
{
	QList<MapFeature *> theList;
	for (int i=0; i<p->Features.size(); ++i)
		if (p->Features[i])
			theList.append(p->Features[i]);
	return theList;
}


MapFeature* MapLayer::get(unsigned int i)
{
	return p->Features[i];
}

MapFeature* MapLayer::get(const QString& id, bool exact)
{
	QHash<QString, MapFeaturePtr>::const_iterator i;
	
	i = p->IdMap.find(id);
	if (i != p->IdMap.end())
		return i.value();

	if (!exact) {
		i = p->IdMap.find(QString("node_"+id));
		if (i != p->IdMap.end())
			return i.value();
		i = p->IdMap.find(QString("way_"+id));
		if (i != p->IdMap.end())
			return i.value();
		i = p->IdMap.find(QString("rel_"+id));
		if (i != p->IdMap.end())
			return i.value();
	}
	return 0;
}

const MapFeature* MapLayer::get(unsigned int i) const
{
	if((int)i>=p->Features.size()) return 0;
	return p->Features[i];
}

LayerWidget* MapLayer::getWidget(void)
{
	return p->theWidget;
}

void MapLayer::deleteWidget(void)
{
//	p->theWidget->deleteLater();
	delete p->theWidget;
	p->theWidget = NULL;
}

void MapLayer::setAlpha(const qreal a)
{
	p->alpha = a;
}

qreal MapLayer::getAlpha() const
{
	return p->alpha;
}

void MapLayer::setId(const QString& id)
{
	Id = id;
}

const QString& MapLayer::id() const
{
	if (Id == "")
		Id = QUuid::createUuid().toString();
	return Id;
}

CoordBox MapLayer::boundingBox(const MapLayer* theLayer)
{
	if(theLayer->size()==0) return CoordBox(Coord(0,0),Coord(0,0));
	CoordBox Box;
	bool haveFirst = false;
	for (unsigned int i=0; i<theLayer->size(); ++i) {
		if (theLayer->get(i)->isDeleted())
			continue;
		if (haveFirst)
			Box.merge(theLayer->get(i)->boundingBox());
		else {
			Box = theLayer->get(i)->boundingBox();
			haveFirst = true;
		}
	}
	return Box;
}

unsigned int MapLayer::incDirtyLevel(unsigned int inc)
{
	return p->dirtyLevel += inc;
}

unsigned int MapLayer::decDirtyLevel(unsigned int inc)
{
	return p->dirtyLevel -= inc;
}

unsigned int MapLayer::setDirtyLevel(unsigned int newLevel)
{
	return (p->dirtyLevel = newLevel);
}

unsigned int MapLayer::getDirtyLevel()
{
	return p->dirtyLevel;
}

unsigned int MapLayer::getDirtySize()
{
	int dirtyObjects = 0;

	QList<MapFeaturePtr>::const_iterator i;
	for (i = p->Features.constBegin(); i != p->Features.constEnd(); i++)
		if (!((*i)->isDeleted()) || ((*i)->isDeleted() && (*i)->hasOSMId()))
			++dirtyObjects;

	return dirtyObjects;
}

bool MapLayer::canDelete()
{
	return (p->dirtyLevel == 0);
}

QString MapLayer::toMainHtml()
{
	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(p->Name);
	if (!p->Description.isEmpty())
		desc += QString("<b>%1</b><br/>").arg(p->Description);
	desc += QString("<small>(%1)</small>").arg(id());

	QString S =
	"<html><head/><body>"
	"<small><i>" + objectName() + "</i></small><br/>"
	+ desc;
	S += "<hr/>";
	S += "<i>"+QApplication::translate("MapLayer", "Size")+": </i>" + QApplication::translate("MapLayer", "%1 features").arg(QLocale().toString(size()))+"<br/>";
	S += "%1";
	S += "</body></html>";

	return S;
}

QString MapLayer::toHtml()
{
	return toMainHtml().arg("");
}


// DrawingMapLayer

DrawingMapLayer::DrawingMapLayer(const QString & aName)
	: MapLayer(aName)
{
	p->Visible = true;
}

DrawingMapLayer::~ DrawingMapLayer()
{
}

void DrawingMapLayer::setVisible(bool b)
{
	p->Visible = b;
}

LayerWidget* DrawingMapLayer::newWidget(void)
{
//	delete p->theWidget;
	p->theWidget = new DrawingLayerWidget(this);
	return p->theWidget;
}


bool DrawingMapLayer::toXML(QDomElement xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(objectName());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("osm");
	e.appendChild(o);
	o.setAttribute("version", "0.5");
	o.setAttribute("generator", "Merkaartor");

	if (p->Features.size()) {
		QDomElement bb = xParent.ownerDocument().createElement("bound");
		o.appendChild(bb);
		CoordBox layBB = boundingBox((const MapLayer*)this);
		QString S = QString().number(intToAng(layBB.bottomLeft().lat()),'f',6) + ",";
		S += QString().number(intToAng(layBB.bottomLeft().lon()),'f',6) + ",";
		S += QString().number(intToAng(layBB.topRight().lat()),'f',6) + ",";
		S += QString().number(intToAng(layBB.topRight().lon()),'f',6);
		bb.setAttribute("box", S);
		bb.setAttribute("origin", "http://www.openstreetmap.org/api/0.5");
	}

	QList<MapFeaturePtr>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++)
		(*it)->toXML(o, progress);

	return OK;
}

DrawingMapLayer * DrawingMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	DrawingMapLayer* l = new DrawingMapLayer(e.attribute("name"));
	d->add(l);
	if (!DrawingMapLayer::doFromXML(l, d, e, progress)) {
		d->remove(l);
		delete l;
		return NULL;
	}
	return l;
}

DrawingMapLayer * DrawingMapLayer::doFromXML(DrawingMapLayer* l, MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));

	QDomElement c = e.firstChildElement();
	if (c.tagName() != "osm")
		return NULL;

// 	QByteArray ba;
// 	QTextStream out(&ba);
// 	c.save(out,2);
//
// 	bool importOK = importOSM(NULL, ba, d, l,NULL);
// 	if (importOK == false) {
// 		d->remove(l);
// 		delete l;
// 		return NULL;
// 	}

	
	int i=0;
	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "bound") {
		} else
		if (c.tagName() == "way") {
			/* Road* R = */ Road::fromXML(d, l, c);
//			l->add(R);
			i++;
		} else
		if (c.tagName() == "relation") {
			/* Relation* r = */ Relation::fromXML(d, l, c);
//			l->add(r);
			i++;
		} else
		if (c.tagName() == "node") {
			/* TrackPoint* N = */ TrackPoint::fromXML(d, l, c);
//			l->add(N);
			i++;
		} else
		if (c.tagName() == "trkseg") {
			TrackSegment* T = TrackSegment::fromXML(d, l, c, progress);
			l->add(T);
			i++;
		}

		if (i >= progress.maximum()/100) {
			progress.setValue(progress.value()+i);
			i=0;
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}

	if (i > 0) progress.setValue(progress.value()+i);
	
	return l;
}

// ImageMapLayer

ImageMapLayer::ImageMapLayer(const QString & aName, LayerManager* aLayerMgr)
	: OsbMapLayer(aName), layermanager(aLayerMgr)
{
	setMapAdapter(MerkaartorPreferences::instance()->getBackgroundPlugin());
	if (MerkaartorPreferences::instance()->getBackgroundPlugin() == NONE_ADAPTER_UUID)
		setVisible(false);
	else
		setVisible(MerkaartorPreferences::instance()->getBgVisible());

	if (M_PREFS->getUseShapefileForBackground())
		if (QDir::isAbsolutePath(WORLD_SHP))
			setFilename(WORLD_SHP);
		else
			setFilename(QCoreApplication::applicationDirPath() + "/" + WORLD_SHP);

	setReadonly(true);
}

ImageMapLayer::~ ImageMapLayer()
{
	if (p->layer_bg) {
		if (layermanager)
			layermanager->removeLayer(p->layer_bg->getLayername());

		IMapAdapter* mapadapter_bg = p->layer_bg->getMapAdapter();
		if (mapadapter_bg && (mapadapter_bg->getId() == WMS_ADAPTER_UUID || mapadapter_bg->getId() == TMS_ADAPTER_UUID)) {
			delete (dynamic_cast<MapAdapter*>(mapadapter_bg));
			mapadapter_bg = NULL;
		}
		SAFE_DELETE(p->layer_bg);
	}
	clear();
}

unsigned int ImageMapLayer::size() const
{
	//return p->Features.size();
	if (p->bgType == SHAPE_ADAPTER_UUID && isVisible())
		return p->Features.size();
	else
		return 0;
}

LayerWidget* ImageMapLayer::newWidget(void)
{
//	delete p->theWidget;
	p->theWidget = new ImageLayerWidget(this);
	return p->theWidget;
}

void ImageMapLayer::updateWidget()
{
    ((ImageLayerWidget*) p->theWidget)->initActions();
	p->theWidget->update();
}

void ImageMapLayer::setVisible(bool b)
{
	p->Visible = b;
	if (p->bgType == NONE_ADAPTER_UUID)
		p->Visible = false;
	else
		if (p->layer_bg)
			p->layer_bg->setVisible(b);
	MerkaartorPreferences::instance()->setBgVisible(p->Visible);
}

Layer* ImageMapLayer::imageLayer()
{
	return p->layer_bg;
}

void ImageMapLayer::setMapAdapter(const QUuid& theAdapterUid)
{
	IMapAdapter* mapadapter_bg;
	WmsServerList* wsl;
	WmsServer ws;
	TmsServerList* tsl;
	TmsServer ts;
	QString selws, selts;
	int idx = -1;

	if (layermanager) {
		if (layermanager->getLayer(id())) {
			idx = layermanager->getLayers().indexOf(id());
			layermanager->removeLayer(id());
		}
	}

	if (p->layer_bg) {
		mapadapter_bg = p->layer_bg->getMapAdapter();
		if (mapadapter_bg && (mapadapter_bg->getId() == WMS_ADAPTER_UUID || mapadapter_bg->getId() == TMS_ADAPTER_UUID)) {
			delete (dynamic_cast<MapAdapter*>(mapadapter_bg));
			mapadapter_bg = NULL;
		}
		SAFE_DELETE(p->layer_bg);
	}

	p->bgType = theAdapterUid;
	MerkaartorPreferences::instance()->setBackgroundPlugin(theAdapterUid);
	if (p->bgType == NONE_ADAPTER_UUID) {
		setName(tr("Map - None"));
		p->Visible = false;
	} else 
	if (p->bgType == WMS_ADAPTER_UUID) {
		wsl = M_PREFS->getWmsServers();
		selws = M_PREFS->getSelectedWmsServer();
		ws = wsl->value(selws);
		wmsa = new WMSMapAdapter(ws.WmsAdress, ws.WmsPath, ws.WmsLayers, ws.WmsProjections,
				ws.WmsStyles, ws.WmsImgFormat, 256);
		wmsa->setImageManager(ImageManager::instance());
		mapadapter_bg = wmsa;
		p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
		p->layer_bg->setVisible(p->Visible);

		setName(tr("Map - WMS - %1").arg(ws.WmsName));
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		tsl = M_PREFS->getTmsServers();
		selts = MerkaartorPreferences::instance()->getSelectedTmsServer();
		ts = tsl->value(selts);
		tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
		tmsa->setImageManager(ImageManager::instance());
		mapadapter_bg = tmsa;
		p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
		p->layer_bg->setVisible(p->Visible);

		setName(tr("Map - TMS - %1").arg(ts.TmsName));
	} else
	if (p->bgType == SHAPE_ADAPTER_UUID) {
		if (!M_PREFS->getUseShapefileForBackground()) {
			p->bgType = NONE_ADAPTER_UUID;
			setName(tr("Map - None"));
			p->Visible = false;
		} else
			setName(tr("Map - OSB Background"));
	} else
	{
		IMapAdapter * thePluginBackground = M_PREFS->getBackgroundPlugin(p->bgType);
		if (thePluginBackground) {
			switch (thePluginBackground->getType()) {
				case IMapAdapter::BrowserBackground :
					thePluginBackground->setImageManager(BrowserImageManager::instance());
					break;
				case IMapAdapter::DirectBackground :
					thePluginBackground->setImageManager(ImageManager::instance());
					break;
			}
			mapadapter_bg = thePluginBackground;
			p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - %1").arg(thePluginBackground->getName()));
		} else
			p->bgType = NONE_ADAPTER_UUID;
	}

	if (layermanager)
		if (p->layer_bg) {
			layermanager->addLayer(p->layer_bg, idx);
			layermanager->setSize();
		}
}

bool ImageMapLayer::toXML(QDomElement xParent, QProgressDialog & /* progress */)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(objectName());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));

	e.setAttribute("bgtype", p->bgType.toString());

	QDomElement c;
	WmsServer ws;
	TmsServer ts;

	if (p->bgType == WMS_ADAPTER_UUID) {
		c = e.ownerDocument().createElement("WmsServer");
		e.appendChild(c);

		c.setAttribute("name", M_PREFS->getSelectedWmsServer());
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		c = e.ownerDocument().createElement("TmsServer");
		e.appendChild(c);

		c.setAttribute("name", M_PREFS->getSelectedTmsServer());
	}

	return OK;
}

ImageMapLayer * ImageMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & /* progress */)
{
	ImageMapLayer* l = d->getImageLayer();
	l->setId(e.attribute("xml:id"));

	QDomElement c = e.firstChildElement();

	if (c.tagName() == "WmsServer") {
		MerkaartorPreferences::instance()->setSelectedWmsServer(c.attribute("name"));
	} else
	if (c.tagName() == "TmsServer") {
		MerkaartorPreferences::instance()->setSelectedTmsServer(c.attribute("name"));
	}
	l->setMapAdapter(QUuid(e.attribute("bgtype")));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));

	return l;
}

// ExtractedMapLayer

ExtractedMapLayer::ExtractedMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = true;
}

ExtractedMapLayer::~ ExtractedMapLayer()
{
}

ExtractedMapLayer* ExtractedMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	ExtractedMapLayer* l = new ExtractedMapLayer(e.attribute("name"));
	d->add(l);
	if (!DrawingMapLayer::doFromXML(l, d, e, progress)) {
		delete l;
		return NULL;
	}
	return l;
}

LayerWidget* ExtractedMapLayer::newWidget(void)
{
	p->theWidget = new ExtractedLayerWidget(this);
	return p->theWidget;
}

// TrackMapLayer

TrackMapLayer::TrackMapLayer(const QString & aName, const QString& filename)
	: MapLayer(aName), Filename(filename)
{
	p->Visible = true;
	p->Readonly = M_PREFS->getReadonlyTracksDefault();
}

TrackMapLayer::~ TrackMapLayer()
{
}

void TrackMapLayer::setVisible(bool b)
{
	p->Visible = b;
}

LayerWidget* TrackMapLayer::newWidget(void)
{
	p->theWidget = new TrackLayerWidget(this);
	return p->theWidget;
}

void TrackMapLayer::extractLayer()
{
	ExtractedMapLayer* extL = new ExtractedMapLayer(tr("Extract - %1").arg(name()));
	CommandList* theList = new CommandList(MainWindow::tr("Extracted Layer '%1'").arg(name()), NULL);

	TrackPoint* P;
	QList<TrackPoint*> PL;

	const double coordPer10M = (double(INT_MAX) * 2 / 40080000) * 2;

	for (unsigned int i=0; i < size(); i++) {
		if (TrackSegment* S = dynamic_cast<TrackSegment*>(get(i))) {

			if (S->size() < 2)
				continue;

			PL.clear();

			P = new TrackPoint( S->getNode(0)->position() );
			P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			P->setTime(S->getNode(0)->time());
			P->setElevation(S->getNode(0)->elevation());
			P->setSpeed(S->getNode(0)->speed());
			//P->setTag("ele", QString::number(S->get(0)->elevation()));
			PL.append(P);
			unsigned int startP = 0;

			P = new TrackPoint( S->getNode(1)->position() );
			P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			P->setTime(S->getNode(1)->time());
			P->setElevation(S->getNode(1)->elevation());
			P->setSpeed(S->getNode(1)->speed());
			//P->setTag("ele", QString::number(S->get(1)->elevation()));
			PL.append(P);
			unsigned int endP = 1;

			for (unsigned int j=2; j < S->size(); j++) {
				P = new TrackPoint( S->getNode(j)->position() );
				P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				P->setTime(S->getNode(j)->time());
				P->setElevation(S->getNode(j)->elevation());
				P->setSpeed(S->getNode(j)->speed());
				//P->setTag("ele", QString::number(S->get(j)->elevation()));
				PL.append(P);
				endP = PL.size()-1;

				LineF l(toQt(PL[startP]->position()), toQt(PL[endP]->position()));
				for (unsigned int k=startP+1; k < endP; k++) {
					double d = l.distance(toQt(PL[k]->position()));
					if (d < coordPer10M) {
						TrackPoint* P = PL[k];
						PL.removeAt(k);
						delete P;
						endP--;
					} else
						startP = k;
				}
			}

			Road* R = new Road();
			R->setLastUpdated(MapFeature::OSMServer);
			R->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			theList->add(new AddFeatureCommand(extL,R,true));
			for (int i=0; i < PL.size(); i++) {
				theList->add(new AddFeatureCommand(extL,PL[i],true));
				theList->add(new RoadAddTrackPointCommand(R,PL[i],extL));
			}
		}
	}

	if (theList->size()) {
		p->theDocument->addHistory(theList);
		//delete theList;
	}
	p->theDocument->add(extL);
}

const QString TrackMapLayer::getFilename()
{
	return Filename;
}

bool TrackMapLayer::toXML(QDomElement xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(objectName());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("gpx");
	e.appendChild(o);
	o.setAttribute("version", "1.1");
	o.setAttribute("creator", "Merkaartor");
	o.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
	o.setAttribute("xmlns:rmc", "urn:net:trekbuddy:1.0:nmea:rmc");

	QList<TrackPoint*>	waypoints;
	QList<TrackSegment*>	segments;
	QList<MapFeaturePtr>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++) {
		if (TrackSegment* S = qobject_cast<TrackSegment*>(*it))
			segments.push_back(S);
		if (TrackPoint* P = qobject_cast<TrackPoint*>(*it))
			if (!P->tagValue("_waypoint_","").isEmpty())
				waypoints.push_back(P);
	}

	for (int i=0; i < waypoints.size(); ++i) {
		waypoints[i]->toGPX(o, progress);
	}

	QDomElement t = o.ownerDocument().createElement("trk");
	o.appendChild(t);

	for (int i=0; i < segments.size(); ++i)
		segments[i]->toXML(t, progress);

	return OK;
}

TrackMapLayer * TrackMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	TrackMapLayer* l = new TrackMapLayer(e.attribute("name"));
	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));
	d->add(l);

	QDomElement c = e.firstChildElement();
	if (c.tagName() != "gpx")
		return NULL;

	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "trk") {
			QDomElement t = c.firstChildElement();
			while(!t.isNull()) {
				if (t.tagName() == "trkseg") {
					TrackSegment* N = TrackSegment::fromXML(d, l, t, progress);
					l->add(N);
				}

				t = t.nextSiblingElement();
			}
		}
		if (c.tagName() == "wpt") {
			/* TrackPoint* N = */ TrackPoint::fromGPX(d, l, c);
			//l->add(N);
			progress.setValue(progress.value()+1);
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}


	return l;
}

// DirtyMapLayer

DirtyMapLayer::DirtyMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = true;
}

DirtyMapLayer::~ DirtyMapLayer()
{
}

DirtyMapLayer* DirtyMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	DrawingMapLayer::doFromXML(dynamic_cast<DrawingMapLayer*>(d->getDirtyOrOriginLayer()), d, e, progress);
	return dynamic_cast<DirtyMapLayer*>(d->getDirtyOrOriginLayer());
}

LayerWidget* DirtyMapLayer::newWidget(void)
{
	p->theWidget = new DirtyLayerWidget(this);
	return p->theWidget;
}



// UploadedMapLayer

UploadedMapLayer::UploadedMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = true;
}

UploadedMapLayer::~ UploadedMapLayer()
{
}

UploadedMapLayer* UploadedMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	DrawingMapLayer::doFromXML(d->getUploadedLayer(), d, e, progress);
	return d->getUploadedLayer();
}

LayerWidget* UploadedMapLayer::newWidget(void)
{
	p->theWidget = new UploadedLayerWidget(this);
	return p->theWidget;
}

// DeletedMapLayer

DeletedMapLayer::DeletedMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = false;
	p->Enabled = false;
}

DeletedMapLayer::~ DeletedMapLayer()
{
}

bool DeletedMapLayer::toXML(QDomElement , QProgressDialog & )
{
	return true;
}

DeletedMapLayer* DeletedMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	/* Only keep DeletedMapLayer for backward compatibility with MDC */
	DrawingMapLayer::doFromXML(dynamic_cast<DrawingMapLayer*>(d->getDirtyOrOriginLayer()), d, e, progress);
	return NULL;
}

LayerWidget* DeletedMapLayer::newWidget(void)
{
	return NULL;
}

// OsbMapLayer

class OsbMapLayerPrivate
{
public:
	OsbMapLayerPrivate()
		: theImp(0) 
	{
	}
	ImportExportOsmBin* theImp;
	QList<qint32> loadedTiles;
	QList<qint32> loadedRegions;
};

OsbMapLayer::OsbMapLayer(const QString & aName)
	: MapLayer(aName)
{
	p->Visible = true;
	pp = new OsbMapLayerPrivate();
	pp->theImp = new ImportExportOsmBin(NULL);
}

OsbMapLayer::OsbMapLayer(const QString & aName, const QString & filename)
	: MapLayer(aName)
{
	p->Visible = true;
	pp = new OsbMapLayerPrivate();
	pp->theImp = new ImportExportOsmBin(NULL);
	if (pp->theImp->loadFile(filename))
		pp->theImp->import(this);
}

OsbMapLayer::~ OsbMapLayer()
{
	delete pp->theImp;
	delete pp;
}

void OsbMapLayer::setFilename(const QString& filename)
{
	delete pp->theImp;

	pp->theImp = new ImportExportOsmBin(NULL);
	if (pp->theImp->loadFile(filename))
		pp->theImp->import(this);
}

LayerWidget* OsbMapLayer::newWidget(void)
{
	p->theWidget = new OsbLayerWidget(this);
	return p->theWidget;
}

void OsbMapLayer::invalidate(MapDocument* d, CoordBox vp)
{
	if (!isVisible())
		return;

	QRectF r(vp.toQRectF());

	int x1 = int((r.topLeft().x() + INT_MAX) / REGION_WIDTH);
	int y1 = int((r.topLeft().y() + INT_MAX) / REGION_WIDTH);
	int x2 = int((r.bottomRight().x() + INT_MAX) / REGION_WIDTH);
	int y2 = int((r.bottomRight().y() + INT_MAX) / REGION_WIDTH);

	QList<qint32> regionToLoad;
	regionToLoad.append(0);
	for (int i=x1; i <= x2; ++i)
		for (int j=y1; j <= y2; ++j)
			regionToLoad.push_back(j*NUM_REGIONS+i);
			//if (!pp->loadedRegions.contains(j*NUM_REGIONS+i))
			//	if (pp->theImp->loadRegion(j*NUM_REGIONS+i))
			//		pp->loadedRegions.append(j*NUM_REGIONS+i);

	x1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH); 
	y1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
	x2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
	y2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);

	QList<qint32> tileToLoad;
	if (intToAng(vp.lonDiff()) <= M_PREFS->getTileToRegionThreshold())
		for (int i=x1; i <= x2; ++i)
			for (int j=y1; j <= y2; ++j)
				tileToLoad.push_back(j*NUM_TILES+i);

	//int span = (x2 - x1 + 1) * (y2 - y1 + 1);

	int j;
	j = 0;
	while (j<pp->loadedTiles.size()) {
		if (!tileToLoad.contains(pp->loadedTiles[j])) {
			if (pp->theImp->clearTile(pp->loadedTiles[j], d, this))
				pp->loadedTiles.removeAt(j);
			else 
				++j;
		} else
			++j;
	}
	j = 0;
	while (j<pp->loadedRegions.size()) {
		if (!regionToLoad.contains(pp->loadedRegions[j])) {
			if (pp->theImp->clearRegion(pp->loadedRegions[j], d, this))
				pp->loadedRegions.removeAt(j);
			else 
				++j;
		} else
			++j;
	}

	for (int i=0; i<regionToLoad.size(); ++i)
		if (!pp->loadedRegions.contains(regionToLoad[i]))
			if (pp->theImp->loadRegion(regionToLoad[i], d, this))
				pp->loadedRegions.append(regionToLoad[i]);
	for (int i=0; i<tileToLoad.size(); ++i)
		if (!pp->loadedTiles.contains(tileToLoad[i]))
			if (pp->theImp->loadTile(tileToLoad[i], d, this))
				pp->loadedTiles.push_back(tileToLoad[i]);

}

//MapFeature*  OsbMapLayer::getFeatureByRef(MapDocument* d, quint64 ref)
//{
//	return pp->theImp->getFeature(d, this, ref);
//}

void OsbMapLayer::setVisible(bool b)
{
	p->Visible = b;
}

bool OsbMapLayer::toXML(QDomElement xParent, QProgressDialog & progress)
{
	Q_UNUSED(progress);

	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(objectName());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));

	e.setAttribute("filename", pp->theImp->getFilename());

	return OK;
}

OsbMapLayer * OsbMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	Q_UNUSED(progress);

	OsbMapLayer* l = new OsbMapLayer(e.attribute("name"));

	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));

	if (l->pp->theImp->loadFile(e.attribute("filename")))
		l->pp->theImp->import(l);
	else {
		delete l;
		return NULL;
	}

	d->add(l);
	return l;
}

QString OsbMapLayer::toHtml()
{
	QString S;

	S += "<i>"+QApplication::translate("OsbMapLayer", "# of loaded Regions")+": </i>" + QApplication::translate("OsbMapLayer", "%1").arg(QLocale().toString(pp->loadedRegions.size()))+"<br/>";
	S += "<i>"+QApplication::translate("OsbMapLayer", "# of loaded Tiles")+": </i>" + QApplication::translate("OsbMapLayer", "%1").arg(QLocale().toString(pp->loadedTiles.size()))+"<br/>";

	return toMainHtml().arg(S);
}

