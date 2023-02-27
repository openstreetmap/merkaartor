#include "SlippyMapWidget.h"

#include <QFile>
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QWheelEvent>
#include <QMenu>

#include <math.h>

#include "MerkaartorPreferences.h"

#define TILESIZE 256
#define MINZOOMLEVEL 0
#define MAXZOOMLEVEL 19


/* SLIPPYMAPWIDGET */

class SlippyMapWidgetPrivate
{
    public:
        SlippyMapWidgetPrivate(SlippyMapWidget* w)
            : theWidget(w), InDrag(false), InSelection(false)
        {
            Sets = M_PREFS->getQSettings();
            Sets->beginGroup("SlippyMapWidget");
            Lat = Sets->value("Lat", 1).toDouble();
            Lon = Sets->value("Lon", 1).toDouble();
            Zoom = Sets->value("Zoom", 1).toInt();
            Sets->endGroup();
            VpLat = Lat;
            VpLon = Lon;
            VpZoom = Zoom;
        }
        ~SlippyMapWidgetPrivate()
        {
            Sets->beginGroup("SlippyMapWidget");
            Sets->setValue("Lat", Lat);
            Sets->setValue("Lon", Lon);
            Sets->setValue("Zoom", Zoom);
            Sets->endGroup();
        }

        QPixmap* getImage(int x, int y);
        void newData(int x, int y, int zoom);

        SlippyMapWidget* theWidget;
        int Zoom, VpZoom;
        qreal Lat,Lon, VpLat, VpLon;
        QPoint PreviousDrag;
        /* Used only during selection */
        bool InSelection;
        QPoint SelectionStart, SelectionEnd;

        /* Current selection in coordinates */
        QRectF CurrentSelectionCoord;

        bool InDrag;
        QSettings* Sets;
};

QPixmap* SlippyMapWidgetPrivate::getImage(int x, int y)
{
    int Max = 1 << Zoom;
    if (x<0 || x>=Max) return 0;
    if (y<0 || y>=Max) return 0;
    QPixmap* img = theWidget->theSlippyCache->getImage(x,y,Zoom);
    if (img) return img;
    img = new QPixmap(TILESIZE,TILESIZE);
    QPainter Painter(img);
    Painter.setPen(QColor(0,0,0));
    Painter.fillRect(0,0,TILESIZE-1,TILESIZE-1,QColor(255,255,255));
    Painter.drawRect(0,0,TILESIZE-1,TILESIZE-1);
    Painter.drawText(10,TILESIZE/2,(QApplication::translate("Downloader","Downloading %1,%2 (zoom %3)...")).arg(x).arg(y).arg(Zoom));
    return img;
}

void SlippyMapWidgetPrivate::newData(int, int, int)
{
    theWidget->update();
}

SlippyMapWidget::SlippyMapWidget(QWidget* aParent)
: QWidget(aParent)
{
    p = new SlippyMapWidgetPrivate(this);
    theSlippyCache->setMap(p);
    resize(500,400);
}

SlippyMapWidget::~SlippyMapWidget(void)
{
    theSlippyCache->setMap(0);
    delete p;
}

/* http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B */
static qreal tile2lon(qreal x, int z)
{
    return x / pow(2.0, z) * 360.0 - 180;
}

/* http://wiki.openstreetmap.org/index.php/Slippy_map_tilenames#C.2FC.2B.2B */
static qreal tile2lat(qreal y, int z)
{
    qreal n = M_PI - 2.0 * M_PI * y / pow(2.0, z);
    return 180.0 / M_PI * atan(0.5 * (exp(n) - exp(-n)));
}

static int long2tile(qreal lon, int z)
{
    return (int)(floor((lon + 180.0) / 360.0 * pow(2.0, z)));
}

static int lat2tile(qreal lat, int z)
{
    return (int)(floor((1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z)));
}

static qreal long2tileF(qreal lon, int z)
{
    return (lon + 180.0) / 360.0 * pow(2.0, z);
}

static qreal lat2tileF(qreal lat, int z)
{
    return (1.0 - log( tan(lat * M_PI/180.0) + 1.0 / cos(lat * M_PI/180.0)) / M_PI) / 2.0 * pow(2.0, z);
}


QRectF SlippyMapWidget::selectedArea()
{
    if (p->CurrentSelectionCoord.isNull())
        makeSelection(QRect(0,0,width(),height()));
    return p->CurrentSelectionCoord;
}

