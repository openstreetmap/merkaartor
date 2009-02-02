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
		LayerWidget(MapLayer* aLayer, QWidget* aParent = 0);
		virtual ~LayerWidget();

		virtual QSize sizeHint () const;
		virtual QSize minimumSizeHint () const;

		virtual void paintEvent(QPaintEvent* anEvent);
		virtual void mouseReleaseEvent(QMouseEvent* anEvent);

		virtual MapLayer* getMapLayer();
		virtual void setLayerVisible(bool b);
		virtual void setLayerReadonly(bool b);
		virtual	void initActions();

		QMenu* getAssociatedMenu();

	protected:
		virtual void contextMenuEvent(QContextMenuEvent* anEvent);

		virtual void checkStateSet();
		virtual void nextCheckState();

		MapLayer* theLayer;
		QPixmap visibleIcon;
		QPixmap hiddenIcon;
		QBrush backColor;
		QMenu* ctxMenu;
		QAction* closeAction;
		QAction* actZoom;
		QAction* actVisible;
		QAction* actReadonly;
		QMenu* associatedMenu;

	signals:
		void layerSelected(LayerWidget *);
		void layerChanged(LayerWidget *, bool adjustViewport);
		void layerClosed(MapLayer *);
		void layerCleared(MapLayer *);
		void layerZoom(MapLayer *);

	protected slots:
		void setOpacity(QAction*);
		void zoomLayer(bool);
		void visibleLayer(bool);
		void readonlyLayer(bool);
		void close();
		void clear();
		void associatedAboutToShow();
};

class DrawingLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		DrawingLayerWidget(DrawingMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~DrawingLayerWidget() {};

	public:
		virtual void initActions();

	private:
		//DrawingMapLayer* theLayer;
};

class TrackLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		TrackLayerWidget(TrackMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~TrackLayerWidget();

	public:
		virtual void initActions();

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
		virtual void initActions();

	private:
		//ImageMapLayer* theLayer;

// 		QActionGroup* actgrAdapter;
// 		QActionGroup* actgrWms;
// 		QActionGroup* actgrTms;

#ifdef USE_GDAL
		QAction* actShape;
#endif
#ifdef YAHOO
		QAction* actLegalYahoo;
#endif
#ifdef YAHOO_ILLEGAL
		QAction* actYahoo;
#endif
#ifdef GOOGLE_ILLEGAL
		QAction* actGoogle;
#endif
#ifdef MSLIVEMAP_ILLEGAL
		QAction* actVirtEarth;
#endif
		QAction* actNone;
// 		QAction* actOSM;
		QMenu* wmsMenu;
		QMenu* tmsMenu;

	private slots:
		void setWms(QAction*);
		void setTms(QAction*);
#ifdef USE_GDAL
		void setShape(bool);
#endif
#ifdef YAHOO
		void setLegalYahoo(bool);
#endif
#ifdef YAHOO_ILLEGAL
		void setYahoo(bool);
#endif
#ifdef GOOGLE_ILLEGAL
		void setGoogle(bool);
#endif
#ifdef MSLIVEMAP_ILLEGAL
		void setMsVirtualEarth(bool);
#endif
// 		void setOSM(bool);
		void setNone(bool);
};

class DirtyLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		DirtyLayerWidget(DirtyMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~DirtyLayerWidget() {};

	public:
		virtual void initActions();
};

class UploadedLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		UploadedLayerWidget(UploadedMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~UploadedLayerWidget() {};

	public:
		virtual void initActions();
};

class ExtractedLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		ExtractedLayerWidget(ExtractedMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~ExtractedLayerWidget() {};

	public:
		virtual void initActions();
};

class OsbLayerWidget : public LayerWidget
{
	Q_OBJECT

	public:
		OsbLayerWidget(OsbMapLayer* aLayer, QWidget* aParent = 0);
		virtual ~OsbLayerWidget() {};

	public:
		virtual void initActions();
};


#endif


