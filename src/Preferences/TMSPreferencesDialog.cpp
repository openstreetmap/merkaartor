//
// C++ Implementation: TMSPreferencesDialog
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, Bart Vanhauwaert (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "Preferences/TMSPreferencesDialog.h"

#include <QMessageBox>
#include <QDir>
#include <QUrl>
#include <QTextEdit>
#include <QComboBox>

TMSPreferencesDialog::TMSPreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    frOSGeo->setVisible(false);

    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    loadPrefs();
}

TMSPreferencesDialog::~TMSPreferencesDialog()
{
}

void TMSPreferencesDialog::addServer(const TmsServer & srv)
{
    theTmsServers.push_back(srv);
    if (!srv.deleted) {
        QListWidgetItem* item = new QListWidgetItem(srv.TmsName);
        item->setData(Qt::UserRole, (int) theTmsServers.size()-1);
        lvTmsServers->addItem(item);
    }
}

void TMSPreferencesDialog::on_btApplyTmsServer_clicked(void)
{
    int idx = static_cast<int>(lvTmsServers->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theTmsServers.size())
        return;

    QUrl theUrl(edTmsUrl->text());
    QString theAdress = theUrl.host();
    if (theUrl.port() != -1)
        theAdress += ":" + QString::number(theUrl.port());

    TmsServer& WS(theTmsServers[idx]);

    if (frOSGeo->isVisible()) {
        if (!lvTmsServices->currentItem())
            return;
        TileService ts = services[lvTmsServices->currentItem()->text()];
        theUrl = ts.Url;
        theUrl.setPath(theUrl.path() + "/%1/%2/%3." + ts.Format);
        WS.TmsBaseUrl = edTmsUrl->text();
    }

    WS.TmsName = edTmsName->text();
    WS.TmsAdress = theAdress;
    WS.TmsPath = theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    WS.TmsTileSize = sbTileSize->value();
    WS.TmsMinZoom = sbMinZoom->value();
    WS.TmsMaxZoom = sbMaxZoom->value();
    WS.TmsBlOrigin = cbBotLeftOrigin->isChecked();
    WS.TmsSourceTag = edSourceTag->text();
    WS.TmsLicenseUrl = edLicenseUrl->text();
    if (cbSRS->currentIndex() == 1)
        WS.TmsProjection = "EPSG:4326";
    else
        WS.TmsProjection = "EPSG:900913";


    lvTmsServers->currentItem()->setText(WS.TmsName);
    selectedServer = WS.TmsName;
}

void TMSPreferencesDialog::on_btAddTmsServer_clicked(void)
{
    QUrl theUrl(edTmsUrl->text());
    QString theAdress = theUrl.host();
    if (theUrl.port() != -1)
        theAdress += ":" + QString::number(theUrl.port());

    QString theBaseUrl;
    if (frOSGeo->isVisible()) {
        if (!lvTmsServices->currentItem())
            return;
        TileService ts = services[lvTmsServices->currentItem()->text()];
        theUrl = ts.Url;
        theUrl.setPath(theUrl.path() + "/%1/%2/%3." + ts.Format);
        theBaseUrl = edTmsUrl->text();
    }
    QString proj;
    if (cbSRS->currentIndex() == 1)
        proj = "EPSG:4326";
    else
        proj = "EPSG:900913";

    addServer(TmsServer(edTmsName->text(), theAdress, theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority), proj, sbTileSize->value(), sbMinZoom->value(), sbMaxZoom->value(), edSourceTag->text(), edLicenseUrl->text(), theBaseUrl, cbBotLeftOrigin->isChecked()));
    lvTmsServers->setCurrentRow(lvTmsServers->count() - 1);
    on_lvTmsServers_itemSelectionChanged();
}

void TMSPreferencesDialog::on_btDelTmsServer_clicked(void)
{
    int idx = static_cast<int>(lvTmsServers->currentItem()->data(Qt::UserRole).toInt());
    if (idx >= theTmsServers.size())
        return;

    theTmsServers[idx].deleted = true;
    delete lvTmsServers->takeItem(idx);
    on_lvTmsServers_itemSelectionChanged();
}

