#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/TrackPoint.h"
#include "Map/TrackSegment.h"
#include "MapView.h"
#include "LayerWidget.h"
#include "Map/ImportOSM.h"

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
	CoordBox Box(theLayer->get(0)->boundingBox());
	for (unsigned int i=1; i<theLayer->size(); ++i)
		Box.merge(theLayer->get(i)->boundingBox());
	return Box;
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

	QDomElement e = xParent.ownerDocument().createElement("DrawingMapLayer");
	xParent.appendChild(e);

	e.setAttribute("id", id());
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
	l->setId(e.attribute("id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	d->add(l);

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
			Road* R = Road::fromXML(d, l, c);
//			l->add(R);
		} else
		if (c.tagName() == "relation") {
			Relation* r = Relation::fromXML(d, l, c);
//			l->add(r);
		} else
		if (c.tagName() == "node") {
			TrackPoint* N = TrackPoint::fromXML(d, l, c);
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
		case Bg_None:
			setName("Map - None");
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

			setName("Map - WMS - " + ws.WmsName);
			break;
		case Bg_Tms:
			tsl = MerkaartorPreferences::instance()->getTmsServers();
			selts = MerkaartorPreferences::instance()->getSelectedTmsServer();
			ts = tsl->value(selts);
			tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
			mapadapter_bg = tmsa;
			p->layer_bg = new Layer(selts, mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Map - TMS - " + ts.TmsName);
/*			mapadapter_bg = new OSMMapAdapter();
			p->layer_bg = new Layer("Custom Layer", mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Background - OSM");*/
			break;
#ifdef YAHOO
		case Bg_Yahoo:
			mapadapter_bg = new YahooLegalMapAdapter();
			p->layer_bg = new Layer("Custom Layer", mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Map - Yahoo");
			break;
#endif
#ifdef YAHOO_ILLEGAL
		case Bg_Yahoo_illegal:
			mapadapter_bg = new YahooMapAdapter("us.maps3.yimg.com", "/aerial.maps.yimg.com/png?v=1.7&t=a&s=256&x=%2&y=%3&z=%1");
			p->layer_bg = new Layer("Custom Layer", mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Map - Illegal Yahoo");
			break;
#endif
#ifdef GOOGLE_ILLEGAL
		case Bg_Google_illegal:
			mapadapter_bg = new GoogleSatMapAdapter();
			p->layer_bg = new Layer("Custom Layer", mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(p->Visible);

			setName("Map - Illegal Google");
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

	QDomElement e = xParent.ownerDocument().createElement("ImageMapLayer");
	xParent.appendChild(e);

	e.setAttribute("id", id());
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

ImageMapLayer * ImageMapLayer::fromXML(MapDocument* /* d */, const QDomElement e)
{
	ImageMapLayer* l = new ImageMapLayer();
	l->setId(e.attribute("id"));

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
	DrawingMapLayer* extL = new DrawingMapLayer("Extract - " + name());
	TrackPoint* P;

	//P = new TrackPoint( ((TrackPoint *)get(0))->position() );
	//P->setTag("created_by", QString("Merkaartor %1.%2").arg(MAJORVERSION).arg(MINORVERSION));
	//extL->add(P);
	//R->add(P);

	//P = new TrackPoint( ((TrackPoint *)get(size()-1))->position() );
	//P->setTag("created_by", QString("Merkaartor %1.%2").arg(MAJORVERSION).arg(MINORVERSION));
	//extL->add(P);
	//R->add(P);


	for (unsigned int i=0; i < size(); i++) {
		if (TrackSegment* S = dynamic_cast<TrackSegment*>(get(i))) {
			Road* R = new Road();
			R->setLastUpdated(MapFeature::OSMServer);
			R->setTag("created_by", QString("Merkaartor %1.%2").arg(MAJORVERSION).arg(MINORVERSION));

			for (unsigned int j=0; j < S->size(); j++) {
				P = new TrackPoint( S->get(j)->position() );
				P->setTag("created_by", QString("Merkaartor %1.%2").arg(MAJORVERSION).arg(MINORVERSION));
				extL->add(P);
				R->add(P);
			}

				extL->add(R);
		}
	}

	p->theDocument->add(extL);
}

bool TrackMapLayer::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("TrackMapLayer");
	xParent.appendChild(e);

	e.setAttribute("id", id());
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
	l->setId(e.attribute("id"));
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

