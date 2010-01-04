#include <errno.h>

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
#include "Maps/LayerIterator.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#ifdef USE_WEBKIT
	#include "QMapControl/browserimagemanager.h"
#endif
#include "Preferences/MerkaartorPreferences.h"
#include "Utils/SvgCache.h"
#include "Utils/SortAccordingToRenderingPriority.h"


#include <QTime>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QToolTip>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0

class MapViewPrivate
{
public:
	//double ScaleLat, ScaleLon;
	//double DeltaLat, DeltaLon;
	QTransform theTransform;
	double PixelPerM;
	CoordBox Viewport;
	QList<CoordBox> invalidRects;
	QPoint theRasterPanDelta, theVectorPanDelta;
	QSet<MapFeature*> theFeatures;
	QSet<Road*> theCoastlines;
	QList<TrackPoint*> theVirtualNodes;

	MapViewPrivate()
	  : PixelPerM(0.0), Viewport(WORLD_COORDBOX)
	{}
};

/****************/

MapView::MapView(MainWindow* aMain) :
	QWidget(aMain), Main(aMain), theDocument(0), theInteraction(0), StaticBackground(0), StaticBuffer(0), StaticMap(0),
		StaticBufferUpToDate(false), StaticMapUpToDate(false), SelectionLocked(false),lockIcon(0), numImages(0),
		p(new MapViewPrivate)
{
	setMouseTracking(true);
	setAttribute(Qt::WA_OpaquePaintEvent);
	setContextMenuPolicy(Qt::CustomContextMenu);
	setFocusPolicy(Qt::ClickFocus);
#ifdef GEOIMAGE
	setAcceptDrops(true);
#endif

	MoveRight = new QShortcut(QKeySequence(Qt::Key_Right), this);
	connect(MoveRight, SIGNAL(activated()), this, SLOT(on_MoveRight_activated()));
	MoveLeft = new QShortcut(QKeySequence(Qt::Key_Left), this);
	connect(MoveLeft, SIGNAL(activated()), this, SLOT(on_MoveLeft_activated()));
	MoveUp = new QShortcut(QKeySequence(Qt::Key_Up), this);
	connect(MoveUp, SIGNAL(activated()), this, SLOT(on_MoveUp_activated()));
	MoveDown = new QShortcut(QKeySequence(Qt::Key_Down), this);
	connect(MoveDown, SIGNAL(activated()), this, SLOT(on_MoveDown_activated()));
}

MapView::~MapView()
{
	if(theInteraction)
		delete theInteraction;
	delete StaticBackground;
	delete StaticBuffer;
	delete StaticMap;
	delete p;
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
	connect(aDoc, SIGNAL(imageRequested(ImageMapLayer*)),
		this, SLOT(on_imageRequested(ImageMapLayer*)), Qt::QueuedConnection);
	connect(aDoc, SIGNAL(imageReceived(ImageMapLayer*)),
		this, SLOT(on_imageReceived(ImageMapLayer*)), Qt::QueuedConnection);
	connect(aDoc, SIGNAL(loadingFinished(ImageMapLayer*)),
		this, SLOT(on_loadingFinished(ImageMapLayer*)), Qt::QueuedConnection);

	setViewport(viewport(), rect());
}

MapDocument *MapView::document()
{
	return theDocument;
}

void MapView::invalidate(bool updateStaticBuffer, bool updateMap)
{
	if (updateStaticBuffer) {
		p->invalidRects.clear();
		p->invalidRects.push_back(p->Viewport);
		StaticBufferUpToDate = false;
	}
	if (theDocument && updateMap) {
		for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
			ImgIt.get()->forceRedraw(*this, rect());
		StaticMapUpToDate = false;
	}
	update();
}

