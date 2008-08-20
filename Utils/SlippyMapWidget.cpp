#include "SlippyMapWidget.h"

#include <QtCore/QFile>
#include <QtGui/QApplication>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>
#include <QtGui/QWheelEvent>

#include <math.h>

#include "Preferences/MerkaartorPreferences.h"

#define TILESIZE 256
#define MINZOOMLEVEL 0
#define MAXZOOMLEVEL 17


/* SLIPPYMAPWIDGET */

class SlippyMapWidgetPrivate
{
	public:
		SlippyMapWidgetPrivate(SlippyMapWidget* w)
			: theWidget(w), Zoom(1), Lat(1), Lon(1), InDrag(false)
		{
			if (!theCache)
				theCache = new SlippyMapCache;
			theCache->setMap(this);
		}
		~SlippyMapWidgetPrivate()
		{
			theCache->setMap(0);
		}

		QPixmap* getImage(int x, int y);
		void newData(int x, int y, int zoom);

		SlippyMapWidget* theWidget;
		unsigned int Zoom;
		double Lat,Lon;
		QPoint PreviousDrag;
		bool InDrag;
		static SlippyMapCache* theCache;
};

SlippyMapCache* SlippyMapWidgetPrivate::theCache = 0;

QPixmap* SlippyMapWidgetPrivate::getImage(int x, int y)
{
	int Max = 1 << Zoom;
	if (x<0 || x>=Max) return 0;
	if (y<0 || y>=Max) return 0;
	QPixmap* img = theCache->getImage(x,y,Zoom);
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
	resize(500,400);
}

SlippyMapWidget::~SlippyMapWidget(void)
{
	delete p;
}

static double ProjectF(double Lat)
{
	double LL = Lat/180*M_PI;
	double Y = log(tan(LL) + (1/cos(LL)));
	return Y;
}

static double ProjectMercToLat(double MercY)
{
	return( 180/M_PI * atan(sinh(MercY)));
}


QRectF SlippyMapWidget::viewArea() const
{
	double X1 = p->Lat - (width()/2.0)/TILESIZE;
	double Y1 = p->Lon - (height()/2.0)/TILESIZE;
	double X2 = p->Lat + (width()/2.0)/TILESIZE;
	double Y2 = p->Lon + (height()/2.0)/TILESIZE;
	double Unit = 1.0 / (1 << p->Zoom);
	double relY1 = Y1 * Unit;
	double relY2 = Y2 * Unit;
	double LimitY = ProjectF(85.0511);
	double RangeY = 2 * LimitY;
	relY1 = LimitY - RangeY * relY1;
	relY2 = LimitY - RangeY * relY2;
	double Lat1 = ProjectMercToLat(relY1);
	double Lat2 = ProjectMercToLat(relY2);
	Unit = 360.0 / (1<<p->Zoom);
	double Long1 = -180 + X1 * Unit;
	double Long2 = -180 + X2 * Unit;
	Long1 *= M_PI/180;
	Long2 *= M_PI/180;
	Lat1 *= M_PI/180;
	Lat2 *= M_PI/180;
	return QRectF(Lat2,Long1,Lat1-Lat2,Long2-Long1);
}

void SlippyMapWidget::paintEvent(QPaintEvent*)
{
	QPainter Painter(this);
	Painter.fillRect(QRect(0,0,width(),height()),QColor(255,255,255));
	int LatRect = int(floor(p->Lat));
	int LatPixel = int(-(p->Lat - LatRect ) * TILESIZE + width()/2);
	int LonRect = int(floor(p->Lon));
	int LonPixel = int(-(p->Lon - LonRect ) * TILESIZE + height()/2);
	while (LatPixel > 0)
		LatPixel -=TILESIZE, --LatRect;
	while (LonPixel > 0)
		LonPixel -= TILESIZE, --LonRect;
	for (int x=LatPixel; x<width(); x += TILESIZE)
		for (int y=LonPixel; y<height(); y+= TILESIZE)
		{
			int ThisLatRect = LatRect + (x-LatPixel)/TILESIZE;
			int ThisLonRect = LonRect + (y-LonPixel)/TILESIZE;
			QPixmap* img = p->getImage(ThisLatRect,ThisLonRect);
			if (img)
				Painter.drawPixmap(x,y,*img);
			delete img;
		}
	Painter.setPen(QPen(Qt::NoPen));
	Painter.setBrush(QBrush(QColor(255,255,255,128)));
	Painter.drawRect(width()-21,0,20,20);
	Painter.drawRect(width()-21,height()-21,20,20);
	Painter.setBrush(QBrush(QColor(0,0,0)));
	Painter.drawRect(width()-19,8,16,4);
	Painter.drawRect(width()-19,height()-13,16,4);
	Painter.drawRect(width()-13,height()-19,4,16);
}

