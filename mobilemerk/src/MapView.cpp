#include "MapView.h"
#include "MouseMachine.h"

#include <QPainter>
#include <QShortcut>

#include <errno.h>

#ifdef USE_GOSMORE
#include "GosmoreAdapter.h"
#else
#include "NavitAdapter.h"
#endif

class MapViewPrivate
{
public:
    MapViewPrivate()
        : PixelPerM(0.0)
        , StaticBufferUpToDate(false)
        , StaticBuffer(0)
    {}

public:
    Projection theProjection;
    QTransform theTransform;

    MouseMachine* mm;
    QShortcut* MoveLeft;
    QShortcut* MoveRight;
    QShortcut* MoveUp;
    QShortcut* MoveDown;
    QShortcut* ZoomIn;
    QShortcut* ZoomOut;

    double PixelPerM;
    double ZoomLevel;
    CoordBox Viewport;
    QList<CoordBox> invalidRects;
    QPoint theVectorPanDelta;

    bool StaticBufferUpToDate;
    QPixmap* StaticBuffer;

#ifdef USE_GOSMORE
    GosmoreAdapter* theAdapter;
#else
    NavitAdapter* theAdapter;
#endif
};

MapView::MapView(QWidget *parent) :
    QWidget(parent)
{
    p = new MapViewPrivate;
    p->theProjection.setProjectionType("EPSG:3857");
    p->mm = new MouseMachine(this);
    connect(p->mm, SIGNAL(scroll(QPoint)), SLOT(panScreen(QPoint)));
//    connect(p->mm, SIGNAL(singleTap(QPoint)), SLOT(slotClicked(QPoint)));
//    connect(p->mm, SIGNAL(doubleTap(QPoint)), SLOT(slotDoubleClicked(QPoint)));
//    connect(p->mm, SIGNAL(tapAndHold(QPoint)), SLOT(slotTapAndHold(QPoint)));

#ifdef USE_GOSMORE
    p->theAdapter = new GosmoreAdapter();
    p->theAdapter->setFile("/home/cbro/Downloads/gosmore.pak");
#else
    p->theAdapter = new NavitAdapter();
    p->theAdapter->setFile("/home/cbro/Downloads/osm_bbox_2.3,49.4,6.6,51.6.bin");
#endif

    setMouseTracking(false);
    setAttribute(Qt::WA_NoSystemBackground);

    p->MoveRight = new QShortcut(QKeySequence(Qt::Key_Right), this);
    connect(p->MoveRight, SIGNAL(activated()), this, SLOT(on_MoveRight_activated()));
    p->MoveLeft = new QShortcut(QKeySequence(Qt::Key_Left), this);
    connect(p->MoveLeft, SIGNAL(activated()), this, SLOT(on_MoveLeft_activated()));
    p->MoveUp = new QShortcut(QKeySequence(Qt::Key_Up), this);
    connect(p->MoveUp, SIGNAL(activated()), this, SLOT(on_MoveUp_activated()));
    p->MoveDown = new QShortcut(QKeySequence(Qt::Key_Down), this);
    connect(p->MoveDown, SIGNAL(activated()), this, SLOT(on_MoveDown_activated()));
    p->ZoomIn = new QShortcut(QKeySequence("+"), this);
    connect(p->ZoomIn, SIGNAL(activated()), this, SLOT(on_ZoomIn_activated()));
    p->ZoomOut = new QShortcut(QKeySequence("-"), this);
    connect(p->ZoomOut, SIGNAL(activated()), this, SLOT(on_ZoomOut_activated()));
}

MapView::~MapView()
{
    delete p;
}

void MapView::on_MoveLeft_activated()
{
    QPoint p(rect().width()/4,0);
    panScreen(p);
}

void MapView::on_MoveRight_activated()
{
    QPoint p(-rect().width()/4,0);
    panScreen(p);
}

void MapView::on_MoveUp_activated()
{
    QPoint p(0,rect().height()/4);
    panScreen(p);
}

