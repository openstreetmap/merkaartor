#include "Global.h"

#include <errno.h>

#include "MapView.h"
#include "MainWindow.h"
#ifndef _MOBILE
#include "ui_MainWindow.h"
#endif
#include "PropertiesDock.h"
#include "Document.h"
#include "Layer.h"
#include "ImageMapLayer.h"
#include "IMapAdapter.h"
#include "IMapWatermark.h"
#include "Feature.h"
#include "Relation.h"
#include "Interaction.h"
#include "EditInteraction.h"
#include "CreateSingleWayInteraction.h"
#include "CreateNodeInteraction.h"
#include "CreateAreaInteraction.h"
#include "MoveNodeInteraction.h"
#include "ExtrudeInteraction.h"
#include "MasPaintStyle.h"
#include "Projection.h"
#include "qgps.h"
#include "qgpsdevice.h"
#include "LayerIterator.h"

#include "MapRenderer.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#ifdef USE_WEBKIT
    #include "browserimagemanager.h"
#endif
#include "MerkaartorPreferences.h"
#include "SvgCache.h"

#include <QTime>
#include <QMainWindow>
#include <QMouseEvent>
#include <QPainter>
#include <QStatusBar>
#include <QToolTip>
#include <QMap>
#include <QSet>
#include <QtConcurrentMap>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define LAT_ANG_PER_M 1.0 / EQUATORIALRADIUS
#define TEST_RFLAGS(x) p->ROptions.options.testFlag(x)

class MapViewPrivate
{
public:
    QTransform theTransform;
    QTransform theInvertedTransform;
    qreal PixelPerM;
    qreal NodeWidth;
    qreal ZoomLevel;
//    int AbstractZoomLevel;
    CoordBox Viewport;
    QList<CoordBox> invalidRects;
    QPoint theVectorPanDelta;
    qreal theVectorRotation;
    QMap<RenderPriority, QSet <Feature*> > theFeatures;
    QList<Node*> theVirtualNodes;
    MapRenderer renderer;
    RendererOptions ROptions;

    bool BackgroundOnlyPanZoom;
    QTransform BackgroundOnlyVpTransform;

    QLabel *TL, *TR, *BL, *BR;

    MapViewPrivate()
      : PixelPerM(0.0), Viewport(WORLD_COORDBOX), theVectorRotation(0.0)
      , BackgroundOnlyPanZoom(false)
    {}
};

/****************/

MapView::MapView(QWidget* parent) :
    QWidget(parent), Main(dynamic_cast<MainWindow*>(parent)), theDocument(0), theInteraction(0), StaticBackground(0), StaticBuffer(0),
        SelectionLocked(false),lockIcon(0), numImages(0),
        p(new MapViewPrivate)
{
    setMouseTracking(true);
    setAttribute(Qt::WA_NoSystemBackground);
    setContextMenuPolicy(Qt::CustomContextMenu);
    setFocusPolicy(Qt::ClickFocus);
#ifdef GEOIMAGE
    setAcceptDrops(true);
#endif

    MoveRightShortcut = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(MoveRightShortcut, SIGNAL(activated()), this, SLOT(on_MoveRight_activated()));
    MoveLeftShortcut = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(MoveLeftShortcut, SIGNAL(activated()), this, SLOT(on_MoveLeft_activated()));
    MoveUpShortcut = new QShortcut(QKeySequence(Qt::Key_Up), this);
    connect(MoveUpShortcut, SIGNAL(activated()), this, SLOT(on_MoveUp_activated()));
    MoveDownShortcut = new QShortcut(QKeySequence(Qt::Key_Down), this);
    connect(MoveDownShortcut, SIGNAL(activated()), this, SLOT(on_MoveDown_activated()));
    ZoomInShortcut = new QShortcut(QKeySequence(Qt::Key_PageUp), this);
    ZoomInShortcut->setContext(Qt::WidgetShortcut);
    connect(ZoomInShortcut, SIGNAL(activated()), this, SLOT(zoomIn()));
    ZoomOutShortcut = new QShortcut(QKeySequence(Qt::Key_PageDown), this);
    ZoomOutShortcut->setContext(Qt::WidgetShortcut);
    connect(ZoomOutShortcut, SIGNAL(activated()), this, SLOT(zoomOut()));

    QVBoxLayout* vlay = new QVBoxLayout(this);

    QHBoxLayout* hlay1 = new QHBoxLayout();
    p->TL = new QLabel(this);
    hlay1->addWidget(p->TL);
    QSpacerItem* horizontalSpacer = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlay1->addItem(horizontalSpacer);
    p->TR = new QLabel(this);
    hlay1->addWidget(p->TR);
    vlay->addLayout(hlay1);

    QSpacerItem* verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    vlay->addItem(verticalSpacer);

    QHBoxLayout* hlay2 = new QHBoxLayout();
    p->BL = new QLabel(this);
    hlay2->addWidget(p->BL);
    QSpacerItem* horizontalSpacer2 = new QSpacerItem(40, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);
    hlay2->addItem(horizontalSpacer2);
    p->BR = new QLabel(this);
    hlay2->addWidget(p->BR);
    vlay->addLayout(hlay2);

    p->TL->setVisible(false);
    p->TR->setVisible(false);
    p->BL->setVisible(false);
    p->BR->setVisible(false);
}

