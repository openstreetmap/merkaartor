#include "LayerDock.h"
#include "LayerWidget.h"

#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"

#include <QPushButton>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

#define LINEHEIGHT 20

LayerDock::LayerDock(MainWindow* aMain)
: QDockWidget(aMain), Main(aMain), Scroller(0), Content(0), Layout(0)
{
	setMinimumSize(220,100);
	setWindowTitle(tr("Layers"));

	updateContent();
}

LayerDock::~LayerDock()
{
}

void LayerDock::updateContent()
{
	//delete Scroller;

	Scroller = new QScrollArea;
	Scroller->setBackgroundRole(QPalette::Base);
	Scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	Content = new QGroupBox();
	Content->setFlat(true);
	Layout = new QVBoxLayout(Content);
	Layout->setSpacing(0);
	Layout->setMargin(0);
	Content->setFixedSize(width(),20+(Main->document()->numLayers()+1)*LINEHEIGHT+1);

	butGroup = new QButtonGroup;
	for (unsigned int i=0; i < Main->document()->numLayers(); i++) {
		LayerWidget* w = Main->document()->layer(i)->newWidget();
		butGroup->addButton(w);
		Layout->addWidget(w);
		w->setChecked(Main->document()->layer(i)->isSelected());

		connect(w, SIGNAL(layerChanged(LayerWidget*,bool)), this, SLOT(layerChanged(LayerWidget*,bool)));
	}
	Layout->addStretch(1);
	setWidget(Scroller);
	Scroller->setWidget(Content);

	update();
}

void LayerDock::resizeEvent(QResizeEvent* )
{
	Content->setFixedWidth(width());
	updateContent();
}

MapLayer* LayerDock::activeLayer()
{
 	return ((LayerWidget *)butGroup->checkedButton())->getMapLayer();
}

void LayerDock::layerChanged(LayerWidget*, bool adjustViewport)
{
	emit(layersChanged(adjustViewport));
}

