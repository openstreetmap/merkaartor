#include "LayerDock.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Map/MapDocument.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter> 

#define LINEHEIGHT 20

LayerWidget::LayerWidget(MainWindow* aMain, QWidget* aParent)
: QWidget(aParent), Main(aMain), ActiveLayer(0)
{
}

void LayerWidget::paintEvent(QPaintEvent*)
{
	QPainter P(this);
	P.fillRect(rect(),QColor(255,255,255));
	P.setPen(QColor(0,0,0));
	for (unsigned int i=0; i<Main->document()->numLayers(); ++i)
	{
		P.drawLine(0,(i+1)*LINEHEIGHT,width(),(i+1)*LINEHEIGHT);
		P.drawText(23,(i+2)*LINEHEIGHT-6,Main->document()->layer(i)->name());
		if (Main->document()->layer(i)->isVisible())
			P.drawPixmap(2,(i+1)*LINEHEIGHT+2,QPixmap(":Icons/eye.xpm"));
		else
			P.drawPixmap(2,(i+1)*LINEHEIGHT+2,QPixmap(":Icons/empty.xpm"));
	}
	P.drawLine(0,(Main->document()->numLayers()+1)*LINEHEIGHT,width(),(Main->document()->numLayers()+1)*LINEHEIGHT);
	P.fillRect(20,(ActiveLayer+1)*LINEHEIGHT+1,width()-19,LINEHEIGHT-1,QBrush(palette().highlight()));
	P.setPen(palette().highlightedText().color());
	P.drawText(23,(ActiveLayer+2)*LINEHEIGHT-6,Main->document()->layer(ActiveLayer)->name());

}

void LayerWidget::mouseReleaseEvent(QMouseEvent* anEvent)
{
	unsigned int Idx = anEvent->pos().y()/LINEHEIGHT;
	if (Idx && (Idx <= Main->document()->numLayers()))
	{
		MapLayer* Layer = Main->document()->layer(Idx-1);
		if (anEvent->pos().x()<20)
		{
			Layer->setVisible(!Layer->isVisible());
			if (Layer->type() == MapLayer::ImageLayer)
				Main->view()->checkLayerManager();
			update();
			Main->invalidateView();
		}
		else
		{
			ActiveLayer = Idx-1;
			update();
		}
	}
}


void LayerWidget::updateContent()
{
}


MapLayer* LayerWidget::activeLayer()
{
	return Main->document()->layer(ActiveLayer);
}

LayerDock::LayerDock(MainWindow* aMain)
: QDockWidget(aMain), Main(aMain)
{
	setMinimumSize(220,100);
	setWindowTitle(tr("Layers"));
	Scroller = new QScrollArea(this);
	setWidget(Scroller);
	Scroller->setBackgroundRole(QPalette::Base);
	Scroller->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	Content = new LayerWidget(Main, Scroller);
	Scroller->setWidget(Content);
	updateContent();
}

LayerDock::~LayerDock()
{
}

void LayerDock::updateContent()
{
	Content->setFixedSize(Scroller->width(),(Main->document()->numLayers()+1)*LINEHEIGHT+1);
}

void LayerDock::resizeEvent(QResizeEvent* )
{
	Scroller->setFixedWidth(width());
	updateContent();
}

MapLayer* LayerDock::activeLayer()
{
	return Content->activeLayer();
}