MapView::~MapView()
{
    if(theInteraction)
        delete theInteraction;
    delete StaticBackground;
    delete StaticBuffer;
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
    if (theDocument) {
        connect(aDoc, SIGNAL(imageRequested(ImageMapLayer*)),
                this, SLOT(on_imageRequested(ImageMapLayer*)), Qt::QueuedConnection);
        connect(aDoc, SIGNAL(imageReceived(ImageMapLayer*)),
                this, SLOT(on_imageReceived(ImageMapLayer*)), Qt::QueuedConnection);
        connect(aDoc, SIGNAL(loadingFinished(ImageMapLayer*)),
                this, SLOT(on_loadingFinished(ImageMapLayer*)), Qt::QueuedConnection);
    }

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
        SAFE_DELETE(StaticBackground)
    }
    if (theDocument && updateMap) {
        IMapWatermark* WatermarkAdapter = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            if (ImgIt.get()->isVisible()) {
                ImgIt.get()->forceRedraw(*this, p->BackgroundOnlyVpTransform, rect());
                WatermarkAdapter = qobject_cast<IMapWatermark*>(ImgIt.get()->getMapAdapter());
            }
        }
        p->BackgroundOnlyVpTransform = QTransform();

        if (WatermarkAdapter) {
            p->TL->setAttribute(Qt::WA_NoMousePropagation);
            p->TL->setOpenExternalLinks(true);
            p->TL->setText(WatermarkAdapter->getLogoHtml());
    //        p->lblLogo->move(10, 10);
            p->TL->show();

            p->BR->setAttribute(Qt::WA_NoMousePropagation);
            p->BR->setOpenExternalLinks(true);
            p->BR->setWordWrap(true);
            p->BR->setText(WatermarkAdapter->getAttributionsHtml(p->Viewport, rect()));
            p->BR->setMinimumWidth(150);
            p->BR->setMaximumWidth(200);
            p->BR->setMaximumHeight(50);
            p->BR->show();
        } else {
            p->TL->setVisible(false);
            p->BR->setVisible(false);
        }
    }
    update();
}

void MapView::panScreen(QPoint delta)
{
    Coord cDelta = fromView(delta) - fromView(QPoint(0, 0));

    if (p->BackgroundOnlyPanZoom)
        p->BackgroundOnlyVpTransform.translate(-cDelta.x(), -cDelta.y());
    else {
        p->theVectorPanDelta += delta;

        CoordBox r1, r2;
        if (delta.x()) {
            if (delta.x() < 0)
                r1 = CoordBox(p->Viewport.bottomRight(), Coord(p->Viewport.topRight().x() - cDelta.x(), p->Viewport.topRight().y())); // OK
            else
                r1 = CoordBox(Coord(p->Viewport.bottomLeft().x() - cDelta.x(), p->Viewport.bottomLeft().y()), p->Viewport.topLeft()); // OK
            p->invalidRects.push_back(r1);
        }
        if (delta.y()) {
            if (delta.y() < 0)
                r2 = CoordBox(Coord(p->Viewport.bottomLeft().x(), p->Viewport.bottomLeft().y() - cDelta.y()), p->Viewport.bottomRight()); // OK
            else
                r2 = CoordBox(p->Viewport.topLeft(), Coord( p->Viewport.bottomRight().x(), p->Viewport.topRight().y() - cDelta.y())); //NOK
            p->invalidRects.push_back(r2);
        }


        //qDebug() << "Inv rects size: " << p->invalidRects.size();
        //    qDebug() << "delta: " << delta;
        //qDebug() << "r1 : " << p->theTransform.map(theProjection.project(r1.topLeft())) << ", " << p->theTransform.map(theProjection.project(r1.bottomRight()));
        //qDebug() << "r2 : " << p->theTransform.map(theProjection.project(r2.topLeft())) << ", " << p->theTransform.map(theProjection.project(r2.bottomRight()));

        p->theTransform.translate(qreal(delta.x())/p->theTransform.m11(), qreal(delta.y())/p->theTransform.m22());
        p->theInvertedTransform = p->theTransform.inverted();
        viewportRecalc(rect());
    }

    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        ImgIt.get()->pan(delta);
    update();
}

