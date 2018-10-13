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
class MapAdapter;
class Interaction;
class ImageMapLayer;

class MapViewPrivate;

class MapView :	public QWidget
{
    Q_OBJECT
    friend class RenderTile;

public:
    MapView(QWidget* parent);
public:
    ~MapView();

    MainWindow* main();
    virtual void setDocument(Document* aDoc);
    Document* document();
    Interaction* interaction();
    void setInteraction(Interaction* anInteraction);

    void drawFeatures(QPainter & painter);
    void drawFeaturesSync(QPainter & P);
    void drawLatLonGrid(QPainter & painter);
    void drawDownloadAreas(QPainter & painter);
    void drawScale(QPainter & painter);

    void panScreen(QPoint delta) ;
    void rotateScreen(QPoint center, qreal angle);
    void invalidate(bool updateWireframe, bool updateOsmMap, bool updateBgMap);

    virtual void paintEvent(QPaintEvent* anEvent);
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mouseDoubleClickEvent(QMouseEvent* event);
    virtual void wheelEvent(QWheelEvent* ev);
    virtual void resizeEvent(QResizeEvent *event);
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dragMoveEvent(QDragMoveEvent *event);
    virtual void dropEvent(QDropEvent *event);

    Projection& projection();
    QTransform& transform();
    QTransform& invertedTransform();
    QPoint toView(const Coord& aCoord) const;
    QPoint toView(Node* aPt) const;
    Coord fromView(const QPoint& aPt) const;

    bool isSelectionLocked();
    void lockSelection();
    void unlockSelection();

    void setViewport(const CoordBox& Map, const QRect& Screen);
    const CoordBox& viewport() const;
    static void transformCalc(QTransform& theTransform, const Projection& theProjection, const qreal& theRotation, const CoordBox& TargetMap, const QRect& Screen);
    qreal pixelPerM() const;

    void setBackgroundOnlyPanZoom(bool val);

    void zoom(qreal d, const QPoint& Around);
    void zoom(qreal d, const QPoint& Around, const QRect& Screen);
    void adjustZoomToBoris();
    void setCenter(Coord& Center, const QRect& Screen);

    void setInteracting(bool val);

    bool toXML(QXmlStreamWriter& stream);
    void fromXML(QXmlStreamReader& stream);

    RendererOptions renderOptions();
    void setRenderOptions(const RendererOptions& opt);

    void stopRendering();
    void resumeRendering();

    qreal nodeWidth();

    QString toPropertiesHtml();

    void on_imageReceived(ImageMapLayer *aLayer);

private:
    void drawGPS(QPainter & painter);
    void updateStaticBackground();
    void updateWireframe();

    MainWindow* Main;
    QPixmap* StaticBackground;
    QPixmap* StaticWireframe;
    QPixmap* StaticTouchup;
    bool StaticMapUpToDate;
    bool SelectionLocked;
    QLabel* lockIcon;

    void viewportRecalc(const QRect& Screen);

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

    virtual void renderingDone();

signals:
    void viewportChanged();

protected:
    MapViewPrivate* p;
};

#endif


