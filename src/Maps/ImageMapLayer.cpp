#include "MapView.h"
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
	QPixmap pm;
	QPoint theDelta;
	Projection theProjection;
	QString selServer;
	IImageManager* theImageManager;
	TileMapAdapter* tmsa;
	WMSMapAdapter* wmsa;
	QRect pr;
	QTransform theTransform;
	CoordBox Viewport;

public:
	ImageMapLayerPrivate()
	{
		theWidget = NULL;
		theMapAdapter = NULL;
		theImageManager = NULL;
		tmsa = NULL;
		wmsa = NULL;
	}
	~ImageMapLayerPrivate()
	{
		SAFE_DELETE(wmsa);
		SAFE_DELETE(tmsa);
		SAFE_DELETE(theImageManager);
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

IImageManager* ImageMapLayer::getImageManger()
{
	return p->theImageManager;
}

void ImageMapLayer::setMapAdapter(const QUuid& theAdapterUid, const QString& server)
{
	WmsServerList* wsl;
	TmsServerList* tsl;

	SAFE_DELETE(p->wmsa);
	SAFE_DELETE(p->tmsa);
	p->theMapAdapter = NULL;
	p->pm = QPixmap();

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
		p->wmsa = new WMSMapAdapter(theWmsServer);
		p->theMapAdapter = p->wmsa;

		setName(tr("Map - WMS - %1").arg(p->theMapAdapter->getName()));
	} else
	if (p->bgType == TMS_ADAPTER_UUID) {
		tsl = M_PREFS->getTmsServers();
		p->selServer = server;
		TmsServer ts = tsl->value(p->selServer);
		p->tmsa = new TileMapAdapter(ts.TmsAdress, ts.TmsPath, ts.TmsTileSize, ts.TmsMinZoom, ts.TmsMaxZoom);
		p->theMapAdapter = p->tmsa;

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
		p->theMapAdapter = M_PREFS->getBackgroundPlugin(p->bgType);
		if (p->theMapAdapter) {
			setName(tr("Map - %1").arg(p->theMapAdapter->getName()));
		} else
			p->bgType = NONE_ADAPTER_UUID;
	}
	if (p->theMapAdapter) {
		p->theProjection.setProjectionType(p->theMapAdapter->projection());
		ImageManager* m;
		BrowserImageManager* b;
		switch (p->theMapAdapter->getType()) {
#ifdef USE_WEBKIT
			case IMapAdapter::BrowserBackground :
				b = new BrowserImageManager();
				connect(b, SIGNAL(imageRequested()),
					this, SLOT(on_imageRequested()), Qt::QueuedConnection);
				connect(b, SIGNAL(imageReceived()),
					this, SLOT(on_imageReceived()), Qt::QueuedConnection);
				connect(b, SIGNAL(loadingFinished()),
					this, SLOT(on_loadingFinished()), Qt::QueuedConnection);
				#ifdef BROWSERIMAGEMANAGER_IS_THREADED
					m->start();
				#endif // BROWSERIMAGEMANAGER_IS_THREADED
				p->theImageManager = b;
				p->theMapAdapter->setImageManager(p->theImageManager);
				break;
#endif
			case IMapAdapter::DirectBackground :
				m = new ImageManager();
				connect(m, SIGNAL(imageRequested()),
					this, SLOT(on_imageRequested()), Qt::QueuedConnection);
				connect(m, SIGNAL(imageReceived()),
					this, SLOT(on_imageReceived()), Qt::QueuedConnection);
				connect(m, SIGNAL(loadingFinished()),
					this, SLOT(on_loadingFinished()), Qt::QueuedConnection);
				p->theImageManager = m;
				p->theMapAdapter->setImageManager(p->theImageManager);
				break;
		}
		p->theImageManager->setCacheDir(M_PREFS->getCacheDir());
		p->theImageManager->setCacheMaxSize(M_PREFS->getCacheSize());
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

	const QSize ps = p->pr.size();
	const QSize pmSize = p->pm.size();
	const qreal ratio = qMax<const qreal>((qreal)pmSize.width()/ps.width()*1.0, (qreal)pmSize.height()/ps.height());
	QPixmap pms;
	if (ratio > 1.0) {
		pms = p->pm.scaled(ps);
	} else {
		const QSizeF drawingSize = pmSize * ratio;
		const QSizeF originSize = pmSize/2 - drawingSize/2;
		const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
		const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

		pms = p->pm.copy(drawingRect).scaled(ps*ratio);
	}

	if (!delta.isNull())
		p->theDelta = delta;
	QPainter P(&thePix);
	P.setOpacity(getAlpha());
	if (p->theMapAdapter->isTiled())
		P.drawPixmap((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2, pms);
	else
		P.drawPixmap(QPoint((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2) + p->theDelta, pms);
}

using namespace geometry;

void ImageMapLayer::zoom(double zoom, const QPoint& pos, const QRect& rect) 
{
	if (!p->theMapAdapter)
		return;

	QPixmap tpm = p->pm.scaled(rect.size() * zoom, Qt::KeepAspectRatio);
	p->pm.fill(Qt::transparent);
	QPainter P(&p->pm);
	P.drawPixmap(pos - (pos * zoom), tpm);
}

void ImageMapLayer::forceRedraw(MapView& theView, QRect Screen)
{
	if (!p->theMapAdapter)
		return;

	if (p->pm.size() != Screen.size()) {
		p->pm = QPixmap(Screen.size());
		p->pm.fill(Qt::transparent);
	}

	MapView::transformCalc(p->theTransform, p->theProjection, theView.viewport(), Screen);

	QRectF fScreen(Screen);
	p->Viewport =
		CoordBox(p->theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomLeft())),
			 p->theProjection.inverse(p->theTransform.inverted().map(fScreen.topRight())));

	p->theImageManager->abortLoading();
	draw(theView, Screen);
}

void ImageMapLayer::draw(MapView& theView, QRect& rect)
{
	if (!p->theMapAdapter)
		return;

	if (p->theMapAdapter->isTiled())
		p->pr = drawTiled(theView, rect);
	else
		p->pr = drawFull(theView, rect);
}

QRect ImageMapLayer::drawFull(MapView& theView, QRect& rect) const
{
	QRectF vp = p->theProjection.getProjectedViewport(p->Viewport, rect);
	QRectF wgs84vp = QRectF(QPointF(intToAng(p->Viewport.bottomLeft().lon()), intToAng(p->Viewport.bottomLeft().lat()))
						, QPointF(intToAng(p->Viewport.topRight().lon()), intToAng(p->Viewport.topRight().lat())));
	QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));
	if (!url.isEmpty()) {

		qDebug() << "ImageMapLayer::drawFull: getting: " << url;

		p->theMapAdapter->getImageManager()->abortLoading();
		QPixmap pm = p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter,url);
		if (!pm.isNull())  {
			p->pm = pm.scaled(rect.size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
			p->theDelta = QPoint();
		}
	}

	const QPointF tl = theView.transform().map(theView.projection().project(p->Viewport.topLeft()));
	const QPointF br = theView.transform().map(theView.projection().project(p->Viewport.bottomRight()));

	return QRectF(tl, br).toRect();
}

