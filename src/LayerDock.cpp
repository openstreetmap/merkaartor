#include "LayerDock.h"
#include "LayerWidget.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"
#include "PropertiesDock.h"
#include "Command/Command.h"
#include "InfoDock.h"

#include <QPushButton>
#include <QDragEnterEvent>

#define LINEHEIGHT 25

#define CHILD_WIDGETS (p->Content->children())
#define CHILD_WIDGET(x) (dynamic_cast<LayerWidget*>(p->Content->children().at(x)))
#define CHILD_LAYER(x) (dynamic_cast<LayerWidget*>(p->Content->children().at(x))->getMapLayer())

class LayerDockPrivate
{
	public:
		LayerDockPrivate(MainWindow* aMain) :
		  Main(aMain), Scroller(0), Content(0), Layout(0), theDropWidget(0),
		  lastSelWidget(0)
		  {}
	public:
		MainWindow* Main;
		QScrollArea* Scroller;
		QWidget* Content;
		QVBoxLayout* Layout;
		QHBoxLayout* frameLayout;
		QTabBar* tab;
		LayerWidget* theDropWidget;
		LayerWidget* lastSelWidget;
};

LayerDock::LayerDock(MainWindow* aMain)
: MDockAncestor(aMain)
{
	p = new LayerDockPrivate(aMain);
//	setMinimumSize(220,100);
	setObjectName("layersDock");
	setAcceptDrops(true);

	createContent();

	retranslateUi();
}

LayerDock::~LayerDock()
{
	delete p;
}

void LayerDock::dragEnterEvent(QDragEnterEvent *event)
{
	p->theDropWidget = NULL;
	if (event->mimeData()->hasFormat("application/x-layer"))
		if ((p->theDropWidget = dynamic_cast<LayerWidget*>(event->source())))
			event->acceptProposedAction();
}

void LayerDock::dragMoveEvent(QDragMoveEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-layer"))
		event->accept();
	else {
		event->ignore();
		return;
	}

	LayerWidget* aW = dynamic_cast<LayerWidget*>(childAt(event->pos()));
	if (aW != p->theDropWidget) {
		if (!aW) {
			p->Layout->removeWidget(p->theDropWidget);
			p->Layout->addWidget(p->theDropWidget);
		} else {
			p->Layout->removeWidget(p->theDropWidget);
			p->Layout->insertWidget(p->Layout->indexOf(aW), p->theDropWidget);
		}
		update();
	}
}

void LayerDock::dragLeaveEvent(QDragLeaveEvent * /*event*/)
{
//	if (p->theDropWidget) {
//		p->Layout->removeWidget(p->theDropWidget);
//		SAFE_DELETE(p->theDropWidget);
//	}
}

void LayerDock::dropEvent(QDropEvent *event)
{
	if (event->mimeData()->hasFormat("application/x-layer"))
		event->accept();
	else {
		event->ignore();
		return;
	}

	p->Main->document()->moveLayer(p->theDropWidget->getMapLayer(), p->Layout->indexOf(p->theDropWidget));
	emit(layersChanged(false));
	update();
}

void LayerDock::clearLayers()
{
	for (int i=CHILD_WIDGETS.size()-1; i >= 0; i--) {
		if (!CHILD_WIDGET(i))
			continue;
		CHILD_WIDGET(i)->deleteLater();
	}
}

