#include "MapView.h"
#include "ImageMapLayer.h"

#include "Document.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Maps/Projection.h"

#include "IMapAdapterFactory.h"
#include "IMapAdapter.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
#include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/tilemapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#include "QMapControl/WmscMapAdapter.h"

#include <QLocale>
#include <QPainter>
#include <QMessageBox>

#include "LayerWidget.h"
#include "Features.h"

#include "ui_LicenseDisplayDialog.h"

// ImageMapLayerPrivate

class ImageMapLayerPrivate
{
public:
    QUuid bgType;
    IMapAdapter* theMapAdapter;

    QPixmap pm;
    Projection theProjection;
    QString selServer;

    IImageManager* theImageManager;
    BrowserImageManager* theBrowserImageManager;
    ImageManager* theNetworkImageManager;

    TileMapAdapter* tmsa;
    WMSMapAdapter* wmsa;
    WmscMapAdapter* wmsca;
    QRect pr;
    QTransform theTransform;
    CoordBox Viewport;

public:
    ImageMapLayerPrivate()
    {
        theMapAdapter = NULL;
        theImageManager = NULL;
        theBrowserImageManager = NULL;
        theNetworkImageManager = NULL;
        tmsa = NULL;
        wmsa = NULL;
        wmsca = NULL;
    }
    ~ImageMapLayerPrivate()
    {
        SAFE_DELETE(theMapAdapter)
        SAFE_DELETE(theImageManager)
    }
};


// ImageMapLayer

ImageMapLayer::ImageMapLayer(const QString & aName)
    : Layer(aName), p(new ImageMapLayerPrivate)
{
    p->bgType = NONE_ADAPTER_UUID;
    setName(tr("Map - None"));
    Layer::setVisible(false);
    setReadonly(true);
}

ImageMapLayer::~ ImageMapLayer()
{
    SAFE_DELETE(p)
}

CoordBox ImageMapLayer::boundingBox()
{
    if (p->bgType == SHAPE_ADAPTER_UUID && isVisible())
        return Layer::boundingBox();
    else
        if (!p->theMapAdapter || p->theMapAdapter->getBoundingbox().isNull())
            return CoordBox();

    p->theProjection.setProjectionType(p->theMapAdapter->projection());
    QRectF r = p->theMapAdapter->getBoundingbox();
    Coord tl = p->theProjection.inverse2Coord(r.topLeft());
    Coord br = p->theProjection.inverse2Coord(r.bottomRight());
    return CoordBox(tl, br);
}

int ImageMapLayer::size() const
{
    return Layer::size();
}

LayerWidget* ImageMapLayer::newWidget(void)
{
//	delete theWidget;
    theWidget = new ImageLayerWidget(this);
    return theWidget;
}

void ImageMapLayer::setEnabled(bool b)
{
    if (!b)
        setMapAdapter(NONE_ADAPTER_UUID);

    Layer::setEnabled(b);
}

void ImageMapLayer::updateWidget()
{
    theWidget->initActions();
    setMapAdapter(M_PREFS->getBackgroundPlugin(), M_PREFS->getSelectedServer());
    theWidget->update();
}

void ImageMapLayer::setVisible(bool b)
{
    Layer::setVisible(b);
    if (p->bgType == NONE_ADAPTER_UUID)
        Layer::setVisible(false);
    M_PREFS->setBgVisible(isVisible());
}

QString ImageMapLayer::projection() const
{
    if (p->theMapAdapter)
        return p->theMapAdapter->projection();

    return "";
}

IImageManager* ImageMapLayer::getImageManger()
{
    return p->theImageManager;
}

IMapAdapter* ImageMapLayer::getMapAdapter()
{
    return p->theMapAdapter;
}