void MapView::rotateScreen(QPoint center, qreal angle)
{
    p->theVectorRotation += angle;

    transformCalc(p->theTransform, theProjection, p->theVectorRotation, p->Viewport, rect());
    p->theInvertedTransform = p->theTransform.inverted();
    viewportRecalc(rect());
    p->invalidRects.clear();
    p->invalidRects.push_back(p->Viewport);

//    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
//        ImgIt.get()->pan(delta);
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

    updateStaticBackground();

    P.drawPixmap(p->theVectorPanDelta, *StaticBackground);
    P.save();
    QTransform AlignTransform;
    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
        if (ImgIt.get()->isVisible()) {
            ImgIt.get()->drawImage(&P);
            AlignTransform = ImgIt.get()->getCurrentAlignmentTransform();
        }
    }
    P.restore();

    if (!p->invalidRects.isEmpty()) {
        updateStaticBuffer();
    }
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

#ifndef _MOBILE
    if (Main) {
        QString vpLabel = QString("%1,%2,%3,%4")
                                           .arg(viewport().bottomLeft().x(),0,'f',4)
                                           .arg(viewport().bottomLeft().y(),0, 'f',4)
                                           .arg(viewport().topRight().x(),0, 'f',4)
                                           .arg(viewport().topRight().y(),0,'f',4)
                                           ;
        if (!theProjection.projIsLatLong()) {
            QRectF pVp = theProjection.toProjectedRectF(viewport(), rect());
            vpLabel += " / " + QString("%1,%2,%3,%4")
                    .arg(pVp.bottomLeft().x(),0,'f',4)
                    .arg(pVp.bottomLeft().y(),0, 'f',4)
                    .arg(pVp.topRight().x(),0, 'f',4)
                    .arg(pVp.topRight().y(),0,'f',4)
                    ;
        }
        Main->ViewportStatusLabel->setText(vpLabel);

        Main->MeterPerPixelLabel->setText(tr("%1 m/pixel").arg(1/p->PixelPerM, 0, 'f', 2));
        if (!AlignTransform.isIdentity()) {
            QLineF l(0, 0, AlignTransform.dx(), AlignTransform.dy());
            l.translate(viewport().center());
            Main->AdjusmentMeterLabel->setVisible(true);
            qreal distance = Coord(l.p2()).distanceFrom(Coord(l.p1()))*1000;
            Main->AdjusmentMeterLabel->setText(tr("Align: %1m @ %2").arg(distance, 0, 'f', 2).arg(l.angle(), 0, 'f', 2) + QString::fromUtf8("°"));
        } else {
            Main->AdjusmentMeterLabel->setVisible(false);
        }
#ifndef NDEBUG
        QTime Stop(QTime::currentTime());
        Main->PaintTimeLabel->setText(tr("%1ms").arg(Start.msecsTo(Stop)));
#endif
    }
#endif
}

