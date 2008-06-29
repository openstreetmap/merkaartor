#ifndef MERKATOR_MAPVIEW_H_
#define MERKATOR_MAPVIEW_H_

#include "Map/Projection.h"

#include <QtGui/QPixmap>
#include <QtGui/QWidget>
#include <QProgressBar>

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

		void invalidate();

		virtual void paintEvent(QPaintEvent* anEvent);
		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseReleaseEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void wheelEvent(QWheelEvent* ev);
		virtual void resizeEvent(QResizeEvent *event);

		Projection& projection();
		PropertiesDock* properties();

		InfoDock* info();

        LayerManager*	layermanager;

		bool toXML(QDomElement xParent);
		void fromXML(const QDomElement e);

	private:
		void sortRenderingPriorityInLayers();
		void drawLayersImage(QPainter & painter);
		void drawFeatures(QPainter & painter);
		void drawDownloadAreas(QPainter & painter);
		void drawScale(QPainter & painter);
		void updateStaticBuffer(QPaintEvent* anEvent);
		MainWindow* Main;
		Projection theProjection;
		MapDocument* theDocument;
		Interaction* theInteraction;
		QPixmap* StaticBuffer;
		bool StaticBufferUpToDate;

		int numImages;
		QProgressBar* pbImages;
		QString StatusMessage;

	private slots:
		void imageRequested();
		void imageReceived();
		void loadingFinished();
		void updateStatusMessage();
};

#endif


