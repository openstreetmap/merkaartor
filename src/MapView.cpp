#include <errno.h>

#include "MapView.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
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
#define TEST_RFLAGS(x) p->ROptions.options.testFlag(x)

class MapViewPrivate
{
public:
    QTransform theTransform;
    double PixelPerM;
    double NodeWidth;
    double ZoomLevel;
//    int AbstractZoomLevel;
    CoordBox Viewport;
    QList<CoordBox> invalidRects;
    QPoint theVectorPanDelta;
    QMap<RenderPriority, QSet <Feature*> > theFeatures;
    QSet<Way*> theCoastlines;
    QList<Node*> theVirtualNodes;
    MapRenderer renderer;
    RendererOptions ROptions;

    MapViewPrivate()
      : PixelPerM(0.0), Viewport(WORLD_COORDBOX)
    {}
};

/****************/

MapView::MapView(QWidget* parent) :
    QWidget(parent), Main(dynamic_cast<MainWindow*>(parent)), theDocument(0), theInteraction(0), StaticBackground(0), StaticBuffer(0), StaticMap(0),
        StaticBufferUpToDate(false), SelectionLocked(false),lockIcon(0), numImages(0),
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
    if (Main)
        return Main->properties();
    else
        return NULL;
}

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
        p->theVectorPanDelta = QPoint(0, 0);
        StaticBufferUpToDate = false;
    }
    if (theDocument && updateMap) {
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
            ImgIt.get()->forceRedraw(*this, rect());
    }
    update();
}

void MapView::panScreen(QPoint delta)
{
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

    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        ImgIt.get()->pan(delta);
    update();
}

void MapView::paintEvent(QPaintEvent * anEvent)
{
    if (!theDocument)
        return;

#ifndef NDEBUG
    QTime Start(QTime::currentTime());
#endif

    p->theFeatures.clear();

    QPainter P;
    P.begin(this);

    if (!p->invalidRects.isEmpty())
        buildFeatureSet();

    updateStaticBackground();

    if (!StaticBufferUpToDate) {
        updateStaticBuffer();
    }

    P.drawPixmap(p->theVectorPanDelta, *StaticBackground);
    P.save();
    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        if (ImgIt.get()->isVisible())
            ImgIt.get()->drawImage(&P);
    P.restore();
    P.drawPixmap(p->theVectorPanDelta, *StaticBuffer);

    drawLatLonGrid(P);
    drawDownloadAreas(P);
    drawScale(P);

    if (theInteraction) {
        P.setRenderHint(QPainter::Antialiasing);
        theInteraction->paintEvent(anEvent, P);
    }

    if (Main)
        drawGPS(P);

    P.end();

    if (Main) {
        Main->ViewportStatusLabel->setText(QString("%1,%2,%3,%4")
                                           .arg(QString::number(coordToAng(viewport().bottomLeft().lon()),'f',4))
                                           .arg(QString::number(coordToAng(viewport().bottomLeft().lat()),'f',4))
                                           .arg(QString::number(coordToAng(viewport().topRight().lon()),'f',4))
                                           .arg(QString::number(coordToAng(viewport().topRight().lat()),'f',4))
                                           );

#ifndef NDEBUG
        QTime Stop(QTime::currentTime());
        Main->PaintTimeLabel->setText(tr("%1ms;ppm:%2").arg(Start.msecsTo(Stop)).arg(p->PixelPerM));
#endif
    }
}

void MapView::drawScale(QPainter & P)
{

    if (!TEST_RFLAGS(RendererOptions::ScaleVisible))
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
    QPointF P1(20,height()-20);
    QPointF P2(20+Length*p->PixelPerM,height()-20);
    P.fillRect(P1.x()-4, P1.y()-20-4, P2.x() - P1.x() + 4, 33, QColor(255, 255, 255, 128));
    P.setPen(QPen(QColor(0,0,0),2));
    P.drawLine(P1-QPointF(0,5),P1+QPointF(0,5));
    P.drawLine(P1,P2);
    if (Length < 1000)
        P.drawText(QRectF(P2-QPoint(200,40),QSize(200,30)),Qt::AlignRight | Qt::AlignBottom, QString(tr("%1 m")).arg(Length, 0, 'f', 0));
    else
        P.drawText(QRectF(P2-QPoint(200,40),QSize(200,30)),Qt::AlignRight | Qt::AlignBottom, QString(tr("%1 km")).arg(Length/1000, 0, 'f', 0));

    P.drawLine(P2-QPointF(0,5),P2+QPointF(0,5));
}

