#include "LayerDock.h"
#include "LayerWidget.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "PropertiesDock.h"
#include "Command/Command.h"

#include <QPushButton>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

#define LINEHEIGHT 25

LayerDock::LayerDock(MainWindow* aMain)
: QDockWidget(aMain), Main(aMain), Scroller(0), Content(0), Layout(0), butGroup(0)
{
//	setMinimumSize(220,100);
	setWindowTitle(tr("Layers"));
	setObjectName("layersDock");

	createContent();
}

LayerDock::~LayerDock()
{
}

void LayerDock::clearLayers()
{
	for (int i=layerList.size()-1; i >= 0; i--) {
		butGroup->removeButton(layerList[i].second);
		Layout->removeWidget(layerList[i].second);
		delete layerList[i].second;
		layerList.removeAt(i);
	}
}

void LayerDock::addLayer(MapLayer* aLayer)
{
	LayerWidget* w = aLayer->newWidget();
	if (w) {
		layerList.append(qMakePair(aLayer, w));
		butGroup->addButton(w);
		Layout->insertWidget(layerList.size()-1, w);
		w->setChecked(aLayer->isSelected());
		w->setVisible(aLayer->isEnabled());

		connect(w, SIGNAL(layerChanged(LayerWidget*,bool)), this, SLOT(layerChanged(LayerWidget*,bool)));
		connect(w, SIGNAL(layerClosed(MapLayer*)), this, SLOT(layerClosed(MapLayer*)));
		connect(w, SIGNAL(layerCleared(MapLayer*)), this, SLOT(layerCleared(MapLayer*)));
		connect(w, SIGNAL(layerZoom(MapLayer*)), this, SLOT(layerZoom(MapLayer*)));

		Main->menuLayers->addMenu(w->getAssociatedMenu());

		update();
	}
}

void LayerDock::deleteLayer(MapLayer* aLayer)
{
	for (int i=layerList.size()-1; i >= 0; i--) {
		if (layerList[i].first == aLayer) {
			if (i) {
				layerList[i-1].first->setSelected(true);
				layerList[i-1].second->setChecked(true);
			}
			butGroup->removeButton(layerList[i].second);
			Layout->removeWidget(layerList[i].second);
			disconnect(layerList[i].second);
			layerList[i].second->setVisible(false);

			Main->menuLayers->removeAction(layerList[i].second->getAssociatedMenu()->menuAction());

			layerList.removeAt(i);
			//aLayer->deleteWidget();
		}
	}

	update();
}

void LayerDock::createContent()
{
	delete Scroller;

	Scroller = new QScrollArea;
	Scroller->setBackgroundRole(QPalette::Base);
	Scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QVBoxLayout scrollerLayout(Scroller);
	Content = new QGroupBox();
	Content->setFlat(true);
	Layout = new QVBoxLayout(Content);
	Layout->setSpacing(0);
	Layout->setMargin(0);

	butGroup = new QButtonGroup(Content);

	Layout->addStretch();
	setWidget(Scroller);
	Scroller->setWidget(Content);
	Scroller->setWidgetResizable(true);

	update();
}

void LayerDock::resizeEvent(QResizeEvent* )
{
}

void LayerDock::layerChanged(LayerWidget* l, bool adjustViewport)
{
	l->getAssociatedMenu()->setTitle(l->getMapLayer()->name());
	emit(layersChanged(adjustViewport));
}

void LayerDock::layerClosed(MapLayer* l)
{
//	Main->document()->getUploadedLayer()->clear();
	//Main->document()->remove(l);
	//delete l;
	l->clear();
	l->setEnabled(false);
	l->getWidget()->setVisible(false);
	l->getWidget()->getAssociatedMenu()->setVisible(false);
	Main->on_editPropertiesAction_triggered();

	update();
}

void LayerDock::layerCleared(MapLayer* l)
{
	l->clear();
	Main->on_editPropertiesAction_triggered();
}

void LayerDock::layerZoom(MapLayer * l)
{
	CoordBox bb = MapLayer::boundingBox(l);
	CoordBox min(bb.center()-0.00001, bb.center()+0.00001);
	bb.merge(min);
	bb = bb.zoomed(1.1);
	Main->view()->projection().setViewport(bb, Main->view()->rect());
	emit(layersChanged(false));
}

