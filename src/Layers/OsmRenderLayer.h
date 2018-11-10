#ifndef OSMRENDERLAYER_H
#define OSMRENDERLAYER_H

#include <QObject>
#include <QRect>
#include <QPointF>
#include <QFuture>
#include <QFutureWatcher>
#include <QTransform>

#include "IRenderer.h"
#include "Projection.h"

class Document;
class Projection;

/* Private containers, defined in .cpp */
class TileContainer;
#define TILE_TYPE QPoint

class OsmRenderLayer : public QObject
{
    Q_OBJECT

    friend class RenderTile;

public:
    OsmRenderLayer(QObject*parent=0);
    void setDocument(Document *aDocument);
    void setTransform(const QTransform& aTransform);
    void setProjection(const Projection& aProjection);

    void forceRedraw(const Projection& aProjection, const QTransform &aTransform, const QRect& rect, qreal ppm, const RendererOptions& roptions);
    void pan(QPoint delta);
    void drawImage(QPainter* P);

    bool isRenderingDone();

    void stopRendering();
    void resumeRendering();

signals:
    void renderingDone();

protected:
    Document* theDocument;

    QRectF projRect;
    qreal tileSizeCoordW;
    qreal tileSizeCoordH;
    QPointF tileOriginCoord;
    QRect tileViewport;

    QFuture<void> renderGathering;
    QFutureWatcher<void> renderGatheringWatcher;

    QTransform theTransform;
    QTransform theInvertedTransform;
    Projection theProjection;

    qreal PixelPerM;
    RendererOptions ROptions;

    TileContainer* tiles;
    /* Contains a list of tiles to be rendered using QtConcurrent. */
    QList<TILE_TYPE> tilesToRender;
    QReadWriteLock tileLock; /* Protects 'tiles' variable */

    /* Read locks indicate rendering threads, Write lock blocks them. This is a
     * global object used to block all rendering used in some workarounds.  */
    static QReadWriteLock renderLock;
};

#endif // OSMRENDERLAYER_H