void ImageMapLayer::setMapAdapter(const QUuid& theAdapterUid, const QString& server)
{
    WmsServerList* wsl;
    TmsServerList* tsl;

    if (p->theImageManager)
        p->theImageManager->abortLoading();
    p->theImageManager = NULL;
    on_loadingFinished();
    if (p->theMapAdapter)
        SAFE_DELETE(p->theMapAdapter);
    p->pm = QPixmap();

    QString id = theAdapterUid.toString();
    p->bgType = theAdapterUid;
    M_PREFS->setBackgroundPlugin(theAdapterUid);
    if (p->bgType == NONE_ADAPTER_UUID) {
        setName(tr("Map - None"));
        setVisible(false);
    } else
    if (p->bgType == WMS_ADAPTER_UUID) {
        wsl = M_PREFS->getWmsServers();
        p->selServer = server;
        WmsServer theWmsServer(wsl->value(p->selServer));
        switch (theWmsServer.WmsIsTiled) {
        case 0:
            p->wmsa = new WMSMapAdapter(theWmsServer);
            p->theMapAdapter = p->wmsa;
            setName(tr("Map - WMS - %1").arg(p->theMapAdapter->getName()));
            break;
        case 1:
            p->wmsca = new WmscMapAdapter(theWmsServer);
            p->theMapAdapter = p->wmsca;
            setName(tr("Map - WMS-C - %1").arg(p->theMapAdapter->getName()));
            break;
        case 2:
            p->wmsca = new WmscMapAdapter(theWmsServer);
            p->theMapAdapter = p->wmsca;
            setName(tr("Map - WMS-Tiled - %1").arg(p->theMapAdapter->getName()));
            break;
        }
        id += p->theMapAdapter->getName();
    } else
    if (p->bgType == TMS_ADAPTER_UUID) {
        tsl = M_PREFS->getTmsServers();
        p->selServer = server;
        TmsServer ts = tsl->value(p->selServer);
        p->tmsa = new TileMapAdapter(ts);
        p->theMapAdapter = p->tmsa;

        setName(tr("Map - TMS - %1").arg(p->theMapAdapter->getName()));
        id += p->theMapAdapter->getName();
    } else
    if (p->bgType == SHAPE_ADAPTER_UUID) {
        if (!M_PREFS->getUseShapefileForBackground()) {
            p->bgType = NONE_ADAPTER_UUID;
            setName(tr("Map - None"));
            setVisible(false);
        } else {
#if defined(Q_OS_MAC)
            QDir resources = QDir(QCoreApplication::applicationDirPath());
            resources.cdUp();
            resources.cd("Resources");
            QString world_shp = resources.absolutePath() + "/" + STRINGIFY(WORLD_SHP);
//            setFilename(world_shp);
#else
//            if (QDir::isAbsolutePath(STRINGIFY(WORLD_SHP)))
//                setFilename(STRINGIFY(WORLD_SHP));
//            else
//                setFilename(QCoreApplication::applicationDirPath() + "/" + STRINGIFY(WORLD_SHP));
#endif
        }
            setName(tr("Map - OSB Background"));
            setVisible(true);
    } else
    {
        p->theMapAdapter = M_PREFS->getBackgroundPlugin(p->bgType)->CreateInstance();
        if (p->theMapAdapter) {
            setName(tr("Map - %1").arg(p->theMapAdapter->getName()));
            p->theMapAdapter->setSettings(M_PREFS->getQSettings());
            ImageLayerWidget* theImageWidget = qobject_cast<ImageLayerWidget*>(theWidget);
            Q_ASSERT(theImageWidget);
            connect(p->theMapAdapter, SIGNAL(forceZoom()), theImageWidget, SLOT(zoomLayer()));
            connect(p->theMapAdapter, SIGNAL(forceProjection()), theImageWidget, SLOT(setProjection()));
            connect(p->theMapAdapter, SIGNAL(forceRefresh()), g_Merk_MainWindow, SLOT(invalidateView()));
        } else
            p->bgType = NONE_ADAPTER_UUID;
    }
    // Display License
    if (p->theMapAdapter) {
        QString s = p->theMapAdapter->getLicenseUrl();
        if (!s.isEmpty()) {
            QSettings* set = M_PREFS->getQSettings();
            QStringList AcceptedLicenses = set->value("backgroundImage/AcceptedLicenses").toStringList();
            if (!AcceptedLicenses.contains(id)) {
                QUrl u(s);
                if (u.isValid()) {
                    Ui::LicenseDisplayDialog ui;
                    QDialog dlg(g_Merk_MainWindow);
                    ui.setupUi(&dlg);
                    dlg.setWindowTitle(tr("Licensing Terms: %1").arg(name()));
                    ui.webView->load(u);

                    bool OK = false;
                    while (!OK) {
                        if (dlg.exec()) {
                            if (!ui.cbAgree->isChecked()) {
                                QMessageBox::StandardButton ret = QMessageBox::warning(&dlg, tr("License Terms not accepted"), tr("You have not ticked the checkbox expressing your agreement with the licensing terms.\nAs such, you won't be able to use this source as a map layer.\nIs it really what you meant?"), QMessageBox::Yes|QMessageBox::No, QMessageBox::No);
                                if (ret == QMessageBox::Yes)
                                    OK = true;
                            } else
                                OK = true;
                        }
                    }
                    if (!ui.cbAgree->isChecked()) {
                        SAFE_DELETE(p->wmsa)
                                SAFE_DELETE(p->wmsca)
                                SAFE_DELETE(p->tmsa)
                                if (p->theImageManager)
                                    p->theImageManager->abortLoading();
                        SAFE_DELETE(p->theImageManager)
                                on_loadingFinished();
                        p->theMapAdapter = NULL;
                        p->pm = QPixmap();

                        p->bgType = NONE_ADAPTER_UUID;
                        setName(tr("Map - None"));
                        setVisible(false);
                    } else {
                        AcceptedLicenses << id;
                        set->setValue("backgroundImage/AcceptedLicenses", AcceptedLicenses);
                    }
                }
            }
        }
    }
    if (p->theMapAdapter) {
        switch (p->theMapAdapter->getType()) {
            case IMapAdapter::DirectBackground:
            case IMapAdapter::VectorBackground:
                break;
            case IMapAdapter::BrowserBackground :
#ifdef USE_WEBKIT
                if (!p->theBrowserImageManager) {
                    p->theBrowserImageManager = new BrowserImageManager();
                    connect(p->theBrowserImageManager, SIGNAL(dataRequested()),
                        this, SLOT(on_imageRequested()), Qt::QueuedConnection);
                    connect(p->theBrowserImageManager, SIGNAL(dataReceived()),
                        this, SLOT(on_imageReceived()), Qt::QueuedConnection);
                    connect(p->theBrowserImageManager, SIGNAL(loadingFinished()),
                        this, SLOT(on_loadingFinished()), Qt::QueuedConnection);
                }
                p->theBrowserImageManager->setCacheDir(M_PREFS->getCacheDir());
                p->theBrowserImageManager->setCacheMaxSize(M_PREFS->getCacheSize());
                #ifdef BROWSERIMAGEMANAGER_IS_THREADED
                    m->start();
                #endif // BROWSERIMAGEMANAGER_IS_THREADED
                p->theImageManager = p->theBrowserImageManager;
                p->theMapAdapter->setImageManager(p->theImageManager);
#endif
                break;
            case IMapAdapter::NetworkBackground :
            case IMapAdapter::NetworkDataBackground :
                if (!p->theNetworkImageManager) {
                    p->theNetworkImageManager = new ImageManager();
                    connect(p->theNetworkImageManager, SIGNAL(dataRequested()),
                        this, SLOT(on_imageRequested()), Qt::QueuedConnection);
                    connect(p->theNetworkImageManager, SIGNAL(dataReceived()),
                        this, SLOT(on_imageReceived()), Qt::QueuedConnection);
                    connect(p->theNetworkImageManager, SIGNAL(loadingFinished()),
                        this, SLOT(on_loadingFinished()), Qt::QueuedConnection);
                }
                p->theNetworkImageManager->setCacheDir(M_PREFS->getCacheDir());
                p->theNetworkImageManager->setCacheMaxSize(M_PREFS->getCacheSize());
                p->theImageManager = p->theNetworkImageManager;
                p->theMapAdapter->setImageManager(p->theImageManager);
                break;
        }
    }
}

