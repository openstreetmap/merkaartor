#ifndef MERKATOR_LAYERDOCK_H_
#define MERKATOR_LAYERDOCK_H_

#include <QtGui/QDockWidget>
#include <QtGui/QScrollArea>

class MainWindow;
class MapLayer;

class LayerWidget : public QWidget
{
	public:
		LayerWidget(MainWindow* aMain, QWidget* aParent);

		void paintEvent(QPaintEvent* anEvent);
		void mouseReleaseEvent(QMouseEvent* anEvent);
		void updateContent();
		MapLayer* activeLayer();

	private:
		MainWindow* Main;
		unsigned int ActiveLayer;
};

class LayerDock : public QDockWidget
{
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
		LayerWidget* Content;
};

#endif


