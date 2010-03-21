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
#include "Preferences/WMSPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

WMSPreferencesDialog::WMSPreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadPrefs();

    edWmsLayers->setVisible(false);
    frWmsSettings->setVisible(false);
    lblWMSC->setVisible(false);
    isTiled = 0;
    frTileIt->setEnabled(false);

    connect(tvWmsLayers, SIGNAL(itemChanged(QTreeWidgetItem *, int)), this, SLOT(on_tvWmsLayers_itemChanged(QTreeWidgetItem *, int)));
}

WMSPreferencesDialog::~WMSPreferencesDialog()
{
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
    selWmscLayer.Styles = edWmsStyles->text();
    selWmscLayer.ImgFormat = cbWmsImgFormat->currentText();
    selWmscLayer.TileHeight = 256;
    selWmscLayer.TileWidth = 256;
    qreal startRes;
    if (selWmscLayer.Projection.contains("4326")) {
        selWmscLayer.BoundingBox = QRectF(QPointF(-180, -90.), QPointF(180., 90.));
        startRes = 0.703125;
    } else {
        selWmscLayer.BoundingBox = QRectF(QPointF(-20037508.34, -20037508.34), QPointF(20037508.34, 20037508.34));
        startRes = 156543.03;
    }
    selWmscLayer.Resolutions.clear();
    selWmscLayer.Resolutions << startRes;
    for (int i=1; i<sbZoomLevels->value(); ++i) {
        startRes /= 2.;
        selWmscLayer.Resolutions << startRes;
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
    WS.WmsStyles = edWmsStyles->text();
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
            edWmsLayers->text(), selWmscLayer.Projection, selWmscLayer.Styles, selWmscLayer.ImgFormat,
            isTiled, selWmscLayer));
    else
        addServer(WmsServer(edWmsName->text(), theAdress, theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority),
            edWmsLayers->text(), cbWmsProj->currentText(), edWmsStyles->text(), cbWmsImgFormat->currentText()));
    lvWmsServers->setCurrentRow(lvWmsServers->count() - 1);
    on_lvWmsServers_itemSelectionChanged();
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
    frWmsSettings->setVisible(false);
    frTileIt->setEnabled(false);
    cbTileIt->setChecked(false);
    sbZoomLevels->setValue(0);
    lblWMSC->setVisible(false);
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
    on_cbWmsProj_currentIndexChanged(WS.WmsProjections);
    edWmsStyles->setText(WS.WmsStyles);
    cbWmsImgFormat->setEditText(WS.WmsImgFormat);
    isTiled = WS.WmsIsTiled;
    if (isTiled > 0)
        selWmscLayer = WS.WmsCLayer;
    if (isTiled == 2) {
        frTileIt->setEnabled(true);
        cbTileIt->setChecked(true);
        sbZoomLevels->setValue(selWmscLayer.Resolutions.size());
    }

    selectedServer = WS.WmsName;

    QUrl theUrl(edWmsUrl->text());
    if ((theUrl.host() != "") && (theUrl.path() != "")) {
        QUrl url(edWmsUrl->text() + "VERSION=1.1.1&SERVICE=WMS&request=GetCapabilities");
        requestCapabilities(url);
    }
}

void WMSPreferencesDialog::on_cbWmsProj_currentIndexChanged(const QString &text)
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
    if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
        savePrefs();
    } else
        if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
            savePrefs();
            this->accept();
        }
}

void WMSPreferencesDialog::loadPrefs()
{
    WmsServerList* L = MerkaartorPreferences::instance()->getWmsServers();
    WmsServerListIterator i(*L);
    while (i.hasNext()) {
        i.next();
        addServer(i.value());
    }
}

void WMSPreferencesDialog::savePrefs()
{
    WmsServerList* L = MerkaartorPreferences::instance()->getWmsServers();
    L->clear();
    for (int i = 0; i < theWmsServers.size(); ++i) {
        WmsServer S(theWmsServers[i]);
        L->insert(theWmsServers[i].WmsName, S);
    }
    //MerkaartorPreferences::instance()->setSelectedWmsServer(getSelectedServer());
    M_PREFS->save();
}