void MapView::drawScale(QPainter & P)
{

    if (!TEST_RFLAGS(RendererOptions::ScaleVisible))
        return;

    errno = 0;
    qreal Log = log10(200./p->PixelPerM);
    if (errno != 0)
        return;

    qreal RestLog = Log-floor(Log);
    if (RestLog < log10(2.))
        Log = floor(Log);
    else if (RestLog < log10(5.))
        Log = floor(Log)+log10(2.);
    else
        Log = floor(Log)+log10(5.);

    qreal Length = pow(10.,Log);
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
    if (Main->gps() && Main->gps()->getGpsDevice()) {
        if (Main->gps()->getGpsDevice()->fixStatus() == QGPSDevice::StatusActive) {
            Coord vp(Main->gps()->getGpsDevice()->longitude(), Main->gps()->getGpsDevice()->latitude());
            QPoint g = toView(vp);
            QPixmap* pm = getPixmapFromFile(":/Gps/Gps_Marker.svg", 32);
            P.drawPixmap(g - QPoint(16, 16), *pm);
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

#define PARALLEL_LINES_NUM 5
#define MEDIAN_LINES_NUM 5

void MapView::drawLatLonGrid(QPainter & P)
{
    if (!TEST_RFLAGS(RendererOptions::LatLonGridVisible))
        return;

    QPointF origin(0., 0.);
    QPoint p1 = toView(origin);
    QPointF p2 = fromView(QPoint(p1.x()+width(), p1.y()-height()));
    CoordBox adjViewport(origin, p2);
    qreal lonInterval = adjViewport.lonDiff() / 4;
    qreal latInterval = adjViewport.latDiff() / 4;

    int prec = log10(lonInterval);
    if (!lonInterval || !latInterval) return; // avoid divide-by-zero
    qreal lonStart = qMax(int((p->Viewport.bottomLeft().x() - origin.x()) / lonInterval) * lonInterval, -COORD_MAX);
    if (lonStart != -COORD_MAX) {
        lonStart -= origin.x();
        if (lonStart<1)
            lonStart -= lonInterval;
    }
    qreal latStart = qMax(int(p->Viewport.bottomLeft().y() / latInterval) * latInterval, -COORD_MAX/2);
    if (latStart != -COORD_MAX/2) {
        latStart -= origin.y();
        if (latStart<1)
            latStart -= lonInterval;
    }

    QList<QPolygonF> medianLines;
    QList<QPolygonF> parallelLines;

    for (qreal y=latStart; y<=p->Viewport.topLeft().y()+latInterval; y+=latInterval) {
        QPolygonF l;
        for (qreal x=lonStart; x<=p->Viewport.bottomRight().x()+lonInterval; x+=lonInterval) {
            QPointF pt = theProjection.project(Coord(qMin(x, COORD_MAX), qMin(y, COORD_MAX/2)));
            l << pt;
        }
        parallelLines << l;
    }
    for (qreal x=lonStart; x<=p->Viewport.bottomRight().x()+lonInterval; x+=lonInterval) {
        QPolygonF l;
        for (qreal y=latStart; y<=p->Viewport.topLeft().y()+latInterval; y+=latInterval) {
            QPointF pt = theProjection.project(Coord(qMin(x, COORD_MAX), qMin(y, COORD_MAX/2)));
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
        P.drawText(ptt, QString("%1").arg(theProjection.inverse2Coord(parallelLines.at(i).at(0)).y(), 0, 'f', 2-prec));
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
        P.drawText(ptt, QString("%1").arg(theProjection.inverse2Coord(medianLines.at(i).at(0)).x(), 0, 'f', 2-prec));
    }

    P.restore();
}

void MapView::drawFeatures(QPainter & P)
{
    QRectF clipRect = p->theInvertedTransform.mapRect(QRectF(rect().adjusted(-200, -200, 200, 200)));

    for (int i=0; i<theDocument->layerSize(); ++i)
        g_backend.getFeatureSet(theDocument->getLayer(i), p->theFeatures, p->invalidRects, theProjection);

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
        else if (M_PREFS->getBackgroundOverwriteStyle())
            StaticBackground->fill(M_PREFS->getBgColor());
        else if (M_STYLE->getGlobalPainter().getDrawBackground())
            StaticBackground->fill(M_STYLE->getGlobalPainter().getBackgroundColor());
        else
            StaticBackground->fill(Qt::white);
    }
}

void MapView::updateStaticBuffer()
{
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
        P.setClipping(true);
        P.setClipRegion(rect());
    }

    if (!p->invalidRects.isEmpty()) {
//        P.setRenderHint(QPainter::Antialiasing);
//        P.setClipping(true);
//        P.setClipRegion(QRegion(rect()));
        if (M_PREFS->getUseAntiAlias())
            if (p->invalidRects[0] == p->Viewport || M_PREFS->getAntiAliasWhilePanning())
                P.setRenderHint(QPainter::Antialiasing);
        drawFeatures(P);
        P.end();
    }

    p->invalidRects.clear();
    p->theVectorPanDelta = QPoint(0, 0);
}

void MapView::mousePressEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (theInteraction) {
        if ((anEvent->modifiers() & Qt::AltModifier) || dynamic_cast<ExtrudeInteraction*>(theInteraction))
            g_Merk_Segment_Mode = true;
        else
            g_Merk_Segment_Mode = false;


        theInteraction->updateSnap(anEvent);
        if (Main && Main->info())
            Main->info()->setHtml(theInteraction->toHtml());

        if (anEvent->button())
            theInteraction->mousePressEvent(anEvent);
    }
}

void MapView::mouseReleaseEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (theInteraction) {
        if ((anEvent->modifiers() & Qt::AltModifier) || dynamic_cast<ExtrudeInteraction*>(theInteraction))
            g_Merk_Segment_Mode = true;
        else
            g_Merk_Segment_Mode = false;


        theInteraction->updateSnap(anEvent);
        theInteraction->mouseReleaseEvent(anEvent);
    }
}

