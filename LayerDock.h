#ifndef MERKATOR_LAYERDOCK_H_
#define MERKATOR_LAYERDOCK_H_

#include <QtGui/QDockWidget>
#include <QtGui/QScrollArea>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>

class MainWindow;
class MapLayer;
class LayerWidget;

class LayerDock : public QDockWidget
{
	Q_OBJECT

	public:
		LayerDock(MainWindow* aParent);
	public:
		~LayerDock(void);

		void createContent();
		//void updateContent();
		void resizeEvent(QResizeEvent* anEvent);
		MapLayer* activeLayer();

		void clearLayers();
		void addLayer(MapLayer* aLayer);
		void deleteLayer(MapLayer* aLayer);

	private:
		MainWindow* Main;
		QScrollArea* Scroller;
		QGroupBox* Content;
		QVBoxLayout* Layout;
		QButtonGroup* butGroup;

		QList < QPair<MapLayer*, LayerWidget*> > layerList;

	private slots:
		void layerChanged(LayerWidget*, bool adjustViewport);
		void layerClosed(MapLayer*);
		void layerCleared(MapLayer*);
		void layerZoom(MapLayer*);

	signals:
		void layersChanged(bool adjustViewport);
};

#endif


