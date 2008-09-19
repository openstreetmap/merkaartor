#include "MapView.h"
#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/Interaction.h"
#include "PaintStyle/EditPaintStyle.h"
#include "Map/Projection.h"
#include "GPS/qgps.h"
#include "GPS/qgpsdevice.h"

#include "QMapControl/layermanager.h"
#include "QMapControl/imagemanager.h"
#ifdef YAHOO
	#include "QMapControl/browserimagemanager.h"
#endif
#include "Preferences/MerkaartorPreferences.h"
#include "Utils/SvgCache.h"


#include <QtCore/QTime>
#include <QtGui/QMainWindow>
#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QtGui/QStatusBar>

MapView::MapView(MainWindow* aMain) :
	Main(aMain), theDocument(0), theInteraction(0), StaticBuffer(0), StaticMap(0), 
		StaticBufferUpToDate(false), StaticMapUpToDate(false),numImages(0)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setContextMenuPolicy(Qt::CustomContextMenu);

	ImageManager::instance()->setCacheDir(MerkaartorPreferences::instance()->getCacheDir());
	ImageManager::instance()->setCacheMaxSize(MerkaartorPreferences::instance()->getCacheSize());
	if (MerkaartorPreferences::instance()->getProxyUse()) {
		ImageManager::instance()->setProxy(MerkaartorPreferences::instance()->getProxyHost(),
			MerkaartorPreferences::instance()->getProxyPort());
#ifdef YAHOO
		BrowserImageManager::instance()->setProxy(MerkaartorPreferences::instance()->getProxyHost(),
			MerkaartorPreferences::instance()->getProxyPort());
#endif
	} else {
		ImageManager::instance()->setProxy("",0);
#ifdef YAHOO
		BrowserImageManager::instance()->setProxy("",0);
#endif
	}
	layermanager = new LayerManager((QWidget *) this, size());
	
	connect(ImageManager::instance(), SIGNAL(imageRequested()),
		this, SLOT(imageRequested()));
	connect(ImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(imageReceived()));
	connect(ImageManager::instance(), SIGNAL(loadingFinished()),
		this, SLOT(loadingFinished()));
#ifdef YAHOO
	connect(BrowserImageManager::instance(), SIGNAL(imageRequested()),
		this, SLOT(imageRequested()));
	connect(BrowserImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(imageReceived()));
	connect(BrowserImageManager::instance(), SIGNAL(loadingFinished()),
		this, SLOT(loadingFinished()));
#endif

	MoveRight = new QShortcut(QKeySequence(Qt::Key_Right), this);
	connect(MoveRight, SIGNAL(activated()), this, SLOT(on_MoveRight_activated()));
	MoveLeft = new QShortcut(QKeySequence(Qt::Key_Left), this);
	connect(MoveLeft, SIGNAL(activated()), this, SLOT(on_MoveLeft_activated()));
	MoveUp = new QShortcut(QKeySequence(Qt::Key_Up), this);
	connect(MoveUp, SIGNAL(activated()), this, SLOT(on_MoveUp_activated()));
	MoveDown = new QShortcut(QKeySequence(Qt::Key_Down), this);
	connect(MoveDown, SIGNAL(activated()), this, SLOT(on_MoveDown_activated()));

	invalidRegion = QRegion(rect());
}

MapView::~MapView()
{
	if(theInteraction)
		delete theInteraction;
	delete ImageManager::instance();
#ifdef YAHOO
	delete BrowserImageManager::instance();
#endif
	delete layermanager;
	delete StaticBuffer;
	delete StaticMap;
}

MainWindow *MapView::main()
{
	return Main;
}

PropertiesDock *MapView::properties()
{
	return Main->properties();
}

//InfoDock *MapView::info()
//{
//	return Main->info();
//}