void SlippyMapWidget::setViewportArea(QRectF theRect)
{
    qreal zoom = 360.0 / theRect.width();
    zoom = log10(zoom)/log10(2.0);
    if (zoom < MINZOOMLEVEL)
        zoom = MINZOOMLEVEL;
    if (zoom > MAXZOOMLEVEL)
        zoom = MAXZOOMLEVEL;
    p->VpZoom = int(zoom);

    p->VpLon = long2tile(theRect.topRight().x(), p->VpZoom);
    p->VpLat = lat2tile(theRect.topRight().y(), p->VpZoom);
}

void SlippyMapWidget::paintEvent(QPaintEvent*)
{
    QPainter Painter(this);
    Painter.fillRect(QRect(0,0,width(),height()),QColor(255,255,255));
    int LonRect = int(floor(p->Lon));
    int LonPixel = int(-(p->Lon - LonRect ) * TILESIZE + width()/2);
    int LatRect = int(floor(p->Lat));
    int LatPixel = int(-(p->Lat - LatRect ) * TILESIZE + height()/2);

    while (LatPixel > 0)
        LatPixel -=TILESIZE, --LatRect;
    while (LonPixel > 0)
        LonPixel -= TILESIZE, --LonRect;

    for (int x=LonPixel; x<width(); x += TILESIZE)
        for (int y=LatPixel; y<height(); y+= TILESIZE)
        {
            int ThisLonRect = LonRect + (x-LonPixel)/TILESIZE;
            int ThisLatRect = LatRect + (y-LatPixel)/TILESIZE;
            QPixmap* img = p->getImage(ThisLonRect,ThisLatRect);
            if (img)
                Painter.drawPixmap(x,y,*img);
            delete img;
        }
    Painter.setPen(QPen(Qt::NoPen));
    Painter.setBrush(QBrush(QColor(255,255,255,128)));
    Painter.drawRect(width()-21,0,20,20);
    Painter.drawRect(width()-21,height()-21,20,20);
    Painter.drawRect(width()-21,(height()/2)-10,20,20);


    Painter.setBrush(QBrush(QColor(0,0,0)));
    Painter.drawRect(width()-19,8,16,4);
    Painter.drawRect(width()-19,height()-13,16,4);
    Painter.drawRect(width()-13,height()-19,4,16);

    Painter.setFont(QFont("Times", 19, QFont::Bold));
    Painter.setPen(QPen(Qt::black, 3));
    Painter.setBrush(Qt::NoBrush);
    Painter.drawText(QPoint(width()-21,(height()/2)+10), "V");

    if (p->InSelection) {
        Painter.setPen(QPen(Qt::blue, 1, Qt::DashLine));
        Painter.drawRect(QRect(p->SelectionStart, p->SelectionEnd));
    } else if (!p->CurrentSelectionCoord.isNull()) {
        Painter.setPen(QPen(Qt::blue, 1));
        QPoint topLeft     = coord2relative(p->CurrentSelectionCoord.topLeft());
        QPoint bottomRight = coord2relative(p->CurrentSelectionCoord.bottomRight());
        Painter.drawRect(QRect(topLeft, bottomRight));
    }
}

void SlippyMapWidget::ZoomTo(const QPoint & NewCenter, int NewZoom)
{
    if (NewZoom < MINZOOMLEVEL)
        NewZoom = MINZOOMLEVEL;
    if (NewZoom > MAXZOOMLEVEL)
        NewZoom = MAXZOOMLEVEL;
    if ((int)p->Zoom == NewZoom)
        return;

    qreal dx = (NewCenter.x()-width()/2)/(TILESIZE*1.0);
    qreal dy = (NewCenter.y()-height()/2)/(TILESIZE*1.0);

    p->Lon = (p->Lon + dx) * (1 << NewZoom) / (1 << p->Zoom) - dx;
    p->Lat = (p->Lat + dy) * (1 << NewZoom) / (1 << p->Zoom) - dy;
    p->Zoom = NewZoom;
    normalizeCoordinates();
    update();
}

/* The Lat and Lon coordinates are in fact tile coordinates. The valid range for
 * world is 0 to 2^zoom-1. Make sure it's so!
 */
void SlippyMapWidget::normalizeCoordinates(void) {
    unsigned limit = (1 << p->Zoom);
    if (p->Lon < 0) p->Lon = 0;
    if (p->Lat < 0) p->Lat = 0;
    if (p->Lon > limit) p->Lon = limit;
    if (p->Lat > limit) p->Lat = limit;
}

void SlippyMapWidget::wheelEvent(QWheelEvent* ev)
{
    int NewZoom = ev->angleDelta().y()/120 + p->Zoom;
    ZoomTo(ev->position().toPoint(), NewZoom);
    emit redraw();
}

