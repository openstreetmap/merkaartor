#ifndef MERKATOR_MAPVIEW_H_
#define MERKATOR_MAPVIEW_H_

#include "Map/Projection.h"

#include <QtGui/QPixmap>
#include <QtGui/QWidget>

class Interaction;
class MainWindow;
class MapDocument;
class PropertiesDock;
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

		Projection* projection();
		PropertiesDock* properties();

        LayerManager*	layermanager;
		void checkLayerManager();

	private:
		void updateStaticBuffer(QPaintEvent* anEvent);
		MainWindow* Main;
		Projection* theProjection;
		MapDocument* theDocument;
		Interaction* theInteraction;
		QPixmap* StaticBuffer;
		bool StaticBufferUpToDate;

	public slots:
		void updateRequestNew();
	private slots:
        	void loadingFinished();
	protected:
		void resizeEvent(QResizeEvent *event);
};

#endif