void LayerDock::addLayer(MapLayer* aLayer)
{
	LayerWidget* w = aLayer->newWidget();
	if (w) {
		p->Layout->addWidget(w);

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
	for (int i=CHILD_WIDGETS.size()-1; i >= 0; i--) {
		if (!CHILD_WIDGET(i))
			continue;
		if (CHILD_LAYER(i) == aLayer) {
			p->Main->menuLayers->removeAction(CHILD_WIDGET(i)->getAssociatedMenu()->menuAction());
			LayerWidget* curW = CHILD_WIDGET(i);
			curW->deleteLater();
			break;
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

	QWidget* aWidget = new QWidget();
	QVBoxLayout* aLayout = new QVBoxLayout(aWidget);
	aLayout->setSpacing(0);
	aLayout->setMargin(0);

	p->Content = new QWidget();
	p->Layout = new QVBoxLayout(p->Content);
	p->Layout->setSpacing(0);
	p->Layout->setMargin(0);

	aLayout->addWidget(p->Content);
	aLayout->addStretch();

	p->Scroller->setWidget(aWidget);

	p->frameLayout->addWidget(p->Scroller);

	setWidget(frame);
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
	p->Main->on_editPropertiesAction_triggered();
	p->Main->document()->removeDownloadBox(l);

	update();
}

void LayerDock::layerCleared(MapLayer* l)
{
	l->clear();
	p->Main->on_editPropertiesAction_triggered();
}

void LayerDock::layerZoom(MapLayer * l)
{
	CoordBox bb = l->boundingBox();
	CoordBox mini(bb.center()-2000, bb.center()+2000);
	bb.merge(mini);
	bb = bb.zoomed(1.1);
	p->Main->view()->setViewport(bb, p->Main->view()->rect());
	emit(layersChanged(false));
}

void LayerDock::tabChanged(int idx)
{
	for (int i=CHILD_WIDGETS.size()-1; i >= 0; i--) {
		if (!CHILD_WIDGET(i))
			continue;
		if ((CHILD_LAYER(i)->isEnabled()) && (CHILD_LAYER(i)->classGroups() & p->tab->tabData(idx).toInt()))
			CHILD_WIDGET(i)->setVisible(true);
		else
			CHILD_WIDGET(i)->setVisible(false);
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
	for (int i=CHILD_WIDGETS.size()-1; i >= 0; i--) {
		if (!CHILD_WIDGET(i))
			continue;
		if (CHILD_LAYER(i)->classGroups() & p->tab->tabData(p->tab->currentIndex()).toInt()) {
			CHILD_WIDGET(i)->setLayerVisible(true);
		}
	}
}

void LayerDock::TabHideAll(bool)
{
	for (int i=CHILD_WIDGETS.size()-1; i >= 0; i--) {
		if (!CHILD_WIDGET(i))
			continue;
		if (CHILD_LAYER(i)->classGroups() & p->tab->tabData(p->tab->currentIndex()).toInt()) {
			CHILD_WIDGET(i)->setLayerVisible(false);
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

void LayerDock::mousePressEvent ( QMouseEvent * ev )
{
	LayerWidget* aWidget = dynamic_cast<LayerWidget*>(childAt(ev->pos()));
    
	if (ev->button() == Qt::RightButton) {
	    for (int i=0; i < CHILD_WIDGETS.size(); ++i) {
    	    if (CHILD_WIDGET(i))
	    	    CHILD_WIDGET(i)->setChecked(false);
        }
		aWidget->setChecked(true);
		p->lastSelWidget = aWidget;
        ev->ignore();
        return;
    }

	if (!aWidget) {
		if (p->lastSelWidget)
			p->lastSelWidget->setChecked(false);
		p->lastSelWidget = NULL;
		ev->ignore();
		return;
	}

    if (ev->modifiers() & Qt::ControlModifier) {
		bool toSelect = !aWidget->isChecked();
		aWidget->setChecked(toSelect);
		if (toSelect)
			p->lastSelWidget = aWidget;
		else
			p->lastSelWidget = NULL;
    } else
    if (ev->modifiers() & Qt::ShiftModifier) {
		bool toSelect = false;
 	    for (int i=0; i < CHILD_WIDGETS.size(); ++i) {
			if (CHILD_WIDGET(i)) {
				if (CHILD_WIDGET(i) == aWidget || CHILD_WIDGET(i) == p->lastSelWidget)
					toSelect = !toSelect;

				if (toSelect)
	    			CHILD_WIDGET(i)->setChecked(true);
			}
        }
		aWidget->setChecked(true);
    } else {
 	    for (int i=0; i < CHILD_WIDGETS.size(); ++i) {
    	    if (CHILD_WIDGET(i))
	    	    CHILD_WIDGET(i)->setChecked(false);
        }
		aWidget->setChecked(true);
		p->lastSelWidget = aWidget;

		if (p->Main->info())
			p->Main->info()->setHtml(aWidget->getMapLayer()->toHtml());
	}
	ev->accept();
}
