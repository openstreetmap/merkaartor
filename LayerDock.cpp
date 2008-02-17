#include "LayerDock.h"

#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "MapView.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

#define LINEHEIGHT 20

LayerWidget::LayerWidget(MainWindow* aMain, QWidget* aParent)
: QWidget(aParent), Main(aMain), ActiveLayer(0), actgrWms(0), wmsMenu(0)
{
	//actgrAdapter = new QActionGroup(this);

	actNone = new QAction(MerkaartorPreferences::instance()->getBgTypes()[Bg_None], this);
	//actNone->setCheckable(true);
	actNone->setChecked((MerkaartorPreferences::instance()->getBgType() == Bg_None));
	connect(actNone, SIGNAL(triggered(bool)), this, SLOT(setNone(bool)));

	actOSM = new QAction(MerkaartorPreferences::instance()->getBgTypes()[Bg_OSM], this);
	//actNone->setCheckable(true);
	actOSM->setChecked((MerkaartorPreferences::instance()->getBgType() == Bg_OSM));
	connect(actOSM, SIGNAL(triggered(bool)), this, SLOT(setOSM(bool)));

#ifdef yahoo_illegal
	actYahoo = new QAction(MerkaartorPreferences::instance()->getBgTypes()[Bg_Yahoo_illegal], this);
	//actYahoo->setCheckable(true);
	actYahoo->setChecked((MerkaartorPreferences::instance()->getBgType() == Bg_Yahoo_illegal));
	connect(actYahoo, SIGNAL(triggered(bool)), this, SLOT(setYahoo(bool)));
#endif
	initWmsActions();
}

void LayerWidget::setWms(QAction* act)
{
	QStringList server = act->data().toStringList();
	MerkaartorPreferences::instance()->setSelectedWmsServer(server[6].toInt());

	MapLayer* Layer = Main->document()->getBgLayer();

	Layer->setMapAdapter(Bg_Wms, Main);
	Layer->setVisible(true);

	Main->view()->invalidate();
	this->update(rect());
}

#ifdef yahoo_illegal
void LayerWidget::setYahoo(bool)
{
	MapLayer* Layer = Main->document()->getBgLayer();
	Layer->setMapAdapter(Bg_Yahoo_illegal,  Main);
	Layer->setVisible(true);

	Main->view()->invalidate();
	this->update(rect());
}
#endif

void LayerWidget::setNone(bool)
{
	MapLayer* Layer = Main->document()->getBgLayer();
	Layer->setMapAdapter(Bg_None, Main);

	Main->view()->invalidate();
	this->update(rect());
}

void LayerWidget::setOSM(bool)
{
	MapLayer* Layer = Main->document()->getBgLayer();
	Layer->setMapAdapter(Bg_OSM,  Main);
	Layer->setVisible(true);

	Main->view()->invalidate();
	this->update(rect());
}

void LayerWidget::initWmsActions()
{
	//if (actgrWms)
	//	delete actgrWms;
	//actgrWms = new QActionGroup(this);

	SAFE_DELETE(wmsMenu);
	wmsMenu = new QMenu(MerkaartorPreferences::instance()->getBgTypes()[Bg_Wms]);

	QStringList Servers = MerkaartorPreferences::instance()->getWmsServers();
	for (int i=0; i<Servers.size(); i+=6) {
		QStringList oneServer;
		oneServer.append(Servers[i]);
		oneServer.append(Servers[i+1]);
		oneServer.append(Servers[i+2]);
		oneServer.append(Servers[i+3]);
		oneServer.append(Servers[i+4]);
		oneServer.append(Servers[i+5]);
		oneServer.append(QString().setNum(int(i/6)));

		QAction* act = new QAction(Servers[i], actgrWms);
		act->setData(oneServer);
		//act->setCheckable(true);
		wmsMenu->addAction(act);
		//actgrAdapter->addAction(act);
		//actgrWms->addAction(act);
		if (MerkaartorPreferences::instance()->getBgType() == Bg_Wms)
			if (int(i/6) == MerkaartorPreferences::instance()->getSelectedWmsServer())
				act->setChecked(true);
	}
	actNone->setChecked((MerkaartorPreferences::instance()->getBgType() == Bg_None));
#ifdef yahoo_illegal
	actYahoo->setChecked((MerkaartorPreferences::instance()->getBgType() == Bg_Yahoo_illegal));
#endif
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

void LayerWidget::contextMenuEvent(QContextMenuEvent* anEvent)
{
	unsigned int Idx = anEvent->pos().y()/LINEHEIGHT;
	if (Idx && (Idx <= Main->document()->numLayers()))
	{
		MapLayer* Layer = Main->document()->layer(Idx-1);

		QMenu menu(this);
		switch (Layer->type()) {
			case MapLayer::ImageLayer:
				menu.addAction(actNone);

				menu.addMenu(wmsMenu);
				connect(wmsMenu, SIGNAL(triggered(QAction*)), this, SLOT(setWms(QAction*)));

				menu.addAction(actOSM);
#ifdef yahoo_illegal
				menu.addAction(actYahoo);
#endif
				menu.exec(anEvent->globalPos());
				break;
			default:
				break;
		}
	}
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
	Content->initWmsActions();
	Content->update();
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
