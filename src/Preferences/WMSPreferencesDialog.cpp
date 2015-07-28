//
// C++ Implementation: WMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "WMSPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>
#include <QNetworkReply>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
#define EQUATORIALMETERCIRCUMFERENCE  40075016.68
#define EQUATORIALMETERHALFCIRCUMFERENCE  20037508.34
#define EQUATORIALMETERPERDEGREE    222638.981555556

WMSPreferencesDialog::WMSPreferencesDialog(QWidget* parent)
    : QDialog(parent), curReply(0)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadPrefs();

    edWmsLayers->setVisible(false);
    frWmsSettings->setVisible(true);
    isTiled = 0;
    frTileIt->setEnabled(false);
    edWmsUrl->setValidator(&wmsValid);

    connect(tvWmsLayers, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(on_tvWmsLayers_itemChanged(QTreeWidgetItem *, int)));
    connect(&m_networkManager, SIGNAL(finished(QNetworkReply *)), this, SLOT(httpRequestFinished(QNetworkReply *)));
}

WMSPreferencesDialog::~WMSPreferencesDialog()
{
}

void WMSPreferencesDialog::updateUrl()
{
    QUrl theUrl(edWmsUrl->text());
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    // FIXME: I'm not sure this is correct. Needs testing.
    QUrlQuery theQuery(theUrl);
#else
#define theQuery theUrl
#endif
    if (!theQuery.hasQueryItem("VERSION"))
        theQuery.addQueryItem("VERSION", "1.1.1");
    if (!theQuery.hasQueryItem("SERVICE"))
        theQuery.addQueryItem("SERVICE", "WMS");

    theQuery.removeQueryItem("TRANSPARENT"); theQuery.addQueryItem("TRANSPARENT", "TRUE");
    theQuery.removeQueryItem("LAYERS"); theQuery.addQueryItem("LAYERS", edWmsLayers->text());
    theQuery.removeQueryItem("SRS"); theQuery.addQueryItem("SRS", cbWmsProj->currentText());
    theQuery.removeQueryItem("STYLES"); theQuery.addQueryItem("STYLES", cbWmsStyle->currentText());
    theQuery.removeQueryItem("FORMAT"); theQuery.addQueryItem("FORMAT", cbWmsImgFormat->currentText());
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    theUrl.setQuery(theQuery);
#else
#undef theQuery
#endif
    edWmsUrl->setText(theUrl.toString());
}

void WMSPreferencesDialog::addServer(const WmsServer & srv)
{
    theWmsServers.push_back(srv);
    if (!srv.deleted) {
        QListWidgetItem* item = new QListWidgetItem(srv.WmsName);
        item->setData(Qt::UserRole, (int)(theWmsServers.size()-1));
        lvWmsServers->addItem(item);
    }
}

void WMSPreferencesDialog::generateWmscLayer()
{
    selWmscLayer.LayerName = edWmsLayers->text();
    selWmscLayer.Projection = cbWmsProj->currentText();
    selWmscLayer.Styles = cbWmsStyle->currentText();
    selWmscLayer.ImgFormat = cbWmsImgFormat->currentText();
    selWmscLayer.TileHeight = 256;
    selWmscLayer.TileWidth = 256;
    qreal startRes;
    if (selWmscLayer.Projection.contains("4326")) {
        selWmscLayer.BoundingBox = QRectF(QPointF(-180, -90.), QPointF(180., 90.));
        startRes = 0.703125;
    } else {
        selWmscLayer.BoundingBox = QRectF(QPointF(-EQUATORIALMETERHALFCIRCUMFERENCE, -EQUATORIALMETERHALFCIRCUMFERENCE), QPointF(EQUATORIALMETERHALFCIRCUMFERENCE, EQUATORIALMETERHALFCIRCUMFERENCE));
        startRes = 156543.03;
    }
    selWmscLayer.Resolutions.clear();
    selWmscLayer.Resolutions << startRes;
    for (int i=1; i<sbZoomLevels->value(); ++i) {
        startRes /= 2.;
        selWmscLayer.Resolutions << startRes;
    }
}

