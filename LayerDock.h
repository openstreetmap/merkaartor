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

		void updateContent();
		void resizeEvent(QResizeEvent* anEvent);
		MapLayer* activeLayer();

	private:
		MainWindow* Main;
		QScrollArea* Scroller;
		QGroupBox* Content;
		QVBoxLayout* Layout;
		QButtonGroup* butGroup;

	private slots:
		void layerChanged(LayerWidget*, bool adjustViewport);

	signals:
		void layersChanged(bool adjustViewport);
};

#endif