void SlippyMapWidget::mousePressEvent(QMouseEvent* ev)
{
    if (ev->button() == Qt::MiddleButton) {
        on_resetViewAction_triggered(true);
    } else if (ev->button() == Qt::LeftButton) {
        if (ev->pos().x() > width()-20)
        {
            if (ev->pos().y() < 20)
            {
                ZoomTo(QPoint(width()/2,height()/2),p->Zoom-1);
                emit redraw();
                return;
            }
            else if (ev->pos().y() > height()-20)
            {
                ZoomTo(QPoint(width()/2,height()/2),p->Zoom+1);
                emit redraw();
                return;
            }
            else if ((ev->pos().y() > (height()/2)-20) && (ev->pos().y() < (height()/2)))
            {
                p->Lon = p->VpLon;
                p->Lat = p->VpLat;
                p->Zoom = p->VpZoom;
                normalizeCoordinates();
                update();
                emit redraw();
                return;
            }
        }

        /* No special place, start selection */
        p->InSelection = true;
        p->SelectionStart = p->SelectionEnd = ev->pos();
        qDebug() << "Enter selection";
    } else {
        /* RightButton, start drag */
        p->PreviousDrag = ev->pos();
        p->InDrag = true;
    }
    emit redraw();
}

/* TODO: relative2coord could be split off this */
void SlippyMapWidget::makeSelection(const QRect& relative) {
    /* Compute viewport boundingbox */
    qreal X1 = p->Lon - (width()/2.0)/TILESIZE;
    qreal Y1 = p->Lat - (height()/2.0)/TILESIZE;
    qreal X2 = p->Lon + (width()/2.0)/TILESIZE;
    qreal Y2 = p->Lat + (height()/2.0)/TILESIZE;

    /* Actual difference per pixel */
    qreal xpixel = (X2-X1)/width();
    qreal ypixel = (Y2-Y1)/height();

    /* Shift the border according to relative */
    X1 += xpixel * relative.topLeft().x();
    Y1 += ypixel * relative.topLeft().y();
    X2 -= xpixel * (width() - relative.bottomRight().x());
    Y2 -= ypixel * (height() - relative.bottomRight().y());

    qreal Lon1 = tile2lon(X1, p->Zoom);
    qreal Lat1 = tile2lat(Y1, p->Zoom);

    qreal Lon2 = tile2lon(X2, p->Zoom);
    qreal Lat2 = tile2lat(Y2, p->Zoom);

    /* Save the selection */
    p->CurrentSelectionCoord =  QRectF(Lon1, Lat2, Lon2-Lon1, Lat1-Lat2);

    qDebug() << "Selection rectangle: " << p->CurrentSelectionCoord;
}

/* Convert absolute coordinates to relative (to the widget) */
QPoint SlippyMapWidget::coord2relative(const QPointF& coord) const {
    /* Convert into local coordinate */
    qreal X = long2tileF(coord.x(), p->Zoom);
    qreal Y = lat2tileF(coord.y(), p->Zoom);

    /* p->Lon and p->Lat are the center, but in the same units as X and Y. */
    X -= p->Lon;
    Y -= p->Lat;

    /* Now, the p->Lon and p->Lat is equal to width()/2 and height()/2, and X
     * and Y are the distance in multiples of TILESIZE */
    QPoint pt(X*TILESIZE, Y*TILESIZE);

    /* Finally shift it relative to topLeft */
    return pt+QPoint(width()/2, height()/2);
}

void SlippyMapWidget::mouseReleaseEvent(QMouseEvent*)
{
    if (p->InSelection) {
        QRect SelectionRect(p->SelectionStart, p->SelectionEnd);
        if (SelectionRect.width() && SelectionRect.height()) {
            makeSelection(SelectionRect);
            update();
            emit redraw();
        } else {
            /* Default to full viewport */
            makeSelection(QRect(0, 0, width(), height()));
        }
    }
    p->InDrag = false;
    p->InSelection = false;
}

void SlippyMapWidget::mouseMoveEvent(QMouseEvent* ev)
{
    if (p->InDrag) {
        QPoint Delta = ev->pos()-p->PreviousDrag;
        if (!Delta.isNull())
        {
            p->Lon -= Delta.x()/(TILESIZE*1.);
            p->Lat -= Delta.y()/(TILESIZE*1.);
            p->PreviousDrag = ev->pos();
            update();
            emit redraw();
        }
    } else if (p->InSelection) {
       p->SelectionEnd = ev->pos();
       update();
       emit redraw();
    }
}

void SlippyMapWidget::on_resetViewAction_triggered(bool)
{
    p->Lon = 1.0;
    p->Lat = 1.0;
    p->Zoom = 1;
    update();
}

bool SlippyMapWidget::isDragging()
{
    return p->InDrag;
}


