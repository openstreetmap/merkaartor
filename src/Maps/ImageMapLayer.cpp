#include "Maps/ImageMapLayer.h"

#include "Maps/MapDocument.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Maps/Projection.h"

#include "IMapAdapter.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
#include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/wmsmapadapter.h"
#include "QMapControl/layer.h"
#include "QMapControl/layermanager.h"

#include "LayerWidget.h"

// ImageMapLayerPrivate

class ImageMapLayerPrivate
{
public:
	QUuid bgType;
	Layer* layer_bg;
	WmsServer* theWmsServer;

	ImageLayerWidget* theWidget;

public:
	ImageMapLayerPrivate()
	{
		layer_bg = NULL;
		theWidget = NULL;
		theWmsServer = NULL;
	}
};


// ImageMapLayer

ImageMapLayer::ImageMapLayer(const QString & aName, LayerManager* aLayerMgr)
	: OsbMapLayer(aName), layermanager(aLayerMgr), p(new ImageMapLayerPrivate)
{
	setMapAdapter(MerkaartorPreferences::instance()->getBackgroundPlugin());
	if (MerkaartorPreferences::instance()->getBackgroundPlugin() == NONE_ADAPTER_UUID)
		setVisible(false);
	else
		setVisible(MerkaartorPreferences::instance()->getBgVisible());

	if (M_PREFS->getUseShapefileForBackground()) {
		if (QDir::isAbsolutePath(STRINGIFY(WORLD_SHP)))
			setFilename(STRINGIFY(WORLD_SHP));
		else
			setFilename(QCoreApplication::applicationDirPath() + "/" + STRINGIFY(WORLD_SHP));
	}

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
	SAFE_DELETE(p);
}

int ImageMapLayer::size() const
{
	//return p->Features.size();
	if (p->bgType == SHAPE_ADAPTER_UUID && isVisible())
		return size();
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
    p->theWidget->initActions();
	setMapAdapter(M_PREFS->getBackgroundPlugin());
	p->theWidget->update();
}

void ImageMapLayer::setVisible(bool b)
{
	MapLayer::setVisible(b);
	if (p->bgType == NONE_ADAPTER_UUID)
		MapLayer::setVisible(false);
	else
		if (p->layer_bg)
			p->layer_bg->setVisible(b);
	MerkaartorPreferences::instance()->setBgVisible(isVisible());
}

Layer* ImageMapLayer::imageLayer()
{
	return p->layer_bg;
}

void ImageMapLayer::setMapAdapter(const QUuid& theAdapterUid)
{
	IMapAdapter* mapadapter_bg;
	WmsServerList* wsl;
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
	SAFE_DELETE(p->theWmsServer);

	p->bgType = theAdapterUid;
	MerkaartorPreferences::instance()->setBackgroundPlugin(theAdapterUid);
	if (p->bgType == NONE_ADAPTER_UUID) {
		setName(tr("Map - None"));
		setVisible(false);
	} else 
	if (p->bgType == WMS_ADAPTER_UUID) {
		wsl = M_PREFS->getWmsServers();
		selws = M_PREFS->getSelectedWmsServer();
		p->theWmsServer = new WmsServer(wsl->value(selws));

		wmsa = new WMSMapAdapter(p->theWmsServer->WmsAdress, p->theWmsServer->WmsPath, p->theWmsServer->WmsLayers, p->theWmsServer->WmsProjections,
			p->theWmsServer->WmsStyles, p->theWmsServer->WmsImgFormat);
		wmsa->setImageManager(ImageManager::instance());
		mapadapter_bg = wmsa;
		p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
		p->layer_bg->setVisible(isVisible());

		setName(tr("Map - WMS - %1").arg(p->theWmsServer->WmsName));
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		tsl = M_PREFS->getTmsServers();
		selts = MerkaartorPreferences::instance()->getSelectedTmsServer();
		ts = tsl->value(selts);
		tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
		tmsa->setImageManager(ImageManager::instance());
		mapadapter_bg = tmsa;
		p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
		p->layer_bg->setVisible(isVisible());

		setName(tr("Map - TMS - %1").arg(ts.TmsName));
	} else
	if (p->bgType == SHAPE_ADAPTER_UUID) {
		if (!M_PREFS->getUseShapefileForBackground()) {
			p->bgType = NONE_ADAPTER_UUID;
			setName(tr("Map - None"));
			setVisible(false);
		} else
			setName(tr("Map - OSB Background"));
	} else
	{
		IMapAdapter * thePluginBackground = M_PREFS->getBackgroundPlugin(p->bgType);
		if (thePluginBackground) {
			switch (thePluginBackground->getType()) {
#ifdef USE_WEBKIT
				case IMapAdapter::BrowserBackground :
					thePluginBackground->setImageManager(BrowserImageManager::instance());
					break;
#endif
				case IMapAdapter::DirectBackground :
					thePluginBackground->setImageManager(ImageManager::instance());
					break;
			}
			mapadapter_bg = thePluginBackground;
			p->layer_bg = new Layer(id(), mapadapter_bg, Layer::MapLayer);
			p->layer_bg->setVisible(isVisible());

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

bool ImageMapLayer::toXML(QDomElement& xParent, QProgressDialog & /* progress */)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", name());
	e.setAttribute("alpha", QString::number(getAlpha(),'f',2));
	e.setAttribute("visible", QString((isVisible() ? "true" : "false")));
	e.setAttribute("selected", QString((isSelected() ? "true" : "false")));
	e.setAttribute("enabled", QString((isEnabled() ? "true" : "false")));

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

ImageMapLayer * ImageMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & /* progress */)
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

void ImageMapLayer::drawImage(QPixmap& /*thePix*/, Projection& /*theProjection*/)
{
	if (!p->theWmsServer)
		return;
}
