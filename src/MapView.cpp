#include "MapView.h"
#include "MainWindow.h"
#include "PropertiesDock.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"
#include "Maps/ImageMapLayer.h"
#include "Maps/MapFeature.h"
#include "Maps/Relation.h"
#include "Interaction/Interaction.h"
#include "Interaction/EditInteraction.h"
#include "PaintStyle/EditPaintStyle.h"
#include "Maps/Projection.h"
#include "GPS/qgps.h"
#include "GPS/qgpsdevice.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#include "QMapControl/layermanager.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
	#include "QMapControl/browserimagemanager.h"
#endif
#include "Preferences/MerkaartorPreferences.h"
#include "Utils/SvgCache.h"


#include <QTime>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QToolTip>

MapView::MapView(MainWindow* aMain) :
	QWidget(aMain), Main(aMain), theDocument(0), theInteraction(0), StaticBackground(0), StaticBuffer(0), StaticMap(0), 
		StaticBufferUpToDate(false), StaticMapUpToDate(false), SelectionLocked(false),lockIcon(0), numImages(0)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setFocusPolicy(Qt::ClickFocus);
#ifdef GEOIMAGE
	setAcceptDrops(true);
#endif

	ImageManager::instance()->setCacheDir(MerkaartorPreferences::instance()->getCacheDir());
	ImageManager::instance()->setCacheMaxSize(MerkaartorPreferences::instance()->getCacheSize());
#ifdef USE_WEBKIT
	BrowserImageManager::instance()->setCacheDir(MerkaartorPreferences::instance()->getCacheDir());
	BrowserImageManager::instance()->setCacheMaxSize(MerkaartorPreferences::instance()->getCacheSize());
#ifdef BROWSERIMAGEMANAGER_IS_THREADED
	BrowserImageManager::instance()->start();
#endif // BROWSERIMAGEMANAGER_IS_THREADED
#endif //USE_WEBKIT

	layermanager = new LayerManager((QWidget *) this, size());
	
	connect(ImageManager::instance(), SIGNAL(imageRequested()),
		this, SLOT(imageRequested()), Qt::QueuedConnection);
	connect(ImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(imageReceived()), Qt::QueuedConnection);
	connect(ImageManager::instance(), SIGNAL(loadingFinished()),
		this, SLOT(loadingFinished()), Qt::QueuedConnection);
#ifdef USE_WEBKIT
	connect(BrowserImageManager::instance(), SIGNAL(imageRequested()),
		this, SLOT(imageRequested()), Qt::QueuedConnection);
	connect(BrowserImageManager::instance(), SIGNAL(imageReceived()),
		this, SLOT(imageReceived()), Qt::QueuedConnection);
	connect(BrowserImageManager::instance(), SIGNAL(loadingFinished()),
		this, SLOT(loadingFinished()), Qt::QueuedConnection);
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
#ifdef USE_WEBKIT
#ifdef BROWSERIMAGEMANAGER_IS_THREADED
	BrowserImageManager::instance()->quit();
	BrowserImageManager::instance()->wait(1000);
#endif // BROWSERIMAGEMANAGER_IS_THREADED
	delete BrowserImageManager::instance();
