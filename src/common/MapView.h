#ifndef MERKATOR_MAPVIEW_H_
#define MERKATOR_MAPVIEW_H_

#include "Projection.h"
#include "IRenderer.h"

#include <QPixmap>
#include <QWidget>
#include <QShortcut>
#include <QLabel>

class MainWindow;
class Feature;
class Way;
class Document;
class PropertiesDock;
class InfoDock;
class MapAdapter;
class Interaction;
class ImageMapLayer;

class MapViewPrivate;

class MapView :	public QWidget
{
    Q_OBJECT

    public:
        MapView(QWidget* parent);
    public:
        ~MapView();

        MainWindow* main();
        virtual void setDocument(Document* aDoc);
        Document* document();
        virtual void launch(Interaction* anInteraction);
        Interaction* interaction();
        virtual Interaction * defaultInteraction();

        void drawFeatures(QPainter & painter);
        void drawLatLonGrid(QPainter & painter);
        void drawDownloadAreas(QPainter & painter);
        void drawScale(QPainter & painter);

        void panScreen(QPoint delta) ;
        void rotateScreen(QPoint center, qreal angle);
        void invalidate(bool updateStaticBuffer, bool updateMap);

        virtual void paintEvent(QPaintEvent* anEvent);
        virtual void mousePressEvent(QMouseEvent * event);
        virtual void mouseReleaseEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void mouseDoubleClickEvent(QMouseEvent* event);
        virtual void wheelEvent(QWheelEvent* ev);
        virtual void resizeEvent(QResizeEvent *event);
        #ifdef GEOIMAGE
        virtual void dragEnterEvent(QDragEnterEvent *event);
        virtual void dragMoveEvent(QDragMoveEvent *event);
        virtual void dropEvent(QDropEvent *event);
        #endif // GEOIMAGE

        Projection& projection();
        QTransform& transform();
        QTransform& invertedTransform();
        QPoint toView(const Coord& aCoord) const;
        QPoint toView(Node* aPt) const;
        Coord fromView(const QPoint& aPt) const;

        PropertiesDock* properties();
        //InfoDock* info();

        bool isSelectionLocked();
        void lockSelection();
        void unlockSelection();

        void setViewport(const CoordBox& Map);
        void setViewport(const CoordBox& Map, const QRect& Screen);
        const CoordBox& viewport() const;
        static void transformCalc(QTransform& theTransform, const Projection& theProjection, const qreal& theRotation, const CoordBox& TargetMap, const QRect& Screen);
        qreal pixelPerM() const;
        qreal nodeWidth() const;

        void zoom(qreal d, const QPoint& Around);
        void zoom(qreal d, const QPoint& Around, const QRect& Screen);
        void adjustZoomToBoris();
        void setCenter(Coord& Center, const QRect& Screen);

        bool toXML(QXmlStreamWriter& stream);
        void fromXML(QXmlStreamReader& stream);

        RendererOptions renderOptions();
        void setRenderOptions(const RendererOptions& opt);

        QString toPropertiesHtml();

private:
        void drawGPS(QPainter & painter);
        void updateStaticBackground();
        void updateStaticBuffer();

        MainWindow* Main;
        Projection theProjection;
        Document* theDocument;
        Interaction* theInteraction;
        QPixmap* StaticBackground;
        QPixmap* StaticBuffer;
        bool StaticMapUpToDate;
        bool SelectionLocked;
        QLabel* lockIcon;
        QList<Feature*> theSnapList;

        void viewportRecalc(const QRect& Screen);

        #ifdef GEOIMAGE
        Node *dropTarget;
        #endif

        int numImages;

        QShortcut* MoveLeftShortcut;
        QShortcut* MoveRightShortcut;
        QShortcut* MoveUpShortcut;
        QShortcut* MoveDownShortcut;
        QShortcut* ZoomInShortcut;
        QShortcut* ZoomOutShortcut;

    public slots:
        virtual void on_MoveLeft_activated();
        virtual void on_MoveRight_activated();
        virtual void on_MoveUp_activated();
        virtual void on_MoveDown_activated();
        virtual void zoomIn();
        virtual void zoomOut();

    signals:
        void interactionChanged(Interaction* anInteraction);
        void viewportChanged();

    protected:
        bool event(QEvent *event);

    protected slots:
        void on_imageRequested(ImageMapLayer*);
        void on_imageReceived(ImageMapLayer*);
        void on_loadingFinished(ImageMapLayer*);
        void on_customContextMenuRequested(const QPoint & pos);

    private:
        MapViewPrivate* p;
};

#endif


