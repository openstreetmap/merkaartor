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
#define TILE_CONSTRUCTOR(x, y) QPoint(x, y)
#define TILE_X(t) t.x()
#define TILE_Y(t) t.y()

/* Static member declaration. */
QReadWriteLock OsmRenderLayer::renderLock;

/**
 * This is a helper class to manage rendered tiles and their lifecycle. Any
 * reference to the images here can vanish at any point in time. Do not escape
 * the pointers!
 */
class TileContainer : public QObject
{
public:
    TileContainer(QObject* parent) : QObject(parent) {}
    /**
     * Insert and take ownership of the image contained. Replaced entries will
     * be automatically deleted.
     */
    void insert(const TILE_TYPE& k, QImage* v)
    {
        if (m_container.contains(k)) {
            delete m_container.value(k);
        }
        m_container.insert(k, v);
    }
    bool contains(const TILE_TYPE& k)
    {
        return m_container.contains(k);
    }
    QImage* get(const TILE_TYPE& k)
    {
        return m_container.value(k, nullptr);
    }
    void clear() {
        for ( auto value : m_container ) {
            delete value;
        }
        m_container.clear();
    }
private:
    QHash<const TILE_TYPE, QImage*> m_container;
};

/**
 * A helper class for QtConcurrent::map(). An instance is created and the
 * operator() is called for each element that needs processing.
 */
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

        if (!p->renderLock.tryLockForRead()) return;
        p->theDocument->lockPainters();

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

        Coord tl = p->theProjection.inverse(projR.topLeft());
        Coord br = p->theProjection.inverse(projR.bottomRight());
        CoordBox invalidRect(tl, br);

        QMap<RenderPriority, QSet <Feature*> > theFeatures;

        g_backend.delayDeletes();
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
        g_backend.resumeDeletes();
        p->theDocument->unlockPainters();
        p->renderLock.unlock();

        /* Insert the tile into the results map. Take care to remove the original item first. */
        p->tileLock.lockForWrite();
        p->tiles->insert(tile,img);
        p->tileLock.unlock();
    }

    OsmRenderLayer* p;
};

/**************************/

OsmRenderLayer::OsmRenderLayer(QObject *parent)
    : QObject(parent)
    , theDocument(0)
    , tiles(new TileContainer(this))
{
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

    if (!renderLock.tryLockForRead()) return;

    setProjection(aProjection);
    setTransform(aTransform);

    PixelPerM = ppm;
    ROptions = roptions;

    /* Clear the cache and rendered tiles. Any settings could have changed. */
    tileLock.lockForWrite();
    tiles->clear();
    tileLock.unlock();

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

    tilesToRender.clear();
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i) {
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            TILE_TYPE tile = TILE_CONSTRUCTOR(j, i);
            tilesToRender << tile;
        }
    }

    if (tilesToRender.size()) {
        renderGathering = QtConcurrent::map(tilesToRender, RenderTile(this));
        renderGatheringWatcher.setFuture(renderGathering);
    }

    renderLock.unlock();
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

    tileLock.lockForWrite();
    tilesToRender.clear();
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i)
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            TILE_TYPE tile = TILE_CONSTRUCTOR(j, i);
            if (!tiles->contains(tile)) {
                tilesToRender << tile;
            }
        }
    tileLock.unlock();

    if (tilesToRender.size()) {
        renderGathering = QtConcurrent::map(tilesToRender, RenderTile(this));
        renderGatheringWatcher.setFuture(renderGathering);
    }
}

void OsmRenderLayer::drawImage(QPainter *P)
{
    tileLock.lockForRead();
    QPointF origin = theTransform.map(tileOriginCoord);
    for (int i=tileViewport.top(); i<=tileViewport.bottom(); ++i) {
        for (int j=tileViewport.left(); j<=tileViewport.right(); ++j) {
            if (tiles->contains(TILE_CONSTRUCTOR(j, i))) {
                QPointF tl = QPointF((j*TILE_SIZE)+origin.x(), (i*TILE_SIZE)+origin.y());
                P->drawImage(tl, *(tiles->get(TILE_CONSTRUCTOR(j, i))));
            }
            /* In some cases, the image is not accessible. This is OK if we are
             * drawing on screen and not everything is ready yet. It might
             * cause trouble when printing, but the code should wait until the
             * rendering is done in that case. */
        }
    }
    tileLock.unlock();
}

bool OsmRenderLayer::isRenderingDone()
{
    return renderGathering.isFinished();
}

void OsmRenderLayer::stopRendering() {
    renderLock.lockForWrite();
}

void OsmRenderLayer::resumeRendering() {
    renderLock.unlock();
}
