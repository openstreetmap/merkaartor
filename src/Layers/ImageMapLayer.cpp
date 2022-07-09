#include "Global.h"

#include "MainWindow.h"
#include "MapView.h"
#include "ImageMapLayer.h"

#include "Document.h"
#include "MerkaartorPreferences.h"
#include "Projection.h"

#include "IMapAdapterFactory.h"
#include "IMapAdapter.h"
#include "imagemanager.h"
#ifdef USE_WEBKIT
#include "browserimagemanager.h"
#endif
#include "tilemapadapter.h"
#include "wmsmapadapter.h"
#include "WmscMapAdapter.h"

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

    QPixmap curPix;
    QPixmap newPix;
    Projection theProjection;
    QString selServer;

    IImageManager* theImageManager;
#ifdef USE_WEBKIT
    BrowserImageManager* theBrowserImageManager;
#endif
    ImageManager* theNetworkImageManager;

    QRect pr;
    QTransform theTransform;
    QRectF Viewport;
    QTransform AlignementTransform;
    QVector<QTransform> AlignementTransformList;
    QPointF cumulatedDelta;

public:
    ImageMapLayerPrivate()
    {
        theMapAdapter = NULL;
        theImageManager = NULL;
#ifdef USE_WEBKIT
        theBrowserImageManager = NULL;
#endif
        theNetworkImageManager = NULL;
    }
    ~ImageMapLayerPrivate()
    {
        delete theMapAdapter;
        delete theImageManager;
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
    delete p;
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
    Coord tl = p->theProjection.inverse(r.topLeft());
    Coord br = p->theProjection.inverse(r.bottomRight());
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

void ImageMapLayer::resetAlign()
{
    p->AlignementTransform = QTransform();
    if (p->theMapAdapter) {
        if (p->theMapAdapter->isTiled())
            p->AlignementTransformList[p->theMapAdapter->getAdaptedZoom()] = QTransform();
        else
            p->AlignementTransformList[0] = QTransform();
    } else
        p->AlignementTransformList.resize(0);
}

QString ImageMapLayer::projection() const
{
    if (p->theMapAdapter)
        return p->theMapAdapter->projection();

    return QString();
}

IImageManager* ImageMapLayer::getImageManger()
{
    return p->theImageManager;
}

IMapAdapter* ImageMapLayer::getMapAdapter()
{
    return p->theMapAdapter;
}

void ImageMapLayer::setNoneAdapter()
{
    p->bgType = NONE_ADAPTER_UUID;
    setName(tr("Map - None"));
    setVisible(false);
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
    p->curPix = QPixmap();
    resetAlign();

    QString id = theAdapterUid.toString();
    p->bgType = theAdapterUid;
    M_PREFS->setBackgroundPlugin(theAdapterUid);
    if (p->bgType == NONE_ADAPTER_UUID) {
        setNoneAdapter();
    } else
    if (p->bgType == WMS_ADAPTER_UUID) {
        wsl = M_PREFS->getWmsServers();
        if (!wsl->contains(server)) {  // WMS not locally found
            setNoneAdapter();
        } else {
            p->selServer = server;
            WmsServer theWmsServer(wsl->value(p->selServer));
            p->theMapAdapter = new WMSMapAdapter(theWmsServer);
            switch (theWmsServer.WmsIsTiled) {
            case 0:
                setName(tr("Map - WMS - %1").arg(p->theMapAdapter->getName()));
                break;
            case 1:
                setName(tr("Map - WMS-C - %1").arg(p->theMapAdapter->getName()));
                break;
            case 2:
                setName(tr("Map - WMS-Tiled - %1").arg(p->theMapAdapter->getName()));
                break;
            }
            id += p->theMapAdapter->getName();
        }
    } else
    if (p->bgType == TMS_ADAPTER_UUID) {
        tsl = M_PREFS->getTmsServers();
        if (!tsl->contains(server)) {  // TMS not locally found
            setNoneAdapter();
        } else {
            p->selServer = server;
            TmsServer ts = tsl->value(p->selServer);
            p->theMapAdapter = new TileMapAdapter(ts);

            setName(tr("Map - TMS - %1").arg(p->theMapAdapter->getName()));
            id += p->theMapAdapter->getName();
        }
    } else
    if (p->bgType == SHAPE_ADAPTER_UUID) {
        setNoneAdapter();
    } else
    {
        IMapAdapterFactory* fac = M_PREFS->getBackgroundPlugin(p->bgType);
        if (fac)
            p->theMapAdapter = fac->CreateInstance();
        if (p->theMapAdapter) {
            setName(tr("Map - %1").arg(p->theMapAdapter->getName()));
            p->theMapAdapter->setSettings(M_PREFS->getQSettings());
            ImageLayerWidget* theImageWidget = qobject_cast<ImageLayerWidget*>(theWidget);
            Q_ASSERT(theImageWidget);
            connect(p->theMapAdapter, SIGNAL(forceZoom()), theImageWidget, SLOT(zoomLayer()));
            connect(p->theMapAdapter, SIGNAL(forceProjection()), theImageWidget, SLOT(setProjection()));
#ifndef _MOBILE
            if (g_Merk_MainWindow)
                connect(p->theMapAdapter, SIGNAL(forceRefresh()), g_Merk_MainWindow, SLOT(invalidateView()));
#endif
        } else
            setNoneAdapter();
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
                    QDialog dlg;
                    ui.setupUi(&dlg);
                    dlg.setWindowTitle(tr("Licensing Terms: %1").arg(name()));
                    ui.urlLabel->setText(QString("<a href='%1'>%2</a>").arg(u.toString()).arg(u.toString()));

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
                        SAFE_DELETE(p->theMapAdapter)
                        if (p->theImageManager)
                            p->theImageManager->abortLoading();
                        SAFE_DELETE(p->theImageManager)
                        on_loadingFinished();
                        p->curPix = QPixmap();

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
                if (!p->theNetworkImageManager) {
                    p->theNetworkImageManager = new ImageManager();
                    connect(p->theNetworkImageManager, SIGNAL(dataRequested()),
                            this, SLOT(on_imageRequested()), Qt::QueuedConnection);
                    connect(p->theNetworkImageManager, SIGNAL(dataReceived()),
                            this, SLOT(on_imageReceived()), Qt::QueuedConnection);
                    connect(p->theNetworkImageManager, SIGNAL(loadingFinished()),
                            this, SLOT(on_loadingFinished()), Qt::QueuedConnection);
                }
                p->theImageManager = p->theNetworkImageManager;
                p->theMapAdapter->setImageManager(p->theImageManager);
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

void QTransformToXml(QXmlStreamWriter& stream, const QTransform& theTransform)
{
    stream.writeAttribute("m11", QString::number(theTransform.m11()));
    stream.writeAttribute("m12", QString::number(theTransform.m12()));
    stream.writeAttribute("m13", QString::number(theTransform.m13()));
    stream.writeAttribute("m21", QString::number(theTransform.m21()));
    stream.writeAttribute("m22", QString::number(theTransform.m22()));
    stream.writeAttribute("m23", QString::number(theTransform.m23()));
    stream.writeAttribute("m31", QString::number(theTransform.m31()));
    stream.writeAttribute("m32", QString::number(theTransform.m32()));
    stream.writeAttribute("m33", QString::number(theTransform.m33()));
}

QTransform QTransformFomXml(QXmlStreamReader& stream)
{
    qreal m11 = stream.attributes().value("m11").toString().toDouble();
    qreal m12 = stream.attributes().value("m12").toString().toDouble();
    qreal m13 = stream.attributes().value("m13").toString().toDouble();
    qreal m21 = stream.attributes().value("m21").toString().toDouble();
    qreal m22 = stream.attributes().value("m11").toString().toDouble();
    qreal m23 = stream.attributes().value("m23").toString().toDouble();
    qreal m31 = stream.attributes().value("m31").toString().toDouble();
    qreal m32 = stream.attributes().value("m32").toString().toDouble();
    qreal m33 = stream.attributes().value("m33").toString().toDouble();

    return QTransform(m11, m12, m13, m21, m22, m23, m31, m32, m33);
}

bool ImageMapLayer::toXML(QXmlStreamWriter& stream, bool asTemplate, QProgressDialog * /* progress */)
{
    stream.writeStartElement(metaObject()->className());

    stream.writeAttribute("xml:id", id());
    stream.writeAttribute("name", name());
    stream.writeAttribute("alpha", QString::number(getAlpha(),'f',2));
    stream.writeAttribute("visible", QString((isVisible() ? "true" : "false")));
    stream.writeAttribute("selected", QString((isSelected() ? "true" : "false")));
    stream.writeAttribute("enabled", QString((isEnabled() ? "true" : "false")));

    stream.writeAttribute("bgtype", p->bgType.toString());

    WmsServer ws;
    TmsServer ts;

    if (p->bgType == WMS_ADAPTER_UUID) {
        stream.writeStartElement("WmsServer");
        stream.writeAttribute("name", p->selServer);
        stream.writeEndElement();
    } else if (p->bgType == TMS_ADAPTER_UUID) {
        stream.writeStartElement("TmsServer");
        stream.writeAttribute("name", p->selServer);
        stream.writeEndElement();
    } else if (p->bgType != NONE_ADAPTER_UUID) {
        stream.writeStartElement("Data");
        if (!asTemplate)
            p->theMapAdapter->toXML(stream);
        stream.writeEndElement();
    }
    if (!asTemplate) {
        stream.writeStartElement("AdjustmentList");
        for (int i=0; i<p->AlignementTransformList.size(); ++i) {
            if (!p->AlignementTransformList.at(i).isIdentity()) {
                stream.writeStartElement("Adjustment");
                stream.writeAttribute("zoom", QString::number(i));
                QTransformToXml(stream, p->AlignementTransformList.at(i));
                stream.writeEndElement();
            }
        }
        stream.writeEndElement();
    }
    stream.writeEndElement();

    return true;
}

ImageMapLayer * ImageMapLayer::fromXML(Document* d, QXmlStreamReader& stream, QProgressDialog * /*progress*/)
{
    ImageMapLayer* l = new ImageMapLayer(stream.attributes().value("name").toString());

    l->setId(stream.attributes().value("xml:id").toString());
    d->addImageLayer(l);

    QString server;
    QUuid bgtype = QUuid(stream.attributes().value("bgtype").toString());

    qreal alpha = stream.attributes().value("alpha").toString().toDouble();
    // TODO: Note that the logic for "enabled" is slightly different. Why?
    bool visible = (stream.attributes().value("visible") == "true" ? true : false);
    bool selected = (stream.attributes().value("selected") == "true" ? true : false);
    bool enabled = (stream.attributes().value("enabled") == "false" ? false : true);

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == QStringLiteral("AdjustmentList")) {
            stream.readNext();
            while(!stream.atEnd() && !stream.isEndElement()) {
                if (stream.name() == QStringLiteral("Adjustment")) {
                    int z = stream.attributes().value("zoom").toString().toInt();
                    if (l->p->AlignementTransformList.size() < z+1)
                        l->p->AlignementTransformList.resize(z+1);
                    l->p->AlignementTransformList[z] = QTransformFomXml(stream);
                    stream.readNext();
                } else if (!stream.isWhitespace()) {
                    qDebug() << "ImgLay: logic error:" << stream.name() << ":" << stream.tokenType() << "(" << stream.lineNumber() << ")";
                    stream.skipCurrentElement();
                }
                stream.readNext();
            }
        } else if (stream.name() == QStringLiteral("WmsServer")) {
            server = stream.attributes().value("name").toString();
            l->setMapAdapter(bgtype, server);
            stream.readNext();
        } else if (stream.name() == QStringLiteral("TmsServer")) {
            server = stream.attributes().value("name").toString();
            l->setMapAdapter(bgtype, server);
            stream.readNext();
        } else if (stream.name() == QStringLiteral("Data")) {
            l->setMapAdapter(bgtype, server);
            stream.readNext();
            if (l->getMapAdapter())
                l->getMapAdapter()->fromXML(stream);
        } else if (!stream.isWhitespace()) {
            qDebug() << "ImgLay: logic error:" << stream.name() << ":" << stream.tokenType() << "(" << stream.lineNumber() << ")";
            stream.skipCurrentElement();
        }
        stream.readNext();
    }

    l->setAlpha(alpha);
    l->setVisible(visible);
    l->setSelected(selected);
    l->setEnabled(enabled);

    return l;
}

void ImageMapLayer::drawImage(QPainter* P)
{
    if (!p->theMapAdapter)
        return;

    P->setOpacity(getAlpha());
    P->drawPixmap(0, 0, p->curPix);
}

void ImageMapLayer::zoom(qreal zoom, const QPoint& pos, const QRect& rect)
{
    if (!p->theMapAdapter)
        return;
    if (p->theMapAdapter->getImageManager())
        p->theMapAdapter->getImageManager()->abortLoading();
    if (p->curPix.isNull())
        return;

    QPixmap tpm = p->curPix.scaled(rect.size() * zoom, Qt::KeepAspectRatio);
    p->curPix.fill(Qt::transparent);
    QPainter P(&p->curPix);
    QPointF fPos(pos);
    fPos -= fPos * zoom;
    P.drawPixmap(fPos, tpm);
}

void ImageMapLayer::pan(QPoint delta)
{
    if (!p->theMapAdapter)
        return;
    if (p->theMapAdapter->getImageManager())
        p->theMapAdapter->getImageManager()->abortLoading();
    if (p->curPix.isNull())
        return;

    QRegion exposed;
    p->curPix.scroll(delta.x(), delta.y(), p->curPix.rect(), &exposed);
    QPainter P(&p->curPix);
    P.setClipping(true);
    P.setClipRegion(exposed);
    P.eraseRect(p->curPix.rect());
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
    QRectF projVp;
    QRectF fRect(rect);

    if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()) {
        projVp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
        projVp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
    } else
        projVp = p->theProjection.toProjectedRectF(viewport, rect);

    qreal tileWidth, tileHeight;
    int maxZoom = p->theMapAdapter->getAdaptedMaxZoom(viewport);
    int tilesizeW = p->theMapAdapter->getTileSizeW();
    int tilesizeH = p->theMapAdapter->getTileSizeH();

    // Set zoom level to 0.
    while (p->theMapAdapter->getAdaptedZoom()) {
        p->theMapAdapter->zoom_out();
    }

    tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
    tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
    qreal w = (fRect.width() / tilesizeW) * tileWidth;
    qreal h = (fRect.height() / tilesizeH) * tileHeight;

    while (!(projVp.width() > w && -projVp.height() > h) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
        p->theMapAdapter->zoom_in();

        tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
        tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
        w = (fRect.width() / tilesizeW) * tileWidth;
        h = (fRect.height() / tilesizeH) * tileHeight;
    }

    QPointF vpCenter = projVp.center();
    QPointF upperLeft = QPointF(vpCenter.x() - w/2, vpCenter.y() + h/2);
    QPointF lowerRight = QPointF(vpCenter.x() + w/2, vpCenter.y() - h/2);
    QRectF vlm = QRectF(upperLeft, lowerRight);
    if (p->theMapAdapter->getAdaptedZoom() && projVp != vlm)
        p->theMapAdapter->zoom_out();
}