void SlippyMapWidget::ZoomTo(const QPoint & NewCenter, int NewZoom)
{
	if (NewZoom < MINZOOMLEVEL)
		NewZoom = MINZOOMLEVEL;
	if (NewZoom > MAXZOOMLEVEL)
		NewZoom = MAXZOOMLEVEL;
	if ((int)p->Zoom == NewZoom)
		return;
	
 	double dx = (NewCenter.x()-width()/2)/(TILESIZE*1.0);
	double dy = (NewCenter.y()-height()/2)/(TILESIZE*1.0);

	p->Lat = (p->Lat + dx) * (1 << NewZoom) / (1 << p->Zoom) - dx;
	p->Lon = (p->Lon + dy) * (1 << NewZoom) / (1 << p->Zoom) - dy;
	p->Zoom = NewZoom;
	update();
}

void SlippyMapWidget::wheelEvent(QWheelEvent* ev)
{
	int NewZoom = ev->delta()/120 + p->Zoom;
	ZoomTo(ev->pos(), NewZoom);
	emit redraw();
}

void SlippyMapWidget::mousePressEvent(QMouseEvent* ev)
{
	if (ev->button() == Qt::MidButton)
	{
		ZoomTo(ev->pos(), p->Zoom + 1);
	}
	else if (ev->button() == Qt::RightButton)
	{
		ZoomTo(ev->pos(), p->Zoom - 1);
	}
	else
	{
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
		}
		p->PreviousDrag = ev->pos();
		p->InDrag = true;
	}
	emit redraw();
}

void SlippyMapWidget::mouseReleaseEvent(QMouseEvent*)
{
	p->InDrag = false;
}

void SlippyMapWidget::mouseMoveEvent(QMouseEvent* ev)
{
	if (p->InDrag)
	{
		QPoint Delta = ev->pos()-p->PreviousDrag;
		p->Lat -= Delta.x()/(TILESIZE*1.);
		p->Lon -= Delta.y()/(TILESIZE*1.);
		p->PreviousDrag = ev->pos();
		update();
		emit redraw();
	}
}


/* SLIPPYMAPCACHE */

SlippyMapCache::SlippyMapCache()
: QObject(0), DownloadId(0), DownloadBusy(false), theMap(0)
{
	Download.setHost("tile.openstreetmap.org");
	if (MerkaartorPreferences::instance()->getProxyUse()) {
		Download.setProxy(MerkaartorPreferences::instance()->getProxyHost(), MerkaartorPreferences::instance()->getProxyPort());
	}

	DownloadBuffer.setBuffer(&DownloadData);
	DownloadBuffer.open(QIODevice::WriteOnly);
	connect(&Download,SIGNAL(requestFinished(int,bool)),this,SLOT(on_requestFinished(int, bool)));

	preload(Coord(0,0,0),":/Tiles/000.png");
	preload(Coord(0,0,1),":/Tiles/100.png");
	preload(Coord(0,1,1),":/Tiles/101.png");
	preload(Coord(1,0,1),":/Tiles/110.png");
	preload(Coord(1,1,1),":/Tiles/111.png");
}

void SlippyMapCache::setMap(SlippyMapWidgetPrivate* aMap)
{
	theMap = aMap;
	unsigned int Use = 0;
	for (std::map<Coord, QByteArray>::iterator i = Memory.begin(); i != Memory.end(); ++i)
		Use += i->second.length();
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


void SlippyMapCache::on_requestFinished(int Id, bool Error)
{
	if (Id == DownloadId)
	{
		DownloadBusy = false;
		if (!Error)
		{
			Memory[DownloadCoord] = DownloadData;
			if (theMap)
				theMap->newData(DownloadCoord.X,DownloadCoord.Y,DownloadCoord.Zoom);
			std::map<Coord,QByteArray>::iterator i = Dirties.find(DownloadCoord);
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
	std::map<Coord,QByteArray>::iterator i = Memory.find(C);
	if (i == Memory.end())
	{
		addToQueue(C);
		return getDirty(x,y,Zoom);
	}
	QPixmap* img = new QPixmap;
	img->loadFromData(i->second);
	return img;
}

QPixmap* SlippyMapCache::getDirty(int x, int y, int Zoom)
{
	if (Zoom == MINZOOMLEVEL) return 0;
	std::map<Coord,QByteArray>::iterator i = Dirties.find(Coord(x,y,Zoom));
	if (i != Dirties.end())
	{
		QPixmap* img = new QPixmap;
		img->loadFromData(i->second);
		return img;
	}
	QPixmap* img = getImage(x/2,y/2,Zoom-1);
	if (!img) return 0;
	QPixmap* pm = new QPixmap(img->copy((x%2)*TILESIZE/2,(y%2)*TILESIZE/2,TILESIZE/2,TILESIZE/2).scaled(TILESIZE,TILESIZE));
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
	for (unsigned int i=Queue.size(); i; --i)
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
		std::map<Coord,QByteArray>::iterator i = Memory.find(Queue[0]);
		if (i == Memory.end())
		{
			DownloadBusy = true;
			DownloadCoord = Queue[0];
			Queue.erase(Queue.begin());
			QString Path("/%1/%2/%3.png");
			Path = Path.arg(DownloadCoord.Zoom).arg(DownloadCoord.X).arg(DownloadCoord.Y);
			DownloadData.clear();
			DownloadBuffer.reset();
			DownloadId = Download.get(Path, &DownloadBuffer);
			return;
		}
		Queue.erase(Queue.begin());
	}
}

