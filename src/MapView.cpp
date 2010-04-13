#include <errno.h>

#include "MapView.h"
#include "MainWindow.h"
#include <ui_MainWindow.h>
#include "PropertiesDock.h"
#include "Document.h"
#include "Layer.h"
#include "ImageMapLayer.h"
#include "Feature.h"
#include "Relation.h"
#include "Interaction.h"
#include "EditInteraction.h"
#include "PaintStyle/MasPaintStyle.h"
#include "Maps/Projection.h"
#include "GPS/qgps.h"
#include "GPS/qgpsdevice.h"
#include "LayerIterator.h"

#include "MapRenderer.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

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
#include <QMap>
#include <QSet>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define LAT_ANG_PER_M 1.0 / EQUATORIALRADIUS

class MapViewPrivate
{
public:
    QTransform theTransform;
    double PixelPerM;
    double ZoomLevel;
    int AbstractZoomLevel;
    CoordBox Viewport;
    QList<CoordBox> invalidRects;
    QPoint theRasterPanDelta, theVectorPanDelta;
    QMap<RenderPriority, QSet <Feature*> > theFeatures;
    QSet<Way*> theCoastlines;
    QList<Node*> theVirtualNodes;
    MapRenderer renderer;

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
    setAttribute(Qt::WA_NoSystemBackground);
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

void MapView::setDocument(Document* aDoc)
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

Document *MapView::document()
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
            ImgIt.get()->forceRedraw(*this, rect(), p->theRasterPanDelta);
        p->theRasterPanDelta = QPoint();
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

    drawLatLonGrid(P);
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
//	qDebug() << "VP: " << QString("%1 (%2,%3,%4,%5)")
//	   .arg(Main->ViewportStatusLabel->text())
//		.arg(QString::number(pbl.x(),'f',4))
//		.arg(QString::number(pbl.y(),'f',4))
//		.arg(QString::number(ptr.x(),'f',4))
//		.arg(QString::number(ptr.y(),'f',4));

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
            ImgIt.get()->drawImage(*StaticMap);

    StaticMapUpToDate = true;
}