bool ImageMapLayer::isTiled()
{
    if (!p->theMapAdapter)
        return false;

    return (p->theMapAdapter->isTiled());
}

bool ImageMapLayer::toXML(QDomElement& xParent, bool asTemplate, QProgressDialog * /* progress */)
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
    xParent.appendChild(e);

    e.setAttribute("xml:id", id());
    e.setAttribute("name", name());
    e.setAttribute("alpha", QString::number(getAlpha(),'f',2));
    e.setAttribute("visible", QString((isVisible() ? "true" : "false")));
    e.setAttribute("selected", QString((isSelected() ? "true" : "false")));
    e.setAttribute("enabled", QString((isEnabled() ? "true" : "false")));

    e.setAttribute("bgtype", p->bgType.toString());

    QDomElement c;
    WmsServer ws;
    TmsServer ts;

    if (p->bgType == WMS_ADAPTER_UUID) {
        c = e.ownerDocument().createElement("WmsServer");
        e.appendChild(c);

        c.setAttribute("name", p->selServer);
    } else if (p->bgType == TMS_ADAPTER_UUID) {
        c = e.ownerDocument().createElement("TmsServer");
        e.appendChild(c);

        c.setAttribute("name", p->selServer);
    } else if (p->bgType != NONE_ADAPTER_UUID && !asTemplate) {
        c = e.ownerDocument().createElement("Data");
        e.appendChild(c);

        p->theMapAdapter->toXML(c);
    }

    return OK;
}

ImageMapLayer * ImageMapLayer::fromXML(Document* d, const QDomElement& e, QProgressDialog * /*progress*/)
{
    ImageMapLayer* l = new ImageMapLayer(e.attribute("name"));
    l->blockIndexing(true);
    l->setId(e.attribute("xml:id"));

    l->blockIndexing(false);

    d->addImageLayer(l);
    l->reIndex();

    QDomElement c = e.firstChildElement();
    QString server;
    QUuid bgtype = QUuid(e.attribute("bgtype"));
    if (c.tagName() == "WmsServer") {
        server = c.attribute("name");
        l->setMapAdapter(bgtype, server);
    } else
    if (c.tagName() == "TmsServer") {
        server = c.attribute("name");
        l->setMapAdapter(bgtype, server);
    } else {
        l->setMapAdapter(bgtype, server);
        if (l->getMapAdapter())
            l->getMapAdapter()->fromXML(c);
    }

    l->setAlpha(e.attribute("alpha").toDouble());
    l->setVisible((e.attribute("visible") == "true" ? true : false));
    l->setSelected((e.attribute("selected") == "true" ? true : false));
    l->setEnabled((e.attribute("enabled") == "false" ? false : true));

    return l;
}

void ImageMapLayer::drawImage(QPainter* P)
{
    if (!p->theMapAdapter)
        return;
    // Do not draw if saved pixmap is null as a copy of a null pixmap seems to crash on Mac (fixes #2262)
    if (p->pm.isNull())
        return;

    P->setOpacity(getAlpha());
    P->drawPixmap(0, 0, p->pm);
}

using namespace ggl;

void ImageMapLayer::zoom(double zoom, const QPoint& pos, const QRect& rect)
{
    if (!p->theMapAdapter)
        return;

    if (p->theMapAdapter->getImageManager())
        p->theMapAdapter->getImageManager()->abortLoading();

    QPixmap tpm = p->pm.scaled(rect.size() * zoom, Qt::KeepAspectRatio);
    p->pm.fill(Qt::transparent);
    QPainter P(&p->pm);
    P.drawPixmap(pos - (pos * zoom), tpm);
}