/* SLIPPYMAPCACHE */

SlippyMapCache::SlippyMapCache()
: QObject(0), DownloadReply(0), DownloadBusy(false), theMap(0)
{
    baseUrl.setUrl("http://tile.openstreetmap.org");
    Download.setProxy(M_PREFS->getProxy(baseUrl));

    connect(&Download,SIGNAL(finished(QNetworkReply*)),this,SLOT(on_requestFinished(QNetworkReply*)));

    preload(Coord(0,0,0),":/Tiles/000.png");
    preload(Coord(0,0,1),":/Tiles/100.png");
    preload(Coord(0,1,1),":/Tiles/101.png");
    preload(Coord(1,0,1),":/Tiles/110.png");
    preload(Coord(1,1,1),":/Tiles/111.png");
}

void SlippyMapCache::setMap(SlippyMapWidgetPrivate* aMap)
{
    theMap = aMap;
    int Use = 0;
    for (QMap<Coord, QByteArray>::iterator i = Memory.begin(); i != Memory.end(); ++i)
        Use += i.value().length();
    Dirties.clear();
}


void SlippyMapCache::preload(const Coord& C, const QString& Filename)
{
    QFile f(Filename);
    f.open(QIODevice::ReadOnly);
    QByteArray ba(f.readAll());
    if (ba.length())
        Memory[C] = ba;
}


void SlippyMapCache::on_requestFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "SlippyMap: Finished with error code " << reply->error() << ": " << reply->errorString();
    }
    if (reply == DownloadReply)
    {
        DownloadBusy = false;
        if (!reply->error())
        {
            Memory[DownloadCoord] = reply->readAll();
            if (theMap)
                theMap->newData(DownloadCoord.X,DownloadCoord.Y,DownloadCoord.Zoom);
            QMap<Coord,QByteArray>::iterator i = Dirties.find(DownloadCoord);
            if (i != Dirties.end())
                Dirties.erase(i);
        }
    }
}

QPixmap* SlippyMapCache::getImage(int x, int y, int Zoom)
{
    Coord C;
    C.X = x;
    C.Y = y;
    C.Zoom = Zoom;
    QMap<Coord,QByteArray>::iterator i = Memory.find(C);
    if (i == Memory.end())
    {
        addToQueue(C);
        return getDirty(x,y,Zoom);
    }
    QPixmap* img = new QPixmap;
    img->loadFromData(i.value());
    return img;
}

QPixmap* SlippyMapCache::getDirty(int x, int y, int Zoom)
{
    if (Zoom == MINZOOMLEVEL) return 0;
    QMap<Coord,QByteArray>::iterator i = Dirties.find(Coord(x,y,Zoom));
    if (i != Dirties.end())
    {
        QPixmap* img = new QPixmap;
        img->loadFromData(i.value());
        return img;
    }
    QPixmap* img = getImage(x/2,y/2,Zoom-1);
    if (!img) return 0;
    QPixmap* pm = new QPixmap(img->copy((x%2)*TILESIZE/2,(y%2)*TILESIZE/2,TILESIZE/2,TILESIZE/2).scaled(TILESIZE,TILESIZE));
    delete img;
    QByteArray Data;
    QBuffer Buffer(&Data);
    Buffer.open(QIODevice::WriteOnly);
    pm->save(&Buffer,"PNG");
    Coord C(x,y,Zoom);
    Dirties[C] = Data;
    return pm;
}

void SlippyMapCache::addToQueue(const Coord& C)
{
    for (int i=Queue.size(); i; --i)
        if (Queue[i-1].Zoom != C.Zoom)
            Queue.erase(Queue.begin()+(i-1));
    Queue.push_back(C);
    startDownload();
}

void SlippyMapCache::startDownload()
{
    if (Queue.empty()) return;
    if (DownloadBusy) return;
    while (Queue.size())
    {
        QMap<Coord,QByteArray>::iterator i = Memory.find(Queue[0]);
        if (i == Memory.end())
        {
            DownloadBusy = true;
            DownloadCoord = Queue[0];
            Queue.erase(Queue.begin());
            QUrl reqUrl(baseUrl);
            reqUrl.setPath(QString("/%1/%2/%3.png").arg(DownloadCoord.Zoom).arg(DownloadCoord.X).arg(DownloadCoord.Y));
            //qDebug() << "Starting download with url:" << reqUrl;
            QNetworkRequest req(reqUrl);
            req.setRawHeader(QByteArray("User-Agent"), USER_AGENT.toLatin1());
            DownloadReply = Download.get(req);
            return;
        }
        Queue.erase(Queue.begin());
    }
}

