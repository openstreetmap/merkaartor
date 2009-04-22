#ifndef MERKAARTOR_SLIPPYMAPWIDGET_H_
#define MERKAARTOR_SLIPPYMAPWIDGET_H_

#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtGui/QWidget>
#include <QtNetwork/QHttp>

#include <QMap>
#include <QList>

class SlippyMapWidgetPrivate;

class SlippyMapCache : public QObject
{
	Q_OBJECT

		struct Coord
		{
			Coord() : X(0), Y(0), Zoom(0) {}
			Coord(int x, int y, int z) : X(x), Y(y), Zoom(z) {}
			int X,Y,Zoom;
			bool operator<(const Coord C) const
			{
				return (X<C.X) ||
					( (X==C.X) && (Y<C.Y) ) ||
					( (X==C.X) && (Y==C.Y) && (Zoom < C.Zoom) );
			}
		};

	public:
		SlippyMapCache();

		void setMap(SlippyMapWidgetPrivate* aMap);

		QPixmap* getImage(int x, int y, int Zoom);
		QPixmap* getDirty(int x, int y, int Zoom);
	private slots:
		void on_requestFinished(int id, bool Error);
	private:
		void addToQueue(const Coord& C);
		void startDownload();
		void preload(const Coord& C, const QString& filename);

		QMap<Coord, QByteArray> Memory, Dirties;
		QList<Coord> Queue;
		QHttp Download;
		QByteArray DownloadData;
		QBuffer DownloadBuffer;
		int DownloadId;
		Coord DownloadCoord;
		bool DownloadBusy;
		SlippyMapWidgetPrivate* theMap;
};

class SlippyMapWidget :	public QWidget
{
	Q_OBJECT

	public:
		SlippyMapWidget(QWidget* aParent);
		~SlippyMapWidget();

		virtual void paintEvent(QPaintEvent* ev);
		virtual void wheelEvent (QWheelEvent* ev );
		virtual void mousePressEvent(QMouseEvent* ev);
		virtual void mouseMoveEvent(QMouseEvent* ev);
		virtual void mouseReleaseEvent(QMouseEvent* ev);

		bool isDragging();

		void setViewportArea(QRectF theRect);
		QRect viewArea() const;

		static SlippyMapCache* theSlippyCache;

	signals:
		void redraw();

	private:
		SlippyMapWidgetPrivate* p;
		void ZoomTo(const QPoint & NewCenter, int NewZoom);

	private slots:
		virtual void on_customContextMenuRequested(const QPoint & pos);
		virtual void on_resetViewAction_triggered(bool);
};

#endif

