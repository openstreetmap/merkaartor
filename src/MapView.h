#ifndef MERKATOR_MAPVIEW_H_
#define MERKATOR_MAPVIEW_H_

#include "Maps/Projection.h"

#include <QPixmap>
#include <QWidget>
#include <QShortcut>
#include <QLabel>

class MainWindow;
class MapFeature;
class Road;
class MapDocument;
class PropertiesDock;
class InfoDock;
class MapAdapter;
class Interaction;
class ImageMapLayer;

class MapViewPrivate;

class MapView :	public QWidget
{
	Q_OBJECT

	public:
		MapView(MainWindow* aMain);
	public:
		~MapView();

		MainWindow* main();
		void setDocument(MapDocument* aDoc);
		MapDocument* document();
		void launch(Interaction* anInteraction);
		Interaction* interaction();

		void buildFeatureSet();
		void drawBackground(QPainter & painter, Projection& aProj);
		void drawFeatures(QPainter & painter, Projection& aProj);

		void panScreen(QPoint delta) ;
		void invalidate(bool updateStaticBuffer, bool updateMap);

		virtual void paintEvent(QPaintEvent* anEvent);
		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseReleaseEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* ev);
		virtual void resizeEvent(QResizeEvent *event);
		#ifdef GEOIMAGE
		virtual void dragEnterEvent(QDragEnterEvent *event);
		virtual void dragMoveEvent(QDragMoveEvent *event);
		virtual void dropEvent(QDropEvent *event);
		#endif // GEOIMAGE

		Projection& projection();
		QTransform& transform();
		QPoint toView(const Coord& aCoord) const;
		QPoint toView(TrackPoint* aPt) const;

		PropertiesDock* properties();
		//InfoDock* info();

		bool isSelectionLocked();
		void lockSelection();
		void unlockSelection();

		void setViewport(const CoordBox& Map);
		void setViewport(const CoordBox& Map, const QRect& Screen);
		CoordBox viewport() const;
		static void transformCalc(QTransform& theTransform, const Projection& theProjection, const CoordBox& TargetMap, const QRect& Screen);
		double pixelPerM() const;

		void zoom(double d, const QPointF& Around, const QRect& Screen);
		void setCenter(Coord& Center, const QRect& Screen);
		void resize(QSize oldS, QSize newS);

		bool toXML(QDomElement xParent);
		void fromXML(const QDomElement e);

	private:
		void drawDownloadAreas(QPainter & painter);
		void drawScale(QPainter & painter);
		void drawGPS(QPainter & painter);
		void updateStaticBackground();
		void updateStaticBuffer();
		void updateLayersImage();

		MainWindow* Main;
		Projection theProjection;
		MapDocument* theDocument;
		Interaction* theInteraction;
		QPixmap* StaticBackground;
		QPixmap* StaticBuffer;
		QPixmap* StaticMap;
		bool StaticBufferUpToDate;
		bool StaticMapUpToDate;
		bool SelectionLocked;
		QLabel* lockIcon;
		QList<MapFeature*> theSnapList;

		void viewportRecalc(const QRect& Screen);

		#ifdef GEOIMAGE
		TrackPoint *dropTarget;
		#endif

		int numImages;

		QShortcut* MoveLeft;
		QShortcut* MoveRight;
		QShortcut* MoveUp;
		QShortcut* MoveDown;

	public slots:
		virtual void on_MoveLeft_activated();
		virtual void on_MoveRight_activated();
		virtual void on_MoveUp_activated();
		virtual void on_MoveDown_activated();

	signals:
		void interactionChanged(Interaction* anInteraction);
		void viewportChanged();

	protected:
		bool event(QEvent *event);

	private slots:
		void on_imageRequested(ImageMapLayer*);
		void on_imageReceived(ImageMapLayer*);
		void on_loadingFinished(ImageMapLayer*);
		void on_customContextMenuRequested(const QPoint & pos);

	private:
		MapViewPrivate* p;
};

#endif