void MapView::setDocument(MapDocument* aDoc)
{
	theDocument = aDoc;

	delete layermanager;
	layermanager = NULL;

	if (theDocument->getImageLayer()) {
		layermanager = new LayerManager((QWidget *) this, size());
		if (theDocument->getImageLayer()->imageLayer())
			layermanager->addLayer(theDocument->getImageLayer()->imageLayer());
		theDocument->getImageLayer()->layermanager = layermanager;
	}

	projection().setLayerManager(layermanager);
//	projection().setViewport(WORLD_COORDBOX, rect());
	projection().setViewport(projection().viewport(), rect());
}

MapDocument *MapView::document()
{
	return theDocument;
}

void MapView::invalidate(bool updateStaticBuffer, bool updateMap)
{
	if (updateStaticBuffer) {
		invalidRegion = QRegion(rect());
		StaticBufferUpToDate = false;
	}
	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible() && updateMap) {
		layermanager->forceRedraw();
		StaticMapUpToDate = false;
	}
	update();
}

void MapView::panScreen(QPoint delta) 
{
	thePanDelta += delta;
	projection().panScreen(delta,rect());

	QRegion panRg = QRegion(rect()) - invalidRegion;
	QRect bbRect = panRg.boundingRect();

	if (delta.x() < 0)
		invalidRegion += QRect(bbRect.x() + bbRect.width() + delta.x(), bbRect.y(), -delta.x(), bbRect.height());
	else
		invalidRegion += QRect(bbRect.x(), bbRect.y(), delta.x(), bbRect.height());
	if (delta.y() < 0)
		invalidRegion += QRect(bbRect.x(), bbRect.y() + bbRect.height() + delta.y(), bbRect.width(), -delta.y());
	else
		invalidRegion += QRect(bbRect.x(), bbRect.y(), bbRect.width(), delta.y());

	StaticBufferUpToDate = false;

	update();
}

void MapView::paintEvent(QPaintEvent * anEvent)
{
	QTime Start(QTime::currentTime());

	if (!(StaticBufferUpToDate && StaticMapUpToDate))
		qDebug() << "PaintEvent: " << StaticBufferUpToDate << "; " << StaticMapUpToDate;

	QPainter P(this);
	QRegion rg(rect());
	P.setClipRegion(rg);
	P.setClipping(true);

	P.fillRect(rect(), QBrush(MerkaartorPreferences::instance()->getBgColor()));

	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible()) {
		updateLayersImage();
		P.setOpacity(theDocument->getImageLayer()->getAlpha());
		P.drawPixmap(thePanDelta, *StaticMap);
	}
	
	updateStaticBuffer();
	P.setOpacity(1.0);
	P.drawPixmap(QPoint(0, 0), *StaticBuffer);

	drawDownloadAreas(P);
	drawScale(P);

	if (theInteraction) {
		P.setRenderHint(QPainter::Antialiasing);
		theInteraction->paintEvent(anEvent, P);
	}

	drawGPS(P);

	Main->ViewportStatusLabel->setText(QString("%1,%2,%3,%4")
		.arg(QString::number(intToAng(theProjection.viewport().bottomLeft().lon()),'f',4)) 
		.arg(QString::number(intToAng(theProjection.viewport().bottomLeft().lat()),'f',4))
		.arg(QString::number(intToAng(theProjection.viewport().topRight().lon()),'f',4))
		.arg(QString::number(intToAng(theProjection.viewport().topRight().lat()),'f',4))
		);

	QTime Stop(QTime::currentTime());
	Main->PaintTimeLabel->setText(tr("%1ms").arg(Start.msecsTo(Stop)));
	QTimer::singleShot(0,this,SLOT(updateStatusMessage()));
}