void MapView::on_MoveDown_activated()
{
    QPoint p(0,-rect().height()/4);
    panScreen(p);
}

void MapView::on_ZoomIn_activated()
{
    zoom(2.0, rect().center());
}

void MapView::on_ZoomOut_activated()
{
    zoom(0.5, rect().center());
}

void MapView::panScreen(QPoint delta)
{
    p->theVectorPanDelta += delta;

    CoordBox r1, r2;

    Coord cDelta = p->theProjection.inverse(p->theTransform.inverted().map(QPointF(delta)))  - p->theProjection.inverse(p->theTransform.inverted().map(QPointF(0., 0.)));
    p->invalidRects.clear();

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

    invalidate();
}

void MapView::invalidate()
{
    p->StaticBufferUpToDate = false;
    update();
}

void MapView::paintEvent(QPaintEvent * anEvent)
{
    QPainter P;
    P.begin(this);

    if (!p->StaticBufferUpToDate)
        updateStaticBuffer();
    P.drawPixmap(p->theVectorPanDelta, *p->StaticBuffer);

    drawScale(P);
    drawGPS(P);

    P.end();
}

void MapView::drawScale(QPainter & P)
{
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
//    if (Main->gps()->getGpsDevice()) {
//        if (Main->gps()->getGpsDevice()->fixStatus() == QGPSDevice::StatusActive) {
//            Coord vp(angToCoord(Main->gps()->getGpsDevice()->latitude()), angToCoord(Main->gps()->getGpsDevice()->longitude()));
//            QPointF g = p->theTransform.map(projection().project(vp));
//            QPixmap pm = getPixmapFromFile(":/Gps/Gps_Marker.svg", 32);
//            P.drawPixmap(g - QPoint(16, 16), pm);
//        }
//    }
}

QPoint MapView::toView(const Coord& aCoord) const
{
    return p->theTransform.map(p->theProjection.project(aCoord)).toPoint();
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

    viewport().toXML("Viewport", e);
    p->theProjection.toXML(e);

    return OK;
}

void MapView::fromXML(const QDomElement xParent)
{
    CoordBox cb;
    QDomElement e = xParent.firstChildElement();
    while(!e.isNull()) {
        if (e.tagName() == "Viewport") {
            cb = CoordBox::fromXML(e);
        }
        else if (e.tagName() == "Projection") {
            p->theProjection.fromXML(e);
        }

        e = e.nextSiblingElement();
    }

    if (!cb.isNull())
        setViewport(cb, rect());
}

CoordBox MapView::viewport() const
{
    return p->Viewport;
}

void MapView::viewportRecalc(const QRect & Screen)
{
    QRectF fScreen(Screen);
    Coord bl = p->theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomLeft()));
    Coord br = p->theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomRight()));
    Coord tr = p->theProjection.inverse(p->theTransform.inverted().map(fScreen.topRight()));
    Coord tl = p->theProjection.inverse(p->theTransform.inverted().map(fScreen.topLeft()));
    double t = qMax(tr.lat(), tl.lat());
    double b = qMin(br.lat(), bl.lat());
    double l = qMin(tl.lon(), bl.lon());
    double r = qMax(tr.lon(), br.lon());
    p->Viewport = CoordBox(Coord(b, l), Coord(t, r));

    if (p->theProjection.projIsLatLong()) {
        p->PixelPerM = Screen.width() / (double)p->Viewport.lonDiff() * p->theProjection.lonAnglePerM(coordToRad(p->Viewport.center().lat()));
    } else {
        // measure geographical distance between mid left and mid right of the screen
        int mid = (fScreen.topLeft().y() + fScreen.bottomLeft().y()) / 2;
        Coord left = p-> theProjection.inverse(p->theTransform.inverted().map(QPointF(fScreen.left(), mid)));
        Coord right = p->theProjection.inverse(p->theTransform.inverted().map(QPointF(fScreen.right(), mid)));
        p->PixelPerM = Screen.width() / (left.distanceFrom(right)*1000);
    }
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

    p->invalidRects.clear();
    p->StaticBufferUpToDate = false;
}