void WMSPreferencesDialog::on_edWmsUrl_textEdited(const QString &newText)
{
    QUrl theUrl(newText);
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery theQuery(theUrl);
#define theQuery theQuery
#else
#define theQuery theUrl
#endif

    if (theQuery.hasQueryItem("LAYERS"))
        edWmsLayers->setText(QUrl::fromPercentEncoding(theQuery.queryItemValue("LAYERS").toLatin1()));
    if (theQuery.hasQueryItem("SRS"))
        cbWmsProj->setEditText(QUrl::fromPercentEncoding(theQuery.queryItemValue("SRS").toLatin1()));
    if (theQuery.hasQueryItem("STYLES"))
        cbWmsStyle->setEditText(QUrl::fromPercentEncoding(theQuery.queryItemValue("STYLES").toLatin1()));
    if (theQuery.hasQueryItem("FORMAT"))
        cbWmsImgFormat->setEditText(QUrl::fromPercentEncoding(theQuery.queryItemValue("FORMAT").toLatin1()));
#undef theQuery
}

void WMSPreferencesDialog::on_edWmsUrl_editingFinished()
{
    QUrl u(edWmsUrl->text());
    if (u.isValid() && !u.host().isEmpty() && !u.path().isEmpty()) {
        showCapabilities();
    }
}

void WMSPreferencesDialog::on_btApplyWmsServer_clicked(void)
{
    int idx = lvWmsServers->currentItem()->data(Qt::UserRole).toInt();
    if (idx >= theWmsServers.size())
        return;

    QUrl theUrl(edWmsUrl->text());

    WmsServer& WS(theWmsServers[idx]);
    WS.WmsName = edWmsName->text();
    WS.WmsAdress = theUrl.host();
    if (theUrl.port() != -1)
        WS.WmsAdress += ":" + QString::number(theUrl.port());
    WS.WmsPath = theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    WS.WmsLayers = edWmsLayers->text();
    WS.WmsProjections = cbWmsProj->currentText();
    WS.WmsStyles = cbWmsStyle->currentText();
    WS.WmsImgFormat = cbWmsImgFormat->currentText();
    if (cbTileIt->isChecked()) {
        isTiled = 2;
        generateWmscLayer();
    }
    if (isTiled) {
        WS.WmsCLayer = selWmscLayer;
        WS.WmsProjections = selWmscLayer.Projection;
        WS.WmsImgFormat = selWmscLayer.ImgFormat;
        WS.WmsStyles = selWmscLayer.Styles;
    }
    WS.WmsIsTiled = isTiled;
    WS.WmsSourceTag = edSourceTag->text();
    WS.WmsLicenseUrl = edLicenseUrl->text();

    lvWmsServers->currentItem()->setText(WS.WmsName);
    selectedServer = WS.WmsName;
}

void WMSPreferencesDialog::on_btAddWmsServer_clicked(void)
{
    QUrl theUrl(edWmsUrl->text());
    QString theAdress = theUrl.host();
    if (theUrl.port() != -1)
        theAdress += ":" + QString::number(theUrl.port());
    if (cbTileIt->isChecked()) {
        isTiled = 2;
        generateWmscLayer();
    }
    if (isTiled)
        addServer(WmsServer(edWmsName->text(), theAdress, theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority),
            edWmsLayers->text()
            , edSourceTag->text()
            , edLicenseUrl->text()
            , selWmscLayer.Projection, selWmscLayer.Styles, selWmscLayer.ImgFormat
            , isTiled, selWmscLayer));
    else
        addServer(WmsServer(edWmsName->text(), theAdress, theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority),
            edWmsLayers->text()
            , edSourceTag->text()
            , edLicenseUrl->text()
            , cbWmsProj->currentText(), cbWmsStyle->currentText(), cbWmsImgFormat->currentText()));
    lvWmsServers->setCurrentRow(lvWmsServers->count() - 1);
//    on_lvWmsServers_itemSelectionChanged();
}

void WMSPreferencesDialog::on_btDelWmsServer_clicked(void)
{
    int idx = lvWmsServers->currentItem()->data(Qt::UserRole).toInt();
    if (idx >= theWmsServers.size())
        return;

    theWmsServers[idx].deleted = true;
    delete lvWmsServers->currentItem();
    on_lvWmsServers_itemSelectionChanged();
}