void MapView::drawGPS(QPainter & P)
{
    if (Main->gps()->getGpsDevice()) {
        if (Main->gps()->getGpsDevice()->fixStatus() == QGPSDevice::StatusActive) {
            Coord vp(angToCoord(Main->gps()->getGpsDevice()->latitude()), angToCoord(Main->gps()->getGpsDevice()->longitude()));
            QPointF g = p->theTransform.map(projection().project(vp));
            QPixmap pm = getPixmapFromFile(":/Gps/Gps_Marker.svg", 32);
            P.drawPixmap(g - QPoint(16, 16), pm);
        }
    }
}

void MapView::buildFeatureSet()
{
    QRectF clipRect = p->theTransform.inverted().mapRect(QRectF(rect().adjusted(-200, -200, 200, 200)));

    p->theCoastlines.clear();
    for (int i=0; i<theDocument->layerSize(); ++i)
        theDocument->getLayer(i)->getFeatureSet(p->theFeatures, p->theCoastlines, theDocument, p->invalidRects, clipRect, theProjection, p->theTransform);
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

void MapView::drawCoastlines(QPainter & theP)
{
    double WW = p->PixelPerM*30.0 + 2.0;

    QPen thePen(M_PREFS->getWaterColor(), WW);
    thePen.setCapStyle(Qt::RoundCap);
    thePen.setJoinStyle(Qt::RoundJoin);
    theP.setPen(thePen);
    theP.setBrush(Qt::NoBrush);
    theP.setRenderHint(QPainter::Antialiasing);

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
    if (!TEST_RFLAGS(RendererOptions::LatLonGridVisible))
        return;

    double lonInterval = coordToAng(p->Viewport.lonDiff()) / 4;
    double latInterval = coordToAng(p->Viewport.latDiff()) / 4;
    int prec = log10(lonInterval);
    if (!lonInterval || !latInterval) return; // avoid divide-by-zero
    double lonStart = qMax(int(p->Viewport.bottomLeft().lon() / lonInterval) * lonInterval, -COORD_MAX);
    if (lonStart<1 && lonStart != -COORD_MAX)
        lonStart -= lonInterval;
    double latStart = qMax(int(p->Viewport.bottomLeft().lat() / latInterval) * latInterval, -COORD_MAX/2);
    if (latStart<1 && latStart != -COORD_MAX/2)
        latStart -= lonInterval;

    QList<QPolygonF> medianLines;
    QList<QPolygonF> parallelLines;

    for (double y=latStart; y<=p->Viewport.topLeft().lat()+latInterval; y+=latInterval) {
        QPolygonF l;
        for (double x=lonStart; x<=p->Viewport.bottomRight().lon()+lonInterval; x+=lonInterval) {
            QPointF pt = theProjection.project(Coord(qMin(y, COORD_MAX/2), qMin(x, COORD_MAX)));
            l << pt;
        }
        parallelLines << l;
    }
    for (double x=lonStart; x<=p->Viewport.bottomRight().lon()+lonInterval; x+=lonInterval) {
        QPolygonF l;
        for (double y=latStart; y<=p->Viewport.topLeft().lat()+latInterval; y+=latInterval) {
            QPointF pt = theProjection.project(Coord(qMin(y, COORD_MAX/2), qMin(x, COORD_MAX)));
            l << pt;
        }
        medianLines << l;
    }

    P.save();
    P.setRenderHint(QPainter::Antialiasing);
    P.setPen(QColor(180, 217, 255));
    QLineF lb = QLineF(rect().topLeft(), rect().bottomLeft());
    QLineF lt = QLineF(rect().topLeft(), rect().topRight());
    QLineF l;
    for (int i=0; i<parallelLines.size(); ++i) {

        if (parallelLines[i].size() == 0)
          continue;

        P.drawPolyline(p->theTransform.map(parallelLines[i]));
        int k=0;
        QPointF pt;
        while (k < parallelLines.at(i).size()-2) {
            l = QLineF(p->theTransform.map(parallelLines.at(i).at(k)), p->theTransform.map(parallelLines.at(i).at(k+1)));
            if (l.intersect(lb, &pt) == QLineF::BoundedIntersection)
                break;
            ++k;
        }
        if (pt.isNull())
            continue;
//        QPoint pt = QPoint(0, p->theTransform.map(parallelLines.at(i).at(0)).y());
        QPoint ptt = pt.toPoint() + QPoint(5, -5);
        P.drawText(ptt, QString("%1").arg(coordToAng(theProjection.inverse(parallelLines.at(i).at(0)).lat()), 0, 'f', 2-prec));
    }
    for (int i=0; i<medianLines.size(); ++i) {

        if (medianLines[i].size() == 0)
          continue;

        P.drawPolyline(p->theTransform.map(medianLines[i]));
        int k=0;
        QPointF pt;
        while (k < medianLines.at(i).size()-2) {
            l = QLineF(p->theTransform.map(medianLines.at(i).at(k)), p->theTransform.map(medianLines.at(i).at(k+1)));
            if (l.intersect(lt, &pt) == QLineF::BoundedIntersection)
                break;
            ++k;
        }
        if (pt.isNull())
            continue;
//        QPoint pt = QPoint(p->theTransform.map(medianLines.at(i).at(0)).x(), 0);
        QPoint ptt = pt.toPoint() + QPoint(5, 10);
        P.drawText(ptt, QString("%1").arg(coordToAng(theProjection.inverse(medianLines.at(i).at(0)).lon()), 0, 'f', 2-prec));
    }

    P.restore();
}

void MapView::drawFeatures(QPainter & P)
{
    p->renderer.render(&P, p->theFeatures, p->ROptions, this);
}

void MapView::printFeatures(QPainter & P)
{
    p->renderer.render(&P, p->theFeatures, p->ROptions, this);
}

void MapView::drawDownloadAreas(QPainter & P)
{
    if (!TEST_RFLAGS(RendererOptions::DownloadedVisible))
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

        if (M_PREFS->getUseShapefileForBackground())
            StaticBackground->fill(M_PREFS->getWaterColor());
        else if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
            StaticBackground->fill(M_PREFS->getBgColor());
        else
            StaticBackground->fill(M_STYLE->getGlobalPainter().getBackgroundColor());
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
#if QT_VERSION < 0x040600
        QPixmap savPix;
        savPix = StaticBuffer->copy();
        StaticBuffer->fill(Qt::transparent);
        P.begin(StaticBuffer);
        P.drawPixmap(p->theVectorPanDelta, savPix);
        P.setClipping(true);
        P.setClipRegion(QRegion(rect()) - QRegion(QRect(p->theVectorPanDelta, size())));
#else
        QRegion exposed;
        StaticBuffer->scroll(p->theVectorPanDelta.x(), p->theVectorPanDelta.y(), StaticBuffer->rect(), &exposed);
        P.begin(StaticBuffer);
        P.setClipping(true);
        P.setClipRegion(exposed);
        P.eraseRect(StaticBuffer->rect());
#endif
    } else {
        StaticBuffer->fill(Qt::transparent);
        P.begin(StaticBuffer);
    }

    if (!p->invalidRects.isEmpty()) {
//        P.setRenderHint(QPainter::Antialiasing);
//        P.setClipping(true);
//        P.setClipRegion(QRegion(rect()));
        drawCoastlines(P);
        drawFeatures(P);
        P.end();
    }

    p->invalidRects.clear();
    p->theVectorPanDelta = QPoint(0, 0);
    StaticBufferUpToDate = true;
}