#endif
	delete layermanager;
	delete StaticBackground;
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
		connect(theDocument->getImageLayer(), SIGNAL(imageReceived()),
			this, SLOT(imageReceived()));
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
	if (theDocument && theDocument->getImageLayer() && updateMap) {
		theDocument->getImageLayer()->forceRedraw(theProjection, rect());
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

	//if (!(StaticBufferUpToDate && StaticMapUpToDate))
	//	qDebug() << "PaintEvent: " << StaticBufferUpToDate << "; " << StaticMapUpToDate;

	QPainter P;
	P.begin(this);
	QRegion rg(rect());
	P.setClipRegion(rg);
	P.setClipping(true);

	updateLayersImage();

	if (!StaticBufferUpToDate) {
		buildFeatureSet(invalidRegion, theProjection);
		updateStaticBackground();
		updateStaticBuffer();
	}

	P.drawPixmap(QPoint(0, 0), *StaticBackground);

	if (theDocument->getImageLayer()->isVisible()) {
		P.setOpacity(theDocument->getImageLayer()->getAlpha());
		P.drawPixmap(thePanDelta, *StaticMap);
		P.setOpacity(1.0);
	}
    P.drawPixmap(QPoint(0, 0), *StaticBuffer);

	drawDownloadAreas(P);
	drawScale(P);

	if (theInteraction) {
		P.setRenderHint(QPainter::Antialiasing);
		theInteraction->paintEvent(anEvent, P);
	}

	drawGPS(P);

	P.end();

	Main->ViewportStatusLabel->setText(QString("%1,%2,%3,%4")
		.arg(QString::number(intToAng(theProjection.viewport().bottomLeft().lon()),'f',4)) 
		.arg(QString::number(intToAng(theProjection.viewport().bottomLeft().lat()),'f',4))
		.arg(QString::number(intToAng(theProjection.viewport().topRight().lon()),'f',4))
		.arg(QString::number(intToAng(theProjection.viewport().topRight().lat()),'f',4))
		);

	QTime Stop(QTime::currentTime());
	Main->PaintTimeLabel->setText(tr("%1ms").arg(Start.msecsTo(Stop)));
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
	for (int i = 0; i < theDocument->layerSize(); ++i) {
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
	StaticMap->fill(Qt::transparent);

	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible()) {
		QPixmap pm(size());
		QPainter pmp(&pm);
		layermanager->drawImage(&pmp);

		const QRectF vlm = layermanager->getViewport();
		const Coord ctl = Coord(angToInt(vlm.bottomLeft().y()), angToInt(vlm.bottomLeft().x()));
		const Coord cbr = Coord(angToInt(vlm.topRight().y()), angToInt(vlm.topRight().x()));

		const QPointF tl = projection().project(ctl);
		const QPointF br = projection().project(cbr);

		const QRect pr = QRectF(tl, br).toRect();
		const QSize ps = pr.size();

		const qreal ratio = qMax<const qreal>((qreal)width()/ps.width()*1.0, (qreal)height()/ps.height());
		QPixmap pms;
		if (ratio > 1.0) {
#ifndef _MOBILE
			pms = pm.scaled(ps, Qt::KeepAspectRatio, Qt::FastTransformation );
#else
			pms = pm.scaled(ps);
#endif
		} else {
			const QSizeF drawingSize = pm.size() * ratio;
			const QSizeF originSize = pm.size()/2 - drawingSize/2;
			const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
			const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

#ifndef _MOBILE
			pms = pm.copy(drawingRect).scaled(ps*ratio, Qt::KeepAspectRatio, Qt::FastTransformation );
#else
			pms = pm.copy(drawingRect).scaled(ps*ratio);
#endif
		}

		QPainter P(StaticMap);
		P.drawPixmap((width()-pms.width())/2, (height()-pms.height())/2, pms);
	}
	for (int i=0; i<theDocument->layerSize(); ++i)
	{
		ImageMapLayer* IL = dynamic_cast<ImageMapLayer*>(theDocument->getLayer(i));
		if ( IL )
		{
			//IL->panScreen(delta);
			IL->drawImage(*StaticMap, thePanDelta);
		}
	}

	thePanDelta = QPoint(0, 0);
	StaticMapUpToDate = true;
}