void WMSPreferencesDialog::on_lvWmsServers_itemSelectionChanged()
{
    frTileIt->setEnabled(false);
    cbTileIt->setChecked(false);
    sbZoomLevels->setValue(0);
    setStatus( Empty );
    frWmsSettings->setVisible(true);
    isTiled = 0;
    selWmscLayer = WmscLayer();

    QListWidgetItem* it = lvWmsServers->item(lvWmsServers->currentRow());

    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= theWmsServers.size())
        return;

    WmsServer& WS(theWmsServers[idx]);
    edWmsName->setText(WS.WmsName);
    edWmsUrl->setText("http://" + WS.WmsAdress + WS.WmsPath);
    edWmsLayers->setText(WS.WmsLayers);
    cbWmsProj->setEditText(WS.WmsProjections);
    cbWmsStyle->setEditText(WS.WmsStyles);
    cbWmsImgFormat->setEditText(WS.WmsImgFormat);
    edSourceTag->setText(WS.WmsSourceTag);
    edLicenseUrl->setText(WS.WmsLicenseUrl);
    isTiled = WS.WmsIsTiled;
    if (isTiled > 0)
        selWmscLayer = WS.WmsCLayer;
    if (isTiled == 2) {
        frTileIt->setEnabled(true);
        cbTileIt->setChecked(true);
        sbZoomLevels->setValue(selWmscLayer.Resolutions.size());
    }
    on_cbWmsProj_activated(WS.WmsProjections);

    selectedServer = WS.WmsName;

    if (curReply) {
        curReply->abort();
    }
    QUrl u(edWmsUrl->text());
    if (u.isValid() && !u.host().isEmpty() && !u.path().isEmpty()) {
        showCapabilities();
    }
}

void WMSPreferencesDialog::on_cbWmsProj_activated(const QString &text)
{
    frTileIt->setEnabled(false);
    if (
            text.toUpper().contains("OSGEO:41001") ||
            text.toUpper().contains("EPSG:3785") ||
            text.toUpper().contains("EPSG:900913") ||
            text.toUpper().contains("EPSG:3857") ||
            text.toUpper().contains("EPSG:4326")
            )
    {
        frTileIt->setEnabled(true);
    }
    updateUrl();
}

void WMSPreferencesDialog::refreshStyles()
{
    QStringList layers = edWmsLayers->text().split(',');
    QStringList theStyles;
    foreach(QString l, layers) {
        theStyles << styles[l];
    }
    cbWmsStyle->setEditText(theStyles.join(","));

    updateUrl();
}

void WMSPreferencesDialog::on_cbWmsStyle_activated(int index)
{
    QString style = cbWmsStyle->itemData(index).toString();
    styles[curLayer] = style;

    refreshStyles();
}

void WMSPreferencesDialog::on_cbWmsStyle_editTextChanged(const QString &/*newText*/)
{
    updateUrl();
}

void WMSPreferencesDialog::on_cbWmsImgFormat_activated(const QString &/*text*/)
{
    updateUrl();
}

QString WMSPreferencesDialog::getSelectedServer()
{
    return selectedServer;
}

void WMSPreferencesDialog::setSelectedServer(QString theValue)
{
    QList<QListWidgetItem *> L = lvWmsServers->findItems(theValue, Qt::MatchExactly);
    if (L.size()) {
        lvWmsServers->setCurrentItem(L[0]);
        on_lvWmsServers_itemSelectionChanged();
    }
}

void WMSPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if (button == buttonBox->button(QDialogButtonBox::Apply)) {
        savePrefs();
    } else
        if (button == buttonBox->button(QDialogButtonBox::Ok)) {
            savePrefs();
            this->accept();
        }
}

void WMSPreferencesDialog::loadPrefs()
{
    WmsServerList* L = M_PREFS->getWmsServers();
    WmsServerListIterator i(*L);
    while (i.hasNext()) {
        i.next();
        addServer(i.value());
    }
}

void WMSPreferencesDialog::savePrefs()
{
    WmsServerList* L = M_PREFS->getWmsServers();
    L->clear();
    for (int i = 0; i < theWmsServers.size(); ++i) {
        WmsServer S(theWmsServers[i]);
        L->insert(theWmsServers[i].WmsName, S);
    }
    //M_PREFS->setSelectedWmsServer(getSelectedServer());
    M_PREFS->save();
}