void MapView::panScreen(QPoint delta)
{
	p->theRasterPanDelta += delta;
	p->theVectorPanDelta += delta;

	CoordBox r1, r2;

	Coord cDelta = theProjection.inverse(p->theTransform.inverted().map(QPointF(delta)))  - theProjection.inverse(p->theTransform.inverted().map(QPointF(0., 0.)));

	if (delta.x()) {
		if (delta.x() < 0)
			r1 = CoordBox(p->Viewport.bottomRight(), Coord(p->Viewport.topRight().lat(), p->Viewport.topRight().lon() - cDelta.lon())); // OK
		else
			r1 = CoordBox(Coord(p->Viewport.bottomLeft().lat(), p->Viewport.bottomLeft().lon() - cDelta.lon()), p->Viewport.topLeft()); // OK
		p->invalidRects.push_back(r1);
	}
	if (delta.y()) {
		if (delta.y() < 0)
			r2 = CoordBox(Coord(p->Viewport.bottomLeft().lat() - cDelta.lat(), p->Viewport.bottomLeft().lon()), p->Viewport.bottomRight()); // OK
		else
			r2 = CoordBox(p->Viewport.topLeft(), Coord(p->Viewport.topRight().lat() - cDelta.lat(), p->Viewport.bottomRight().lon())); //NOK
		p->invalidRects.push_back(r2);
	}


	//qDebug() << "Inv rects size: " << p->invalidRects.size();
	//qDebug() << "delta: " << delta;
	//qDebug() << "r1 : " << p->theTransform.map(theProjection.project(r1.topLeft())) << ", " << p->theTransform.map(theProjection.project(r1.bottomRight()));
	//qDebug() << "r2 : " << p->theTransform.map(theProjection.project(r2.topLeft())) << ", " << p->theTransform.map(theProjection.project(r2.bottomRight()));

	p->theTransform.translate(qreal(delta.x())/p->theTransform.m11(), qreal(delta.y())/p->theTransform.m22());
	viewportRecalc(rect());

	StaticBufferUpToDate = false;
	update();
}

void MapView::paintEvent(QPaintEvent * anEvent)
{
#ifndef NDEBUG
	QTime Start(QTime::currentTime());
#endif

	p->theFeatures.clear();
	p->theCoastlines.clear();

	QPainter P;
	P.begin(this);

	if (!p->invalidRects.isEmpty())
		buildFeatureSet();

	if (!StaticMapUpToDate)
		updateLayersImage();

	updateStaticBackground();

	if (!StaticBufferUpToDate) {
		updateStaticBuffer();
	}

	P.drawPixmap(p->theVectorPanDelta, *StaticBackground);
	P.drawPixmap(p->theRasterPanDelta, *StaticMap);
	P.drawPixmap(p->theVectorPanDelta, *StaticBuffer);

	drawDownloadAreas(P);
	drawScale(P);

	if (theInteraction) {
		P.setRenderHint(QPainter::Antialiasing);
		theInteraction->paintEvent(anEvent, P);
	}

	drawGPS(P);

	P.end();

	Main->ViewportStatusLabel->setText(QString("%1,%2,%3,%4")
		.arg(QString::number(intToAng(viewport().bottomLeft().lon()),'f',4))
		.arg(QString::number(intToAng(viewport().bottomLeft().lat()),'f',4))
		.arg(QString::number(intToAng(viewport().topRight().lon()),'f',4))
		.arg(QString::number(intToAng(viewport().topRight().lat()),'f',4))
		);

#ifndef NDEBUG
	QPointF pbl = theProjection.project(viewport().bottomLeft());
	QPointF ptr = theProjection.project(viewport().topRight());
	qDebug() << "VP: " << QString("%1 (%2,%3,%4,%5)")
	   .arg(Main->ViewportStatusLabel->text())
		.arg(QString::number(pbl.x(),'f',4))
		.arg(QString::number(pbl.y(),'f',4))
		.arg(QString::number(ptr.x(),'f',4))
		.arg(QString::number(ptr.y(),'f',4));

	QTime Stop(QTime::currentTime());
	//Main->PaintTimeLabel->setText(tr("%1ms").arg(Start.msecsTo(Stop)));
	Main->PaintTimeLabel->setText(tr("%1ms;ppm:%2").arg(Start.msecsTo(Stop)).arg(p->PixelPerM));
#endif
}