void TMSPreferencesDialog::on_lvTmsServers_itemSelectionChanged()
{
    QListWidgetItem* it = lvTmsServers->item(lvTmsServers->currentRow());

    int idx = it->data(Qt::UserRole).toInt();
    if (idx >= theTmsServers.size())
        return;

    TmsServer& WS(theTmsServers[idx]);
    edTmsName->setText(WS.TmsName);
    edTmsUrl->setText("http://" + WS.TmsAdress + WS.TmsPath);
    sbTileSize->setValue(WS.TmsTileSize);
    sbMinZoom->setValue(WS.TmsMinZoom);
    sbMaxZoom->setValue(WS.TmsMaxZoom);
    edSourceTag->setText(WS.TmsSourceTag);
    edLicenseUrl->setText(WS.TmsLicenseUrl);
    cbBotLeftOrigin->setChecked(WS.TmsBlOrigin);
    int ix = cbSRS->findText(WS.TmsProjection, Qt::MatchContains | Qt::MatchCaseSensitive);
    cbSRS->setCurrentIndex(ix);

    if (WS.TmsBaseUrl.isEmpty()) {
        frOSGeo->setVisible(false);
        cbSRS->setEnabled(true);
        cbBotLeftOrigin->setEnabled(true);
        sbTileSize->setEnabled(true);
        sbMaxZoom->setEnabled(true);
        sbMinZoom->setEnabled(true);
    } else {
        edTmsUrl->setText(WS.TmsBaseUrl);

        frOSGeo->setVisible(true);
        cbSRS->setEnabled(false);
        cbBotLeftOrigin->setEnabled(false);
        sbTileSize->setEnabled(false);
        sbMaxZoom->setEnabled(false);
        sbMinZoom->setEnabled(false);

        on_btGetServices_clicked();
    }

    selectedServer = WS.TmsName;
}

QString TMSPreferencesDialog::getSelectedServer()
{
    return selectedServer;
}

void TMSPreferencesDialog::setSelectedServer(QString theValue)
{
    QList<QListWidgetItem *> L = lvTmsServers->findItems(theValue, Qt::MatchExactly);
    lvTmsServers->setCurrentItem(L[0]);
    on_lvTmsServers_itemSelectionChanged();
}

void TMSPreferencesDialog::on_buttonBox_clicked(QAbstractButton * button)
{
    if ((button == buttonBox->button(QDialogButtonBox::Apply))) {
        savePrefs();
    } else
        if ((button == buttonBox->button(QDialogButtonBox::Ok))) {
            savePrefs();
            this->accept();
        }
}

void TMSPreferencesDialog::loadPrefs()
{
    TmsServerList* L = M_PREFS->getTmsServers();
    TmsServerListIterator i(*L);
    while (i.hasNext()) {
        i.next();
        addServer(i.value());
    }
}

void TMSPreferencesDialog::savePrefs()
{
    TmsServerList* L = M_PREFS->getTmsServers();
    L->clear();
    for (int i = 0; i < theTmsServers.size(); ++i) {
        TmsServer S(theTmsServers[i]);
        L->insert(theTmsServers[i].TmsName, S);
    }
    //M_PREFS->setSelectedTmsServer(getSelectedServer());
    M_PREFS->save();
}

void TMSPreferencesDialog::on_btGetServices_clicked()
{
    QUrl theUrl(edTmsUrl->text());
    if ((theUrl.host() == "") || (theUrl.path() == "")) {
        QMessageBox::critical(this, tr("Merkaartor: GetServices"), tr("Address and Path cannot be blank."), QMessageBox::Ok);
    }

    lvTmsServices->clear();
    services.clear();

    http = new QHttp(this);
    connect (http, SIGNAL(requestFinished(int, bool)), this, SLOT(httpRequestFinished(int, bool)));
    connect(http, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)),
        this, SLOT(readResponseHeader(const QHttpResponseHeader &)));

    httpGetId = sendRequest(theUrl);

}

int TMSPreferencesDialog::sendRequest(QUrl url)
{
    QString requestUrl = url.encodedPath();
    if (!url.encodedQuery().isNull())
        requestUrl += "?" + url.encodedQuery();
    QHttpRequestHeader header("GET", requestUrl);
    qDebug() << header.toString();

    QString host = url.host();
    if (url.port() != -1)
        host += ":" + QString::number(url.port());
    header.setValue("Host", host);
    header.setValue("User-Agent", USER_AGENT);

    http->setHost(url.host(), url.port() == -1 ? 80 : url.port());
    http->setProxy(M_PREFS->getProxy(url));

    return http->request(header);
}

void TMSPreferencesDialog::readResponseHeader(const QHttpResponseHeader &responseHeader)
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
            sendRequest(QUrl(responseHeader.value("Location")));
            break;

        default:
            http->abort();
            QMessageBox::information(this, tr("Merkaartor: GetServices"),
                                  tr("Download failed: %1.")
                                  .arg(responseHeader.reasonPhrase()));
    }
}