void WMSPreferencesDialog::showCapabilities(void)
{
    if (curReply)
        return;

    QUrl theUrl(edWmsUrl->text());
#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    QUrlQuery theQuery(theUrl);
#define theQuery theQuery
#else
#define theQuery theUrl
#endif
    if ((theUrl.host() == "") || (theUrl.path() == "")) {
        QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Address and Path cannot be blank."), QMessageBox::Ok);
    }

    if (!theQuery.hasQueryItem("VERSION"))
        theQuery.addQueryItem("VERSION", "1.1.1");
    if (!theQuery.hasQueryItem("SERVICE"))
        theQuery.addQueryItem("SERVICE", "WMS");
    theQuery.removeAllQueryItems("LAYERS");
    theQuery.removeAllQueryItems("SRS");
    theQuery.removeAllQueryItems("FORMAT");
    theQuery.removeAllQueryItems("STYLES");
    theQuery.addQueryItem("REQUEST", "GetCapabilities");

#if QT_VERSION >= QT_VERSION_CHECK(5,0,0)
    theUrl.setQuery(theQuery);
#endif
#undef theQuery
//    QUrl url(edWmsUrl->text() + "VERSION=1.1.1&SERVICE=WMS&request=GetCapabilities");

    requestCapabilities(theUrl);
}

void WMSPreferencesDialog::requestCapabilities(QUrl url)
{
    if (curReply)
        return;

    tvWmsLayers->clear();

    QString oldSrs = cbWmsProj->currentText();
    cbWmsProj->clear();
    cbWmsProj->setEditText(oldSrs);

    QString oldFormat = cbWmsImgFormat->currentText();
    cbWmsImgFormat->clear();
    cbWmsImgFormat->setEditText(oldFormat);

    m_networkManager.setProxy(M_PREFS->getProxy(url));
    QNetworkRequest req(url);
    req.setRawHeader("Host", url.host().toLatin1());
    req.setRawHeader("User-Agent", USER_AGENT.toLatin1());

    curReply = m_networkManager.get(req);
}

void WMSPreferencesDialog::httpRequestFinished(QNetworkReply * reply)
{
    if (reply != curReply)
        return;

    setStatus( Empty );

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            setStatus( ErrorGetCapabilities, reply->errorString() );
            //QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Error reading capabilities.\n") + reply->errorString(), QMessageBox::Ok);
    } else {
        switch (statusCode) {
            case 200:
                break;
            case 301:
            case 302:
            case 307:
            // Redirected
                requestCapabilities(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl());
                curReply->deleteLater();
                curReply = NULL;
                return;
            default:
                QString reason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
                setStatus( ErrorGetCapabilities, reason);
                /*
                QMessageBox::information(this, tr("Merkaartor: GetCapabilities"),
                                  tr("Download failed: %1.")
                                  .arg(statusCode)); */
                curReply->deleteLater();
                curReply = NULL;
                return;
        }
    }

    frWmsSettings->setVisible(true);
    isTiled = 0;

    QDomDocument theXmlDoc;
    QByteArray buf;
    buf = reply->readAll();
//    QString src(buf);
//    qDebug() << src;
    theXmlDoc.setContent(buf);
    curReply->deleteLater();
    curReply = NULL;


    QDomElement docElem = theXmlDoc.documentElement();

    if (edWmsName->text().isEmpty()) {
        QDomElement svcElem = docElem.firstChildElement("Service");
        if (!svcElem.isNull()) {
            QDomNodeList aNodeList = svcElem.elementsByTagName("Title");
            if (aNodeList.size())
                edWmsName->setText(aNodeList.item(0).firstChild().nodeValue());
        }
    }

    QDomElement capElem = docElem.firstChildElement("Capability");
    if (capElem.isNull()) {
        // No "Capability"
        return;
    }

    srsList.clear();
    styleList.clear();
    QDomElement layElem = capElem.firstChildElement("Layer");
    while(!layElem.isNull()) {
        QTreeWidgetItem* it = parseLayer(layElem, NULL);
        tvWmsLayers->addTopLevelItem(it);
        tvWmsLayers->expandItem(it);

        layElem = layElem.nextSiblingElement("Layer");
    }

    QString oldStyle = cbWmsStyle->currentText();
//    cbWmsProj->clear();
//    srsList.sort();
//    cbWmsProj->addItems(srsList);
    cbWmsStyle->setEditText(oldStyle);
//    on_cbWmsProj_currentIndexChanged(oldSrs);

    QDomElement reqElem = capElem.firstChildElement("Request");
    QDomElement GetMapElem = reqElem.firstChildElement("GetMap");
    QDomNodeList formatNodeList = GetMapElem.elementsByTagName("Format");

    QStringList formatList;
    QString oldFormat = cbWmsImgFormat->currentText();
    cbWmsImgFormat->clear();
    for (int i=0; i<formatNodeList.size(); ++i) {
        if (!formatList.contains(formatNodeList.item(i).firstChild().nodeValue()))
            formatList.append(formatNodeList.item(i).firstChild().nodeValue());
    }
    formatList.sort();
    cbWmsImgFormat->addItems(formatList);
    cbWmsImgFormat->setEditText(oldFormat);
    on_cbWmsImgFormat_activated(oldFormat);

    QDomElement vendorElem = capElem.firstChildElement("VendorSpecificCapabilities");
    if (!vendorElem.isNull()) {
        parseVendorSpecific(vendorElem);
    }

    tvWmsLayers->setCurrentItem(tvWmsLayers->topLevelItem(0));
}