void ImageMapLayer::pan(QPoint delta)
{
    if (!p->theMapAdapter)
        return;
    if (p->pm.isNull())
        return;
    if (p->theMapAdapter->getImageManager())
        p->theMapAdapter->getImageManager()->abortLoading();

#if QT_VERSION < 0x040600
        QPixmap savPix;
        savPix = p->pm.copy();
        p->pm.fill(Qt::transparent);
        QPainter P(&p->pm);
        P.drawPixmap(delta, savPix);
#else
        QRegion exposed;
        p->pm.scroll(delta.x(), delta.y(), p->pm.rect(), &exposed);
        QPainter P(&p->pm);
        P.setClipping(true);
        P.setClipRegion(exposed);
        P.eraseRect(p->pm.rect());
#endif
//        on_imageReceived();
}

void ImageMapLayer::zoom_in()
{
    if (!isTiled())
        return;

    p->theMapAdapter->zoom_in();
}

void ImageMapLayer::zoom_out()
{
    if (!isTiled())
        return;

    p->theMapAdapter->zoom_out();
}

int ImageMapLayer::getCurrentZoom()
{
    if (!isTiled())
        return -1;

    return p->theMapAdapter->getAdaptedZoom();
}

void ImageMapLayer::setCurrentZoom(MapView& theView, const CoordBox& viewport, const QRect& rect)
{
    QRectF vp;
    QRectF fRect(rect);

    if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()) {
        vp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
        vp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
    } else
        vp = p->theProjection.getProjectedViewport(viewport, rect);

    qreal tileWidth, tileHeight;
    int maxZoom = p->theMapAdapter->getAdaptedMaxZoom();
    int tilesizeW = p->theMapAdapter->getTileSizeW();
    int tilesizeH = p->theMapAdapter->getTileSizeH();
    QPointF mapmiddle_px = vp.center();

    // Set zoom level to 0.
    while (p->theMapAdapter->getAdaptedZoom()) {
        p->theMapAdapter->zoom_out();
    }

    tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
    tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
    qreal w = ((qreal)rect.width() / tilesizeW) * tileWidth;
    qreal h = ((qreal)rect.height() / tilesizeH) * tileHeight;
    QPointF upperLeft = QPointF(mapmiddle_px.x() - w/2, mapmiddle_px.y() + h/2);
    QPointF lowerRight = QPointF(mapmiddle_px.x() + w/2, mapmiddle_px.y() - h/2);
    QRectF vlm = QRectF(upperLeft, lowerRight);

    while ((!vp.contains(vlm)) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
        p->theMapAdapter->zoom_in();

        tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
        tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
        w = ((qreal)rect.width() / tilesizeW) * tileWidth;
        h = ((qreal)rect.height() / tilesizeH) * tileHeight;
        upperLeft = QPointF(mapmiddle_px.x() - w/2, mapmiddle_px.y() + h/2);
        lowerRight = QPointF(mapmiddle_px.x() + w/2, mapmiddle_px.y() - h/2);
        vlm = QRectF(upperLeft, lowerRight);
    }
    if (p->theMapAdapter->getAdaptedZoom()  && vp != vlm)
        p->theMapAdapter->zoom_out();
}

qreal ImageMapLayer::pixelPerCoord()
{
    if (!isTiled())
        return -1.;

    return (p->theMapAdapter->getTileSizeW() * p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom())) / 360.;
}

void ImageMapLayer::forceRedraw(MapView& theView, QRect Screen)
{
    if (!p->theMapAdapter)
        return;

    if (p->pm.size() != Screen.size()) {
        p->pm = QPixmap(Screen.size());
        p->pm.fill(Qt::transparent);
    }

//    QRectF fScreen(Screen);
//    p->Viewport =
//        CoordBox(p->theProjection.inverse(p->theTransform.inverted().map(fScreen.bottomLeft())),
//             p->theProjection.inverse(p->theTransform.inverted().map(fScreen.topRight())));
    p->Viewport = theView.viewport();

//    if (p->theMapAdapter->getImageManager())
//        p->theMapAdapter->getImageManager()->abortLoading();
    draw(theView, Screen);
}