void MapView::buildFeatureSet(QRegion invalidRegion, Projection& aProj)
{
	theFeatures.clear();
	theCoastlines.clear();

	for (int i=0; i<theDocument->layerSize(); ++i) {
		theDocument->getLayer(i)->invalidate(theDocument, aProj.viewport());
		Main->properties()->adjustSelection();
	}

	QList <CoordBox> coordRegion;
	for (int i=0; i < invalidRegion.rects().size(); ++i) {
		Coord tl = aProj.inverse(invalidRegion.rects()[i].topLeft());
		Coord br = aProj.inverse(invalidRegion.rects()[i].bottomRight());
		coordRegion += CoordBox(tl, br);
	}

	QRect clipRect = invalidRegion.boundingRect().adjusted(int(-20), int(-20), int(20), int(20));

	for (VisibleFeatureIterator vit(theDocument); !vit.isEnd(); ++vit) {
		bool OK = false;
		for (int k=0; k<coordRegion.size() && !OK; ++k)
			if (coordRegion[k].intersects(vit.get()->boundingBox()))
				OK = true;
		if (OK) {
			if (Road * R = dynamic_cast < Road * >(vit.get())) {
				R->buildPath(aProj, clipRect);
				theFeatures.push_back(R);
				if (R->isCoastline())
					theCoastlines.push_back(R);
			} else
			if (Relation * RR = dynamic_cast < Relation * >(vit.get())) {
				RR->buildPath(aProj, clipRect);
				theFeatures.push_back(RR);
			} else {
				theFeatures.push_back(vit.get());
			}
		}
	}
}

bool testColor(const QImage& theImage, const QPoint& P, const QRgb& targetColor)
{
	if (!theImage.rect().contains(P)) return false;
	return (theImage.pixel(P) == targetColor);
}

void floodFill(QImage& theImage, const QPoint& P, const QRgb& targetColor, const QRgb& replaceColor)
{
	if (!testColor(theImage, P, targetColor)) return;

	QStack<QPoint> theStack;
	QPoint aP;
	QPainter theP(&theImage);
	theP.setPen(QPen(QColor::fromRgb(replaceColor), 1));
	theP.setBrush(Qt::NoBrush);

	theStack.push(P);
	while (!theStack.isEmpty()) {
		aP = theStack.pop();
        QPoint W = aP;
        QPoint E = aP;
        if (testColor(theImage, aP + QPoint(0, 1), targetColor))
            theStack.push(aP + QPoint(0, 1));
        if (testColor(theImage, aP + QPoint(0, -1), targetColor))
            theStack.push(aP + QPoint(0, -1));
        while (testColor(theImage, W + QPoint(-1, 0),targetColor) && W.x() > 0) {
            W += QPoint(-1, 0);
            if (testColor(theImage, W + QPoint(0, 1), targetColor))
                theStack.push(W + QPoint(0, 1));
            if (testColor(theImage, W + QPoint(0, -1), targetColor))
                theStack.push(W + QPoint(0, -1));
        }
        while (testColor(theImage, E + QPoint(1, 0), targetColor) && E.x() < theImage.width()-1) {
            E += QPoint(1, 0);
            if (testColor(theImage, E + QPoint(0, 1), targetColor))
                theStack.push(E + QPoint(0, 1));
            if (testColor(theImage, E + QPoint(0, -1), targetColor))
                theStack.push(E + QPoint(0, -1));
        }
        theP.drawLine(W, E);
	}
}