void MapView::mousePressEvent(QMouseEvent* event)
{
    if (!document())
        return;

    if (theInteraction) {
        if (Main)
            Main->info()->setHtml(theInteraction->toHtml());
        theInteraction->mousePressEvent(event);
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* event)
{
    if (!document())
        return;

    if (theInteraction)
        theInteraction->mouseReleaseEvent(event);
}

void MapView::mouseMoveEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (!updatesEnabled())
        return;
    if (theInteraction)
        theInteraction->mouseMoveEvent(anEvent);
}

void MapView::mouseDoubleClickEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (!updatesEnabled())
        return;
    if (theInteraction)
        theInteraction->mouseDoubleClickEvent(anEvent);
}

void MapView::wheelEvent(QWheelEvent* ev)
{
    if (!document())
        return;

    if (theInteraction)
        theInteraction->wheelEvent(ev);
}

void MapView::launch(Interaction* anInteraction)
{
    EditInteraction* EI = dynamic_cast<EditInteraction*>(theInteraction);
    if (EI)
        theSnapList = EI->snapList();
    if (!theSnapList.size())
        if (Main)
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
    if (!Main)
        return;

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
    if (Main) {
        ++numImages;
        Main->pbImages->setRange(0, numImages);
        //pbImages->setValue(0);
        Main->pbImages->update();
        if (Main->pbImages->value() < 0)
            Main->pbImages->setValue(0);
    }
}

