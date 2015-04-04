#include "Global.h"

#include "OsmRenderLayer.h"

#include "Document.h"
#include "MapRenderer.h"
#include "MerkaartorPreferences.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent>
#endif

inline uint qHash(const QPoint& p)
{
    return (uint)(p.y() + (p.x() << 16));
}

#define TILE_SIZE 256
QList<TILE_TYPE> tiles;
QReadWriteLock tileLock;
#define TILE_CONSTRUCTOR(x, y) QPoint(x, y)
#define TILE_X(t) t.x()
#define TILE_Y(t) t.y()

class TileCache : public QObject
{
public:
    TileCache(QObject* parent) : QObject(parent) {}
    void insert(const TILE_TYPE& k, QImage* v)
    {
        m_tileCache.insert(k, v);
    }
    bool contains(const TILE_TYPE& k)
    {
        return m_tileCache.contains(k);
    }
    QImage* get(const TILE_TYPE& k)
    {
        return m_tileCache[k];
    }

private:
    QCache<const TILE_TYPE, QImage> m_tileCache;
};

TileCache* tileCache;

class RenderTile
{
public:
    RenderTile(OsmRenderLayer* orl)
        : p(orl) { }

    typedef void result_type;

    void operator()(const TILE_TYPE& theTile)
    {
        if (!p->theDocument)
            return;

        TILE_TYPE tile = theTile;

        QPointF projTL((TILE_X(tile)*p->tileSizeCoordW)+p->tileOriginCoord.x(), (TILE_Y(tile)*p->tileSizeCoordH)+p->tileOriginCoord.y());
        QPointF projBR(((TILE_X(tile)+1)*p->tileSizeCoordW)+p->tileOriginCoord.x(), ((TILE_Y(tile)+1)*p->tileSizeCoordH)+p->tileOriginCoord.y());
        QRectF projR(projTL, projBR);

#define TILE_SURROUND 2.0
        qreal z = TILE_SURROUND * ((TILE_SIZE*TILE_SURROUND) / (p->theTransform.m11()*projR.width()*TILE_SURROUND));    // Adjust to main transform
        qreal dlat = (projR.top()-projR.bottom())*(z-1)/2;
        qreal dlon = (projR.right()-projR.left())*(z-1)/2;
        projR.setBottom(projR.bottom()-dlat);
        projR.setLeft(projR.left()-dlon);
        projR.setTop(projR.top()+dlat);
        projR.setRight(projR.right()+dlon);

        Coord tl = p->theProjection.inverse2Coord(projR.topLeft());
        Coord br = p->theProjection.inverse2Coord(projR.bottomRight());
        CoordBox invalidRect(tl, br);

        QMap<RenderPriority, QSet <Feature*> > theFeatures;

        for (int i=0; i<p->theDocument->layerSize(); ++i)
            g_backend.getFeatureSet(p->theDocument->getLayer(i), theFeatures, invalidRect, p->theProjection);

        QImage* img = new QImage(TILE_SIZE, TILE_SIZE, QImage::Format_ARGB32);
        img->fill(Qt::transparent);

        QPainter P(img);
        if (M_PREFS->getUseAntiAlias())
            P.setRenderHint(QPainter::Antialiasing);
        MapRenderer r;
        r.render(&P, theFeatures, projR, /*QRect(0, 0, TILE_SIZE, TILE_SIZE)*/QRect(-((TILE_SIZE*TILE_SURROUND)-TILE_SIZE)/2, -((TILE_SIZE*TILE_SURROUND)-TILE_SIZE)/2, TILE_SIZE*TILE_SURROUND, TILE_SIZE*TILE_SURROUND), p->PixelPerM, p->ROptions);
        P.end();
        tileLock.lockForWrite();
        tileCache->insert(tile, img);

        //            if (theFeatures.size())
        //                img->save(QString("c:/temp/%1-%2.png").arg(tile.x()).arg(tile.y()));
        tileLock.unlock();
    }

    OsmRenderLayer* p;
};

/**************************/

OsmRenderLayer::OsmRenderLayer(QObject *parent)
    : QObject(parent)
    , theDocument(0)
{
    tileCache = new TileCache(this);
    connect(&(renderGatheringWatcher), SIGNAL(finished()), SIGNAL(renderingDone()));
}

void OsmRenderLayer::setDocument(Document *aDocument)
{
    theDocument = aDocument;
}