void ImageMapLayer::draw(MapView& theView, QRect& rect)
{
    if (!p->theMapAdapter)
        return;

    p->theProjection.setProjectionType(p->theMapAdapter->projection());

    if (p->theMapAdapter->isTiled())
        p->pr = drawTiled(theView, rect);
    else
        p->pr = drawFull(theView, rect);

    if (p->pm.isNull())
        return;

    const QSize ps = p->pr.size();
    const QSize pmSize = p->pm.size();
    const qreal ratio = qMax<const qreal>((qreal)pmSize.width()/ps.width()*1.0, (qreal)pmSize.height()/ps.height()*1.0);
    qDebug() << "Bg image ratio " << ratio;
    QPixmap pms;
    if (ratio >= 1.0) {
        qDebug() << "Bg image scale 1 " << ps << " : " << p->pm.size();
        pms = p->pm.scaled(ps);
    } else {
        const QSizeF drawingSize = pmSize * ratio;
        const QSizeF originSize = pmSize/2 - drawingSize/2;
        const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
        const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

        qDebug() << "Bg image scale 2 " << ps << " : " << p->pm.size();
        if (ps*ratio != drawingRect.size())
            pms = p->pm.copy(drawingRect).scaled(ps*ratio);
        else
            pms = p->pm.copy(drawingRect);
    }

    p->pm.fill(Qt::transparent);
    QPainter P(&p->pm);
    P.drawPixmap((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2, pms);
    //    if (p->theMapAdapter->isTiled())
    //        P.drawPixmap((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2, pms);
    //    else
    //        P.drawPixmap(QPoint((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2) + p->theDelta, pms);
}

QRect ImageMapLayer::drawFull(MapView& theView, QRect& rect)
{
    QRectF fRect(rect);
    MapView::transformCalc(p->theTransform, p->theProjection, 0.0, theView.viewport(), rect);

    CoordBox Viewport(p->theProjection.inverse2Coord(p->theTransform.inverted().map(fRect.bottomLeft())),
                     p->theProjection.inverse2Coord(p->theTransform.inverted().map(fRect.topRight())));
    QPointF bl = theView.toView(Viewport.bottomLeft());
    QPointF tr = theView.toView(Viewport.topRight());

    if (
            Viewport.bottomLeft().lat() >= -90. && Viewport.bottomLeft().lat() <= 90.
            && Viewport.bottomLeft().lon() >= -180. && Viewport.bottomLeft().lon() <= 180.
            && Viewport.topRight().lat() >= -90. && Viewport.topRight().lat() <= 90.
            && Viewport.topRight().lon() >= -180. && Viewport.topRight().lon() <= 180.
            ) {
        QRectF vp;
        if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()) {
            bl = QPointF(rect.bottomLeft());
            tr = QPointF(rect.topRight());
            vp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
            vp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
        } else
            vp = p->theProjection.getProjectedViewport(p->Viewport, rect);

        QRectF wgs84vp = QRectF(QPointF(coordToAng(Viewport.bottomLeft().lon()), coordToAng(Viewport.bottomLeft().lat()))
                                , QPointF(coordToAng(Viewport.topRight().lon()), coordToAng(Viewport.topRight().lat())));

        // Act depending on adapter type
        if (p->theMapAdapter->getType() == IMapAdapter::DirectBackground) {
            QPixmap pm = p->theMapAdapter->getPixmap(wgs84vp, vp, rect);
            if (!pm.isNull() && pm.rect() != rect)
                p->pm = pm.scaled(rect.size(), Qt::IgnoreAspectRatio);
            else
                p->pm = pm;
        } else if (p->theMapAdapter->getType() == IMapAdapter::VectorBackground) {
            const QList<IFeature*>* theFeatures = p->theMapAdapter->getPaths(wgs84vp, NULL);
            if (theFeatures) {
                foreach(IFeature* f, *theFeatures) {
                    const QPainterPath& thePath = f->getPath();
                    if (thePath.elementCount() == 1) {
                        IFeature::FId id(IFeature::Point, -(f->id().numId));
                        if (get(id))
                            continue;
                        Node* N = new Node(Coord::fromQPointF((QPointF)thePath.elementAt(0)));
                        N->setId(id);
                        add(N);
                        for (int i=0; i<f->tagSize(); ++i)
                            N->setTag(f->tagKey(i),f->tagValue(i));
                    } else {
                        IFeature::FId id(IFeature::LineString, -(f->id().numId));
                        if (get(id))
                            continue;
                        Way* W = new Way();
                        W->setId(id);
                        for (int i=0; i<thePath.elementCount(); ++i) {
                            Node* N = new Node(Coord::fromQPointF((QPointF)thePath.elementAt(i)));
                            add(N);
                            W->add(N);
                        }
                        add(W);
                        for (int i=0; i<f->tagSize(); ++i)
                            W->setTag(f->tagKey(i),f->tagValue(i));
                    }
                }
            }
            QPixmap pm(rect.size());
            pm.fill(Qt::transparent);
            p->pm = pm;
        } else if (p->theMapAdapter->getType() == IMapAdapter::NetworkDataBackground) {
//            QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));
//            if (!url.isEmpty()) {
//                qDebug() << "ImageMapLayer::drawFull: getting: " << url;
//                QByteArray ba = p->theMapAdapter->getImageManager()->getData(p->theMapAdapter,url);
//                QDomDocument* theXmlDoc = new QDomDocument();
//                theXmlDoc->setContent(ba);
//                Document* doc = Document::getDocumentFromXml(theXmlDoc);
//                if (doc) {
//                    QList<Feature*> theFeats;
//                    for (int i=0; i<doc->layerSize(); ++i)
//                        for (int j=0; j<doc->getLayer(i)->size(); ++j)
//                            if (!doc->getLayer(i)->get(j)->isNull())
//                                theFeats.push_back(doc->getLayer(i)->get(j));
//                    for (int i=0; i<theFeats.size(); ++i) {
//                        Feature*F = theFeats.at(i);
//                        // TODO Make reproducable id's or delete everything or ...
//                        if (get(F->id()))
//                            continue;

//                        // get tags
//                        QList<QPair<QString, QString> > Tags;
//                        for (int j=0; j<F->tagSize(); ++j) {
//                            Tags << qMakePair(F->tagKey(j), F->tagValue(j));
//                        }
//                        F->clearTags();

//                        // Re-link null features to the ones in the current document
//                        for (int j=0; j<F->size(); ++j) {
//                            Feature* C = F->get(j);
//                            if (C->isNull()) {
//                                if (Feature* CC = get(C->id())) {
//                                    if (Relation* R = CAST_RELATION(F)) {
//                                        QString role = R->getRole(j);
//                                        R->remove(j);
//                                        R->add(role, CC, j);
//                                    } else if (Way* W = CAST_WAY(F)) {
//                                        Node* N = CAST_NODE(CC);
//                                        W->remove(j);
//                                        W->add(N, j);
//                                    }
//                                } else
//                                    theFeats.push_back(C);
//                            }
//                        }
//                        F->layer()->remove(F);
//                        add(F);

//                        //Put tags
//                        for (int j=0; j<Tags.size(); ++j) {
//                            F->setTag(Tags[j].first, Tags[j].second);
//                        }
//                    }
//                }
//                delete doc;
//                delete theXmlDoc;
//            }
            QPixmap pm(rect.size());
            pm.fill(Qt::transparent);
            p->pm = pm;

        } else {
            QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));
            if (!url.isEmpty()) {
                qDebug() << "ImageMapLayer::drawFull: getting: " << url;
                QPixmap pm = p->theMapAdapter->getImageManager()->getPixmap(p->theMapAdapter,url);
                if (!pm.isNull())
                    p->pm = pm.scaled(rect.size(), Qt::IgnoreAspectRatio);
                else
                    return rect;
            }
        }
    }

    return QRectF(bl.x(), tr.y(), tr.x() - bl.x() +1, bl.y() - tr.y() + 1).toRect();
}

