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
		virtual void contextMenuEvent(QContextMenuEvent* anEvent);

	protected:
		virtual void checkStateSet();

		MapLayer* theLayer;
		QPixmap visibleIcon;
		QPixmap hiddenIcon;
		QBrush backColor;
		QMenu* ctxMenu;

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

class TrackLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		TrackLayerWidget(TrackMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~TrackLayerWidget();

	private slots:
		void extractLayer(bool);
};

class ImageLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		ImageLayerWidget(ImageMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~ImageLayerWidget();

	public:
		void initActions();

	private:
		//ImageMapLayer* theLayer;

// 		QActionGroup* actgrAdapter;
// 		QActionGroup* actgrWms;
// 		QActionGroup* actgrTms;
#ifdef yahoo_illegal
		QAction* actYahoo;
#endif
#ifdef google_illegal
		QAction* actGoogle;
#endif
		QAction* actNone;
// 		QAction* actOSM;
		QMenu* wmsMenu;
		QMenu* tmsMenu;

	private slots:
		void setWms(QAction*);
		void setTms(QAction*);
#ifdef yahoo_illegal
		void setYahoo(bool);
#endif
#ifdef google_illegal
		void setGoogle(bool);
#endif
// 		void setOSM(bool);
		void setNone(bool);
};



#endif


