#include "SlippyMapWidget.h"

#include <QtCore/QFile>
#include <QtGui/QPainter>
#include <Qtgui/QPixmap>
#include <QtGui/QWheelEvent>

#include <math.h>

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
			theCache = new SlippyMapCache(this);
		}
		~SlippyMapWidgetPrivate()
		{
			delete theCache;
		}

		QPixmap* getImage(int x, int y);
		void newData(int x, int y, int zoom);

		SlippyMapWidget* theWidget;
		unsigned int Zoom;
		double Lat,Lon;
		QPoint PreviousDrag;
		bool InDrag;
		SlippyMapCache* theCache;
};

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
	Painter.drawText(10,TILESIZE/2,QString("Downloading %1,%2 (zoom %3)...").arg(x).arg(y).arg(Zoom));
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
	double LL = Lat/360*3.141592;
	double Y = log(tan(LL) + (1/cos(LL)));
	return Y;
}

static double ProjectMercToLat(double MercY)
{
	return( 180/3.141592* atan(sinh(MercY)));
}


QRect SlippyMapWidget::viewArea() const
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
	Unit = 360 / (1<<p->Zoom);
	double Long1 = -180 + X1 * Unit;
	double Long2 = -180 + X2 * Unit;
	return QRect(Lat1,Long1,Lat2-Lat1,Long2-Long1);
}

void SlippyMapWidget::paintEvent(QPaintEvent*)
{
	QPainter Painter(this);
	Painter.fillRect(QRect(0,0,width(),height()),QColor(255,255,255));
	int LatRect = floor(p->Lat);
	int LatPixel = -(p->Lat - LatRect ) * TILESIZE + width()/2;
	int LonRect = floor(p->Lon);
	int LonPixel = -(p->Lon - LonRect ) * TILESIZE + height()/2;
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
}

void SlippyMapWidget::wheelEvent(QWheelEvent* ev)
{
	int NewZoom = ev->delta()/120 + p->Zoom;
	if (NewZoom < MINZOOMLEVEL)
		NewZoom = MINZOOMLEVEL;
	if (NewZoom > MAXZOOMLEVEL)
		NewZoom = MAXZOOMLEVEL;
	if (p->Zoom != NewZoom)
	{
		double dx = (ev->pos().x()-width()/2)/(TILESIZE*1.0);
		double dy = (ev->pos().y()-height()/2)/(TILESIZE*1.0);
		p->Lat = (p->Lat + dx) * (1 << NewZoom) / (1 << p->Zoom) - dx;
		p->Lon = (p->Lon + dy) * (1 << NewZoom) / (1 << p->Zoom) - dy;
		p->Zoom = NewZoom;
		update();
	}
}

void SlippyMapWidget::mousePressEvent(QMouseEvent* ev)
{
	p->PreviousDrag = ev->pos();
	p->InDrag = true;
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
	}
}


/* SLIPPYMAPCACHE */

SlippyMapCache::SlippyMapCache(SlippyMapWidgetPrivate* p)
: QObject(0), DownloadBusy(false), theMap(p)
{
	Download.setHost("tile.openstreetmap.org");
	DownloadBuffer.setBuffer(&DownloadData);
	DownloadBuffer.open(QIODevice::WriteOnly);
	connect(&Download,SIGNAL(requestFinished(int,bool)),this,SLOT(on_requestFinished(int, bool)));

	preload(Coord(0,0,0),":/Tiles/000.png");
	preload(Coord(0,0,1),":/Tiles/100.png");
	preload(Coord(0,1,1),":/Tiles/101.png");
	preload(Coord(1,0,1),":/Tiles/110.png");
	preload(Coord(1,1,1),":/Tiles/111.png");
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