void MapView::drawBackground(QPainter & theP, Projection& aProj)
{
	QColor theFillColor;
	
	double PixelPerM = aProj.pixelPerM();
	double WW = PixelPerM*30.0 + 2.0;

	QPen thePen(M_PREFS->getWaterColor(), WW);
	thePen.setCapStyle(Qt::RoundCap);
	thePen.setJoinStyle(Qt::RoundJoin);
    theP.setPen(thePen);
	theP.setBrush(Qt::NoBrush);

	if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
		theFillColor = M_PREFS->getBgColor();
	else
		theFillColor = M_STYLE->getGlobalPainter().getBackgroundColor();
	theP.fillRect(theP.clipRegion().boundingRect(), theFillColor);

	if (theCoastlines.isEmpty()) {
		if (M_PREFS->getUseShapefileForBackground() && theDocument->getImageLayer()->isVisible() && !LAYERMANAGER_OK) {
			theP.fillRect(theP.clipRegion().boundingRect(), M_PREFS->getWaterColor());
		}
		return;
	}

	QList<QPainterPath*> theCoasts;
	for (int i=0; i<theCoastlines.size(); i++) {
		if (theCoastlines[i]->getPath().elementCount() < 2) continue;

		QPainterPath* aPath = new QPainterPath;
		for (int j=1; j < theCoastlines[i]->getPath().elementCount(); j++) {

			QLineF l(QPointF(theCoastlines[i]->getPath().elementAt(j)), QPointF(theCoastlines[i]->getPath().elementAt(j-1)));
			QLineF l1 = l.normalVector().unitVector();
            l1.setLength(WW / 2.0);
			if (j == 1) {
				QLineF l3(l1);
				l3.translate(l.p2() - l.p1());
				aPath->moveTo(l3.p2());
			} else
				if (j < theCoastlines[i]->getPath().elementCount() - 1) {
					QLineF l4(QPointF(theCoastlines[i]->getPath().elementAt(j)), QPointF(theCoastlines[i]->getPath().elementAt(j+1)));
					double theAngle = (l4.angle() - l.angle()) / 2.0;
					if (theAngle < 0.0) theAngle += 180.0;
					l1.setAngle(l.angle() + theAngle);
				}
            //theP.drawEllipse(l2.p2(), 5, 5);
			aPath->lineTo(l1.p2());

		}
		theCoasts.append(aPath);
	}

	for (int i=0; i < theCoasts.size(); i++) {
		theP.drawPath(*theCoasts[i]);
		delete theCoasts[i];
	}
}