qreal ImageMapLayer::pixelPerCoord()
{
    if (!isTiled())
        return -1.;

    return (p->theMapAdapter->getTileSizeW() * p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom())) / 360.;
}

void ImageMapLayer::forceRedraw(MapView& theView, QTransform& aTransform, QRect Screen)
{
    if (!p->theMapAdapter)
        return;

    if (!p->Viewport.intersects(theView.viewport())) {
        p->curPix = QPixmap(Screen.size());
        p->curPix.fill(Qt::transparent);
    }
    p->AlignementTransform = aTransform;
    p->Viewport = theView.viewport();

    draw(theView, Screen);
}

void ImageMapLayer::draw(MapView& theView, QRect& rect)
{
    if (!p->theMapAdapter)
        return;

    p->theProjection.setProjectionType(p->theMapAdapter->projection());
    p->cumulatedDelta = QPointF();

    if (p->theMapAdapter->isTiled())
        p->pr = drawTiled(theView, rect);
    else
        p->pr = drawFull(theView, rect);

    if (p->curPix.size() != rect.size()) {
        p->curPix = QPixmap(rect.size());
    }

    if (p->newPix.isNull())
        return;

    const QSize ps = p->pr.size();
    const QSize pmSize = p->newPix.size();
    const qreal ratio = qMax<const qreal>((qreal)pmSize.width()/ps.width()*1.0, (qreal)pmSize.height()/ps.height()*1.0);
//    qDebug() << "Bg image ratio" << ratio;
    QPixmap pms;
    if (ratio >= 1.0) {
//        qDebug() << "Bg image scale 1" << ps << ":" << p->newPix.size();
        pms = p->newPix.scaled(ps);
    } else {
        const QSizeF drawingSize = pmSize * ratio;
        const QSizeF originSize = pmSize/2 - drawingSize/2;
        const QPointF drawingOrigin = QPointF(originSize.width(), originSize.height());
        const QRect drawingRect = QRect(drawingOrigin.toPoint(), drawingSize.toSize());

//        qDebug() << "Bg image scale 2" << ps << ":" << p->newPix.size();
        if (ps*ratio != drawingRect.size())
            pms = p->newPix.copy(drawingRect).scaled(ps*ratio);
        else
            pms = p->newPix.copy(drawingRect);
    }

    /* We need to clear the pixmap in case we're painting transparent stuff over */
    p->curPix.fill(Qt::transparent);

    QPainter P(&p->curPix);
    P.drawPixmap((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2, pms);
    //    if (p->theMapAdapter->isTiled())
    //        P.drawPixmap((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2, pms);
    //    else
    //        P.drawPixmap(QPoint((pmSize.width()-pms.width())/2, (pmSize.height()-pms.height())/2) + p->theDelta, pms);
}

QRect ImageMapLayer::drawFull(MapView& theView, QRect& rect)
{
    QRectF fRect(rect);
    p->AlignementTransformList.resize(1);
    p->AlignementTransformList[0] *= p->AlignementTransform;
    p->AlignementTransform = QTransform();
    QRectF alignedViewport = p->AlignementTransformList.at(0).mapRect(p->Viewport);

    MapView::transformCalc(p->theTransform, p->theProjection, 0.0, CoordBox(alignedViewport), rect);

    CoordBox cViewport(p->theProjection.inverse(p->theTransform.inverted().map(fRect.bottomLeft())),
                     p->theProjection.inverse(p->theTransform.inverted().map(fRect.topRight())));
    CoordBox Viewport = CoordBox(p->AlignementTransformList.at(0).mapRect(cViewport));
    QPointF bl = theView.toView(Viewport.bottomLeft());
    QPointF tr = theView.toView(Viewport.topRight());

    if (
            Viewport.bottomLeft().y() >= -90. && Viewport.bottomLeft().y() <= 90.
            && Viewport.bottomLeft().x() >= -180. && Viewport.bottomLeft().x() <= 180.
            && Viewport.topRight().y() >= -90. && Viewport.topRight().y() <= 90.
            && Viewport.topRight().x() >= -180. && Viewport.topRight().x() <= 180.
            ) {
        QRectF vp;
        if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()  && alignedViewport == theView.viewport()) {
            bl = QPointF(rect.bottomLeft());
            tr = QPointF(rect.topRight());
            vp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
            vp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
        } else
            vp = p->theProjection.toProjectedRectF(CoordBox(alignedViewport), rect);

        QRectF wgs84vp = QRectF(QPointF(Viewport.bottomLeft().x(), Viewport.bottomLeft().y())
                                , QPointF(Viewport.topRight().x(), Viewport.topRight().y()));

        // Act depending on adapter type
        if (p->theMapAdapter->getType() == IMapAdapter::DirectBackground) {
            QPixmap pm = p->theMapAdapter->getPixmap(wgs84vp, vp, rect);
            if (!pm.isNull()) {
                p->curPix = QPixmap();
                if (pm.rect() != rect)
                    p->newPix = pm.scaled(rect.size(), Qt::IgnoreAspectRatio);
                else
                    p->newPix = pm;
            } else
                return rect;
        } else if (p->theMapAdapter->getType() == IMapAdapter::VectorBackground) {
            const QList<IFeature*>* theFeatures = p->theMapAdapter->getPaths(wgs84vp, NULL);
            if (theFeatures) {
                foreach(IFeature* f, *theFeatures) {
                    const QPainterPath& thePath = f->getPath();
                    if (thePath.elementCount() == 1) {
                        IFeature::FId id(IFeature::Point, -(f->id().numId));
                        if (get(id))
                            continue;
                        Node* N = g_backend.allocNode(this, Coord((QPointF)thePath.elementAt(0)));
                        N->setId(id);
                        add(N);
                        for (int i=0; i<f->tagSize(); ++i)
                            N->setTag(f->tagKey(i),f->tagValue(i));
                    } else {
                        IFeature::FId id(IFeature::LineString, -(f->id().numId));
                        if (get(id))
                            continue;
                        Way* W = g_backend.allocWay(this);
                        W->setId(id);
                        for (int i=0; i<thePath.elementCount(); ++i) {
                            Node* N = g_backend.allocNode(this, Coord((QPointF)thePath.elementAt(i)));
                            add(N);
                            W->add(N);
                        }
                        add(W);
                        for (int i=0; i<f->tagSize(); ++i)
                            W->setTag(f->tagKey(i),f->tagValue(i));
                    }
                }
            }
            p->newPix = QPixmap(rect.size());
            p->newPix.fill(Qt::transparent);
        } else if (p->theMapAdapter->getType() == IMapAdapter::NetworkDataBackground) {
//            QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));
//            if (!url.isEmpty()) {
//                qDebug() << "ImageMapLayer::drawFull: getting:" << url;
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
            p->newPix = QPixmap();
            return rect;

        } else if (p->theMapAdapter->getType() == IMapAdapter::NetworkBackground || p->theMapAdapter->getType() == IMapAdapter::BrowserBackground) {
            QString url (p->theMapAdapter->getQuery(wgs84vp, vp, rect));
            if (!url.isEmpty()) {
                //qDebug() << "ImageMapLayer::drawFull: getting:" << url;
                QPixmap pm = QPixmap::fromImage(p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter,url));
                if (!pm.isNull()) {
                    p->curPix = QPixmap();
                    p->newPix = pm.scaled(rect.size(), Qt::IgnoreAspectRatio);
                } else {
                    p->newPix = QPixmap();
                    return rect;
                }
            }
        }
    }

    return QRectF(bl.x(), tr.y(), tr.x() - bl.x() +1, bl.y() - tr.y() + 1).toRect();
}