void MapView::buildFeatureSet()
{
    if (!theDocument)
        return;

    QRectF clipRect = p->theTransform.inverted().mapRect(QRectF(rect().adjusted(-1000, -1000, 1000, 1000)));

    p->theCoastlines.clear();
    for (int i=0; i<theDocument->layerSize(); ++i) {
        if (Main)
            Main->properties()->adjustSelection();
        theDocument->getLayer(i)->getFeatureSet(p->theFeatures, p->theCoastlines, theDocument, p->invalidRects, clipRect, theProjection, p->theTransform);
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
    QSet<Way*>::const_iterator it = p->theCoastlines.constBegin();
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

#define PARALLEL_LINES_NUM 5
#define MEDIAN_LINES_NUM 5

void MapView::drawLatLonGrid(QPainter & P)
{
    if (!M_PREFS->getLatLonGridVisible())
        return;

    int lonInterval = angToInt(0.002/p->ZoomLevel);
    int latInterval = angToInt(0.002/p->ZoomLevel);
    int lonStart = qMax(int(p->Viewport.bottomLeft().lon() / lonInterval) * lonInterval, -INT_MAX);
    int latStart = qMax(int(p->Viewport.bottomLeft().lat() / latInterval) * latInterval, -INT_MAX/2);

    QList<QPolygonF> medianLines;
    QList<QPolygonF> parallelLines;

    for (double y=latStart; y<=qMin(p->Viewport.topLeft().lat()+latInterval, INT_MAX/2); y+=latInterval) {
        QPolygonF l;
        for (double x=lonStart; x<=qMin(p->Viewport.bottomRight().lon()+lonInterval, INT_MAX); x+=lonInterval) {
            QPointF pt = QPointF(theProjection.project(Coord(qRound(y), qRound(x))));
            l << pt;
        }
        parallelLines << l;
    }
    for (double x=lonStart; x<=qMin(p->Viewport.bottomRight().lon()+lonInterval, INT_MAX); x+=lonInterval) {
        QPolygonF l;
        for (double y=latStart; y<=qMin(p->Viewport.topLeft().lat()+latInterval, INT_MAX/2); y+=latInterval) {
            QPointF pt = QPointF(theProjection.project(Coord(qRound(y), qRound(x))));
            l << pt;
        }
        medianLines << l;
    }

    P.setPen(QColor(180, 217, 255));
    for (int i=0; i<parallelLines.size(); ++i) {
        P.drawPolyline(p->theTransform.map(parallelLines[i]));
        QPoint pt = QPoint(0, p->theTransform.map(parallelLines.at(i).at(0)).y());
        QPoint ptt = pt + QPoint(5, -5);
        P.drawText(ptt, QString("%1").arg(intToAng(theProjection.inverse(parallelLines.at(i).at(0)).lat()), 0, 'f', 2));
    }
    for (int i=0; i<medianLines.size(); ++i) {
        P.drawPolyline(p->theTransform.map(medianLines[i]));
        QPoint pt = QPoint(p->theTransform.map(medianLines.at(i).at(0)).x(), 0);
        QPoint ptt = pt + QPoint(5, 10);
        P.drawText(ptt, QString("%1").arg(intToAng(theProjection.inverse(medianLines.at(i).at(0)).lon()), 0, 'f', 2));
    }
}

void MapView::drawFeatures(QPainter & P, Projection& /*aProj*/)
{
    p->renderer.render(&P, p->theFeatures, this);
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
    if (theInteraction)
        theInteraction->wheelEvent(ev);
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

QPoint MapView::toView(Node* aPt) const
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

        menu.addAction(Main->ui->viewZoomOutAction);
        menu.addAction(Main->ui->viewZoomWindowAction);
        menu.addAction(Main->ui->viewZoomInAction);

        QMenu featureMenu(tr("Feature"));
        for(int i=0; i<Main->ui->menu_Feature->actions().size(); ++i) {
            if (Main->ui->menu_Feature->actions()[i]->isEnabled())
                featureMenu.addAction(Main->ui->menu_Feature->actions()[i]);
        }
        if (featureMenu.actions().size())
            menu.addMenu(&featureMenu);


        QMenu nodeMenu(tr("Node"));
        for(int i=0; i<Main->ui->menu_Node->actions().size(); ++i) {
            if (Main->ui->menu_Node->actions()[i]->isEnabled())
                nodeMenu.addAction(Main->ui->menu_Node->actions()[i]);
        }
        if (nodeMenu.actions().size())
            menu.addMenu(&nodeMenu);

        QMenu roadMenu(tr("Road"));
        for(int i=0; i<Main->ui->menuRoad->actions().size(); ++i) {
            if (Main->ui->menuRoad->actions()[i]->isEnabled())
                roadMenu.addAction(Main->ui->menuRoad->actions()[i]);
        }
        if (roadMenu.actions().size())
            menu.addMenu(&roadMenu);

        QMenu relationMenu(tr("Relation"));
        for(int i=0; i<Main->ui->menuRelation->actions().size(); ++i) {
            if (Main->ui->menuRelation->actions()[i]->isEnabled())
                relationMenu.addAction(Main->ui->menuRelation->actions()[i]);
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
    aLayer->forceRedraw(*this, rect(), p->theRasterPanDelta);
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
    if (event->mimeData()->hasUrls() && event->mimeData()->urls().first().toLocalFile().endsWith(".jpg", Qt::CaseInsensitive)) {
        dropTarget = NULL;
        event->accept();
    } else
        event->ignore();
}

void MapView::dragMoveEvent(QDragMoveEvent *event)
{
    {
        QMouseEvent mE(QEvent::MouseMove, event->pos(), Qt::LeftButton, Qt::LeftButton, qApp->keyboardModifiers());
        mouseMoveEvent(&mE);
    }
    Node *tP;
    for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it) {
        if ((tP = qobject_cast<Node*>(it.get())) && tP->pixelDistance(event->pos(), 5.01, true, this) < 5.01) {
            dropTarget = tP;
            QRect acceptedRect(tP->projection().toPoint() - QPoint(3, 3), tP->projection().toPoint() + QPoint(3, 3));
            event->acceptProposedAction();
            event->accept(acceptedRect);
            return;
        }
    }
    event->acceptProposedAction();
    event->accept();
}

void MapView::dropEvent(QDropEvent *event)
{
    // first save the image url because the even->mimeData() releases its data very soon
    // this is probably because the drop action ends with calling of this event
    // so the program that started the drag-action thinks the data isn't needed anymore
    QList<QUrl> imageUrls = event->mimeData()->urls();
    QStringList locFiles;
    foreach(QUrl url, imageUrls)
        locFiles << url.toLocalFile();

    if (locFiles.size() > 1)
        Main->geoImage()->loadImages(locFiles);
    else {
        QMenu menu(this);
        QString imageFn = locFiles[0];
        Coord mapC = theProjection.inverse(p->theTransform.inverted().map(event->pos()));
        Coord pos = GeoImageDock::getGeoDataFromImage(imageFn);

        if (pos.isNull()) {
            QAction *add, *load;
            load = menu.addAction(tr("Load image"));
            if (dropTarget)
                add = menu.addAction(tr("Add node position to image"));
            else
                add = menu.addAction(tr("Geotag image with this position"));
            menu.addSeparator();
            menu.addAction(tr("Cancel"));
            QAction* res = menu.exec(QCursor::pos());
            if (res == add) {
                if (dropTarget)
                    Main->geoImage()->addGeoDataToImage(dropTarget->position(), imageFn);
                else
                    Main->geoImage()->addGeoDataToImage(mapC,imageFn);
                Main->geoImage()->loadImages(locFiles);
            } else if (res == load)
                Main->geoImage()->loadImage(imageFn, mapC);
        } else
            Main->geoImage()->loadImages(locFiles);
    }
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
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            l->setCurrentZoom(p->Viewport, rect());
            qreal z = l->pixelPerM() / p->PixelPerM;
            zoom(z, rect().center(), rect());
        }
    }
}

