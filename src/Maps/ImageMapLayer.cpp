#include "Maps/ImageMapLayer.h"

#include "Maps/MapDocument.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Maps/Projection.h"

#include "IMapAdapter.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
#include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/tilemapadapter.h"
#include "QMapControl/wmsmapadapter.h"

#include <QLocale>
#include <QPainter>

#include "LayerWidget.h"

// ImageMapLayerPrivate

class ImageMapLayerPrivate
{
public:
	QUuid bgType;
	IMapAdapter* theMapAdapter;

	ImageLayerWidget* theWidget;
	QPixmap* pm;
	QPoint theDelta;
	Projection theProjection;
	QString selServer;

public:
	ImageMapLayerPrivate()
	{
		theWidget = NULL;
		theMapAdapter = NULL;
		pm = NULL;
	}
};


// ImageMapLayer

ImageMapLayer::ImageMapLayer(const QString & aName)
	: OsbMapLayer(aName), p(new ImageMapLayerPrivate)
{
	p->bgType = NONE_ADAPTER_UUID;
	setName(tr("Map - None"));
	MapLayer::setVisible(false);
	setReadonly(true);
}

ImageMapLayer::~ ImageMapLayer()
{
	clear();
	SAFE_DELETE(p);
}

int ImageMapLayer::size() const
{
	//return p->Features.size();
	if (p->bgType == SHAPE_ADAPTER_UUID && isVisible())
		return MapLayer::size();
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
	MerkaartorPreferences::instance()->setBgVisible(isVisible());
}

QString ImageMapLayer::projection() const
{
	if (p->theMapAdapter)
		return p->theMapAdapter->projection();

	return "";
}


void ImageMapLayer::setMapAdapter(const QUuid& theAdapterUid, const QString& server)
{
	IMapAdapter* mapadapter_bg;
	WmsServerList* wsl;
	TmsServerList* tsl;
	TmsServer ts;

	if (p->theMapAdapter && p->theMapAdapter->getId() == WMS_ADAPTER_UUID)
		SAFE_DELETE(p->theMapAdapter);

	p->bgType = theAdapterUid;
	MerkaartorPreferences::instance()->setBackgroundPlugin(theAdapterUid);
	if (p->bgType == NONE_ADAPTER_UUID) {
		setName(tr("Map - None"));
		setVisible(false);
	} else 
	if (p->bgType == WMS_ADAPTER_UUID) {
		wsl = M_PREFS->getWmsServers();
		p->selServer = server;
		WmsServer theWmsServer(wsl->value(p->selServer));
		p->theMapAdapter = new WMSMapAdapter(theWmsServer);
		p->theMapAdapter->setImageManager(ImageManager::instance());

		setName(tr("Map - WMS - %1").arg(p->theMapAdapter->getName()));
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		tsl = M_PREFS->getTmsServers();
		p->selServer = server;
		ts = tsl->value(p->selServer);
		TileMapAdapter* tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
		tmsa->setImageManager(ImageManager::instance());
		p->theMapAdapter = tmsa;

		setName(tr("Map - TMS - %1").arg(ts.TmsName));
	} else
	if (p->bgType == SHAPE_ADAPTER_UUID) {
		if (!M_PREFS->getUseShapefileForBackground()) {
			p->bgType = NONE_ADAPTER_UUID;
			setName(tr("Map - None"));
			setVisible(false);
		} else {
			if (QDir::isAbsolutePath(STRINGIFY(WORLD_SHP)))
				setFilename(STRINGIFY(WORLD_SHP));
			else
				setFilename(QCoreApplication::applicationDirPath() + "/" + STRINGIFY(WORLD_SHP));
		}
			setName(tr("Map - OSB Background"));
	} else
	{
		mapadapter_bg = M_PREFS->getBackgroundPlugin(p->bgType);
		if (mapadapter_bg) {
			switch (mapadapter_bg->getType()) {
#ifdef USE_WEBKIT
				case IMapAdapter::BrowserBackground :
					mapadapter_bg->setImageManager(BrowserImageManager::instance());
					break;
#endif
				case IMapAdapter::DirectBackground :
					mapadapter_bg->setImageManager(ImageManager::instance());
					break;
			}
			p->theMapAdapter = mapadapter_bg;

			setName(tr("Map - %1").arg(mapadapter_bg->getName()));
		} else
			p->bgType = NONE_ADAPTER_UUID;
	}
	if (p->theMapAdapter) {
		p->theProjection.setProjectionType(p->theMapAdapter->projection());
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

		c.setAttribute("name", p->selServer);
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		c = e.ownerDocument().createElement("TmsServer");
		e.appendChild(c);

		c.setAttribute("name", p->selServer);
	}

	return OK;
}

