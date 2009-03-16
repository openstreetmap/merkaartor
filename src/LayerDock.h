#ifndef MERKATOR_LAYERDOCK_H_
#define MERKATOR_LAYERDOCK_H_

#include "Utils/MDockAncestor.h"
#include <QtGui/QScrollArea>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QTabBar>

class MainWindow;
class MapLayer;
class LayerWidget;
class LayerDockPrivate;

class LayerDock : public MDockAncestor
{
	Q_OBJECT

	public:
		LayerDock(MainWindow* aParent);
	public:
		~LayerDock(void);

		void createContent();
		//void updateContent();
		void resizeEvent(QResizeEvent* anEvent);

		void clearLayers();
		void addLayer(MapLayer* aLayer);
		void deleteLayer(MapLayer* aLayer);

	private slots:
		void layerSelected(QAbstractButton * l);
		void layerChanged(LayerWidget*, bool adjustViewport);
		void layerClosed(MapLayer*);
		void layerCleared(MapLayer*);
		void layerZoom(MapLayer*);
		void tabChanged(int idx);
		void tabContextMenuRequested(const QPoint& pos);
		void TabShowAll(bool);
		void TabHideAll(bool);

	signals:
		void layersChanged(bool adjustViewport);

	protected:
		LayerDockPrivate* p;

	private:
		void changeEvent(QEvent*);
		void retranslateUi();
		void retranslateTabBar();
};

#endif