void WMSPreferencesDialog::parseVendorSpecific(QDomElement &vendorElem)
{
    QDomElement elem = vendorElem.firstChildElement();
    while(!elem.isNull()) {
        if (elem.tagName() == "TileSet") {
            WmscLayer aLayer;
            parseTileSet(elem, aLayer);
            if (!aLayer.LayerName.isNull())
                wmscLayers << aLayer;
        }
        elem = elem.nextSiblingElement();
    }
}

void WMSPreferencesDialog::parseTileSet(QDomElement &tilesetElem, WmscLayer &aLayer)
{
    setStatus( Caching );
    frWmsSettings->setVisible(false);
    isTiled = 1;

    QDomElement elem = tilesetElem.firstChildElement();
    while(!elem.isNull()) {
        if (elem.tagName() == "Layers") {
            aLayer.LayerName = elem.firstChild().nodeValue();
        } else if (elem.tagName() == "SRS") {
            aLayer.Projection = elem.firstChild().nodeValue();
        } else if (elem.tagName() == "Format") {
            aLayer.ImgFormat = elem.firstChild().nodeValue();
        } else if (elem.tagName() == "Styles") {
            aLayer.Styles = elem.firstChild().nodeValue();
        } else if (elem.tagName() == "BoundingBox") {
            QPointF tl, br;
            tl.setX(elem.attribute("minx").toDouble());
            tl.setY(elem.attribute("miny").toDouble());
            br.setX(elem.attribute("maxx").toDouble());
            br.setY(elem.attribute("maxy").toDouble());
            aLayer.BoundingBox = QRectF(tl, br);
        } else if (elem.tagName() == "Resolutions") {
            QStringList resL;
            resL = elem.firstChild().nodeValue().split(" ", QString::SkipEmptyParts);
            foreach(QString res, resL)
                aLayer.Resolutions << res.toDouble();
        } else if (elem.tagName() == "Width") {
            aLayer.TileWidth = elem.firstChild().nodeValue().toInt();
        } else if (elem.tagName() == "Height") {
            aLayer.TileHeight = elem.firstChild().nodeValue().toInt();
        }

        elem = elem.nextSiblingElement();
    }
}