//void MapView::drawBackground(QPainter & P, Projection& aProj)
//{
//	QImage theImage(size(), QImage::Format_RGB32);
//	QList <QPoint> theFloodStarts;
//	QColor theFillColor;
//	
//	QPainter theP;
//	theP.begin(&theImage);
//    theP.setRenderHint(QPainter::Antialiasing);
//    theP.setPen(QPen(M_PREFS->getWaterColor(), 1));
//	theP.setBrush(Qt::NoBrush);
//
//	if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
//		theFillColor = M_PREFS->getBgColor();
//	else
//		theFillColor = M_STYLE->getGlobalPainter().getBackgroundColor();
//	theP.fillRect(rect(), theFillColor);
//
//	if (theCoastlines.isEmpty()) {
//		if (M_PREFS->getUseShapefileForBackground() && theDocument->getImageLayer()->isVisible()) {
//			theP.fillRect(rect(), M_PREFS->getWaterColor());
//		}
//		P.drawImage(QPoint(0, 0), theImage);
//		return;
//	}
//
//	for (int i=0; i<theCoastlines.size(); i++) {
//		if (theCoastlines[i]->getPath().elementCount() < 2) continue;
//
//		for (int j=1; j < theCoastlines[i]->getPath().elementCount(); j++) {
//			QLineF l(QPointF(theCoastlines[i]->getPath().elementAt(j)), QPointF(theCoastlines[i]->getPath().elementAt(j-1)));
//			QLineF l1 = l.normalVector().unitVector();
//			QLineF l2(l1);
//			l2.translate(-l1.p1());
//            //l2.setP2(l2.p2() * 2.0);
//			QPointF p2 = QPointF(l.p1().x() + ((l.p2().x() - l.p1().x()) / 2.0), l.p1().y() + ((l.p2().y() - l.p1().y()) / 2.0));
//			//l2.translate(l1.p1());
//			l2.translate(p2);
//            theFloodStarts.append(l2.p2().toPoint());
//            //theP.drawEllipse(l2.p2(), 5, 5);
//		}
//		theP.drawPath(theCoastlines[i]->getPath());
//	}
//	theP.end();
//
//	for (int i=0; i < theFloodStarts.size(); i++) {
//		floodFill(theImage, theFloodStarts[i], theFillColor.rgb(), M_PREFS->getWaterColor().rgb() );
//	}
//	P.drawImage(QPoint(0, 0), theImage);
//}
//
//void MapView::drawBackground(QPainter & P, Projection& aProj)
//{
//	/*
//	p->thePath = QPainterPath();
//	if (!p->Nodes.size())
//		return;
//
//	bool lastPointVisible = true;
//	QPoint lastPoint = theProjection.project(p->Nodes[0]);
//	QPoint aP = lastPoint;
//
//	double PixelPerM = theProjection.pixelPerM();
//	double WW = PixelPerM*widthOf()*10+10;
//	QRect clipRect = paintRegion.boundingRect().adjusted(int(-WW-20), int(-WW-20), int(WW+20), int(WW+20));
//
//
//	if (M_PREFS->getDrawingHack()) {
//		if (!clipRect.contains(aP)) {
//			aP.setX(qMax(clipRect.left(), aP.x()));
//			aP.setX(qMin(clipRect.right(), aP.x()));
//			aP.setY(qMax(clipRect.top(), aP.y()));
//			aP.setY(qMin(clipRect.bottom(), aP.y()));
//			lastPointVisible = false;
//		}
//	}
//	p->thePath.moveTo(aP);
//	QPoint firstPoint = aP;
//	if (smoothed().size())
//	{
//		for (int i=3; i<smoothed().size(); i+=3)
//			p->thePath.cubicTo(
//				theProjection.project(smoothed()[i-2]),
//				theProjection.project(smoothed()[i-1]),
//				theProjection.project(smoothed()[i]));
//	}
//	else
//		for (int j=1; j<size(); ++j) {
//			aP = theProjection.project(p->Nodes[j]);
//			if (M_PREFS->getDrawingHack()) {
//				QLine l(lastPoint, aP);
//				if (!clipRect.contains(aP)) {
//					if (!lastPointVisible) {
//						QPoint a, b;
//						if (QRectInterstects(clipRect, l, a, b)) {
//							p->thePath.lineTo(a);
//							lastPoint = aP;
//							aP = b;
//						} else {
//							lastPoint = aP;
//							aP.setX(qMax(clipRect.left(), aP.x()));
//							aP.setX(qMin(clipRect.right(), aP.x()));
//							aP.setY(qMax(clipRect.top(), aP.y()));
//							aP.setY(qMin(clipRect.bottom(), aP.y()));
//						}
//					} else {
//						QPoint a, b;
//						QRectInterstects(clipRect, l, a, b);
//						lastPoint = aP;
//						aP = a;
//					}
//					lastPointVisible = false;
//				} else {
//					if (!lastPointVisible) {
//						QPoint a, b;
//						QRectInterstects(clipRect, l, a, b);
//						p->thePath.lineTo(a);
//					}
//					lastPoint = aP;
//					lastPointVisible = true;
//				}
//			}
//			p->thePath.lineTo(aP);
//		}
//		if (area() > 0.0 && !lastPointVisible)
//			p->thePath.lineTo(firstPoint);
//*/
//		
//	if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
//		P.fillRect(rect(), QBrush(M_PREFS->getBgColor()));
//	else
//		P.fillRect(rect(), QBrush(M_STYLE->getGlobalPainter().getBackgroundColor()));
//
//	if (theCoastlines.isEmpty()) {
//		if (M_PREFS->getUseShapefileForBackground() && theDocument->getImageLayer()->isVisible()) {
//			P.fillRect(rect(), QBrush(M_PREFS->getWaterColor()));
//		}
//		return;
//	}
//
//	QPainterPath theCoast;
//	QRect clipRect = rect().adjusted(int(-40), int(-40), int(40), int(40));
//	QPointF firstPoint, lastPoint;
//	QList<Road*> aCoastlines;
//	QList < QPair <TrackPoint*, Road*> > aStartPoints;
//	QList < QPair <TrackPoint*, Road*> > aEndPoints;
//
//	for (int i=0; i < theCoastlines.size(); i++) {
//		if (theCoastlines[i]->isClosed()) {
//			theCoast.addPath(theCoastlines[i]->getPath());
//			continue;
//		}
//		aStartPoints.append(qMakePair(dynamic_cast<TrackPoint*>(theCoastlines[i]->get(0)), theCoastlines[i]));
//		aEndPoints.append(qMakePair(dynamic_cast<TrackPoint*>(theCoastlines[i]->get(theCoastlines[i]->size()-1)), theCoastlines[i]));
//	}
//
//	int curIndex, tmpIndex;
//	for (int i=0; i < aStartPoints.size(); i++) {
//		if ((curIndex = aCoastlines.indexOf(aStartPoints[i].second)) == -1) {
//			aCoastlines.append(aStartPoints[i].second);
//			curIndex = aStartPoints.size()-1;
//		}
//		for (int j=0; j < aEndPoints.size(); j++) {
//			if (aEndPoints[j].first == aStartPoints[i].first)
//				if ((tmpIndex = aCoastlines.indexOf(aEndPoints[j].second)) == -1)
//					aCoastlines.insert(curIndex, aEndPoints[j].second);
//				else
//					aCoastlines.move(tmpIndex, curIndex);
//		}
//	}
//
//	QList<QPainterPath*> theIncompleteCoasts;
//	QPainterPath* curPath = NULL;
//	for (int i=0; i<aCoastlines.size(); i++) {
//		if (aCoastlines[i]->getPath().elementAt(0) != lastPoint) {
//			if (curPath)
//				theCoast.addPath(*curPath);
//			curPath = new QPainterPath;
//			theIncompleteCoasts.append(curPath);
//			lastPoint = QPointF(aCoastlines[i]->getPath().elementAt(0));
//		}
//		curPath->moveTo(lastPoint);
//		for (int j=1; j < aCoastlines[i]->getPath().elementCount(); j++) {
//			lastPoint = QPointF(aCoastlines[i]->getPath().elementAt(j));
//			curPath->lineTo(lastPoint);
//		}
//	}
//	if (curPath)
//		theCoast.addPath(*curPath);
//
//	P.setBrush(M_PREFS->getWaterColor());
//	P.setPen(QPen(M_PREFS->getWaterColor(), 1));
//	P.drawPath(theCoast);
//}