QRect ImageMapLayer::drawTiled(MapView& theView, QRect& rect)
{
    QRectF projVp;
//    QRectF fRect(-rect.width(), -rect.height(), rect.width()*3.0, rect.height()*3.0);
    QRectF fRect(rect);

    if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4()) {
        projVp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
        projVp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
    } else
        projVp = p->theProjection.toProjectedRectF(CoordBox(p->Viewport), fRect.toRect());

    qreal tileWidth, tileHeight;
    int maxZoom = p->theMapAdapter->getAdaptedMaxZoom(p->Viewport);
    int tilesizeW = p->theMapAdapter->getTileSizeW();
    int tilesizeH = p->theMapAdapter->getTileSizeH();

    if (!M_PREFS->getZoomBoris()) {
        // Set zoom level to 0.
        while (p->theMapAdapter->getAdaptedZoom()) {
            p->theMapAdapter->zoom_out();
        }
    }

    tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
    tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
    qreal w = (fRect.width() / tilesizeW) * tileWidth;
    qreal h = (fRect.height() / tilesizeH) * tileHeight;

    if (!M_PREFS->getZoomBoris()) {
        while (!(projVp.width() > w && -projVp.height() > h) && (p->theMapAdapter->getAdaptedZoom() < maxZoom)) {
            p->theMapAdapter->zoom_in();

            tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
            tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
            w = (fRect.width() / tilesizeW) * tileWidth;
            h = (fRect.height() / tilesizeH) * tileHeight;
        }
        if (p->theMapAdapter->getAdaptedZoom() && projVp.width() > w && -projVp.height() > h) {
            p->theMapAdapter->zoom_out();
            tileWidth = p->theMapAdapter->getBoundingbox().width() / p->theMapAdapter->getTilesWE(p->theMapAdapter->getZoom());
            tileHeight = p->theMapAdapter->getBoundingbox().height() / p->theMapAdapter->getTilesNS(p->theMapAdapter->getZoom());
            w = (fRect.width() / tilesizeW) * tileWidth;
            h = (fRect.height() / tilesizeH) * tileHeight;
        }
    }

    p->AlignementTransformList.resize(maxZoom+1);
    p->AlignementTransformList[p->theMapAdapter->getAdaptedZoom()] *= p->AlignementTransform;
    p->AlignementTransform = QTransform();
    QRectF alignedViewport = p->AlignementTransformList.at(p->theMapAdapter->getAdaptedZoom()).mapRect(p->Viewport);

    if (alignedViewport != p->Viewport) {
        if (p->theProjection.getProjectionProj4() == theView.projection().getProjectionProj4() && alignedViewport == theView.viewport()) {
            projVp.setTopLeft(theView.invertedTransform().map(fRect.topLeft()));
            projVp.setBottomRight(theView.invertedTransform().map(fRect.bottomRight()));
        } else
            projVp = p->theProjection.toProjectedRectF(CoordBox(alignedViewport), rect);
    }

    QPointF vpCenter = projVp.center();
    QPointF upperLeft = QPointF(vpCenter.x() - w/2, vpCenter.y() + h/2);
    QPointF lowerRight = QPointF(vpCenter.x() + w/2, vpCenter.y() - h/2);
    QRectF vlm = QRectF(upperLeft, lowerRight);

    QPointF vp0Center = QPointF(projVp.width()/2, -projVp.height()/2);

    Coord ulCoord, lrCoord;
    ulCoord = p->theProjection.inverse(vlm.topLeft());
    lrCoord = p->theProjection.inverse(vlm.bottomRight());

    const QPointF tl = theView.transform().map(theView.projection().project(ulCoord));
    const QPointF br = theView.transform().map(theView.projection().project(lrCoord));
    QRect retRect = QRectF(tl, br).toRect();

    // Actual drawing
    int i, j;
    QPointF vpCenter0 = QPointF(vpCenter.x()-p->theMapAdapter->getBoundingbox().left(), p->theMapAdapter->getBoundingbox().bottom()-vpCenter.y());
    qreal mapmiddle_tile_x = qRound(vpCenter0.x()/tileWidth);
    qreal mapmiddle_tile_y = qRound(vpCenter0.y()/tileHeight);
    //qDebug() << "z:" << p->theMapAdapter->getAdaptedZoom() << "; t_x:" << mapmiddle_tile_x << "; t_y:" << mapmiddle_tile_y ;

    qreal cross_x = vpCenter0.x() - mapmiddle_tile_x*tileWidth;		// position on middle tile
    qreal cross_y = vpCenter0.y() - mapmiddle_tile_y*tileHeight;
    //qDebug() << "cross_x:" << cross_x << "; cross_y:" << cross_y;

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

    QSize pmSize = fRect.size().toSize();
    p->newPix = QPixmap(pmSize);
    p->newPix.fill(Qt::transparent);
    QPainter painter(&p->newPix);
