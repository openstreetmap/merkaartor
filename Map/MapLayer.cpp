#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"
#include "MapView.h"
#include "LayerWidget.h"
#include "Map/ImportOSM.h"

#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"

#include "Map/MapLayer.h"

#include "Utils/LineF.h"

#include "QMapControl/mapadapter.h"
#include "QMapControl/osmmapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#ifdef YAHOO
	#include "QMapControl/yahoolegalmapadapter.h"
#endif
#ifdef YAHOO_ILLEGAL
	#include "QMapControl/yahoomapadapter.h"
#endif
#ifdef GOOGLE_ILLEGAL
	#include "QMapControl/googlesatmapadapter.h"
#endif
#ifdef MSLIVEMAP_ILLEGAL
	#include "QMapControl/mslivemapadapter.h"
#endif
#include "QMapControl/layer.h"
#include "QMapControl/layermanager.h"

#include <QtCore/QString>
#include <QMultiMap>

#include <algorithm>
#include <map>
#include <vector>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

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
	}
	~MapLayerPrivate()
	{
		for (unsigned int i=0; i<Features.size(); ++i)
			delete Features[i];
	}
	std::vector<MapFeature*> Features;
	std::map<QString, MapFeature*> IdMap;
	QString Name;
	bool Visible;
	bool selected;
	LayerWidget* theWidget;
	qreal alpha;
	unsigned int dirtyLevel;

	ImageBackgroundType bgType;
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
}

MapLayer::MapLayer(const QString& aName)
:  p(new MapLayerPrivate)
{
	p->Name = aName;
	p->alpha = 1.0;
	p->dirtyLevel = 0;
}

MapLayer::MapLayer(const MapLayer&)
: p(0)
{
}

MapLayer::~MapLayer()
{
	delete p;
}

void MapLayer::sortRenderingPriority(double aPixelPerM)
{
	if (!p->RenderPriorityUpToDate || (aPixelPerM != p->RenderPriorityForPixelPerM) )
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

void MapLayer::add(MapFeature* aFeature)
{
	p->Features.push_back(aFeature);
	notifyIdUpdate(aFeature->id(),aFeature);
	aFeature->setLayer(this);
	p->RenderPriorityUpToDate = false;
}

void MapLayer::add(MapFeature* aFeature, unsigned int Idx)
{
	add(aFeature);
	std::rotate(p->Features.begin()+Idx,p->Features.end()-1,p->Features.end());
	aFeature->setLayer(this);
	p->RenderPriorityUpToDate = false;
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
		aFeature->setLayer(0);
		notifyIdUpdate(aFeature->id(),0);
		p->RenderPriorityUpToDate = false;
	}
}

void MapLayer::clear()
{
	std::vector<MapFeature*>::iterator i;
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
	std::vector<MapFeature*>::iterator i = std::find(p->Features.begin(),p->Features.end(), F);
	return i != p->Features.end();
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
	if(i>=p->Features.size()) return 0;
	return p->Features[i];
}

LayerWidget* MapLayer::getWidget(void)
{
	return p->theWidget;
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
	CoordBox Box(theLayer->get(0)->boundingBox());
	for (unsigned int i=1; i<theLayer->size(); ++i)
		Box.merge(theLayer->get(i)->boundingBox());
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

bool MapLayer::canDelete()
{
	return (p->dirtyLevel == 0);
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


bool DrawingMapLayer::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("osm");
	e.appendChild(o);
	o.setAttribute("version", "0.5");
	o.setAttribute("generator", "Merkaartor");

	if (p->Features.size()) {
		QDomElement bb = xParent.ownerDocument().createElement("bound");
		o.appendChild(bb);
		CoordBox layBB = boundingBox((const MapLayer*)this);
		QString S = QString().number(radToAng(layBB.bottomLeft().lat())) + ",";
		S += QString().number(radToAng(layBB.bottomLeft().lon())) + ",";
		S += QString().number(radToAng(layBB.topRight().lat())) + ",";
		S += QString().number(radToAng(layBB.topRight().lon()));
		bb.setAttribute("box", S);
		bb.setAttribute("origin", "http://www.openstreetmap.org/api/0.5");
	}

	std::vector<MapFeature*>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++)
		(*it)->toXML(o);

	return OK;
}

DrawingMapLayer * DrawingMapLayer::fromXML(MapDocument* d, const QDomElement e)
{
	DrawingMapLayer* l = new DrawingMapLayer(e.attribute("name"));
	d->add(l);
	if (!DrawingMapLayer::doFromXML(l, d, e)) {
		delete l;
		return NULL;
	}
	return l;
}