QRect ImageMapLayer::drawTiled(MapView& theView, QRect& rect)
{
    QRectF vp;
    QRectF fRect(rect);
    if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()) {
        vp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
        vp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
    } else
        vp = p->theProjection.getProjectedViewport(p->Viewport, rect);

    qreal tileWidth, tileHeight;
    int maxZoom = p->theMapAdapter->getAdaptedMaxZoom();
    int tilesizeW = p->theMapAdapter->getTileSizeW();
    int tilesizeH = p->theMapAdapter->getTileSizeH();
    QPointF vp0Center = QPointF(vp.width()/2, -vp.height()/2);
    QPointF vpCenter = vp.center();

    if (!M_PREFS->getZoomBoris()) {
        // Set zoom level to 0.
        while (p->theMapAdapter->getAdaptedZoom()) {
            p->theMapAdapter->zoom_out();
        }
    }

    tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
    tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
    qreal w = ((qreal)rect.width() / tilesizeW) * tileWidth;
    qreal h = ((qreal)rect.height() / tilesizeH) * tileHeight;
    QPointF upperLeft = QPointF(vpCenter.x() - w/2, vpCenter.y() + h/2);
    QPointF lowerRight = QPointF(vpCenter.x() + w/2, vpCenter.y() - h/2);
    QRectF vlm = QRectF(upperLeft, lowerRight);

    if (!M_PREFS->getZoomBoris()) {
        while ((!vp.contains(vlm)) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
            p->theMapAdapter->zoom_in();

            tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
            tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
            w = ((qreal)rect.width() / tilesizeW) * tileWidth;
            h = ((qreal)rect.height() / tilesizeH) * tileHeight;
            upperLeft = QPointF(vpCenter.x() - w/2, vpCenter.y() + h/2);
            lowerRight = QPointF(vpCenter.x() + w/2, vpCenter.y() - h/2);
            vlm = QRectF(upperLeft, lowerRight);
        }
        if (p->theMapAdapter->getAdaptedZoom() && vp.contains(vlm)) {
            p->theMapAdapter->zoom_out();
            tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
            tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
        }
    }

    // Actual drawing
    int i, j;
    QPointF vpCenter0 = QPointF(vpCenter.x()-p->theMapAdapter->getBoundingbox().left(), p->theMapAdapter->getBoundingbox().bottom()-vpCenter.y());
    qreal mapmiddle_tile_x = qRound(vpCenter0.x()/tileWidth);
    qreal mapmiddle_tile_y = qRound(vpCenter0.y()/tileHeight);
    qDebug() << "z: " << p->theMapAdapter->getAdaptedZoom() << "; t_x: " << mapmiddle_tile_x << "; t_y: " << mapmiddle_tile_y ;

    qreal cross_x = vpCenter0.x() - mapmiddle_tile_x*tileWidth;		// position on middle tile
    qreal cross_y = vpCenter0.y() - mapmiddle_tile_y*tileHeight;
    qDebug() << "cross_x: " << cross_x << "; cross_y: " << cross_y;

        // calculate how many surrounding tiles have to be drawn to fill the display
    qreal space_left = vp0Center.x() - cross_x;
    int tiles_left = space_left/tileWidth;
    if (space_left>0)
        tiles_left+=1;
    qreal space_above = vp0Center.y() - cross_y;
    int tiles_above = space_above/tileHeight;
    if (space_above>0)
        tiles_above+=1;

    qreal space_right = vp0Center.x() - (tileWidth-cross_x);
    int tiles_right = space_right/tileWidth;
    if (space_right>0)
        tiles_right+=1;

    qreal space_bottom = vp0Center.y() - (tileHeight-cross_y);
    int tiles_bottom = space_bottom/tileHeight;
    if (space_bottom>0)
        tiles_bottom+=1;

    QList<Tile> tiles;
    int cross_scr_x = cross_x * tilesizeW / tileWidth;
    int cross_scr_y = cross_y * tilesizeH / tileHeight;

    QSize pmSize = rect.size();
