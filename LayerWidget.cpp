#include "LayerWidget.h"

#include "MainWindow.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "MapView.h"
#include "Preferences/MerkaartorPreferences.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#define SAFE_DELETE(x) {delete (x); x = NULL;}

#define LINEHEIGHT 20

LayerWidget::LayerWidget(QWidget* aParent)
: QAbstractButton(aParent)
{
	setCheckable(true);
	setAutoExclusive(true) ;
	setFocusPolicy(Qt::NoFocus);
	visibleIcon = QPixmap(":Icons/eye.xpm");
	hiddenIcon = QPixmap(":Icons/empty.xpm");
}

QSize LayerWidget::minimumSizeHint () const
{
	return QSize(100, LINEHEIGHT);
}

QSize LayerWidget::sizeHint () const
{
	return QSize(100, LINEHEIGHT);
}

void LayerWidget::paintEvent(QPaintEvent*)
{
	QPainter P(this);

	P.drawLine(rect().bottomLeft(), rect().bottomRight());
	if (theLayer->isSelected()) {
		P.fillRect(rect().adjusted(20,0,0,-1),QBrush(palette().highlight()));
//		P.fillRect(20, 1, width()-19, rect().height()-1, QBrush(palette().highlight()));
		P.setPen(palette().highlightedText().color());
		P.drawText(rect().adjusted(23,0,0,-1), Qt::AlignLeft | Qt::AlignVCenter , theLayer->name());
	} else {
		P.fillRect(rect().adjusted(0,0,0,-1),backColor);
		P.setPen(QColor(0,0,0));
		P.drawText(rect().adjusted(23,0,0,-1), Qt::AlignLeft | Qt::AlignVCenter , theLayer->name());
	}

	if (theLayer->isVisible())
		P.drawPixmap(QPoint(2, rect().center().y()-visibleIcon.height()/2), visibleIcon);
	else
		P.drawPixmap(QPoint(2, rect().center().y()-hiddenIcon.height()/2), hiddenIcon);
}

void LayerWidget::mouseReleaseEvent(QMouseEvent* anEvent)
{
	if (anEvent->pos().x()<20)
	{
		theLayer->setVisible(!theLayer->isVisible());
		anEvent->ignore();
		update();
		emit(layerChanged(this, false));
	}
	else
	{
		if (!(dynamic_cast<ImageLayerWidget *>(this)))
			toggle();
	}
}

void LayerWidget::checkStateSet()
{
	theLayer->setSelected(isChecked());
	//emit (layerChanged(this));
}

MapLayer* LayerWidget::getMapLayer()
{
	return theLayer;
}

// DrawingLayerWidget

DrawingLayerWidget::DrawingLayerWidget(DrawingMapLayer* aLayer, QWidget* aParent)
: LayerWidget(aParent)
{
	theLayer = aLayer;
	backColor = QColor(255,255,255);
}

// ImageLayerWidget

ImageLayerWidget::ImageLayerWidget(ImageMapLayer* aLayer, QWidget* aParent)
: LayerWidget(aParent), actgrWms(0), imageMenu(0)
{
	theLayer = aLayer;
	backColor = QColor(128,128,128);
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

ImageLayerWidget::~ImageLayerWidget()
{
}

void ImageLayerWidget::setWms(QAction* act)
{
	QStringList server = act->data().toStringList();
	MerkaartorPreferences::instance()->setSelectedWmsServer(server[6].toInt());

	((ImageMapLayer *)theLayer)->setMapAdapter(Bg_Wms);
	theLayer->setVisible(true);

	this->update(rect());
	emit (layerChanged(this, true));
}

#ifdef yahoo_illegal
void ImageLayerWidget::setYahoo(bool)
{
	((ImageMapLayer *)theLayer)->setMapAdapter(Bg_Yahoo_illegal);
	theLayer->setVisible(true);

	this->update(rect());
	emit (layerChanged(this, true));
}
#endif

void ImageLayerWidget::setNone(bool)
{
	((ImageMapLayer *)theLayer)->setMapAdapter(Bg_None);

	this->update(rect());
	emit (layerChanged(this, true));
}

void ImageLayerWidget::setOSM(bool)
{
	((ImageMapLayer *)theLayer)->setMapAdapter(Bg_OSM);
	theLayer->setVisible(true);

	this->update(rect());
	emit (layerChanged(this, true));
}

void ImageLayerWidget::initWmsActions()
{
	//if (actgrWms)
	//	delete actgrWms;
	//actgrWms = new QActionGroup(this);

	SAFE_DELETE(imageMenu);

	wmsMenu = new QMenu(MerkaartorPreferences::instance()->getBgTypes()[Bg_Wms], this);

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

	imageMenu = new QMenu(this);
	imageMenu->addAction(actNone);

	imageMenu->addMenu(wmsMenu);
	connect(wmsMenu, SIGNAL(triggered(QAction*)), this, SLOT(setWms(QAction*)));

	imageMenu->addAction(actOSM);
#ifdef yahoo_illegal
	imageMenu->addAction(actYahoo);
#endif
}

void ImageLayerWidget::contextMenuEvent(QContextMenuEvent* anEvent)
{
	imageMenu->exec(anEvent->globalPos());
}


