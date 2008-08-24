#ifndef MERKATOR_MAPVIEW_H_
#define MERKATOR_MAPVIEW_H_

#include "Map/Projection.h"

#include <QtGui/QPixmap>
#include <QtGui/QWidget>

class Interaction;
class MainWindow;
class MapDocument;
class PropertiesDock;
class InfoDock;
class MapAdapter;
class Layer;
class LayerManager;


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

		void panScreen(QPoint delta) ;
		void invalidate(bool updateStaticBuffer, bool updateMap);

		virtual void paintEvent(QPaintEvent* anEvent);
		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseReleaseEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* ev);
		virtual void resizeEvent(QResizeEvent *event);

		Projection& projection();

		PropertiesDock* properties();
		//InfoDock* info();

        LayerManager*	layermanager;

		bool toXML(QDomElement xParent);
		void fromXML(const QDomElement e);

	private:
		void sortRenderingPriorityInLayers();
		void drawFeatures(QPainter & painter);
		void drawDownloadAreas(QPainter & painter);
		void drawScale(QPainter & painter);
		void drawGPS(QPainter & painter);
		void updateStaticBuffer(QPaintEvent* anEvent);
		void updateLayersImage(QPaintEvent* anEvent);
		MainWindow* Main;
		Projection theProjection;
		MapDocument* theDocument;
		Interaction* theInteraction;
		QPixmap* StaticBuffer;
		QPixmap* StaticMap;
		bool StaticBufferUpToDate;
		bool StaticMapUpToDate;
		QPoint thePanDelta;

		int numImages;
		QString StatusMessage;

	private slots:
		void imageRequested();
		void imageReceived();
		void loadingFinished();
		void updateStatusMessage();
		void on_customContextMenuRequested(const QPoint & pos);
};

#endif