//    QSize pmSize((tiles_right+tiles_left+1)*tilesizeW, (tiles_bottom+tiles_above+1)*tilesizeH);
    p->pm = QPixmap(pmSize);
    p->pm.fill(Qt::transparent);
    QPainter painter(&p->pm);

    qDebug() << "Tiles: " << tiles_right+tiles_left+1 << "x" << tiles_bottom+tiles_above+1;
    for (i=-tiles_left+mapmiddle_tile_x; i<=tiles_right+mapmiddle_tile_x; i++)
    {
        for (j=-tiles_above+mapmiddle_tile_y; j<=tiles_bottom+mapmiddle_tile_y; j++)
        {
            if (p->theMapAdapter->isValid(i, j, p->theMapAdapter->getZoom()))
            {
#ifdef Q_CC_MSVC
                double priority = _hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#else
                double priority = hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
#endif
                tiles.append(Tile(i, j, priority));
            }
        }
    }

    qSort(tiles);

    int n=0; // Arbitrarily limit the number of tiles to 100
    for (QList<Tile>::const_iterator tile = tiles.begin(); tile != tiles.end() && n<100; ++tile)
    {
        QPixmap pm = p->theMapAdapter->getImageManager()->getPixmap(p->theMapAdapter, tile->i, tile->j, p->theMapAdapter->getZoom());
        if (!pm.isNull())
            painter.drawPixmap(((tile->i-mapmiddle_tile_x)*tilesizeW)+pmSize.width()/2 -cross_scr_x,
                               ((tile->j-mapmiddle_tile_y)*tilesizeH)+pmSize.height()/2-cross_scr_y,
                               pm);

        if (M_PREFS->getDrawTileBoundary()) {
            painter.drawRect(((tile->i-mapmiddle_tile_x)*tilesizeW)+pmSize.width()/2 -cross_scr_x,
                             ((tile->j-mapmiddle_tile_y)*tilesizeH)+pmSize.height()/2-cross_scr_y,
                             tilesizeW, tilesizeH);
        }
        ++n;
    }
    painter.end();

    w = ((qreal)pmSize.width() / tilesizeW) * tileWidth;
    h = ((qreal)pmSize.height() / tilesizeH) * tileHeight;
    upperLeft = QPointF(vpCenter.x() - w/2, vpCenter.y() + h/2);
    lowerRight = QPointF(vpCenter.x() + w/2, vpCenter.y() - h/2);
    vlm = QRectF(upperLeft, lowerRight);

    Coord ulCoord, lrCoord;
    ulCoord = p->theProjection.inverse2Coord(vlm.topLeft());
    lrCoord = p->theProjection.inverse2Coord(vlm.bottomRight());

    const QPointF tl = theView.transform().map(theView.projection().project(ulCoord));
    const QPointF br = theView.transform().map(theView.projection().project(lrCoord));
    qDebug() << "tl: " << tl << "; br: " << br;
    qDebug() << "vp: " << vp;
    qDebug() << "vlm: " << vlm;
    return QRectF(tl, br).toRect();
}