void MapView::drawScale(QPainter & P)
{
	if (!M_PREFS->getScaleVisible())
		return;

	double Log = log10(200/projection().pixelPerM());
	double RestLog = Log-floor(Log);
	if (RestLog < log10(2.))
		Log = floor(Log);
	else if (RestLog < log10(5.))
		Log = floor(Log)+log10(2.);
	else
		Log = floor(Log)+log10(5.);

	double Length = pow(10.,Log);
	P.setPen(QPen(QColor(0,0,0),2));
	QPointF P1(20,height()-20);
	QPointF P2(20+Length*projection().pixelPerM(),height()-20);
	P.drawLine(P1-QPointF(0,5),P1+QPointF(0,5));
	P.drawLine(P1,P2);
	if (Length < 1000)
		P.drawText(QRectF(P2-QPoint(100,40),QSize(200,30)),Qt::AlignHCenter | Qt::AlignBottom, QString(tr("%1 m")).arg(Length, 0, 'f', 0));
	else
		P.drawText(QRectF(P2-QPoint(100,40),QSize(200,30)),Qt::AlignHCenter | Qt::AlignBottom, QString(tr("%1 km")).arg(Length/1000, 0, 'f', 0));

	P.drawLine(P2-QPointF(0,5),P2+QPointF(0,5));
}

void MapView::drawGPS(QPainter & P)
{
	if (Main->gps()->getGpsDevice()) {
		if (Main->gps()->getGpsDevice()->fixStatus() == QGPSDevice::StatusActive) {
			Coord vp(angToInt(Main->gps()->getGpsDevice()->latitude()), angToInt(Main->gps()->getGpsDevice()->longitude()));
			QPoint g = projection().project(vp);
			QPixmap pm = getPixmapFromFile(":/Gps/Gps_Marker.svg", 32);
			P.drawPixmap(g - QPoint(16, 16), pm);
		}
	}
}

void MapView::sortRenderingPriorityInLayers()
{
	for (unsigned int i = 0; i < theDocument->layerSize(); ++i) {
		theDocument->getLayer(i)->
			sortRenderingPriority(theProjection.pixelPerM());
	}
}

void MapView::updateLayersImage()
{
	if (StaticMapUpToDate)
		return;

	if (!StaticMap || (StaticMap->size() != size()))
	{
		delete StaticMap;
		StaticMap = new QPixmap(size());
	}
	QPainter P(StaticMap);

	if (MerkaartorPreferences::instance()->getProjectionType() == Proj_Background)
	{
		thePanDelta = QPoint(0, 0);
		StaticMapUpToDate = true;
		layermanager->drawImage(&P);
		return;
	}

	const QRectF vlm = layermanager->getViewport();
	const Coord ctl = Coord(angToInt(vlm.bottomLeft().y()), angToInt(vlm.bottomLeft().x()));
	const Coord cbr = Coord(angToInt(vlm.topRight().y()), angToInt(vlm.topRight().x()));

	const QPointF tl = projection().project(ctl);
	const QPointF br = projection().project(cbr);

	const QRect pr = QRectF(tl, br).toRect();
	const QSize ps = pr.size();
	QPixmap pm(size());
	QPainter pmp(&pm);
	layermanager->drawImage(&pmp);

	const qreal ratio = qMax<const qreal>((qreal)width()/ps.width()*1.0, (qreal)height()/ps.height());
	QPixmap pms;
	if (ratio > 1.0) {
		pms = pm.scaled(ps /*, Qt::IgnoreAspectRatio, Qt::SmoothTransformation */ );
	} else {
		const QSizeF drawingSize = pm.size() * ratio;
		const QSizeF originSize = pm.size()/2 - drawingSize/2;
		const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
		const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

		pms = pm.copy(drawingRect).scaled(ps*ratio /*, Qt::IgnoreAspectRatio, Qt::SmoothTransformation */ );
	}

	P.drawPixmap((width()-pms.width())/2, (height()-pms.height())/2, pms);

	thePanDelta = QPoint(0, 0);
	StaticMapUpToDate = true;
}