void MapView::setViewport(const CoordBox & TargetMap,
                                    const QRect & Screen)
{
    transformCalc(p->theTransform, theProjection, TargetMap, Screen);
    viewportRecalc(Screen);

    if (theProjection.projIsLatLong()) {
        p->PixelPerM = Screen.width() / (double)p->Viewport.lonDiff() * LAT_ANG_PER_M / M_PI * INT_MAX;
    } else {
        QRectF vp = theProjection.getProjectedViewport(p->Viewport, Screen);
        p->PixelPerM = Screen.width() / vp.width();
    }

    QPointF pt = theProjection.project(Coord(0, angToInt(180)));
    double earthWidth = pt.x() * 2;
    double zoomPixPerMat0 = 512. / earthWidth;
    double z = 0;
    p->AbstractZoomLevel = 0;
    for (;z<p->theTransform.m11(); ++p->AbstractZoomLevel) {
        double zoomPixPerMatCur = zoomPixPerMat0 * pow(2., p->AbstractZoomLevel);
        z = zoomPixPerMatCur / p->PixelPerM;
    }
    p->ZoomLevel = p->theTransform.m11();
    qDebug() << "Abstract zoom level: " << p->AbstractZoomLevel;

    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            l->setCurrentZoom(p->Viewport, rect());
            qreal z = l->pixelPerM() / p->PixelPerM;
            zoom(z, Screen.center(), Screen);
        }
    }
}

void MapView::zoom(double d, const QPoint & Around)
{
    if (p->PixelPerM > 100 && d > 1.0)
        return;

    qreal z = d;
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            if (d > 1.0) {
                l->zoom_in();
            } else {
                l->zoom_out();
            }
            z = l->pixelPerM() / p->PixelPerM;
        }
    }

    zoom(z, Around, rect());
}

void MapView::adjustZoomToBoris()
{
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            qreal z = l->pixelPerM() / p->PixelPerM;
            zoom(z, rect().center(), rect());
        }
    }
}

void MapView::zoom(double d, const QPoint & Around,
                             const QRect & Screen)
{
    Coord Before = theProjection.inverse(p->theTransform.inverted().map(QPointF(Around)));
    QPointF pBefore = theProjection.project(Before);

    double ScaleLon = p->theTransform.m11() * d;
    double ScaleLat = p->theTransform.m22() * d;
    double DeltaLat = (Around.y() - pBefore.y() * ScaleLat);
    double DeltaLon = (Around.x() - pBefore.x() * ScaleLon);

    p->theTransform.setMatrix(ScaleLon, 0, 0, 0, ScaleLat, 0, DeltaLon, DeltaLat, 1);
    viewportRecalc(Screen);

    if (theProjection.projIsLatLong()) {
        p->PixelPerM = Screen.width() / (double)p->Viewport.lonDiff() * LAT_ANG_PER_M / M_PI * INT_MAX;
    } else {
        QRectF vp = theProjection.getProjectedViewport(p->Viewport, Screen);
        p->PixelPerM = Screen.width() / vp.width();
    }

    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        ImgIt.get()->zoom(d, Around, Screen);

    QPointF pt = theProjection.project(Coord(0, angToInt(180)));
    double earthWidth = pt.x() * 2;
    double zoomPixPerMat0 = 512. / earthWidth;
    double z = 0;
    p->AbstractZoomLevel = 0;
    for (;z<p->theTransform.m11(); ++p->AbstractZoomLevel) {
        double zoomPixPerMatCur = zoomPixPerMat0 * pow(2., p->AbstractZoomLevel);
        z = zoomPixPerMatCur / p->PixelPerM;
    }
    p->ZoomLevel = ScaleLon;
    qDebug() << "Zoom: " << ScaleLon << "; Abstract zoom level: " << p->AbstractZoomLevel;
}

void MapView::resize(QSize oldS, QSize newS)
{
    Q_UNUSED(oldS)
    viewportRecalc(QRect(QPoint(0,0), newS));
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