void MapView::drawScale(QPainter & P)
{
	if (!M_PREFS->getScaleVisible())
		return;

	errno = 0;
	double Log = log10(200./p->PixelPerM);
	if (errno != 0)
		return;

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
	QPointF P2(20+Length*p->PixelPerM,height()-20);
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
			QPointF g = p->theTransform.map(projection().project(vp));
			QPixmap pm = getPixmapFromFile(":/Gps/Gps_Marker.svg", 32);
			P.drawPixmap(g - QPoint(16, 16), pm);
		}
	}
}

void MapView::sortRenderingPriorityInLayers()
{
	for (int i = 0; i < theDocument->layerSize(); ++i) {
		theDocument->getLayer(i)->
			sortRenderingPriority();
	}
}

void MapView::sortRenderingPriority()
{
	// TODO Avoid copy
	QList<MapFeature*> aList = p->theFeatures.toList();
	qSort(aList.begin(),aList.end(),SortAccordingToRenderingPriority());
	p->theFeatures.fromList(aList);
}

void MapView::updateLayersImage()
{
	if (!StaticMap || (StaticMap->size() != size()))
	{
		delete StaticMap;
		StaticMap = new QPixmap(size());
	}
	StaticMap->fill(Qt::transparent);

	for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
		if (ImgIt.get()->isVisible())
			ImgIt.get()->drawImage(*StaticMap, p->theRasterPanDelta);

	p->theRasterPanDelta = QPoint(0, 0);
	StaticMapUpToDate = true;
}