void MapView::drawFeatures(QPainter & P)
{
	EditPaintStyle EP(P, projection());

	for (unsigned int i=0; i<theDocument->layerSize(); ++i) {
		theDocument->getLayer(i)->invalidate(theDocument, theProjection.viewport());
	}

	QVector <CoordBox> coordRegion;
	for (int i=0; i < P.clipRegion().rects().size(); ++i) {
		Coord tl = theProjection.inverse(P.clipRegion().rects()[i].topLeft());
		Coord br = theProjection.inverse(P.clipRegion().rects()[i].bottomRight());
		coordRegion += CoordBox(tl, br);
	}

	QVector<MapFeature*> theFeatures;
	for (unsigned int i=0; i<theDocument->layerSize(); ++i) {
		if (!theDocument->getLayer(i)->isVisible())
			continue;
		for (unsigned int j=0; j<theDocument->getLayer(i)->size(); ++j) {
			bool OK = false;
			for (int k=0; k<coordRegion.size() && !OK; ++k)
				if (coordRegion[k].intersects(theDocument->getLayer(i)->get(j)->boundingBox()))
					OK = true;
			if (OK) {
				theFeatures.push_back(theDocument->getLayer(i)->get(j));
				if (Road * R = dynamic_cast < Road * >(theDocument->getLayer(i)->get(j)))
					R->buildPath(theProjection, P.clipRegion());
				if (Relation * RR = dynamic_cast < Relation * >(theDocument->getLayer(i)->get(j)))
					RR->buildPath(theProjection, P.clipRegion());
			}
		}
	}

	for (unsigned int i = 0; i < EP.size(); ++i)
	{
		PaintStyleLayer *Current = EP.get(i);

#ifndef NDEBUG
		EPBackgroundLayer* bl = dynamic_cast<EPBackgroundLayer*> (Current);
		if ((bl) && (!M_PREFS->getStyleBackgroundVisible()))
			continue;
		EPForegroundLayer* fl = dynamic_cast<EPForegroundLayer*> (Current);
		if ((fl) && (!M_PREFS->getStyleForegroundVisible()))
			continue;
		EPTouchupLayer* tl = dynamic_cast<EPTouchupLayer*> (Current);
		if ((tl) && (!M_PREFS->getStyleTouchupVisible()))
			continue;
#endif
		EPLabelLayer* nl = dynamic_cast<EPLabelLayer*> (Current);
		if ((nl) && (!M_PREFS->getNamesVisible()))
			continue;

		P.save();
		for (int i=0; i<theFeatures.size(); i++)
		{
			P.setOpacity(theFeatures[i]->layer()->getAlpha());
			if (Road * R = dynamic_cast < Road * >(theFeatures[i]))
				Current->draw(R);
			else if (TrackPoint * Pt = dynamic_cast < TrackPoint * >(theFeatures[i]))
				Current->draw(Pt);
			else if (Relation * RR = dynamic_cast < Relation * >(theFeatures[i]))
				Current->draw(RR);
		}
		P.restore();
	}
	
	for (int i=0; i<theFeatures.size(); i++)
	{
		theFeatures[i]->draw(P, theProjection);
	}
}

void MapView::drawDownloadAreas(QPainter & P)
{
	if (MerkaartorPreferences::instance()->getDownloadedVisible() == false)
		return;

	QPixmap pxDownloadAreas(width(), height());
	pxDownloadAreas.fill(Qt::transparent);
	QPainter D(&pxDownloadAreas);
	QRegion r(0, 0, width(), height());


	//QBrush b(Qt::red, Qt::DiagCrossPattern);
	QBrush b(Qt::red, Qt::Dense7Pattern);

	QList<CoordBox>::iterator bb;
	for (bb = theDocument->getDownloadBoxes()->begin(); bb != theDocument->getDownloadBoxes()->end(); ++bb) {
		if (projection().viewport().disjunctFrom(*bb)) continue;
		QPolygonF poly;
		poly << projection().project((*bb).topLeft());
		poly << projection().project((*bb).bottomLeft());
		poly << projection().project((*bb).bottomRight());
		poly << projection().project((*bb).topRight());
		poly << projection().project((*bb).topLeft());

		r -= QRegion(poly.toPolygon());
	}

	D.setClipRegion(r);
	D.setClipping(true);
	D.fillRect(pxDownloadAreas.rect(), b);
	P.drawPixmap(0, 0, pxDownloadAreas);
}