void MapView::mouseMoveEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (!updatesEnabled())
        return;

    if (theInteraction) {
        if ((anEvent->modifiers() & Qt::AltModifier) || dynamic_cast<ExtrudeInteraction*>(theInteraction))
            g_Merk_Segment_Mode = true;
        else
            g_Merk_Segment_Mode = false;

        theInteraction->updateSnap(anEvent);

        if (!M_PREFS->getSeparateMoveMode()) {
            EditInteraction* EI = dynamic_cast<EditInteraction*>(theInteraction);
            if (EI && EI->isIdle()) {
                if (EI->lastSnap() && Main && Main->properties()->isSelected(EI->lastSnap())) {
                    MoveNodeInteraction* MI = new MoveNodeInteraction(this);
                    launch(MI);
//                    main()->info()->setHtml(interaction()->toHtml());
#ifndef _MOBILE
                    setCursor(MI->cursor());
#endif
                    update();
                    return;
                }
            }
            MoveNodeInteraction* MI = dynamic_cast<MoveNodeInteraction*>(theInteraction);
            if (MI && !MI->lastSnap() && MI->isIdle()) {
                EditInteraction* EI = new EditInteraction(this);
                launch(EI);
//                main()->info()->setHtml(interaction()->toHtml());
#ifndef _MOBILE
                setCursor(EI->cursor());
#endif
                update();
                return;
            }

        }

        theInteraction->mouseMoveEvent(anEvent);
    }
}

void MapView::mouseDoubleClickEvent(QMouseEvent* anEvent)
{
    if (!document())
        return;

    if (!updatesEnabled())
        return;

    if (theInteraction) {
        if ((anEvent->modifiers() & Qt::AltModifier) || dynamic_cast<ExtrudeInteraction*>(theInteraction))
            g_Merk_Segment_Mode = true;
        else
            g_Merk_Segment_Mode = false;

        theInteraction->updateSnap(anEvent);

        if (M_PREFS->getSelectModeCreation()) {
            MoveNodeInteraction* MI = NULL;
            if (!M_PREFS->getSeparateMoveMode()) {
                MI = dynamic_cast<MoveNodeInteraction*>(theInteraction);
            }
            EditInteraction* EI = dynamic_cast<EditInteraction*>(theInteraction);
            if ((EI && EI->isIdle()) || (MI && MI->isIdle())) {
                if ((theInteraction->lastSnap() && theInteraction->lastSnap()->getType() & IFeature::LineString) || !theInteraction->lastSnap())
                    CreateNodeInteraction::createNode(fromView(anEvent->pos()), theInteraction->lastSnap());
                else if (theInteraction->lastSnap() && theInteraction->lastSnap()->getType() == IFeature::Point) {
                    Node* N = CAST_NODE(theInteraction->lastSnap());
                    CreateSingleWayInteraction* CI = new CreateSingleWayInteraction(main(), this, N, false);
                    N->invalidatePainter();
                    launch(CI);
                    main()->info()->setHtml(interaction()->toHtml());
#ifndef _MOBILE
                    setCursor(CI->cursor());
#endif
                    update();
                    return;
                }
            }
        }
        theInteraction->mouseDoubleClickEvent(anEvent);
    }
}

void MapView::wheelEvent(QWheelEvent* anEvent)
{
    if (!document())
        return;

    if (theInteraction) {
        theInteraction->wheelEvent(anEvent);
    }
}

void MapView::launch(Interaction* anInteraction)
{
    EditInteraction* EI = dynamic_cast<EditInteraction*>(theInteraction);
    if (EI)
        theSnapList = EI->snapList();
    if (!theSnapList.size())
        if (Main && Main->properties())
            theSnapList = Main->properties()->selection();
    if (theInteraction)
        delete theInteraction;
    theInteraction = anInteraction;
    EI = dynamic_cast<EditInteraction*>(theInteraction);
    if (theInteraction) {
        emit interactionChanged(anInteraction);
        if (EI)
            EI->setSnap(theSnapList);
    } else {
#ifndef _MOBILE
        setCursor(QCursor(Qt::ArrowCursor));
#endif
        launch(defaultInteraction());
        //Q_ASSERT(theInteraction);
    }
}

