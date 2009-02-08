#include "LayerDock.h"
#include "LayerWidget.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "PropertiesDock.h"
#include "Command/Command.h"
#include "InfoDock.h"

#include <QPushButton>

#define LINEHEIGHT 25

class LayerDockPrivate
{
	public:
		LayerDockPrivate(MainWindow* aMain) :
		  Main(aMain), Scroller(0), Content(0), Layout(0), butGroup(0) {}
	public:
		MainWindow* Main;
		QScrollArea* Scroller;
		QWidget* Content;
		QVBoxLayout* Layout;
		QHBoxLayout* frameLayout;
		QButtonGroup* butGroup;
		QTabBar* tab;

		QList < QPair<MapLayer*, LayerWidget*> > layerList;
};

LayerDock::LayerDock(MainWindow* aMain)
: MDockAncestor(aMain)
{
	p = new LayerDockPrivate(aMain);
//	setMinimumSize(220,100);
	setObjectName("layersDock");

	createContent();

	retranslateUi();
}

LayerDock::~LayerDock()
{
	delete p;
}

void LayerDock::clearLayers()
{
	for (int i=p->layerList.size()-1; i >= 0; i--) {
		p->butGroup->removeButton(p->layerList[i].second);
		p->Layout->removeWidget(p->layerList[i].second);
		delete p->layerList[i].second;
		p->layerList.removeAt(i);
	}
}

void LayerDock::addLayer(MapLayer* aLayer)
{
	LayerWidget* w = aLayer->newWidget();
	if (w) {
		p->layerList.append(qMakePair(aLayer, w));
		p->butGroup->addButton(w);
		p->Layout->insertWidget(p->layerList.size()-1, w);

		connect(w, SIGNAL(layerChanged(LayerWidget*,bool)), this, SLOT(layerChanged(LayerWidget*,bool)));
		connect(w, SIGNAL(layerClosed(MapLayer*)), this, SLOT(layerClosed(MapLayer*)));
		connect(w, SIGNAL(layerCleared(MapLayer*)), this, SLOT(layerCleared(MapLayer*)));
		connect(w, SIGNAL(layerZoom(MapLayer*)), this, SLOT(layerZoom(MapLayer*)));

		p->Main->menuLayers->addMenu(w->getAssociatedMenu());

		//w->setChecked(aLayer->isSelected());
		w->setVisible(aLayer->isEnabled());
		w->setEnabled(aLayer->isEnabled());
		w->getAssociatedMenu()->menuAction()->setVisible(aLayer->isEnabled());

		update();
	}
}

void LayerDock::deleteLayer(MapLayer* aLayer)
{
	for (int i=p->layerList.size()-1; i >= 0; i--) {
		if (p->layerList[i].first == aLayer) {
			if (i) {
				p->layerList[i-1].first->setSelected(true);
				p->layerList[i-1].second->setChecked(true);
			}
			p->butGroup->removeButton(p->layerList[i].second);
			p->Layout->removeWidget(p->layerList[i].second);
			disconnect(p->layerList[i].second);
			p->layerList[i].second->setVisible(false);

			p->Main->menuLayers->removeAction(p->layerList[i].second->getAssociatedMenu()->menuAction());

			p->layerList.removeAt(i);
			//aLayer->deleteWidget();
		}
	}

	update();
}