//QRect ImageMapLayer::drawTiled(MapView& theView, QRect& rect) const
//{
//    int tilesize = p->theMapAdapter->getTileSize();
//    QRectF vp = QRectF(QPointF(intToAng(p->Viewport.bottomLeft().lon()), intToAng(p->Viewport.bottomLeft().lat()))
//                        , QPointF(intToAng(p->Viewport.topRight().lon()), intToAng(p->Viewport.topRight().lat())));
//
//    // Set zoom level to 0.
//    while (p->theMapAdapter->getAdaptedZoom()) {
//        p->theMapAdapter->zoom_out();
//    }
//
//    // Find zoom level where tilesize < viewport wdth
//    QPoint mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center());
//    QPoint screenmiddle = rect.center();
//    QRectF vlm = QRectF(QPointF(-180., -90.), QSizeF(360., 180.));
//    int maxZoom = p->theMapAdapter->getAdaptedMaxZoom();
//    while ((!vp.contains(vlm)) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
//        p->theMapAdapter->zoom_in();
//
//        mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center());
//
//        QPoint upperLeft = QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()+screenmiddle.y());
//        QPoint lowerRight = QPoint(mapmiddle_px.x()+screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y());
//
//        QPointF ulCoord = p->theMapAdapter->displayToCoordinate(upperLeft);
//        QPointF lrCoord = p->theMapAdapter->displayToCoordinate(lowerRight);
//
//        vlm = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));
//    }
//
//    if (p->theMapAdapter->getAdaptedZoom() && vp.contains(vlm))
//        p->theMapAdapter->zoom_out();
//
//    mapmiddle_px = p->theMapAdapter->coordinateToDisplay(vp.center());
//
//    QPoint upperLeft = QPoint(mapmiddle_px.x()-screenmiddle.x(), mapmiddle_px.y()+screenmiddle.y());
//    QPoint lowerRight = QPoint(mapmiddle_px.x()+screenmiddle.x(), mapmiddle_px.y()-screenmiddle.y());
//
//    QPointF ulCoord = p->theMapAdapter->displayToCoordinate(upperLeft);
//    QPointF lrCoord = p->theMapAdapter->displayToCoordinate(lowerRight);
//
//    vlm = QRectF(ulCoord, QSizeF( (lrCoord-ulCoord).x(), (lrCoord-ulCoord).y()));
//
//    p->pm = QPixmap(rect.size());
//    QPainter painter(&p->pm);
//
//    // Actual drawing
//    int i, j;
//
//    int cross_x = int(mapmiddle_px.x())%tilesize;		// position on middle tile
//    int cross_y = int(mapmiddle_px.y())%tilesize;
//
//        // calculate how many surrounding tiles have to be drawn to fill the display
//    int space_left = screenmiddle.x() - cross_x;
//    int tiles_left = space_left/tilesize;
//    if (space_left>0)
//        tiles_left+=1;
//
//    int space_above = screenmiddle.y() - cross_y;
//    int tiles_above = space_above/tilesize;
//    if (space_above>0)
//        tiles_above+=1;
//
//    int space_right = screenmiddle.x() - (tilesize-cross_x);
//    int tiles_right = space_right/tilesize;
//    if (space_right>0)
//        tiles_right+=1;
//
//    int space_bottom = screenmiddle.y() - (tilesize-cross_y);
//    int tiles_bottom = space_bottom/tilesize;
//    if (space_bottom>0)
//        tiles_bottom+=1;
//
//// 	int tiles_displayed = 0;
//    int mapmiddle_tile_x = mapmiddle_px.x()/tilesize;
//    int mapmiddle_tile_y = mapmiddle_px.y()/tilesize;
//
//    QList<Tile> tiles;
//
//    for (i=-tiles_left+mapmiddle_tile_x; i<=tiles_right+mapmiddle_tile_x; i++)
//    {
//        for (j=-tiles_above+mapmiddle_tile_y; j<=tiles_bottom+mapmiddle_tile_y; j++)
//        {
//#ifdef Q_CC_MSVC
//            double priority = _hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
//#else
//            double priority = hypot(i - mapmiddle_tile_x, j - mapmiddle_tile_y);
//#endif
//            tiles.append(Tile(i, j, priority));
//        }
//    }
//
//    qSort(tiles);
//
//    for (QList<Tile>::const_iterator tile = tiles.begin(); tile != tiles.end(); ++tile)
//    {
//        if (p->theMapAdapter->isValid(tile->i, tile->j, p->theMapAdapter->getZoom()))
//        {
//            QPixmap pm = p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter, tile->i, tile->j, p->theMapAdapter->getZoom());
//            if (!pm.isNull())
//                painter.drawPixmap(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+rect.width()/2,
//                            ((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+rect.height()/2,
//                                                    pm);
//
//            if (M_PREFS->getDrawTileBoundary()) {
//                painter.drawRect(((tile->i-mapmiddle_tile_x)*tilesize)-cross_x+rect.width()/2,
//                          ((tile->j-mapmiddle_tile_y)*tilesize)-cross_y+rect.height()/2,
//                                            tilesize, tilesize);
//            }
//        }
//    }
//    painter.end();
//
//    const Coord ctl = Coord(angToInt(vlm.bottomLeft().y()), angToInt(vlm.bottomLeft().x()));
//    const Coord cbr = Coord(angToInt(vlm.topRight().y()), angToInt(vlm.topRight().x()));
//
//    const QPointF tl = theView.transform().map(theView.projection().project(ctl));
//    const QPointF br = theView.transform().map(theView.projection().project(cbr));
//
//    return QRectF(tl, br).toRect();
//}
//
void ImageMapLayer::on_imageRequested()
{
    emit imageRequested(this);
}

void ImageMapLayer::on_imageReceived()
{
    emit imageReceived(this);
}

void ImageMapLayer::on_loadingFinished()
{
    emit loadingFinished(this);
}

QString ImageMapLayer::toPropertiesHtml()
{
    QString h;

    h += "<u>" + name() + "</u><br/>";
    if (p->theMapAdapter) {
        if (p->theMapAdapter->getType() != IMapAdapter::DirectBackground) {
            h += "<i>" + tr("Server") + ": </i>" + p->theMapAdapter->getHost();
            h += "<br/>";
            if (p->theMapAdapter->isTiled()) {
                h += "<i>" + tr("Tile size") + ": </i>" + QString("%1x%2").arg(p->theMapAdapter->getTileSizeW()).arg(p->theMapAdapter->getTileSizeH());
                h += "<br/>";
                h += "<i>" + tr("Min/Max zoom") + ": </i>" + QString("%1/%2").arg(p->theMapAdapter->getMinZoom()).arg(p->theMapAdapter->getMaxZoom());
                h += "<br/>";
            }
        }
        h += "<i>" + tr("Projection") + ": </i>" + p->theMapAdapter->projection();
        h += "<br/>";
        h += p->theMapAdapter->toPropertiesHtml();
    }
    h += "";

    return h;
}