void MapView::drawFeatures(QPainter & P, Projection& aProj)
{
	M_STYLE->initialize(P, aProj);

	for (int i = 0; i < M_STYLE->size(); ++i)
	{
		PaintStyleLayer *Current = M_STYLE->get(i);

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
			/* //It is non-sense to paint a relation, isn't it? (The drawing of the boundaries is in Relation::draw)
			else if (Relation * RR = dynamic_cast < Relation * >(theFeatures[i]))
				Current->draw(RR); */
		}
		P.restore();
	}
	
	for (int i=0; i<theFeatures.size(); i++)
	{
		P.setOpacity(theFeatures[i]->layer()->getAlpha());
		theFeatures[i]->draw(P, aProj);
	}
}

void MapView::drawDownloadAreas(QPainter & P)
{
	if (MerkaartorPreferences::instance()->getDownloadedVisible() == false)
		return;

	P.save();
	QRegion r(0, 0, width(), height());


	//QBrush b(Qt::red, Qt::DiagCrossPattern);
	QBrush b(Qt::red, Qt::Dense7Pattern);

	QList<CoordBox> db = theDocument->getDownloadBoxes();
	QList<CoordBox>::const_iterator bb;
	for (bb = db.constBegin(); bb != db.constEnd(); ++bb) {
		if (projection().viewport().disjunctFrom(*bb)) continue;
		QPolygonF poly;
		poly << projection().project((*bb).topLeft());
		poly << projection().project((*bb).bottomLeft());
		poly << projection().project((*bb).bottomRight());
		poly << projection().project((*bb).topRight());
		poly << projection().project((*bb).topLeft());

		r -= QRegion(poly.toPolygon());
	}

	P.setClipRegion(r);
	P.setClipping(true);
	P.fillRect(rect(), b);

	P.restore();
}