void MapView::setViewport(const CoordBox & TargetMap,
                                    const QRect & Screen)
{
    CoordBox targetVp;
    if (TargetMap.latDiff() == 0 || TargetMap.lonDiff() == 0)
        targetVp = CoordBox (TargetMap.center()-COORD_ENLARGE*10, TargetMap.center()+COORD_ENLARGE*10);
    else
        targetVp = TargetMap;
    transformCalc(p->theTransform, p->theProjection, targetVp, Screen);
    viewportRecalc(Screen);

    p->ZoomLevel = p->theTransform.m11();

    p->invalidRects.clear();
    p->StaticBufferUpToDate = false;
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
    zoom(z, Around, rect());
}

void MapView::zoom(double d, const QPoint & Around,
                             const QRect & Screen)
{
    Coord Before = p->theProjection.inverse(p->theTransform.inverted().map(QPointF(Around)));
    QPointF pBefore = p->theProjection.project(Before);

    double ScaleLon = p->theTransform.m11() * d;
    double ScaleLat = p->theTransform.m22() * d;
    double DeltaLat = (Around.y() - pBefore.y() * ScaleLat);
    double DeltaLon = (Around.x() - pBefore.x() * ScaleLon);

    p->theTransform.setMatrix(ScaleLon, 0, 0, 0, ScaleLat, 0, DeltaLon, DeltaLat, 1);
    viewportRecalc(Screen);

    p->ZoomLevel = ScaleLon;

    invalidate();
}

void MapView::resize(QSize oldS, QSize newS)
{
    Q_UNUSED(oldS)
    viewportRecalc(QRect(QPoint(0,0), newS));
}

void MapView::setCenter(Coord & Center, const QRect & /*Screen*/)
{
    Coord curCenter(p->Viewport.center());
    QPointF curCenterScreen = p->theTransform.map(p->theProjection.project(curCenter));
    QPointF newCenterScreen = p->theTransform.map(p->theProjection.project(Center));

    QPointF panDelta = (curCenterScreen - newCenterScreen);
    panScreen(panDelta.toPoint());
}

double MapView::pixelPerM() const
{
    return p->PixelPerM;
}

void MapView::updateStaticBuffer()
{
#ifndef QT_NO_DEBUG_OUTPUT
    QDateTime tm = QDateTime::currentDateTime();
#endif
    if (!p->StaticBuffer || (p->StaticBuffer->size() != size()))
    {
        delete p->StaticBuffer;
        p->StaticBuffer = new QPixmap(size());
    }

    QPainter P;

    if (!p->theVectorPanDelta.isNull()) {
        QRegion exposed;
        p->StaticBuffer->scroll(p->theVectorPanDelta.x(), p->theVectorPanDelta.y(), p->StaticBuffer->rect(), &exposed);
        P.begin(p->StaticBuffer);
        P.setClipping(true);
        P.setClipRegion(exposed);
        P.eraseRect(p->StaticBuffer->rect());
    } else {
        p->StaticBuffer->fill(Qt::transparent);
        P.begin(p->StaticBuffer);
    }
    P.setRenderHint(QPainter::Antialiasing);

    QRectF fullbox = p->Viewport.toRectF();
    if (!p->invalidRects.isEmpty()) {
        foreach (CoordBox cb, p->invalidRects) {
            QRectF selbox = cb.toRectF();
            p->theAdapter->render(&P, fullbox, selbox, rect());
        }
    } else {
        QRectF selbox = fullbox;
        p->theAdapter->render(&P, fullbox, selbox, rect());
    }
    P.end();

#ifndef QT_NO_DEBUG_OUTPUT
    int ms = tm.time().msecsTo(QDateTime::currentDateTime().time());
    qDebug() << "Paint buffer: " << ms << " msecs";
#endif

    p->invalidRects.clear();
    p->theVectorPanDelta = QPoint(0, 0);
    p->StaticBufferUpToDate = true;
}