Interaction *MapView::defaultInteraction()
{
    return new EditInteraction(this);
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

QTransform& MapView::invertedTransform()
{
    return p->theInvertedTransform;
}

QPoint MapView::toView(const Coord& aCoord) const
{
    return p->theTransform.map(theProjection.project(aCoord)).toPoint();
}

QPoint MapView::toView(Node* aPt) const
{
    return p->theTransform.map(theProjection.project(aPt)).toPoint();
}

Coord MapView::fromView(const QPoint& aPt) const
{
    return theProjection.inverse2Coord(p->theInvertedTransform.map(QPointF(aPt)));
}

void MapView::on_customContextMenuRequested(const QPoint & pos)
{
#ifndef _MOBILE
    if (!Main)
        return;

    if (/*EditInteraction* ei = */dynamic_cast<EditInteraction*>(theInteraction) || dynamic_cast<MoveNodeInteraction*>(theInteraction)) {
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
#endif
}


void MapView::on_imageRequested(ImageMapLayer*)
{
    if (Main) {
#ifndef _MOBILE
        ++numImages;
        Main->pbImages->setRange(0, numImages);
        //pbImages->setValue(0);
        Main->pbImages->update();
        if (Main->pbImages->value() < 0)
            Main->pbImages->setValue(0);
#endif
    }
}

void MapView::on_imageReceived(ImageMapLayer* aLayer)
{
    if (Main) {
#ifndef _MOBILE
        if (Main->pbImages->value() < Main->pbImages->maximum())
            Main->pbImages->setValue(Main->pbImages->value()+1);
#endif
    }
    aLayer->forceRedraw(*this, p->BackgroundOnlyVpTransform, rect());
    p->BackgroundOnlyVpTransform = QTransform();
    update();
}

void MapView::on_loadingFinished(ImageMapLayer* aLayer)
{
    Q_UNUSED(aLayer)
    numImages = 0;
#ifndef _MOBILE
    if (Main)
        Main->pbImages->reset();
#endif
}

void MapView::resizeEvent(QResizeEvent * ev)
{
    viewportRecalc(QRect(QPoint(0,0), ev->size()));

    QWidget::resizeEvent(ev);

    if (!StaticBuffer || (StaticBuffer->size() != size()))
    {
        delete StaticBuffer;
        StaticBuffer = new QPixmap(size());
    }

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
        QList<Feature*> NoSnap;
        if ((tP = CAST_NODE(it.get())) && tP->pixelDistance(event->pos(), 5.01, NoSnap, this) < 5.01) {
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
        Coord mapC = fromView(event->pos());
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

bool MapView::toXML(QXmlStreamWriter& stream)
{
    bool OK = true;

    stream.writeStartElement("MapView");
    viewport().toXML("Viewport", stream);
    theProjection.toXML(stream);
    stream.writeEndElement();

    return OK;
}

void MapView::fromXML(QXmlStreamReader& stream)
{
    CoordBox cb;
    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "Viewport") {
            cb = CoordBox::fromXML(stream);
        } else if (stream.name() == "Projection") {
            theProjection.fromXML(stream);
        }
        stream.readNext();
    }

    if (!cb.isNull())
        setViewport(cb, rect());
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

void MapView::zoomIn()
{
    zoom(M_PREFS->getZoomIn()/100., rect().center());
    invalidate(true, true);
}

void MapView::zoomOut()
{
    zoom(M_PREFS->getZoomOut()/100., rect().center());
    invalidate(true, true);
}

bool MapView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::ToolTip: {
            QHelpEvent *helpEvent = static_cast<QHelpEvent *>(event);
            //Coord p = theProjection.inverse(helpEvent->pos());
            if (M_PREFS->getMapTooltip()) {
                if (!toolTip().isEmpty())
                    QToolTip::showText(helpEvent->globalPos(), toolTip());
                else
                    QToolTip::hideText();
            }
            return true;
        }

    case QEvent::KeyPress: {
            QKeyEvent *ke = static_cast< QKeyEvent* >( event );
            switch ( ke->key() ) {
            case Qt::Key_Space:
                ke->accept();
                p->BackgroundOnlyPanZoom = true;
                return true;

            case Qt::Key_Tab:
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

            case Qt::Key_Backtab:
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

            case Qt::Key_T:
                {
                    rotateScreen(rect().center(), 15.);
                }

            case Qt::Key_O:
                {
                    CreateSingleWayInteraction* intr = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (!intr)
                        return false;

                    setFocus();
                    ke->accept();
                    intr->setSnapAngle(45.);

                    return true;
                }

            case Qt::Key_H:
                {
                    CreateSingleWayInteraction* intr = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (!intr)
                        return false;

                    setFocus();
                    ke->accept();
                    intr->setSnapAngle(30.);

                    return true;
                }

            case Qt::Key_P:
                {
                    CreateSingleWayInteraction* intr = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (!intr)
                        return false;

                    setFocus();
                    ke->accept();
                    intr->setParallelMode(true);

                    return true;
                }

            case Qt::Key_C:
                {
                    CreateSingleWayInteraction* CI = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (CI) {
                        setFocus();
                        ke->accept();
                        CI->closeAndFinish();
                    } else {
                        CreateAreaInteraction* AI = dynamic_cast<CreateAreaInteraction*>(interaction());
                        if (AI) {
                            setFocus();
                            ke->accept();
                            AI->closeAndFinish();
                        }
                        else
                            return false;
                    }
                    return true;
                }

            default:
                break;

            }
        }

    case QEvent::KeyRelease: {
            QKeyEvent *ke = static_cast< QKeyEvent* >( event );
            switch ( ke->key() ) {
            case Qt::Key_Space:
                ke->accept();
                p->BackgroundOnlyPanZoom = false;
                return true;

            case Qt::Key_O:
            case Qt::Key_H:
                {
                    CreateSingleWayInteraction* intr = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (!intr)
                        return false;

                    ke->accept();
                    intr->setSnapAngle(0);

                    return true;
                }

            case Qt::Key_P:
                {
                    CreateSingleWayInteraction* intr = dynamic_cast<CreateSingleWayInteraction*>(interaction());
                    if (!intr)
                        return false;

                    ke->accept();
                    intr->setParallelMode(false);

                    return true;
                }

            default:
                break;
            }
        }

    case QEvent::Leave: {
            if (Main && Main->info())
                Main->info()->unsetHoverHtml();
            FeatureSnapInteraction* intr = dynamic_cast<FeatureSnapInteraction*>(interaction());
            if (intr)
                intr->clearLastSnap();
            update();
        }

    default:
        break;
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
#ifndef _MOBILE
        lockIcon = new QLabel(this);
        lockIcon->setPixmap(QPixmap(":/Icons/emblem-readonly.png"));
        Main->statusBar()->clearMessage();
        Main->statusBar()->addWidget(lockIcon);
#endif
        SelectionLocked = true;
    }
}

void MapView::unlockSelection()
{
    if (!Main)
        return;

    if (SelectionLocked) {
#ifndef _MOBILE
        Main->statusBar()->removeWidget(lockIcon);
        SAFE_DELETE(lockIcon)
#endif
        SelectionLocked = false;
    }
}

const CoordBox& MapView::viewport() const
{
    return p->Viewport;
}

void MapView::viewportRecalc(const QRect & Screen)
{
    Coord bl = fromView(Screen.bottomLeft());
    Coord br = fromView(Screen.bottomRight());
    Coord tr = fromView(Screen.topRight());
    Coord tl = fromView(Screen.topLeft());
//    qDebug() << bl.toQPointF() << br.toQPointF() << tr.toQPointF() << tl.toQPointF();
    qreal t = qMax(tr.y(), tl.y());
    qreal b = qMin(br.y(), bl.y());
    qreal l = qMin(tl.x(), bl.x());
    qreal r = qMax(tr.x(), br.x());
    p->Viewport = CoordBox(Coord(l, b), Coord(r, t));

    // measure geographical distance between mid left and mid right of the screen
    int mid = (Screen.topLeft().y() + Screen.bottomLeft().y()) / 2;
    Coord left = fromView(QPoint(Screen.left(), mid));
    Coord right = fromView(QPoint(Screen.right(), mid));
    p->PixelPerM = Screen.width() / (left.distanceFrom(right)*1000);

    emit viewportChanged();
}

void MapView::transformCalc(QTransform& theTransform, const Projection& theProjection, const qreal& theRotation, const CoordBox& TargetMap, const QRect& screen)
{
    QRectF pViewport = theProjection.toProjectedRectF(TargetMap, screen);
//    QPointF pCenter(pViewport.center());

    qreal Aspect = (double)screen.width() / screen.height();
    qreal pAspect = fabs(pViewport.width() / pViewport.height());

    qreal wv, hv;
    if (pAspect > Aspect) {
        wv = fabs(pViewport.width());
        hv = fabs(pViewport.height() * pAspect / Aspect);
    } else {
        wv = fabs(pViewport.width() * Aspect / pAspect);
        hv = fabs(pViewport.height());
    }

    qreal ScaleLon = screen.width() / wv;
    qreal ScaleLat = screen.height() / hv;

//    qreal PLon = pCenter.x() /* * ScaleLon*/;
//    qreal PLat = pCenter.y() /* * ScaleLat*/;
//    qreal DeltaLon = Screen.width() / 2 - PLon;
//    qreal DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);

//    theTransform.setMatrix(ScaleLon, 0, 0, 0, -ScaleLat, 0, DeltaLon, DeltaLat, 1);
    theTransform.reset();
    theTransform.scale(ScaleLon, -ScaleLat);
//    theTransform.rotate(theRotation, Qt::ZAxis);
    theTransform.translate(-pViewport.topLeft().x(), -pViewport.topLeft().y());
//    theTransform.translate(-pCenter.x(), -pCenter.y());
//    QLineF l(QPointF(0, 0), pCenter);
//    l.setAngle(l.angle()+theRotation);
//    qDebug() << "p2:" << l.p2();
//    theTransform.translate(l.p2().x(), l.p2().y());
//    theTransform.translate(Screen.width()/2, -Screen.height()/2);
//    theTransform.rotateRadians(theRotation);
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
            l->setCurrentZoom(*this, p->Viewport, rect());
            qreal pixpercoord = width() / p->Viewport.lonDiff();
            qreal z = l->pixelPerCoord() / pixpercoord;
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
    transformCalc(p->theTransform, theProjection, p->theVectorRotation, targetVp, Screen);
    p->theInvertedTransform = p->theTransform.inverted();
    viewportRecalc(Screen);

    GlobalPainter theGlobalPainter = M_STYLE->getGlobalPainter();
    if (theGlobalPainter.DrawNodes) {
        p->NodeWidth = p->PixelPerM*theGlobalPainter.NodesProportional+theGlobalPainter.NodesFixed;
    } else {
        p->NodeWidth = p->PixelPerM * M_PREFS->getNodeSize();
        if (p->NodeWidth > M_PREFS->getNodeSize())
            p->NodeWidth = M_PREFS->getNodeSize();
    }
    p->ZoomLevel = p->theTransform.m11();

    if (TEST_RFLAGS(RendererOptions::LockZoom) && theDocument) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            l->setCurrentZoom(*this, p->Viewport, rect());
            qreal pixpercoord = width() / p->Viewport.lonDiff();
            qreal z = l->pixelPerCoord() / pixpercoord;
            zoom(z, Screen.center(), Screen);
        }
    }
}

