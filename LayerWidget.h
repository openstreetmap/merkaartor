#ifndef MERKATOR_LAYERWIDGET_H_
#define MERKATOR_LAYERWIDGET_H_

#include "Map/MapLayer.h"

#include <QActionGroup>
#include <QAbstractButton>

class MainWindow;
class MapLayer;

class LayerWidget : public QAbstractButton
{
	Q_OBJECT

	public:
		LayerWidget(QWidget* aParent = 0);
		virtual ~LayerWidget() {};

		virtual QSize sizeHint () const;
		virtual QSize minimumSizeHint () const;

		virtual void paintEvent(QPaintEvent* anEvent);
		virtual void mouseReleaseEvent(QMouseEvent* anEvent);

		virtual MapLayer* getMapLayer();

	protected:
		virtual void checkStateSet();

		MapLayer* theLayer;
		QPixmap visibleIcon;
		QPixmap hiddenIcon;
		QBrush backColor;

	signals:
		void layerChanged(LayerWidget *, bool adjustViewport);
};

class DrawingLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		DrawingLayerWidget(DrawingMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~DrawingLayerWidget() {};

	private:
		//DrawingMapLayer* theLayer;
};

class TrackLayerWidget : public DrawingLayerWidget
{
	Q_OBJECT
};

class ImageLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		ImageLayerWidget(ImageMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~ImageLayerWidget();

	protected:
		void contextMenuEvent(QContextMenuEvent* anEvent);

	public:
		void initWmsActions();

	private:
		//ImageMapLayer* theLayer;

		QActionGroup* actgrAdapter;
		QActionGroup* actgrWms;
#ifdef yahoo_illegal
		QAction* actYahoo;
#endif
		QAction* actNone;
		QAction* actOSM;
		QMenu* wmsMenu;
		QMenu* imageMenu;

	private slots:
		void setWms(QAction*);
#ifdef yahoo_illegal
		void setYahoo(bool);
#endif
		void setOSM(bool);
		void setNone(bool);
};



#endif