void MapView::updateStaticBuffer()
{
	if (StaticBufferUpToDate)
		return;

	QPixmap savPix;
	QPoint pan;
	QRect bbRect;

	QRegion panRg = QRegion(rect()) - invalidRegion;
	if (!panRg.isEmpty()) {
		//pan = (panRg.boundingRect().center() - rect().center()) * 2.0;
		bbRect = panRg.boundingRect();
		pan = bbRect.topLeft() - rect().topLeft() + bbRect.bottomRight() - rect().bottomRight();
		savPix = StaticBuffer->copy();
	}

	//pan = thePanDelta - theLastDelta;
	//if (!pan.isNull()) {
	//	savPix = StaticBuffer->copy();
	//	theLastDelta = thePanDelta;
	//}
	if (!StaticBuffer || (StaticBuffer->size() != size()))
	{
		delete StaticBuffer;
		StaticBuffer = new QPixmap(size());
	}

	StaticBuffer->fill(Qt::transparent);

	QPainter painter(StaticBuffer);

	painter.setClipping(true);
	if (!panRg.isEmpty()) {
		painter.setClipRegion(panRg);
		painter.drawPixmap(pan, savPix);
	}

	painter.setClipRegion(invalidRegion);

	painter.setRenderHint(QPainter::Antialiasing);

	if (theDocument)
	{
		sortRenderingPriorityInLayers();
		//drawLayersImage(painter);
		drawFeatures(painter);
	}

	invalidRegion = QRegion();
	StaticBufferUpToDate = true;
}

void MapView::updateStatusMessage()
{
	Main->statusBar()->showMessage(StatusMessage);
}

void MapView::mousePressEvent(QMouseEvent* event)
{
	if (theInteraction)
		theInteraction->mousePressEvent(event);
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
	if (theInteraction)
		theInteraction->mouseReleaseEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent* anEvent)
{
	if (!updatesEnabled())
		return;
	if (theInteraction)
		theInteraction->mouseMoveEvent(anEvent);
}

void MapView::wheelEvent(QWheelEvent* ev)
{
	int Steps = ev->delta() / 120;
	if (Steps > 0) {
		for (int i = 0; i < Steps; ++i) {
			projection().zoom(MerkaartorPreferences::instance()->getZoomInPerc()/100.0, ev->pos(), rect());
		}
		invalidate(true, true);
	} else if (Steps < 0) {
		for (int i = 0; i < -Steps; ++i) {
			projection().zoom(MerkaartorPreferences::instance()->getZoomOutPerc()/100.0, ev->pos(), rect());
		}
		invalidate(true, true);
	}
}

void MapView::launch(Interaction* anInteraction)
{
	if (theInteraction)
		delete theInteraction;
	theInteraction = anInteraction;
	if (theInteraction)
		setCursor(theInteraction->cursor());
	else {
		setCursor(QCursor(Qt::ArrowCursor));
		launch(new EditInteraction(this));
	}
}

Interaction *MapView::interaction()
{
	return theInteraction;
}

Projection& MapView::projection()
{
	return theProjection;
}