void WMSPreferencesDialog::on_btShowCapabilities_clicked(void)
{
    QUrl theUrl(edWmsUrl->text());
    if ((theUrl.host() == "") || (theUrl.path() == "")) {
        QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Address and Path cannot be blank."), QMessageBox::Ok);
    }

    QUrl url(edWmsUrl->text() + "VERSION=1.1.1&SERVICE=WMS&request=GetCapabilities");
    requestCapabilities(url);
}

void WMSPreferencesDialog::requestCapabilities(QUrl url)
{
    tvWmsLayers->clear();

    QString oldSrs = cbWmsProj->currentText();
    cbWmsProj->clear();
    cbWmsProj->setEditText(oldSrs);

    QString oldFormat = cbWmsImgFormat->currentText();
    cbWmsImgFormat->clear();
    cbWmsImgFormat->setEditText(oldFormat);

    http = new QHttp(this);
    connect (http, SIGNAL(done(bool)), this, SLOT(httpRequestFinished(bool)));
    connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
        this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

    QHttpRequestHeader header("GET", url.path() + "?" + url.encodedQuery());
    qDebug() << header.toString();
    const char *userAgent = "Mozilla/9.876 (X11; U; Linux 2.2.12-20 i686, en) Gecko/25250101 Netscape/5.432b1";

    header.setValue("Host", url.host());
    header.setValue("User-Agent", userAgent);

    http->setHost(url.host(), url.port() == -1 ? 80 : url.port());
    http->setProxy(M_PREFS->getProxy(url));

    httpGetId = http->request(header);
}

void WMSPreferencesDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
{
    qDebug() << responseHeader.toString();
    switch (responseHeader.statusCode())
    {
        case 200:
            break;

        case 301:
        case 302:
        case 307:
            http->abort();
            requestCapabilities(QUrl(responseHeader.value("Location")));
            break;

        default:
            http->abort();
            QMessageBox::information(this, tr("Merkaartor: GetCapabilities"),
                                  tr("Download failed: %1.")
                                  .arg(responseHeader.reasonPhrase()));
    }
}

