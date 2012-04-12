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

class IDocument;
class Projection;

class OsmRenderLayer : public QObject
{
    Q_OBJECT

    friend class RenderTile;

public:
    OsmRenderLayer(QObject*parent=0);
    void setDocument(IDocument *aDocument);
    void setTransform(const QTransform& aTransform);
    void setProjection(const Projection& aProjection);

    void forceRedraw(const Projection& aProjection, const QTransform &aTransform, const QRect& rect, qreal ppm, const RendererOptions& roptions);
    void pan(QPoint delta);
    void drawImage(QPainter* P);

    bool isRenderingDone();

signals:
    void renderingDone();

protected:
    IDocument* theDocument;

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
};

#endif // OSMRENDERLAYER_H