void MapView::on_customContextMenuRequested(const QPoint & pos)
{
	if (/*EditInteraction* ei = */dynamic_cast<EditInteraction*>(theInteraction)) {
		QMenu menu;

		//FIXME Some of these actions on WIN32-MSVC corrupts the heap.

		//QMenu editMenu(tr("Edit"));
		//for(int i=0; i<Main->menuEdit->actions().size(); ++i) {
		//	if (Main->menuEdit->actions()[i]->isEnabled())
		//		editMenu.addAction(Main->menuEdit->actions()[i]);
		//}
		//if (editMenu.actions().size())
		//	menu.addMenu(&editMenu);

		//QMenu createMenu(tr("Create"));
		//for(int i=0; i<Main->menuCreate->actions().size(); ++i) {
		//	if (Main->menuCreate->actions()[i]->isEnabled())
		//		createMenu.addAction(Main->menuCreate->actions()[i]);
		//}
		//if (createMenu.actions().size())
		//	menu.addMenu(&createMenu);

		menu.addAction(Main->viewZoomInAction);
		menu.addAction(Main->viewZoomOutAction);

		QMenu featureMenu(tr("Feature"));
		for(int i=0; i<Main->menu_Feature->actions().size(); ++i) {
			if (Main->menu_Feature->actions()[i]->isEnabled())
				featureMenu.addAction(Main->menu_Feature->actions()[i]);
		}
		if (featureMenu.actions().size())
			menu.addMenu(&featureMenu);

		QMenu roadMenu(tr("Road"));
		for(int i=0; i<Main->menuRoad->actions().size(); ++i) {
			if (Main->menuRoad->actions()[i]->isEnabled())
				roadMenu.addAction(Main->menuRoad->actions()[i]);
		}
		if (roadMenu.actions().size())
			menu.addMenu(&roadMenu);

		QMenu nodeMenu(tr("Node"));
		for(int i=0; i<Main->menu_Node->actions().size(); ++i) {
			if (Main->menu_Node->actions()[i]->isEnabled())
				nodeMenu.addAction(Main->menu_Node->actions()[i]);
		}
		if (nodeMenu.actions().size())
			menu.addMenu(&nodeMenu);

		if (menu.actions().size()) {
			if (menu.actions().size() == 1) {
				for (int i=0; i < menu.actions()[0]->menu()->actions().size(); ++i) {
					menu.addAction(menu.actions()[0]->menu()->actions()[i]);
				}
				menu.removeAction(menu.actions()[0]);
			}
			menu.exec(mapToGlobal(pos));
		}
	}
}


void MapView::imageRequested()
{
	++numImages;
	Main->pbImages->setRange(0, numImages);
	//pbImages->setValue(0);
	Main->pbImages->update();
	if (Main->pbImages->value() < 0)
		Main->pbImages->setValue(0);
}

void MapView::imageReceived()
{
	Main->pbImages->setValue(Main->pbImages->value()+1);
	invalidate(false, true);
}

void MapView::loadingFinished()
{
	layermanager->removeZoomImage();
	numImages = 0;
	Main->pbImages->reset();

	//invalidate(false, true);
}

void MapView::resizeEvent(QResizeEvent * event)
{
	StaticBufferUpToDate = false;
	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible())
		layermanager->setSize(size());
	projection().zoom(1, QPoint(width() / 2, height() / 2), rect());

	QWidget::resizeEvent(event);

	invalidate(true, true);
}

bool MapView::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.namedItem("MapView").toElement();
	if (!e.isNull()) {
		xParent.removeChild(e);
	}
	e = xParent.ownerDocument().createElement("MapView");
	xParent.appendChild(e);

	projection().toXML(e);

	return OK;
}

void MapView::fromXML(const QDomElement e)
{
	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "Projection") {
			projection().fromXML(c, rect());
		}

		c = c.nextSiblingElement();
	}
	invalidate(true, true);
}

void MapView::on_MoveLeft_activated()
{
	QPoint p(rect().width()/4,0);
	projection().panScreen(p, rect());

	invalidate(true, true);
}
void MapView::on_MoveRight_activated()
{
	QPoint p(-rect().width()/4,0);
	projection().panScreen(p, rect());

	invalidate(true, true);
}

void MapView::on_MoveUp_activated()
{
	QPoint p(0,rect().height()/4);
	projection().panScreen(p, rect());

	invalidate(true, true);
}

void MapView::on_MoveDown_activated()
{
	QPoint p(0,-rect().height()/4);
	projection().panScreen(p, rect());

	invalidate(true, true);
}