void TMSPreferencesDialog::httpRequestFinished(int /*id*/, bool error)
{
    if (error) {
        if (http->error() != QHttp::Aborted)
            QMessageBox::critical(this, tr("Merkaartor: GetServices"), tr("Error reading services.\n") + http->errorString(), QMessageBox::Ok);
        return;
    }

    QDomDocument doc;
    QString content = http->readAll();
    qDebug() << content;
    doc.setContent(content);
    if (doc.isNull())
        return;

    QDomElement e = doc.firstChildElement();
    if (e.nodeName().toLower() == "services") {
        QDomNodeList l = e.elementsByTagName("TileMapService");
        for (int i=0; i<l.size(); ++i) {
            QString href = l.at(i).toElement().attribute("href");
            if (!href.isNull()) {
                sendRequest(QUrl(href));
            }
        }
    } else if (e.nodeName().toLower() == "tilemapservice") {
        QDomNodeList l = e.elementsByTagName("TileMap");
        for (int i=0; i<l.size(); ++i) {
            QString href = l.at(i).toElement().attribute("href");
            if (!href.isNull()) {
                sendRequest(QUrl(href));
            }
        }
    } else if (e.nodeName().toLower() == "tilemap") {
        QString url;
        QString title;
        QString srs;
        CoordBox bbox;
        Coord origin;
        QSize tilesize;
        QString tileformat;
        int minzoom = 9999, maxzoom = 0;

        url = e.attribute("tilemapservice");
        QDomElement c = e.firstChildElement();
        while (!c.isNull()) {
            if (c.nodeName().toLower() == "title") {
                title = c.firstChild().toText().nodeValue();
                url = url + title + "/";
            } else if (c.nodeName().toLower() == "srs") {
                srs = c.firstChild().toText().nodeValue();
            } else if (c.nodeName().toLower() == "boundingbox") {
                Coord bl(angToCoord(c.attribute("miny").toDouble()), angToCoord(c.attribute("minx").toDouble()));
                Coord tr(angToCoord(c.attribute("maxy").toDouble()), angToCoord(c.attribute("maxx").toDouble()));
                bbox = CoordBox(bl, tr);
            } else if (c.nodeName().toLower() == "origin") {
                Coord pt(angToCoord(c.attribute("y").toDouble()), angToCoord(c.attribute("x").toDouble()));
                origin = pt;
            } else if (c.nodeName().toLower() == "tileformat") {
                tilesize.setWidth(c.attribute("width").toInt());
                tilesize.setHeight(c.attribute("height").toInt());
                tileformat = c.attribute("extension");
            } else if (c.nodeName().toLower() == "tilesets") {
                QDomElement t = c.firstChildElement();
                while (!t.isNull()) {
                    int o = t.attribute("order").toInt();
                    minzoom = o < minzoom ? o : minzoom;
                    maxzoom = o > maxzoom ? o : maxzoom;
                    t = t.nextSiblingElement();
                }
            }
            c = c.nextSiblingElement();
        }
        TileService ts;
        ts.Title = title;
        ts.Url = url;
        ts.SRS = srs;
        ts.TileSize = tilesize.width();
        ts.MinZoom = minzoom;
        ts.MaxZoom = maxzoom;
        ts.BBox = bbox;
        ts.Format = tileformat;
        Coord center = bbox.center();
        if (origin.lat() < center.lat())
            ts.Origin = true;
        else
            ts.Origin = false;

        services.insert(title, ts);

        lvTmsServices->addItem(title);
        frOSGeo->setVisible(true);
        cbSRS->setEnabled(false);
        cbBotLeftOrigin->setEnabled(false);
        sbTileSize->setEnabled(false);
        sbMaxZoom->setEnabled(false);
        sbMinZoom->setEnabled(false);
    }
}

void TMSPreferencesDialog::on_lvTmsServices_itemSelectionChanged()
{
    TileService ts = services[lvTmsServices->currentItem()->text()];
    int ix = cbSRS->findText(ts.SRS, Qt::MatchContains | Qt::MatchCaseSensitive);
    cbSRS->setCurrentIndex(ix);
    cbBotLeftOrigin->setChecked(ts.Origin);
    sbTileSize->setValue(ts.TileSize);
    sbMinZoom->setValue(ts.MinZoom);
    sbMaxZoom->setValue(ts.MaxZoom);
}