QRect ImageMapLayer::drawTiled(MapView& theView, QRect& rect) const
{
	int tilesize = p->theMapAdapter->getTileSize();
	QRectF vp = QRectF(QPointF(intToAng(p->Viewport.bottomLeft().lon()), intToAng(p->Viewport.bottomLeft().lat()))
						, QPointF(intToAng(p->Viewport.topRight().lon()), intToAng(p->Viewport.topRight().lat())));

	// Set zoom level to 0.
	while (p->theMapAdapter->getAdaptedZoom()) {
		p->theMapAdapter->zoom_out();
	}

	// Find zoom level where tilesize < viewport wdth
	QPoint mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center()); 
	QPoint screenmiddle = rect.center();
	QRectF vlm = QRectF(QPointF(-180., -90.), QSizeF(360., 180.));
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

	p->pm = QPixmap(rect.size());
	QPainter painter(&p->pm);

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

	const QPointF tl = theView.transform().map(theView.projection().project(ctl));
	const QPointF br = theView.transform().map(theView.projection().project(cbr));

	return QRectF(tl, br).toRect();
}

void ImageMapLayer::on_imageRequested()
{
	emit imageRequested(this);
}

void ImageMapLayer::on_imageReceived()
{
	emit imageReceived(this);
}

void ImageMapLayer::on_loadingFinished()
{
	emit loadingFinished(this);
}