void MapView::on_imageReceived(ImageMapLayer* aLayer)
{
    if (Main) {
        if (Main->pbImages->value() < Main->pbImages->maximum())
            Main->pbImages->setValue(Main->pbImages->value()+1);
    }
    aLayer->forceRedraw(*this, rect());
    update();
}

void MapView::on_loadingFinished(ImageMapLayer* aLayer)
{
    Q_UNUSED(aLayer)
    numImages = 0;
    if (Main)
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
    if (!Main) {
        event->ignore();
        return;
    }

    if (event->mimeData()->hasUrls() && event->mimeData()->urls().first().toLocalFile().endsWith(".jpg", Qt::CaseInsensitive)) {
        dropTarget = NULL;
        event->accept();
    } else
        event->ignore();
}

void MapView::dragMoveEvent(QDragMoveEvent *event)
{
    if (!Main) {
        event->ignore();
        return;
    }

    QMouseEvent mE(QEvent::MouseMove, event->pos(), Qt::LeftButton, Qt::LeftButton, qApp->keyboardModifiers());
    mouseMoveEvent(&mE);

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
    if (!Main) {
        event->ignore();
        return;
    }

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
    theProjection.toXML(e);

    return OK;
}

void MapView::fromXML(const QDomElement p)
{
    CoordBox cb;
    QDomElement e = p.firstChildElement();
    while(!e.isNull()) {
        if (e.tagName() == "Viewport") {
            cb = CoordBox::fromXML(e);
        }
        else if (e.tagName() == "Projection") {
            theProjection.fromXML(e);
        }

        e = e.nextSiblingElement();
    }

    if (!cb.isNull())
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

//            if (!isSelectionLocked())
//                lockSelection();
//            else
            {
                FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
                if (intr)
                    intr->nextSnap();
            }

            return true;
        } else
        if ( ke->key() == Qt::Key_Backtab ) {
            setFocus();
            ke->accept();

//            if (!isSelectionLocked())
//                lockSelection();
//            else
            {
                FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
                if (intr)
                    intr->nextSnap();
            }

            return true;
        }
    } else
    if ( event->type() == QEvent::Leave ) {
        if (Main)
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
    if (!Main)
        return;

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
    if (!Main)
        return;

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
    Coord bl = theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomLeft()));
    Coord br = theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomRight()));
    Coord tr = theProjection.inverse(p->theTransform.inverted().map(fScreen.topRight()));
    Coord tl = theProjection.inverse(p->theTransform.inverted().map(fScreen.topLeft()));
    double t = qMax(tr.lat(), tl.lat());
    double b = qMin(br.lat(), bl.lat());
    double l = qMin(tl.lon(), bl.lon());
    double r = qMax(tr.lon(), br.lon());
    p->Viewport = CoordBox(Coord(b, l), Coord(t, r));

    if (theProjection.projIsLatLong()) {
        p->PixelPerM = Screen.width() / (double)p->Viewport.lonDiff() * theProjection.lonAnglePerM(coordToRad(p->Viewport.center().lat()));
    } else {
        // measure geographical distance between mid left and mid right of the screen
        int mid = (fScreen.topLeft().y() + fScreen.bottomLeft().y()) / 2;
        Coord left = theProjection.inverse(p->theTransform.inverted().map(QPointF(fScreen.left(), mid)));
        Coord right = theProjection.inverse(p->theTransform.inverted().map(QPointF(fScreen.right(), mid)));
        p->PixelPerM = Screen.width() / (left.distanceFrom(right)*1000);
    }

    emit viewportChanged();
}