void MapView::updateStaticBackground()
{
	QPixmap savPix;
	QPoint pan;
	QRect bbRect;

	QRegion panRg = QRegion(rect()) - invalidRegion;
	if (!panRg.isEmpty()) {
		//pan = (panRg.boundingRect().center() - rect().center()) * 2.0;
		bbRect = panRg.boundingRect();
		pan = bbRect.topLeft() - rect().topLeft() + bbRect.bottomRight() - rect().bottomRight();
		savPix = StaticBackground->copy();
	}

	if (!StaticBackground || (StaticBackground->size() != size()))
	{
		delete StaticBackground;
		StaticBackground = new QPixmap(size());
	}

	//StaticBackground->fill(Qt::transparent);

	QPainter painter(StaticBackground);

	painter.setClipping(true);
	if (!panRg.isEmpty()) {
		painter.setClipRegion(panRg);
		painter.drawPixmap(pan, savPix);
	}

	painter.setClipRegion(invalidRegion);
	painter.setRenderHint(QPainter::Antialiasing);

	drawBackground(painter, theProjection);
}

void MapView::updateStaticBuffer()
{
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
		drawFeatures(painter, theProjection);
	}

	invalidRegion = QRegion();
	StaticBufferUpToDate = true;
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
	double finalZoom = 1.;
	int Steps = ev->delta() / 120;
	if (Steps > 0) {
		for (int i = 0; i < Steps; ++i) {
			finalZoom *= M_PREFS->getZoomInPerc()/100.0;
		}
	} else if (Steps < 0) {
		for (int i = 0; i < -Steps; ++i) {
			finalZoom *= M_PREFS->getZoomOutPerc()/100.0;
		}
	}
	projection().zoom(finalZoom, ev->pos(), rect());
	for (int i=0; i<theDocument->layerSize(); ++i)
	{
		ImageMapLayer* IL = dynamic_cast<ImageMapLayer*>(theDocument->getLayer(i));
		if ( IL )
		{
			IL->zoom(finalZoom, ev->pos(), rect());
		}
	}
	invalidate(true, true);
}

void MapView::launch(Interaction* anInteraction)
{
	EditInteraction* EI = dynamic_cast<EditInteraction*>(theInteraction);
	if (EI)
		theSnapList = EI->snapList();
	if (!theSnapList.size())
		theSnapList = Main->properties()->selection();
	if (theInteraction)
		delete theInteraction;
	theInteraction = anInteraction;
	EI = dynamic_cast<EditInteraction*>(theInteraction);
	if (theInteraction) {
#ifndef Q_OS_SYMBIAN
		setCursor(theInteraction->cursor());
#endif
		emit interactionChanged(anInteraction);
		if (EI)
			EI->setSnap(theSnapList);
	}
	else {
#ifndef Q_OS_SYMBIAN
		setCursor(QCursor(Qt::ArrowCursor));
#endif
		launch(new EditInteraction(this));
		//Q_ASSERT(theInteraction);
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


		QMenu nodeMenu(tr("Node"));
		for(int i=0; i<Main->menu_Node->actions().size(); ++i) {
			if (Main->menu_Node->actions()[i]->isEnabled())
				nodeMenu.addAction(Main->menu_Node->actions()[i]);
		}
		if (nodeMenu.actions().size())
			menu.addMenu(&nodeMenu);

		QMenu roadMenu(tr("Road"));
		for(int i=0; i<Main->menuRoad->actions().size(); ++i) {
			if (Main->menuRoad->actions()[i]->isEnabled())
				roadMenu.addAction(Main->menuRoad->actions()[i]);
		}
		if (roadMenu.actions().size())
			menu.addMenu(&roadMenu);

		QMenu relationMenu(tr("Relation"));
		for(int i=0; i<Main->menuRelation->actions().size(); ++i) {
			if (Main->menuRelation->actions()[i]->isEnabled())
				relationMenu.addAction(Main->menuRelation->actions()[i]);
		}
		if (relationMenu.actions().size())
			menu.addMenu(&relationMenu);

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
	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible()) {
		Main->pbImages->setValue(Main->pbImages->value()+1);
		layermanager->forceRedraw();
	}
	StaticMapUpToDate = false;
	update();
}

void MapView::loadingFinished()
{
	layermanager->removeZoomImage();
	numImages = 0;
	Main->pbImages->reset();

	//invalidate(false, true);
}

