#ifndef MERKATOR_LAYERDOCK_H_
#define MERKATOR_LAYERDOCK_H_

#include <QtGui/QDockWidget>
#include <QtGui/QScrollArea>
#include <QActionGroup>

class MainWindow;
class MapLayer;

class LayerWidget : public QWidget
{
	Q_OBJECT

	public:
		LayerWidget(MainWindow* aMain, QWidget* aParent);

		void paintEvent(QPaintEvent* anEvent);
		void mouseReleaseEvent(QMouseEvent* anEvent);
		void updateContent();
		MapLayer* activeLayer();

	protected:
		void contextMenuEvent(QContextMenuEvent* anEvent);

	public:
		void initWmsActions();

	private:
		MainWindow* Main;
		unsigned int ActiveLayer;
		QActionGroup* actgrAdapter;
		QActionGroup* actgrWms;
#ifdef yahoo_illegal
		QAction* actYahoo;
#endif
		QAction* actNone;
		QAction* actOSM;
		QMenu* wmsMenu;

	private slots:
		void setWms(QAction*);
#ifdef yahoo_illegal
		void setYahoo(bool);
#endif
		void setOSM(bool);
		void setNone(bool);

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