QTreeWidgetItem * WMSPreferencesDialog::parseLayer(const QDomElement& aLayerElem,
                              QTreeWidgetItem* aLayerItem)
{
    if (aLayerElem.tagName() != "Layer")
        return NULL;

    QDomElement title = aLayerElem.firstChildElement("Title");
    QDomElement name = aLayerElem.firstChildElement("Name");

    QString theTitle, theName;
    if (!name.isNull())
        theName = name.firstChild().nodeValue();
    if (!title.isNull()) {
        theTitle = title.firstChild().nodeValue();
    } else theTitle = tr("Unnamed layer");


    QTreeWidgetItem *newItem = new QTreeWidgetItem;
    newItem->setFlags(Qt::NoItemFlags |Qt::ItemIsEnabled);


    newItem->setText(0,theTitle);
    newItem->setText(1,theName);

    if (!title.isNull()) {
        newItem->setToolTip(0, title.firstChild().nodeValue());
    } else {
        newItem->setToolTip(0, tr("Untitled"));
    }

    if (!name.isNull()) {
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        newItem->setData(0, Qt::UserRole, theName);
        newItem->setCheckState(0, Qt::Unchecked);
        foreach (QString s, edWmsLayers->text().split(',')) {
            if (theName == s) {
                newItem->setCheckState(0, Qt::Checked);
                break;
            }
        }
    } else {
//        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    if (aLayerItem)
        aLayerItem->addChild(newItem);

    QStringList upSrsList;
    if (aLayerItem)
        upSrsList = srsList.value(aLayerItem->text(0));
    QDomElement srsNode = aLayerElem.firstChildElement("SRS");
    while (!srsNode.isNull()) {
        if (!upSrsList.contains(srsNode.firstChild().nodeValue()))
            upSrsList.append(srsNode.firstChild().nodeValue());
        srsNode = srsNode.nextSiblingElement("SRS");
    }
    srsList[newItem->text(0)] = upSrsList;

    QList <QPair<QString, QString> > upStyleList;
    if (aLayerItem)
        upStyleList = styleList.value(aLayerItem->text(0));
    QDomElement styleNode = aLayerElem.firstChildElement("Style");
    while (!styleNode.isNull()) {
        QDomElement title = styleNode.firstChildElement("Title");
        QDomElement name = styleNode.firstChildElement("Name");

        if (!name.isNull() && !upStyleList.contains(qMakePair(name.firstChild().nodeValue(), title.firstChild().nodeValue())))
            upStyleList.append(qMakePair(name.firstChild().nodeValue(), title.firstChild().nodeValue()));

        styleNode = styleNode.nextSiblingElement("Style");
    }
    styleList[newItem->text(0)] = upStyleList;

    QDomElement layElem = aLayerElem.firstChildElement("Layer");
    while(!layElem.isNull()) {
        parseLayer(layElem, newItem);
        layElem = layElem.nextSiblingElement("Layer");
    }

    tvWmsLayers->expandItem(newItem);
    tvWmsLayers->header()->setStretchLastSection(false);
    tvWmsLayers->header()->setResizeMode(QHeaderView::ResizeToContents);
    return newItem;
}

void WMSPreferencesDialog::setStatus( WMSStatus status, QString message ) {
    switch (status) {
        case Caching:
            lblStatus->setText(tr("This is a caching WMS server."));
            break;
        case ErrorNetwork:
            lblStatus->setText(tr("Could not contact WMS server: %1").arg(message));
            break;
        case ErrorGetCapabilities:
            lblStatus->setText(tr("Could not get capabilities: %1").arg(message));
            break;
        default:
            lblStatus->setText("");
    }

    if (status > Error) {
        lblStatus->setStyleSheet("QLabel { color : red; }");
    } else {
        lblStatus->setStyleSheet("");
    }
}

void WMSPreferencesDialog::setTreeCheckState(Qt::CheckState state, QTreeWidgetItem *twi) {
    twi->setCheckState(0, state);
    for (int i = 0; i < twi->childCount(); i++)
        setTreeCheckState(state, twi->child(i));
}

void WMSPreferencesDialog::on_tvWmsLayers_itemChanged(QTreeWidgetItem *twi, int)
{
    QStringList theLayers;
    bool hasSelection = false;

    /* Set the checkbox status recursively for the whole subtree */
    setTreeCheckState(twi->checkState(0), twi);

    if (isTiled == 1 && twi->checkState(0) == Qt::Checked) {
        theLayers.append(twi->data(0, Qt::UserRole).toString());
        hasSelection = true;
        foreach(WmscLayer layer, wmscLayers)
            if (layer.LayerName == twi->data(0, Qt::UserRole).toString()) {
                selWmscLayer = layer;
            }
    }

    QTreeWidgetItemIterator it(tvWmsLayers);
    while (*it) {
        if ((*it)->checkState(0) == Qt::Checked) {
            if (isTiled == 1) {
                if (hasSelection && *it != twi)
                    (*it)->setCheckState(0, Qt::Unchecked);
            } else {
                theLayers.append((*it)->data(0, Qt::UserRole).toString());
            }
        }
        ++it;
    }
    edWmsLayers->setText(theLayers.join(","));
    refreshStyles();
}

void WMSPreferencesDialog::on_tvWmsLayers_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem */*previous*/)
{
    if (current)
        curLayer = current->text(0);
    else
        return;

    QStringList theSRS = srsList[curLayer];
    QString oldSrs = cbWmsProj->currentText();
    cbWmsProj->clear();
    theSRS.sort();
    cbWmsProj->addItems(theSRS);
    cbWmsProj->setEditText(oldSrs);
    on_cbWmsProj_activated(oldSrs);

    QList <QPair<QString, QString> > theStyles = styleList[curLayer];
    QString oldStyle = cbWmsStyle->currentText();
    cbWmsStyle->clear();
    QPair<QString, QString> aStyle;
    foreach(aStyle, theStyles) {
        cbWmsStyle->addItem(aStyle.second, QVariant(aStyle.first));
    }
    cbWmsStyle->setEditText(oldStyle);
    on_cbWmsProj_activated(oldSrs);
}