void MapView::transformCalc(QTransform& theTransform, const Projection& theProjection, const CoordBox& TargetMap, const QRect& Screen)
{
    QPointF bl = theProjection.project(TargetMap.bottomLeft());
    QPointF tr = theProjection.project(TargetMap.topRight());
    QRectF pViewport = QRectF(bl, QSizeF(tr.x() - bl.x(), tr.y() - bl.y()));

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
    if (TargetMap.latDiff() == 0 || TargetMap.lonDiff() == 0)
        p->Viewport = CoordBox (TargetMap.center()-COORD_ENLARGE*10, TargetMap.center()+COORD_ENLARGE*10);
    else
        p->Viewport = TargetMap;
    if (TEST_RFLAGS(RendererOptions::LockZoom) && theDocument) {
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
    CoordBox targetVp;
    if (TargetMap.latDiff() == 0 || TargetMap.lonDiff() == 0)
        targetVp = CoordBox (TargetMap.center()-COORD_ENLARGE*10, TargetMap.center()+COORD_ENLARGE*10);
    else
        targetVp = TargetMap;
    transformCalc(p->theTransform, theProjection, targetVp, Screen);
    viewportRecalc(Screen);

    p->NodeWidth = p->PixelPerM * M_PREFS->getNodeSize();
    if (p->NodeWidth > M_PREFS->getNodeSize())
        p->NodeWidth = M_PREFS->getNodeSize();
    p->ZoomLevel = p->theTransform.m11();

    if (TEST_RFLAGS(RendererOptions::LockZoom) && theDocument) {
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
    // Sensible limits on zoom range (circular scroll on touchpads can scroll
    // very fast).
    if (p->PixelPerM * d > 100 && d > 1.0)
        return;
    if (p->PixelPerM * d < 1e-5 && d < 1.0)
        return;

    qreal z = d;
    if (TEST_RFLAGS(RendererOptions::LockZoom)) {
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
    if (TEST_RFLAGS(RendererOptions::LockZoom)) {
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

    p->NodeWidth = p->PixelPerM * M_PREFS->getNodeSize();
    if (p->NodeWidth > M_PREFS->getNodeSize())
        p->NodeWidth = M_PREFS->getNodeSize();

    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        ImgIt.get()->zoom(d, Around, Screen);

    p->ZoomLevel = ScaleLon;

//    QPointF pt = theProjection.project(Coord(0, angToInt(180)));
//    double earthWidth = pt.x() * 2;
//    double zoomPixPerMat0 = 512. / earthWidth;
//    double z = 0;
//    p->AbstractZoomLevel = 0;
//    for (;z<p->theTransform.m11(); ++p->AbstractZoomLevel) {
//        double zoomPixPerMatCur = zoomPixPerMat0 * pow(2., p->AbstractZoomLevel);
//        z = zoomPixPerMatCur / p->PixelPerM;
//    }
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

double MapView::nodeWidth() const
{
    return p->NodeWidth;
}

RendererOptions MapView::renderOptions()
{
    return p->ROptions;
}

void MapView::setRenderOptions(const RendererOptions &opt)
{
    p->ROptions = opt;
}

QString MapView::toPropertiesHtml()
{
    QString h;

    h += "<big><strong>" + tr("View") + "</strong></big><hr/>";
    h += "<u>" + tr("Bounding Box") + "</u><br/>";
    h += QString("%1, %2, %3, %4 (%5, %6, %7, %8)")
         .arg(QString::number(coordToAng(viewport().bottomLeft().lon()),'f',4))
         .arg(QString::number(coordToAng(viewport().bottomLeft().lat()),'f',4))
         .arg(QString::number(coordToAng(viewport().topRight().lon()),'f',4))
         .arg(QString::number(coordToAng(viewport().topRight().lat()),'f',4))
         .arg(Coord2Sexa(viewport().bottomLeft().lon()))
         .arg(Coord2Sexa(viewport().bottomLeft().lat()))
         .arg(Coord2Sexa(viewport().topRight().lon()))
         .arg(Coord2Sexa(viewport().topRight().lat()))
         ;
    h += "<br/>";
    h += "<u>" + tr("Projection") + "</u><br/>";
    h += theProjection.getProjectionType();
    h += "";

    return h;
}