//    painter.drawPixmap(0, 0, tmpPm);

//    qDebug() << "Tiles:" << tiles_right+tiles_left+1 << "x" << tiles_bottom+tiles_above+1;
    for (i=-tiles_left; i<=tiles_right; i++)
    {
        for (j=-tiles_above; j<=tiles_bottom; j++)
        {
            if (p->theMapAdapter->isValid(mapmiddle_tile_x+i, mapmiddle_tile_y+j, p->theMapAdapter->getZoom()))
            {
#ifdef Q_CC_MSVC
                qreal priority = _hypot(i, j);
#else
                qreal priority = hypot(i, j);
#endif
                tiles.append(Tile(i, j, priority));
            }
        }
    }

    qSort(tiles);

    int n=0; // Arbitrarily limit the number of tiles to 100
    for (QList<Tile>::const_iterator tile = tiles.begin(); tile != tiles.end() && n<100; ++tile)
    {
        QImage pm = p->theMapAdapter->getImageManager()->getImage(p->theMapAdapter, p->theMapAdapter->getQuery(mapmiddle_tile_x+tile->i, mapmiddle_tile_y+tile->j, p->theMapAdapter->getZoom()));
        int x = (tile->i*tilesizeW)+pmSize.width()/2 -cross_scr_x;
        int y = (tile->j*tilesizeH)+pmSize.height()/2-cross_scr_y;
        if (!pm.isNull())
            painter.drawImage(x, y, pm);

        if (M_PREFS->getDrawTileBoundary())
            painter.drawRect(x, y, tilesizeW, tilesizeH);

        ++n;
    }
    painter.end();