void WMSPreferencesDialog::httpRequestFinished(bool error)
{
    //if (error) {
    //	if (http->error() != QHttp::Aborted)
    //		QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Error reading capabilities.\n") + http->errorString(), QMessageBox::Ok);
    //} else {
    //	QVBoxLayout *mainLayout = new QVBoxLayout;
    //	QTextEdit* edit = new QTextEdit();
    //	edit->setPlainText(QString(http->readAll()));
    //	mainLayout->addWidget(edit);

    //	QDialog* dlg = new QDialog(this);
    //	dlg->setLayout(mainLayout);
    //	dlg->show();
    //	//delete dlg;
    //}

    if (error) {
        if (http->error() != QHttp::Aborted)
            QMessageBox::critical(this, tr("Merkaartor: GetCapabilities"), tr("Error reading capabilities.\n") + http->errorString(), QMessageBox::Ok);
        return;
    }

    frWmsSettings->setVisible(true);
    lblWMSC->setVisible(false);
    isTiled = 0;

    QDomDocument theXmlDoc;
    theXmlDoc.setContent(http->readAll());

    QDomElement docElem = theXmlDoc.documentElement();
    QDomElement capElem = docElem.firstChildElement("Capability");
    if (capElem.isNull()) {
        // No "Capability"
        return;
    }

    QStringList srsList;
    QDomElement layElem = capElem.firstChildElement("Layer");
    while(!layElem.isNull()) {
        QTreeWidgetItem* it = parseLayers(layElem, NULL);
        tvWmsLayers->addTopLevelItem(it);
        tvWmsLayers->expandItem(it);
        QDomNodeList aNodeList = layElem.elementsByTagName("SRS");
        for (int i=0; i<aNodeList.size(); ++i) {
            if (!srsList.contains(aNodeList.item(i).firstChild().toText().nodeValue()))
                srsList.append(aNodeList.item(i).firstChild().toText().nodeValue());
        }

        layElem = layElem.nextSiblingElement("Layer");
    }
    QString oldSrs = cbWmsProj->currentText();
    cbWmsProj->clear();
    srsList.sort();
    cbWmsProj->addItems(srsList);
    cbWmsProj->setEditText(oldSrs);
    on_cbWmsProj_currentIndexChanged(oldSrs);

    QDomElement reqElem = capElem.firstChildElement("Request");
    QDomElement GetMapElem = reqElem.firstChildElement("GetMap");
    QDomNodeList formatNodeList = GetMapElem.elementsByTagName("Format");

    QStringList formatList;
    QString oldFormat = cbWmsImgFormat->currentText();
    cbWmsImgFormat->clear();
    for (int i=0; i<formatNodeList.size(); ++i) {
        if (!formatList.contains(formatNodeList.item(i).firstChild().toText().nodeValue()))
            formatList.append(formatNodeList.item(i).firstChild().toText().nodeValue());
    }
    formatList.sort();
    cbWmsImgFormat->addItems(formatList);
    cbWmsImgFormat->setEditText(oldFormat);

    QDomElement vendorElem = capElem.firstChildElement("VendorSpecificCapabilities");
    if (!vendorElem.isNull()) {
        parseVendorSpecific(vendorElem);
    }
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
    frWmsSettings->setVisible(false);
    lblWMSC->setVisible(true);
    isTiled = 1;

    QDomElement elem = tilesetElem.firstChildElement();
    while(!elem.isNull()) {
        if (elem.tagName() == "Layers") {
            aLayer.LayerName = elem.firstChild().toText().nodeValue();
        } else if (elem.tagName() == "SRS") {
            aLayer.Projection = elem.firstChild().toText().nodeValue();
        } else if (elem.tagName() == "Format") {
            aLayer.ImgFormat = elem.firstChild().toText().nodeValue();
        } else if (elem.tagName() == "Styles") {
            aLayer.Styles = elem.firstChild().toText().nodeValue();
        } else if (elem.tagName() == "BoundingBox") {
            QPointF tl, br;
            tl.setX(elem.attribute("minx").toDouble());
            tl.setY(elem.attribute("miny").toDouble());
            br.setX(elem.attribute("maxx").toDouble());
            br.setY(elem.attribute("maxy").toDouble());
            aLayer.BoundingBox = QRectF(tl, br);
        } else if (elem.tagName() == "Resolutions") {
            QStringList resL;
            resL = elem.firstChild().toText().nodeValue().split(" ", QString::SkipEmptyParts);
            foreach(QString res, resL)
                aLayer.Resolutions << res.toDouble();
        } else if (elem.tagName() == "Width") {
            aLayer.TileWidth = elem.firstChild().toText().nodeValue().toInt();
        } else if (elem.tagName() == "Height") {
            aLayer.TileHeight = elem.firstChild().toText().nodeValue().toInt();
        }

        elem = elem.nextSiblingElement();
    }
}

QTreeWidgetItem * WMSPreferencesDialog::parseLayers(QDomElement& aLayerElem, QTreeWidgetItem* aLayerItem)
{
    if (aLayerElem.tagName() != "Layer")
        return NULL;

    QDomElement title = aLayerElem.firstChildElement("Title");
    QDomElement name = aLayerElem.firstChildElement("Name");

    QTreeWidgetItem *newItem = new QTreeWidgetItem;
    newItem->setFlags(Qt::NoItemFlags |Qt::ItemIsEnabled);
    if (!title.isNull())
        newItem->setText(0, title.firstChild().toText().nodeValue());
    else
        if (!name.isNull())
            newItem->setText(0,name.firstChild().toText().nodeValue());

    if (!name.isNull()) {
        QString theName = name.firstChild().toText().nodeValue();
        newItem->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsSelectable | Qt::ItemIsEnabled);
        newItem->setData(0, Qt::UserRole, theName);
        if (edWmsLayers->text().contains(theName))
            newItem->setCheckState(0, Qt::Checked);
        else
            newItem->setCheckState(0, Qt::Unchecked);
    }
    if (aLayerItem)
        aLayerItem->addChild(newItem);

    QDomElement layElem = aLayerElem.firstChildElement("Layer");
    while(!layElem.isNull()) {
        parseLayers(layElem, newItem);
        layElem = layElem.nextSiblingElement("Layer");
    }

    tvWmsLayers->expandItem(newItem);
    return newItem;
}

void WMSPreferencesDialog::on_tvWmsLayers_itemChanged(QTreeWidgetItem *twi, int)
{
    QStringList theLayers;
    bool hasSelection = false;

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
}