void MapView::zoom(qreal d, const QPoint & Around)
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
            qreal pixpercoord = width() / p->Viewport.lonDiff();
            z = l->pixelPerCoord() / pixpercoord;
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
            qreal pixpercoord = width() / p->Viewport.lonDiff();
            qreal z = l->pixelPerCoord() / pixpercoord;
            zoom(z, rect().center(), rect());
        }
    }
}

void MapView::zoom(qreal d, const QPoint & Around,
                             const QRect & Screen)
{
    QPointF pBefore = p->theInvertedTransform.map(QPointF(Around));

    qreal ScaleLon = p->theTransform.m11() * d;
    qreal ScaleLat = p->theTransform.m22() * d;
    qreal DeltaLat = (Around.y() - pBefore.y() * ScaleLat);
    qreal DeltaLon = (Around.x() - pBefore.x() * ScaleLon);

//    p->theTransform.setMatrix(ScaleLon*cos(p->theVectorRotation), 0, 0, 0, ScaleLat*cos(p->theVectorRotation), 0, DeltaLon, DeltaLat, 1);
    p->theTransform.reset();
    p->theTransform.scale(ScaleLon, ScaleLat);
//    p->theTransform.rotate(p->theVectorRotation, Qt::ZAxis);
    p->theTransform.translate(DeltaLon/ScaleLon, DeltaLat/ScaleLat);

    p->theInvertedTransform = p->theTransform.inverted();
    viewportRecalc(Screen);

    GlobalPainter theGlobalPainter = M_STYLE->getGlobalPainter();
    if (theGlobalPainter.DrawNodes) {
        p->NodeWidth = p->PixelPerM*theGlobalPainter.NodesProportional+theGlobalPainter.NodesFixed;
    } else {
        p->NodeWidth = p->PixelPerM * M_PREFS->getNodeSize();
        if (p->NodeWidth > M_PREFS->getNodeSize())
            p->NodeWidth = M_PREFS->getNodeSize();
    }

    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
        ImgIt.get()->zoom(d, Around, Screen);

    p->ZoomLevel = ScaleLon;

//    QPointF pt = theProjection.project(Coord(0, angToInt(180)));
//    qreal earthWidth = pt.x() * 2;
//    qreal zoomPixPerMat0 = 512. / earthWidth;
//    qreal z = 0;
//    p->AbstractZoomLevel = 0;
//    for (;z<p->theTransform.m11(); ++p->AbstractZoomLevel) {
//        qreal zoomPixPerMatCur = zoomPixPerMat0 * pow(2., p->AbstractZoomLevel);
//        z = zoomPixPerMatCur / p->PixelPerM;
//    }
    update();
}

void MapView::setCenter(Coord & Center, const QRect & /*Screen*/)
{
    Coord curCenter(p->Viewport.center());
    QPoint curCenterScreen = toView(curCenter);
    QPoint newCenterScreen = toView(Center);

    QPoint panDelta = (curCenterScreen - newCenterScreen);
    panScreen(panDelta);
}

qreal MapView::pixelPerM() const
{
    return p->PixelPerM;
}

qreal MapView::nodeWidth() const
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
         .arg(QString::number(viewport().bottomLeft().x()),'f',4)
         .arg(QString::number(viewport().bottomLeft().y()),'f',4)
         .arg(QString::number(viewport().topRight().x()),'f',4)
         .arg(QString::number(viewport().topRight().y()),'f',4)
         .arg(Coord2Sexa(viewport().bottomLeft().x()))
         .arg(Coord2Sexa(viewport().bottomLeft().y()))
         .arg(Coord2Sexa(viewport().topRight().x()))
         .arg(Coord2Sexa(viewport().topRight().y()))
         ;
    h += "<br/>";
    h += "<u>" + tr("Projection") + "</u><br/>";
    h += theProjection.getProjectionType();
    h += "";

    return h;
}