DrawingMapLayer * DrawingMapLayer::doFromXML(DrawingMapLayer* l, MapDocument* d, const QDomElement e)
{
	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));

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

	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "bound") {
		} else
		if (c.tagName() == "way") {
			/* Road* R = */ Road::fromXML(d, l, c);
//			l->add(R);
		} else
		if (c.tagName() == "relation") {
			/* Relation* r = */ Relation::fromXML(d, l, c);
//			l->add(r);
		} else
		if (c.tagName() == "node") {
			/* TrackPoint* N = */ TrackPoint::fromXML(d, l, c);
//			l->add(N);
		}

		c = c.nextSiblingElement();
	}

	return l;
}

// ImageMapLayer

ImageMapLayer::ImageMapLayer(const QString & aName)
	: MapLayer(aName), layermanager(0)
{
	setMapAdapter(MerkaartorPreferences::instance()->getBgType());
	if (MerkaartorPreferences::instance()->getBgType() == Bg_None)
		setVisible(false);
	else
		setVisible(MerkaartorPreferences::instance()->getBgVisible());
}

ImageMapLayer::~ ImageMapLayer()
{
	if (layermanager && layermanager->getLayers().size())
		layermanager->removeLayer();
	SAFE_DELETE(p->layer_bg);
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
	if (p->bgType == Bg_None)
		p->Visible = false;
	else
	p->layer_bg->setVisible(b);
	MerkaartorPreferences::instance()->setBgVisible(p->Visible);
}

Layer* ImageMapLayer::imageLayer()
{
	return p->layer_bg;
}

void ImageMapLayer::setMapAdapter(ImageBackgroundType typ)
{
	MapAdapter* mapadapter_bg;
	WmsServerList* wsl;
	WmsServer ws;
	TmsServerList* tsl;
	TmsServer ts;
	QString selws, selts;

	if (layermanager)
		if (layermanager->getLayers().size() > 0) {
			layermanager->removeLayer();
		}
	SAFE_DELETE(p->layer_bg);

	p->bgType = typ;
	MerkaartorPreferences::instance()->setBgType(typ);
	switch (p->bgType) {
		default:
			p->bgType = Bg_None; /* no break */
		case Bg_None:
			setName(tr("Map - None"));
			p->Visible = false;
			break;

		case Bg_Wms:
			wsl = MerkaartorPreferences::instance()->getWmsServers();
			selws = MerkaartorPreferences::instance()->getSelectedWmsServer();
			ws = wsl->value(selws);
			wmsa = new WMSMapAdapter(ws.WmsAdress, ws.WmsPath, ws.WmsLayers, ws.WmsProjections,
					ws.WmsStyles, ws.WmsImgFormat, 256);
			mapadapter_bg = wmsa;
			p->layer_bg = new Layer(selws, mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - WMS - %1").arg(ws.WmsName));
			break;
		case Bg_Tms:
			tsl = MerkaartorPreferences::instance()->getTmsServers();
			selts = MerkaartorPreferences::instance()->getSelectedTmsServer();
			ts = tsl->value(selts);
			tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
			mapadapter_bg = tmsa;
			p->layer_bg = new Layer(selts, mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - TMS - %1").arg(ts.TmsName));
/*			mapadapter_bg = new OSMMapAdapter();
			p->layer_bg = new Layer("Custom Layer", mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Background - OSM");*/
			break;
#ifdef YAHOO
		case Bg_Yahoo:
			mapadapter_bg = new YahooLegalMapAdapter();
			p->layer_bg = new Layer(tr("Custom Layer"), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - Yahoo"));
			break;
#endif
#ifdef YAHOO_ILLEGAL
		case Bg_Yahoo_illegal:
			mapadapter_bg = new YahooMapAdapter("us.maps3.yimg.com", "/aerial.maps.yimg.com/png?v=1.7&t=a&s=256&x=%2&y=%3&z=%1");
			p->layer_bg = new Layer(tr("Custom Layer"), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - Illegal Yahoo"));
			break;
#endif
#ifdef GOOGLE_ILLEGAL
		case Bg_Google_illegal:
			mapadapter_bg = new GoogleSatMapAdapter();
			p->layer_bg = new Layer(tr("Custom Layer"), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - Illegal Google"));
			break;
#endif
#ifdef MSLIVEMAP_ILLEGAL
		case Bg_MsVirtualEarth_illegal:
			mapadapter_bg = new MsLiveMapAdapter();
			p->layer_bg = new Layer(tr("Custom Layer"), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName(tr("Map - Illegal Ms Virtual Earth"));
			break;
#endif
	}
	if (layermanager)
		if (p->layer_bg) {
			layermanager->addLayer(p->layer_bg);
		}

}

bool ImageMapLayer::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));

	e.setAttribute("bgtype", QString::number((int)p->bgType));

	QDomElement c;
	WmsServer ws;
	TmsServer ts;
	switch (p->bgType) {
		case Bg_Wms:
			c = e.ownerDocument().createElement("WmsServer");
			e.appendChild(c);

			c.setAttribute("name", p->layer_bg->getLayername());
			//e.setAttribute("adress", wmsa->host);
			//e.setAttribute("path", wmsa->serverPath);
			//e.setAttribute("layers", wmsa->wms_layers);
			//e.setAttribute("projection", wmsa->wms_srs);
			//e.setAttribute("styles", wmsa->wms_styles);
			//e.setAttribute("format", wmsa->wms_format);

			break;
		case Bg_Tms:
			c = e.ownerDocument().createElement("TmsServer");
			e.appendChild(c);

			c.setAttribute("name", p->layer_bg->getLayername());
			//e.setAttribute("adress", tmsa->host);
			//e.setAttribute("path", tmsa->serverPath);
			//e.setAttribute("path", tmsa->tilesize);
			//e.setAttribute("minzoom", tmsa->min_zoom);
			//e.setAttribute("maxzoom", tmsa->max_zoom);

			break;
		default:
			break;
	}

	return OK;
}

ImageMapLayer * ImageMapLayer::fromXML(MapDocument* d, const QDomElement e)
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
	l->setMapAdapter((ImageBackgroundType)(e.attribute("bgtype").toInt()));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));

	return l;
}