void LayerDock::createContent()
{
	delete p->Scroller;

	QWidget* frame = new QWidget();
	p->frameLayout = new QHBoxLayout(frame);
	p->frameLayout->setMargin(0);
	p->frameLayout->setSpacing(0);

	p->tab = new QTabBar(frame);
	p->tab->setShape(QTabBar::RoundedWest);
	p->tab->setContextMenuPolicy(Qt::CustomContextMenu);
	int t;
	t = p->tab->addTab(NULL);
	p->tab->setTabData(t, MapLayer::All);
	t = p->tab->addTab(NULL);
	p->tab->setTabData(t, MapLayer::Default);
	t = p->tab->addTab(NULL);
	p->tab->setTabData(t, MapLayer::OSM);
	t = p->tab->addTab(NULL);
	p->tab->setTabData(t, MapLayer::Tracks);
	retranslateTabBar();
	connect(p->tab, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
	connect(p->tab, SIGNAL(customContextMenuRequested (const QPoint&)), this, SLOT(tabContextMenuRequested(const QPoint&)));

	QVBoxLayout* tabLayout = new QVBoxLayout();
	tabLayout->addWidget(p->tab);
	QSpacerItem* tabSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
	tabLayout->addItem(tabSpacer);

	p->frameLayout->addLayout(tabLayout);

	p->Scroller = new QScrollArea(frame);
	p->Scroller->setBackgroundRole(QPalette::Base);
	p->Scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	p->Scroller->setWidgetResizable(true);

	p->Content = new QWidget();
	p->Layout = new QVBoxLayout(p->Content);
	p->Layout->setSpacing(0);
	p->Layout->setMargin(0);

	p->butGroup = new QButtonGroup(p->Content);
	connect(p->butGroup, SIGNAL(buttonClicked (QAbstractButton *)), this, SLOT(layerSelected(QAbstractButton *)));

	p->Layout->addStretch();

	p->Scroller->setWidget(p->Content);

	p->frameLayout->addWidget(p->Scroller);

	setWidget(frame);
	update();
}

void LayerDock::resizeEvent(QResizeEvent* )
{
}

void LayerDock::layerSelected(QAbstractButton * l)
{
	if (p->Main->info())
		p->Main->info()->setHtml(((LayerWidget*)l)->getMapLayer()->toHtml());
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
	p->Main->on_editPropertiesAction_triggered();

	update();
}

void LayerDock::layerCleared(MapLayer* l)
{
	l->clear();
	p->Main->on_editPropertiesAction_triggered();
}

void LayerDock::layerZoom(MapLayer * l)
{
	CoordBox bb = MapLayer::boundingBox(l);
	CoordBox mini(bb.center()-10, bb.center()+10);
	bb.merge(mini);
	bb = bb.zoomed(1.1);
	p->Main->view()->projection().setViewport(bb, p->Main->view()->rect());
	emit(layersChanged(false));
}

void LayerDock::tabChanged(int idx)
{
	for (int i=p->layerList.size()-1; i >= 0; i--) {
		if ((p->layerList[i].first->isEnabled()) && (p->layerList[i].first->classGroups() & p->tab->tabData(idx).toInt()))
			p->layerList[i].second->setVisible(true);
		else
			p->layerList[i].second->setVisible(false);
	}
}

void LayerDock::tabContextMenuRequested(const QPoint& pos)
{
	int idx = p->tab->tabAt(pos);
	p->tab->setCurrentIndex(idx);

	QMenu* ctxMenu = new QMenu(this);

	QAction* actTabShow = new QAction(tr("Show All"), ctxMenu);
	ctxMenu->addAction(actTabShow);
	connect(actTabShow, SIGNAL(triggered(bool)), this, SLOT(TabShowAll(bool)));

	QAction* actTabHide = new QAction(tr("Hide All"), ctxMenu);
	ctxMenu->addAction(actTabHide);
	connect(actTabHide, SIGNAL(triggered(bool)), this, SLOT(TabHideAll(bool)));

	ctxMenu->exec(mapToGlobal(pos));

}

void LayerDock::TabShowAll(bool)
{
	for (int i=p->layerList.size()-1; i >= 0; i--) {
		if (p->layerList[i].first->classGroups() & p->tab->tabData(p->tab->currentIndex()).toInt()) {
			p->layerList[i].second->setLayerVisible(true);
		}
	}
}

void LayerDock::TabHideAll(bool)
{
	for (int i=p->layerList.size()-1; i >= 0; i--) {
		if (p->layerList[i].first->classGroups() & p->tab->tabData(p->tab->currentIndex()).toInt()) {
			p->layerList[i].second->setLayerVisible(false);
		}
	}
}

void LayerDock::changeEvent(QEvent * event)
{
	if (event->type() == QEvent::LanguageChange)
		retranslateUi();
	MDockAncestor::changeEvent(event);
}

void LayerDock::retranslateUi()
{
	setWindowTitle(tr("Layers"));
	retranslateTabBar();
}

void LayerDock::retranslateTabBar()
{
	p->tab->setTabText(0, tr("All"));
	p->tab->setTabText(1, tr("Default"));
	p->tab->setTabText(2, tr("OSM"));
	p->tab->setTabText(3, tr("Tracks"));
}