void MapView::resizeEvent(QResizeEvent * ev)
{
	StaticBufferUpToDate = false;
	if (LAYERMANAGER_OK && layermanager->getLayer()->isVisible())
		layermanager->setSize(size());
	projection().resize(ev->oldSize(), ev->size());

	QWidget::resizeEvent(ev);

	invalidate(true, true);
}

#ifdef GEOIMAGE
void MapView::dragEnterEvent(QDragEnterEvent *event)
{
	if (event->mimeData()->hasUrls() && event->mimeData()->urls().first().toLocalFile().endsWith(".jpg", Qt::CaseInsensitive))
		event->accept();
	else
		event->ignore();
}

void MapView::dragMoveEvent(QDragMoveEvent *event)
{
	{
		QMouseEvent mE(QEvent::MouseMove, event->pos(), Qt::LeftButton, Qt::LeftButton, qApp->keyboardModifiers());
		mouseMoveEvent(&mE);
	}
	TrackPoint *tP;
	for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it) {
		if ((tP = qobject_cast<TrackPoint*>(it.get())) && tP->pixelDistance(event->pos(), 5.01, projection()) < 5.01) {
			dropTarget = tP;
			QRect acceptedRect(tP->projection().toPoint() - QPoint(3.5, 3.5), tP->projection().toPoint() + QPoint(3.5, 3.5));
			event->acceptProposedAction();
			event->accept(acceptedRect);
			return;
		}
	}
	event->ignore();
}

void MapView::dropEvent(QDropEvent *event)
{
	// first save the image url because the even->mimeData() releases its data very soon
	// this is probably because the drop action ends with calling of this event
	// so the program that started the drag-action thinks the data isn't needed anymore
	QUrl imageUrl = event->mimeData()->urls().first();

	QMenu menu(this);
	QAction *add = menu.addAction(tr("Add trackpoint position to image"));
	menu.addSeparator();
	menu.addAction(tr("Cancel"));
	if (menu.exec(QCursor::pos()) == add)
		Main->geoImage()->addGeoDataToImage(dropTarget->position(), imageUrl.toLocalFile());
}
#endif // GEOIMAGE

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

bool MapView::event(QEvent *event)
{
	if (event->type() == QEvent::ToolTip) {
		QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
		 //Coord p = theProjection.inverse(helpEvent->pos());
		if (M_PREFS->getMapTooltip()) {
			if (!toolTip().isEmpty())
				QToolTip::showText(helpEvent->globalPos(), toolTip());
			else
				QToolTip::hideText();
		 }
		return true;
	} else
    if ( event->type() == QEvent::KeyPress ) {
		QKeyEvent *ke = static_cast< QKeyEvent* >( event );
		if ( ke->key() == Qt::Key_Tab ) {
			setFocus();
			ke->accept();

			if (!isSelectionLocked())
				lockSelection();
			else {
				FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
				if (intr)
					intr->nextSnap();
			}

			return true;
		} else
		if ( ke->key() == Qt::Key_Backtab ) {
			setFocus();
			ke->accept();

			if (!isSelectionLocked())
				lockSelection();
			else {
				FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
				if (intr)
					intr->nextSnap();
			}

			return true;
		} 
	} else
    if ( event->type() == QEvent::Leave ) {
		Main->info()->unsetHoverHtml();
		FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
		if (intr)
			intr->clearLastSnap();
		update();
	} 

	return QWidget::event(event);
 }

bool MapView::isSelectionLocked()
{
	return SelectionLocked;
}

void MapView::lockSelection()
{
	if (!SelectionLocked && Main->properties()->selection().size()) {
		lockIcon = new QLabel(this);
		lockIcon->setPixmap(QPixmap(":/Icons/emblem-readonly.png"));
		Main->statusBar()->addWidget(lockIcon);
		SelectionLocked = true;
	}
}

void MapView::unlockSelection()
{
	if (SelectionLocked) {
		Main->statusBar()->removeWidget(lockIcon);
		SAFE_DELETE(lockIcon);
		SelectionLocked = false;
	}
}