void OsmRenderLayer::setTransform(const QTransform &aTransform)
{
    theTransform = aTransform;
    theInvertedTransform = theTransform.inverted();
}

void OsmRenderLayer::setProjection(const Projection& aProjection)
{
    theProjection = aProjection;
}

void OsmRenderLayer::forceRedraw(const Projection& aProjection, const QTransform &aTransform, const QRect& rect, qreal ppm, const RendererOptions& roptions)
{
    if (renderGathering.isRunning()) {
        renderGathering.cancel();
        renderGathering.waitForFinished();
    }

    if (!theDocument)
        return;

    setProjection(aProjection);
    setTransform(aTransform);

    PixelPerM = ppm;
    ROptions = roptions;

    tileLock.lockForWrite();
    tileCache->deleteLater();
    tileCache = new TileCache(this);
    tileOriginCoord = theInvertedTransform.map(QPointF(rect.topLeft()));

    QPointF tl = theInvertedTransform.map(QPointF(rect.topLeft()));
    QPointF br = theInvertedTransform.map(QPointF(rect.bottomRight())+QPointF(1,1));
    projRect = QRectF(tl, br);

    tileSizeCoordW = (projRect.width()) / rect.width() * TILE_SIZE;
    //            tileSizeCoordH = (projRect.height()) / rect.height() * TILE_SIZE;
    tileSizeCoordH = tileSizeCoordW * projRect.height() / fabs(projRect.height());

    tileViewport.setLeft(((projRect.left()-tileOriginCoord.x()) / tileSizeCoordW) - 1);
    tileViewport.setTop(((projRect.top()-tileOriginCoord.y()) / tileSizeCoordH) - 1);
    tileViewport.setRight(((projRect.right()-tileOriginCoord.x()) / tileSizeCoordW) + 1);
    tileViewport.setBottom(((projRect.bottom()-tileOriginCoord.y()) / tileSizeCoordH) + 1);

    tiles.clear();
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i)
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            TILE_TYPE tile = TILE_CONSTRUCTOR(j, i);
            tiles << tile;
        }
    tileLock.unlock();

    if (tiles.size()) {
        renderGathering = QtConcurrent::map(tiles, RenderTile(this));
        renderGatheringWatcher.setFuture(renderGathering);
    }
}

void OsmRenderLayer::pan(QPoint delta)
{
    if (renderGathering.isRunning()) {
        renderGathering.cancel();
        renderGathering.waitForFinished();
    }

    theTransform.translate((qreal)(delta.x())/theTransform.m11(), (qreal)(delta.y())/theTransform.m22());
    theInvertedTransform = theTransform.inverted();

    projRect.translate(-(qreal)(delta.x())/theTransform.m11(), -(qreal)(delta.y())/theTransform.m22());

    tileViewport.setLeft(((projRect.left()-tileOriginCoord.x()) / tileSizeCoordW) - 1);
    tileViewport.setTop(((projRect.top()-tileOriginCoord.y()) / tileSizeCoordH) - 1);
    tileViewport.setRight(((projRect.right()-tileOriginCoord.x()) / tileSizeCoordW) + 1);
    tileViewport.setBottom(((projRect.bottom()-tileOriginCoord.y()) / tileSizeCoordH) + 1);

    tileLock.lockForRead();
    tiles.clear();
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i)
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            TILE_TYPE tile = TILE_CONSTRUCTOR(j, i);
            if (!tileCache->contains(tile))
                tiles << tile;
        }
    tileLock.unlock();

    if (tiles.size()) {
        renderGathering = QtConcurrent::map(tiles, RenderTile(this));
        renderGatheringWatcher.setFuture(renderGathering);
    }
}

void OsmRenderLayer::drawImage(QPainter *P)
{
    QPointF origin = theTransform.map(tileOriginCoord);
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i)
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            tileLock.lockForRead();
            if (tileCache->contains(TILE_CONSTRUCTOR(j, i))) {
                QPointF tl = QPointF((j*TILE_SIZE)+origin.x(), (i*TILE_SIZE)+origin.y());
                P->drawImage(tl, *(tileCache->get(TILE_CONSTRUCTOR(j, i))));
            }
            tileLock.unlock();
            //            qDebug() << QPoint(j, i) << tl;
        }
}

bool OsmRenderLayer::isRenderingDone()
{
    return renderGathering.isFinished();
}