//    qDebug() << "tl:" << tl << "; br:" << br;
//    qDebug() << "vp:" << projVp;
    //    qDebug() << "vlm:" << vlm;
    //qDebug() << "retRect:" << retRect;
//    QRect expR = QRect(-retRect.left(), -retRect.top(), retRect.width()+retRect.left(), retRect.height()+retRect.top());
//    p->newPix.save("c:/tmp.png");
//    p->newPix.copy(expR).save("c:/tmp2.png");
    return retRect;
}

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
    QRectF alignedViewport = p->AlignementTransform.mapRect(p->Viewport);

    h += "<u>" + name() + "</u><br/>";
    if (p->theMapAdapter) {
        if (p->theMapAdapter->getType() != IMapAdapter::DirectBackground) {
            h += "<i>" + tr("Server") + ": </i>" + p->theMapAdapter->getHost();
            h += "<br/>";
            if (p->theMapAdapter->isTiled()) {
                h += "<i>" + tr("Tile size") + ": </i>" + QString("%1x%2").arg(p->theMapAdapter->getTileSizeW()).arg(p->theMapAdapter->getTileSizeH());
                h += "<br/>";
                h += "<i>" + tr("Min/Max zoom") + ": </i>" + QString("%1/%2").arg(p->theMapAdapter->getMinZoom(alignedViewport)).arg(p->theMapAdapter->getMaxZoom(alignedViewport));
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

QTransform ImageMapLayer::getCurrentAlignmentTransform()
{
    if (p->theMapAdapter && p->AlignementTransformList.size()) {
        if (p->theMapAdapter->isTiled()) {
            return p->AlignementTransformList.at(p->theMapAdapter->getAdaptedZoom());
        } else {
            return p->AlignementTransformList.at(0);
        }
    } else
        return QTransform();
}