// TrackMapLayer

TrackMapLayer::TrackMapLayer(const QString & aName)
	: MapLayer(aName)
{
	p->Visible = true;
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

	const double radPer10M = (M_PI * 2 / 40080000) * 2;

	for (unsigned int i=0; i < size(); i++) {
		if (TrackSegment* S = dynamic_cast<TrackSegment*>(get(i))) {

			if (S->size() < 2)
				continue;

			PL.clear();

			P = new TrackPoint( S->get(0)->position() );
			P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			P->setTime(S->get(0)->time());
			PL.append(P);
			unsigned int startP = 0;

			P = new TrackPoint( S->get(1)->position() );
			P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
			P->setTime(S->get(1)->time());
			PL.append(P);
			unsigned int endP = 1;

			for (unsigned int j=2; j < S->size(); j++) {
				P = new TrackPoint( S->get(j)->position() );
				P->setTag("created_by", QString("Merkaartor %1").arg(VERSION));
				P->setTime(S->get(j)->time());
				PL.append(P);
				endP = PL.size()-1;

				LineF l(toQt(PL[startP]->position()), toQt(PL[endP]->position()));
				for (unsigned int k=startP+1; k < endP; k++) {
					double d = l.distance(toQt(PL[k]->position()));
					if (d < radPer10M) {
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

bool TrackMapLayer::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("gpx");
	e.appendChild(o);
	o.setAttribute("version", "1.1");
	o.setAttribute("creator", "Merkaartor");
	o.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1");
	o.setAttribute("xmlns:rmc", "urn:net:trekbuddy:1.0:nmea:rmc");

	QDomElement t = o.ownerDocument().createElement("trk");
	o.appendChild(t);

	std::vector<MapFeature*>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++) {
		TrackSegment* S = dynamic_cast<TrackSegment*>(*it);
		if (S)
			S->toXML(t);
	}

	return OK;
}

TrackMapLayer * TrackMapLayer::fromXML(MapDocument* d, const QDomElement e)
{
	TrackMapLayer* l = new TrackMapLayer(e.attribute("name"));
	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	d->add(l);

	QDomElement c = e.firstChildElement();
	if (c.tagName() != "gpx")
		return NULL;

	c = c.firstChildElement();
	if (c.tagName() != "trk")
		return NULL;

	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "trkseg") {
			TrackSegment* N = TrackSegment::fromXML(d, l, c);
			l->add(N);
		}

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

DirtyMapLayer* DirtyMapLayer::fromXML(MapDocument* d, const QDomElement e)
{
	DrawingMapLayer::doFromXML(d->getDirtyLayer(), d, e);
	return d->getDirtyLayer();
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

UploadedMapLayer* UploadedMapLayer::fromXML(MapDocument* d, const QDomElement e)
{
	DrawingMapLayer::doFromXML(d->getUploadedLayer(), d, e);
	return d->getUploadedLayer();
}

LayerWidget* UploadedMapLayer::newWidget(void)
{
	p->theWidget = new UploadedLayerWidget(this);
	return p->theWidget;
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

ExtractedMapLayer* ExtractedMapLayer::fromXML(MapDocument* d, const QDomElement e)
{
	ExtractedMapLayer* l = new ExtractedMapLayer(e.attribute("name"));
	d->add(l);
	if (!DrawingMapLayer::doFromXML(l, d, e)) {
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