ImageMapLayer * ImageMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & /* progress */)
{
	ImageMapLayer* l = new ImageMapLayer(e.attribute("name"));
	d->addImageLayer(l);
	l->setId(e.attribute("xml:id"));

	QDomElement c = e.firstChildElement();

	QString server;
	if (c.tagName() == "WmsServer") {
		server = c.attribute("name");
	} else
	if (c.tagName() == "TmsServer") {
		server = c.attribute("name");
	}
	l->setMapAdapter(QUuid(e.attribute("bgtype")), server);

	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));

	return l;
}

void ImageMapLayer::drawImage(QPixmap& thePix, QPoint delta)
{
	if (!p->theMapAdapter)
		return;

	if (!delta.isNull())
		p->theDelta = delta;
	QPainter P(&thePix);
	P.setOpacity(getAlpha());
	if (p->theMapAdapter->isTiled())
		P.drawPixmap(0, 0, *p->pm);
	else
		P.drawPixmap(p->theDelta, *p->pm);
}

using namespace geometry;

void ImageMapLayer::zoom(double zoom, const QPoint& pos, const QRect& rect) 
{
	if (!p->theMapAdapter)
		return;

	QPixmap tpm = p->pm->scaled(rect.size() * zoom, Qt::KeepAspectRatio);
	p->pm->fill(Qt::transparent);
	QPainter P(p->pm);
	P.drawPixmap(pos - (pos * zoom), tpm);
}

void ImageMapLayer::forceRedraw(const Projection& mainProj, QRect rect)
{
	if (!p->theMapAdapter)
		return;

	if (!p->pm || (p->pm->size() != rect.size())) {
		SAFE_DELETE(p->pm);
		p->pm = new QPixmap(rect.size());
		p->pm->fill(Qt::transparent);
	}
	p->theProjection.setViewport(mainProj.viewport(), rect);
	draw(mainProj, rect);
}

void ImageMapLayer::draw(const Projection& mainProj, QRect& rect)
{
	if (!p->theMapAdapter)
		return;

	if (p->theMapAdapter->isTiled())
		drawTiled(mainProj, rect);
	else
		drawFull(mainProj, rect);
}

void ImageMapLayer::drawFull(const Projection& mainProj, QRect& rect) const
{
	QRectF vp = p->theProjection.getProjectedViewport(rect);
	QRectF wgs84vp = QRectF(QPointF(intToAng(p->theProjection.viewport().bottomLeft().lon()), intToAng(p->theProjection.viewport().bottomLeft().lat()))
						, QPointF(intToAng(p->theProjection.viewport().topRight().lon()), intToAng(p->theProjection.viewport().topRight().lat())));
	QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));

	qDebug() << "ImageMapLayer::drawFull: getting: " << url;

	//p->theMapAdapter->getImageManager()->abortLoading();
	QPixmap pm = p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter,url);
	if (pm.isNull())
		return;

	p->theDelta = QPoint();
	p->pm->fill(Qt::transparent);

	const QPointF tl = mainProj.project(p->theProjection.viewport().topLeft());
	const QPointF br = mainProj.project(p->theProjection.viewport().bottomRight());

	const QRect pr = QRectF(tl, br).toRect();
	const QSize ps = pr.size();

	const qreal ratio = qMax<const qreal>((qreal)rect.width()/ps.width()*1.0, (qreal)rect.height()/ps.height());
	QPixmap pms;
	if (ratio > 1.0) {
		pms = pm.scaled(ps);
	} else {
		const QSizeF drawingSize = pm.size() * ratio;
		const QSizeF originSize = pm.size()/2 - drawingSize/2;
		const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
		const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

		pms = pm.copy(drawingRect).scaled(ps*ratio);
	}
	p->pm->fill(Qt::transparent);
	QPainter P(p->pm);
	P.drawPixmap((rect.width()-pms.width())/2, (rect.height()-pms.height())/2, pms);
}