void MapView::buildFeatureSet()
{
	if (!theDocument)
		return;

	for (int i=0; i<theDocument->layerSize(); ++i) {
		theDocument->getLayer(i)->invalidate(theDocument, p->Viewport);
		if (Main)
			Main->properties()->adjustSelection();
	}

	CoordBox coordRegion;
	QRectF clipRect = p->theTransform.inverted().mapRect(QRectF(rect().adjusted(-1000, -1000, 1000, 1000)));
	//QRectF clipRect = p->theTransform.inverted().mapRect(QRectF(rect().adjusted(10, 10, -10, -10)));

#if 1

	//qDebug() << "Inv rects size: " << p->invalidRects.size();
	for (int i=0; i < p->invalidRects.size(); ++i) {
		Coord bl = p->invalidRects[i].bottomLeft();
		Coord tr = p->invalidRects[i].topRight();
		//qDebug() << "rect : " << p->theTransform.map(theProjection.project(bl)) << ", " << p->theTransform.map(theProjection.project(tr));
		for (int j=0; j<theDocument->layerSize(); ++j) {
			if (!theDocument->getLayer(j)->size() || !theDocument->getLayer(j)->isVisible())
				continue;

			std::deque < MapFeaturePtr > ret = theDocument->getLayer(j)->indexFind(CoordBox(bl, tr));
			for (std::deque < MapFeaturePtr >::const_iterator it = ret.begin(); it != ret.end(); ++it) {
				if (Road * R = CAST_WAY(*it)) {
					R->buildPath(theProjection, p->theTransform, clipRect);
					p->theFeatures.insert(R);
					if (R->isCoastline())
						p->theCoastlines.insert(R);
				} else
				if (Relation * RR = CAST_RELATION(*it)) {
					RR->buildPath(theProjection, p->theTransform, clipRect);
					p->theFeatures.insert(RR);
				} else
				if (TrackPoint * pt = CAST_NODE(*it)) {
					if (theDocument->getLayer(j)->arePointsDrawable())
						p->theFeatures.insert(pt);
				} else
					p->theFeatures.insert(*it);
			}
		}

		coordRegion.merge(CoordBox(bl, tr));
	}
	sortRenderingPriority();

#else

	for (int i=0; i < invalidRegion.rects().size(); ++i) {
		Coord bl = aProj.inverse(invalidRegion.rects()[i].bottomLeft());
		Coord tr = aProj.inverse(invalidRegion.rects()[i].topRight());
		coordRegion += CoordBox(bl, tr);
	}

	sortRenderingPriorityInLayers();
	for (VisibleFeatureIterator vit(theDocument); !vit.isEnd(); ++vit) {
		bool OK = false;
		for (int k=0; k<coordRegion.size() && !OK; ++k)
			if (coordRegion[k].intersects(vit.get()->boundingBox()))
				OK = true;
		if (OK) {
			if (Road * R = dynamic_cast < Road * >(vit.get())) {
				R->buildPath(aProj, clipRect);
				p->theFeatures.push_back(R);
				if (R->isCoastline())
					p->theCoastlines.push_back(R);
			} else
			if (Relation * RR = dynamic_cast < Relation * >(vit.get())) {
				RR->buildPath(aProj, clipRect);
				p->theFeatures.push_back(RR);
			} else {
				p->theFeatures.push_back(vit.get());
			}
		}
	}
#endif
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

void MapView::drawBackground(QPainter & theP, Projection& /*aProj*/)
{
	QColor theFillColor;

	double WW = p->PixelPerM*30.0 + 2.0;

	QPen thePen(M_PREFS->getWaterColor(), WW);
	thePen.setCapStyle(Qt::RoundCap);
	thePen.setJoinStyle(Qt::RoundJoin);
	theP.setPen(thePen);
	theP.setBrush(Qt::NoBrush);

	if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
		theFillColor = M_PREFS->getBgColor();
	else
		theFillColor = M_STYLE->getGlobalPainter().getBackgroundColor();
	theP.fillRect(rect(), theFillColor);

	if (p->theCoastlines.isEmpty()) {
//		if (M_PREFS->getUseShapefileForBackground() && theDocument->getImageLayer()->isVisible() && !LAYERMANAGER_OK) {
		if (M_PREFS->getUseShapefileForBackground())
			theP.fillRect(rect(), M_PREFS->getWaterColor());
		return;
	}

	QList<QPainterPath*> theCoasts;
	QSet<Road*>::const_iterator it = p->theCoastlines.constBegin();
	for (;it != p->theCoastlines.constEnd(); ++it) {
		if ((*it)->getPath().elementCount() < 2) continue;

		QPainterPath* aPath = new QPainterPath;
		for (int j=1; j < (*it)->getPath().elementCount(); j++) {

			QLineF l(QPointF((*it)->getPath().elementAt(j)), QPointF((*it)->getPath().elementAt(j-1)));
			QLineF l1 = l.normalVector().unitVector();
			l1.setLength(WW / 2.0);
			if (j == 1) {
				QLineF l3(l1);
				l3.translate(l.p2() - l.p1());
				aPath->moveTo(l3.p2());
			} else
				if (j < (*it)->getPath().elementCount() - 1) {
					QLineF l4(QPointF((*it)->getPath().elementAt(j)), QPointF((*it)->getPath().elementAt(j+1)));
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
		theP.drawPath(p->theTransform.map(*theCoasts[i]));
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
//	if (p->theCoastlines.isEmpty()) {
//		if (M_PREFS->getUseShapefileForBackground() && theDocument->getImageLayer()->isVisible()) {
//			theP.fillRect(rect(), M_PREFS->getWaterColor());
//		}
//		P.drawImage(QPoint(0, 0), theImage);
//		return;
//	}
//
//	for (int i=0; i<p->theCoastlines.size(); i++) {
//		if (p->theCoastlines[i]->getPath().elementCount() < 2) continue;
//
//		for (int j=1; j < p->theCoastlines[i]->getPath().elementCount(); j++) {
//			QLineF l(QPointF(p->theCoastlines[i]->getPath().elementAt(j)), QPointF(p->theCoastlines[i]->getPath().elementAt(j-1)));
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
//		theP.drawPath(p->theCoastlines[i]->getPath());
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
//	if (p->theCoastlines.isEmpty()) {
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
//	for (int i=0; i < p->theCoastlines.size(); i++) {
//		if (p->theCoastlines[i]->isClosed()) {
//			theCoast.addPath(p->theCoastlines[i]->getPath());
//			continue;
//		}
//		aStartPoints.append(qMakePair(dynamic_cast<TrackPoint*>(p->theCoastlines[i]->get(0)), p->theCoastlines[i]));
//		aEndPoints.append(qMakePair(dynamic_cast<TrackPoint*>(p->theCoastlines[i]->get(p->theCoastlines[i]->size()-1)), p->theCoastlines[i]));
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

void MapView::drawFeatures(QPainter & P, Projection& /*aProj*/)
{
	M_STYLE->initialize(P, *this);

	QSet<MapFeature*>::const_iterator it;
	for (int i = 0; i < M_STYLE->size(); ++i)
	{
		PaintStyleLayer *Current = M_STYLE->get(i);

		P.save();
		P.setRenderHint(QPainter::Antialiasing);

		for (it = p->theFeatures.constBegin() ;it != p->theFeatures.constEnd(); ++it)
		{
			P.setOpacity((*it)->layer()->getAlpha());
			if (Road * R = dynamic_cast < Road * >((*it)))
				Current->draw(R);
			else if (TrackPoint * Pt = dynamic_cast < TrackPoint * >((*it)))
				Current->draw(Pt);
			else if (Relation * RR = dynamic_cast < Relation * >((*it)))
				Current->draw(RR);
		}
		P.restore();
	}

	for (it = p->theFeatures.constBegin(); it != p->theFeatures.constEnd(); ++it)
	{
		P.setOpacity((*it)->layer()->getAlpha());
		(*it)->draw(P, this);
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
		if (viewport().disjunctFrom(*bb)) continue;
		QPolygonF poly;
		poly << projection().project((*bb).topLeft());
		poly << projection().project((*bb).bottomLeft());
		poly << projection().project((*bb).bottomRight());
		poly << projection().project((*bb).topRight());
		poly << projection().project((*bb).topLeft());

		r -= QRegion(p->theTransform.map(poly.toPolygon()));
	}

	P.setClipRegion(r);
	P.setClipping(true);
	P.fillRect(rect(), b);

	P.restore();
}

void MapView::updateStaticBackground()
{
	if (!StaticBackground || (StaticBackground->size() != size()))
	{
		delete StaticBackground;
		StaticBackground = new QPixmap(size());
	}

	QPainter P;

	if (!p->theVectorPanDelta.isNull()) {
		QPixmap savPix;
		savPix = StaticBackground->copy();
		StaticBackground->fill(Qt::transparent);
		P.begin(StaticBackground);
		P.drawPixmap(p->theVectorPanDelta, savPix);
		P.end();
	//} else {
	//	StaticBackground->fill(Qt::transparent);
	}

	if (!p->invalidRects.isEmpty()) {
		P.begin(StaticBackground);
		P.setRenderHint(QPainter::Antialiasing);
		if (!p->theVectorPanDelta.isNull()) {
			P.setClipping(true);
			P.setClipRegion(QRegion(rect()) - QRegion(QRect(p->theVectorPanDelta, size())));
		}
		drawBackground(P, theProjection);
		P.setClipping(false);
		P.end();
	}
}

void MapView::updateStaticBuffer()
{
	if (!StaticBuffer || (StaticBuffer->size() != size()))
	{
		delete StaticBuffer;
		StaticBuffer = new QPixmap(size());
	}

	QPainter P;

	if (!p->theVectorPanDelta.isNull()) {
		QPixmap savPix;
		savPix = StaticBuffer->copy();
		StaticBuffer->fill(Qt::transparent);
		P.begin(StaticBuffer);
		P.drawPixmap(p->theVectorPanDelta, savPix);
		P.end();
	} else {
		StaticBuffer->fill(Qt::transparent);
	}

	if (!p->invalidRects.isEmpty()) {
		P.begin(StaticBuffer);
		P.setRenderHint(QPainter::Antialiasing);
		P.setClipping(true);
		P.setClipRegion(QRegion(rect()));
		if (!p->theVectorPanDelta.isNull()) {
			P.setClipRegion(QRegion(rect()) - QRegion(QRect(p->theVectorPanDelta, size())));
		}
		drawFeatures(P, theProjection);
		P.setClipping(false);
		P.end();
	}

	p->invalidRects.clear();
	p->theVectorPanDelta = QPoint(0, 0);
	StaticBufferUpToDate = true;
}

void MapView::mousePressEvent(QMouseEvent* event)
{
	if (theInteraction) {
		Main->info()->setHtml(theInteraction->toHtml());
		theInteraction->mousePressEvent(event);
	}
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
	zoom(finalZoom, ev->pos(), rect());
	for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
		ImgIt.get()->zoom(finalZoom, ev->pos(), rect());
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

QTransform& MapView::transform()
{
	return p->theTransform;
}

QPoint MapView::toView(const Coord& aCoord) const
{
	return p->theTransform.map(theProjection.project(aCoord)).toPoint();
}

QPoint MapView::toView(TrackPoint* aPt) const
{
	return p->theTransform.map(theProjection.project(aPt)).toPoint();
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


void MapView::on_imageRequested(ImageMapLayer*)
{
	++numImages;
	Main->pbImages->setRange(0, numImages);
	//pbImages->setValue(0);
	Main->pbImages->update();
	if (Main->pbImages->value() < 0)
		Main->pbImages->setValue(0);
}

void MapView::on_imageReceived(ImageMapLayer* aLayer)
{
	Main->pbImages->setValue(Main->pbImages->value()+1);
	aLayer->forceRedraw(*this, rect());
	StaticMapUpToDate = false;
	update();
}

void MapView::on_loadingFinished(ImageMapLayer*)
{
	numImages = 0;
	Main->pbImages->reset();
}

void MapView::resizeEvent(QResizeEvent * ev)
{
	StaticBufferUpToDate = false;
	resize(ev->oldSize(), ev->size());

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
		if ((tP = qobject_cast<TrackPoint*>(it.get())) && tP->pixelDistance(event->pos(), 5.01, projection(), p->theTransform) < 5.01) {
			dropTarget = tP;
			QRect acceptedRect(tP->projection().toPoint() - QPoint(3, 3), tP->projection().toPoint() + QPoint(3, 3));
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

	viewport().toXML("Viewport", e);

	return OK;
}

void MapView::fromXML(const QDomElement e)
{
	CoordBox cb = CoordBox::fromXML(e.firstChildElement("Viewport"));
	setViewport(cb, rect());

	invalidate(true, true);
}

void MapView::on_MoveLeft_activated()
{
	QPoint p(rect().width()/4,0);
	panScreen(p);

	invalidate(true, true);
}
void MapView::on_MoveRight_activated()
{
	QPoint p(-rect().width()/4,0);
	panScreen(p);

	invalidate(true, true);
}

void MapView::on_MoveUp_activated()
{
	QPoint p(0,rect().height()/4);
	panScreen(p);

	invalidate(true, true);
}

void MapView::on_MoveDown_activated()
{
	QPoint p(0,-rect().height()/4);
	panScreen(p);

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
		Main->statusBar()->clearMessage();
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

CoordBox MapView::viewport() const
{
	return p->Viewport;
}

void MapView::viewportRecalc(const QRect & Screen)
{
	QRectF fScreen(Screen);
	p->Viewport =
		CoordBox(theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomLeft())),
			 theProjection.inverse(p->theTransform.inverted().map(fScreen.topRight())));

	emit viewportChanged();
}

void MapView::transformCalc(QTransform& theTransform, const Projection& theProjection, const CoordBox& TargetMap, const QRect& Screen)
{
	QPointF bl = theProjection.project(TargetMap.bottomLeft());
	QPointF tr = theProjection.project(TargetMap.topRight());
	QRectF pViewport = QRectF(bl, QSizeF(tr.x() - bl.x(), tr.y() - bl.y()));

	Coord Center(TargetMap.center());
	QPointF pCenter(pViewport.center());

	double Aspect = (double)Screen.width() / Screen.height();
	double pAspect = pViewport.width() / pViewport.height();

	double wv, hv;
	if (pAspect > Aspect) {
		wv = pViewport.width();
		hv = pViewport.height() * pAspect / Aspect;
	} else {
		wv = pViewport.width() * Aspect / pAspect;
		hv = pViewport.height();
	}

	double ScaleLon = Screen.width() / wv;
	double ScaleLat = Screen.height() / hv;

	double PLon = pCenter.x() * ScaleLon;
	double PLat = pCenter.y() * ScaleLat;
	double DeltaLon = Screen.width() / 2 - PLon;
	double DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);

	theTransform.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);
}

void MapView::setViewport(const CoordBox & TargetMap)
{
	p->Viewport = TargetMap;
}

void MapView::setViewport(const CoordBox & TargetMap,
									const QRect & Screen)
{
#ifndef _MOBILE
	transformCalc(p->theTransform, theProjection, TargetMap, Screen);
	viewportRecalc(Screen);

	// Calculate PixelPerM
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(TargetMap.center().lat())));
	double degAspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	double so = Screen.width() / (double)p->Viewport.lonDiff();
	double sa = so / degAspect;

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	p->PixelPerM = LatAngPerM / M_PI * INT_MAX * sa;
	//
#else
	Viewport = TargetMap;
	Coord Center(Viewport.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(Center.lat())));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / (double)Viewport.lonDiff();
	ScaleLat = ScaleLon / Aspect;

	if ((ScaleLat * Viewport.latDiff()) > Screen.height())
	{
		ScaleLat = Screen.height() / (double)Viewport.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	PixelPerM = LatAngPerM / M_PI * INT_MAX * ScaleLat;

	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
	viewportRecalc(Screen);
#endif
}

void MapView::zoom(double d, const QPointF & Around,
							 const QRect & Screen)
{
#ifndef _MOBILE
	if (p->PixelPerM > 100 && d > 1.0)
		return;

	Coord Before = theProjection.inverse(p->theTransform.inverted().map(Around));
	QPointF pBefore = theProjection.project(Before);

	double ScaleLon = p->theTransform.m11() * d;
	double ScaleLat = p->theTransform.m22() * d;
	double DeltaLat = (Around.y() - pBefore.y() * ScaleLat);
	double DeltaLon = (Around.x() - pBefore.x() * ScaleLon);

	p->theTransform.setMatrix(ScaleLon, 0, 0, 0, ScaleLat, 0, DeltaLon, DeltaLat, 1);
	viewportRecalc(Screen);

	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(Before.lat())));
	double degAspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	double so = Screen.width() / (double)p->Viewport.lonDiff();
	double sa = so / degAspect;

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	p->PixelPerM = LatAngPerM / M_PI * INT_MAX * sa;

#else
	if (ScaleLat * d < 1.0 && ScaleLon * d < 1.0) {
		Coord Before = inverse(Around);
		ScaleLon *= d;
		ScaleLat *= d;
		DeltaLat = int(Around.y() + Before.lat() * ScaleLat);
		DeltaLon = int(Around.x() - Before.lon() * ScaleLon);

		double LatAngPerM = 1.0 / EQUATORIALRADIUS;
		PixelPerM = LatAngPerM / M_PI * INT_MAX * ScaleLat;

		viewportRecalc(Screen);
	}
#endif
}

void MapView::resize(QSize oldS, QSize newS)
{
	Q_UNUSED(oldS)
#ifndef _MOBILE
	viewportRecalc(QRect(QPoint(0,0), newS));
#else
	Q_UNUSED(newS)
#endif
}

void MapView::setCenter(Coord & Center, const QRect & /*Screen*/)
{
	Coord curCenter(p->Viewport.center());
	QPointF curCenterScreen = p->theTransform.map(theProjection.project(curCenter));
	QPointF newCenterScreen = p->theTransform.map(theProjection.project(Center));

	QPointF panDelta = (curCenterScreen - newCenterScreen);
	panScreen(panDelta.toPoint());
}

double MapView::pixelPerM() const
{
	return p->PixelPerM;
}
