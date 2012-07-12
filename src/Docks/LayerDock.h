#ifndef MERKATOR_LAYERDOCK_H_
#define MERKATOR_LAYERDOCK_H_

#include "MDockAncestor.h"
#include <QtGui/QScrollArea>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QButtonGroup>
#include <QTabBar>

class Layer;
class LayerWidget;
class LayerDockPrivate;

class LayerDock : public MDockAncestor
{
    Q_OBJECT

    public:
        LayerDock();
    public:
        ~LayerDock(void);

        void createContent();
        //void updateContent();
        void resizeEvent(QResizeEvent* anEvent);

        void clearLayers();
        void addLayer(Layer* aLayer);
        void deleteLayer(Layer* aLayer);

        Layer* getSelectedLayer();

    private slots:
        void layerChanged(LayerWidget*, bool adjustViewport);
        void layerClosed(Layer*);
        void layerCleared(Layer*);
        void layerZoom(Layer*);
        void layerProjection(const QString&);

        void tabChanged(int idx);
        void tabContextMenuRequested(const QPoint& pos);
        void TabShowAll(bool);
        void TabHideAll(bool);

        void showAllLayers(bool);
        void hideAllLayers(bool);
        void readonlyAllLayers(bool);
        void readonlyNoneLayers(bool);
        void closeLayers(bool);
        void resetLayers();

    signals:
        void layersChanged(bool adjustViewport);
        void layersClosed();
        void layersCleared();
        void layersProjection(const QString&);

    protected:
        LayerDockPrivate* p;
#if QT_VERSION < 0x040500
        virtual bool event (QEvent* ev);
#endif

        virtual void contextMenuEvent(QContextMenuEvent* anEvent);
        virtual void mousePressEvent ( QMouseEvent * event );

        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dragLeaveEvent(QDragLeaveEvent *event);
        void dropEvent(QDropEvent *event);

    private:
        void changeEvent(QEvent*);
        void retranslateUi();
        void retranslateTabBar();
};

#endif