void ImageMapLayer::drawTiled(const Projection& mainProj, QRect& rect) const
{
	int tilesize = p->theMapAdapter->getTileSize();
	QRectF vp = QRectF(QPointF(intToAng(p->theProjection.viewport().bottomLeft().lon()), intToAng(p->theProjection.viewport().bottomLeft().lat()))
						, QPointF(intToAng(p->theProjection.viewport().topRight().lon()), intToAng(p->theProjection.viewport().topRight().lat())));

	// Set zoom level to 0.
	while (p->theMapAdapter->getAdaptedZoom()) {
		p->theMapAdapter->zoom_out();
	}

	// Find zoom level where tilesize < viewport wdth
	QPoint mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center()); 
	QPoint screenmiddle = rect.center();
	QRectF vlm = QRectF(QPointF(-180., -90.), QSize(360., 180.));
	int maxZoom = p->theMapAdapter->getAdaptedMaxZoom();
	while ((!vp.contains(vlm)) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
		p->theMapAdapter->zoom_in();

		mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center()); 

		QPoint upperLeft = QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()+screenmiddle.y());
		QPoint lowerRight = QPoint(mapmiddle_px.x()+screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y());

		QPointF ulCoord = p->theMapAdapter->displayToCoordinate(upperLeft);
		QPointF lrCoord = p->theMapAdapter->displayToCoordinate(lowerRight);

		vlm = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));
	}

	if (p->theMapAdapter->getAdaptedZoom() && vp.contains(vlm))
		p->theMapAdapter->zoom_out();

	mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center()); 

	QPoint upperLeft = QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()+screenmiddle.y());
	QPoint lowerRight = QPoint(mapmiddle_px.x()+screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y());

	QPointF ulCoord = p->theMapAdapter->displayToCoordinate(upperLeft);
	QPointF lrCoord = p->theMapAdapter->displayToCoordinate(lowerRight);

	vlm = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));

	QPixmap pm(rect.size());
	QPainter painter(&pm);

	// Actual drawing
	int i, j;

	int cross_x = int(mapmiddle_px.x())%tilesize;		// position on middle tile
	int cross_y = int(mapmiddle_px.y())%tilesize;

		// calculate how many surrounding tiles have to be drawn to fill the display
	int space_left = screenmiddle.x() - cross_x;
	int tiles_left = space_left/tilesize;
	if (space_left>0)
		tiles_left+=1;

	int space_above = screenmiddle.y() - cross_y;
	int tiles_above = space_above/tilesize;
	if (space_above>0)
		tiles_above+=1;

	int space_right = screenmiddle.x() - (tilesize-cross_x);
	int tiles_right = space_right/tilesize;
	if (space_right>0)
		tiles_right+=1;

	int space_bottom = screenmiddle.y() - (tilesize-cross_y);
	int tiles_bottom = space_bottom/tilesize;
	if (space_bottom>0)
		tiles_bottom+=1;

// 	int tiles_displayed = 0;
	int mapmiddle_tile_x = mapmiddle_px.x()/tilesize;
	int mapmiddle_tile_y = mapmiddle_px.y()/tilesize;

	const QPoint from =	QPoint((-tiles_left+mapmiddle_tile_x)*tilesize, (-tiles_above+mapmiddle_tile_y)*tilesize);
	const QPoint to =		QPoint((tiles_right+mapmiddle_tile_x+1)*tilesize, (tiles_bottom+mapmiddle_tile_y+1)*tilesize);

	QList<Tile> tiles;

	for (i=-tiles_left+mapmiddle_tile_x; i<=tiles_right+mapmiddle_tile_x; i++)
	{
		for (j=-tiles_above+mapmiddle_tile_y; j<=tiles_bottom+mapmiddle_tile_y; j++)
		{
#ifdef Q_CC_MSVC
			double priority = _hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#else
			double priority = hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#endif
			tiles.append(Tile(i, j, priority));
		}
	}

	qSort(tiles);

	for (QList<Tile>::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile)
	{
		if (p->theMapAdapter->isValid(tile->i, tile->j, p->theMapAdapter->getZoom()))
		{
			QPixmap pm = p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter, tile->i, tile->j, p->theMapAdapter->getZoom());
			if (!pm.isNull())
				painter.drawPixmap(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+rect.width()/2,
							((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+rect.height()/2,
													pm);

			if (MerkaartorPreferences::instance()->getDrawTileBoundary()) {
				painter.drawRect(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+rect.width()/2,
						  ((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+rect.height()/2,
											tilesize, tilesize);
			}
		}
	}
	painter.end();

	const Coord ctl = Coord(angToInt(vlm.bottomLeft().y()), angToInt(vlm.bottomLeft().x()));
	const Coord cbr = Coord(angToInt(vlm.topRight().y()), angToInt(vlm.topRight().x()));

	const QPointF tl = mainProj.project(ctl);
	const QPointF br = mainProj.project(cbr);

	const QRect pr = QRectF(tl, br).toRect();
	const QSize ps = pr.size();

	const qreal ratio = qMax<const qreal>((qreal)rect.width()/ps.width()*1.0, (qreal)rect.height()/ps.height());
	QPixmap pms;
	if (ratio > 1.0) {
		pms = pm.scaled(ps);
	} else {
		const QSizeF drawingSize = pm.size() * ratio;
		const QSizeF originSize = pm.size()/2 - drawingSize/2;
		const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
		const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

		pms = pm.copy(drawingRect).scaled(ps*ratio);
	}
	p->pm->fill(Qt::transparent);
	QPainter P(p->pm);
	P.drawPixmap((rect.width()-pms.width())/2, (rect.height()-pms.height())/2, pms);
}
