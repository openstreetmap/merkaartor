//
// C++ Implementation: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, bvh, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Global.h"
#include "MerkaartorPreferences.h"

#include <QApplication>
#include <QByteArray>
#include <QMessageBox>


#include "MainWindow.h"
#include <ui_MainWindow.h>
#include "MapView.h"

#include "IMapAdapterFactory.h"
#include "PaintStyle/IPaintStyle.h"
#include "PaintStyle/MasPaintStyle.h"


#define M_PARAM_IMPLEMENT_BOOL(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(bool theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    bool MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else \
                m_##Param = Sets->value(#Category"/"#Param, Default).toBool(); \
        } \
        return  m_##Param; \
    }

#define M_PARAM_IMPLEMENT_STRING(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(QString theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    QString MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else \
                m_##Param = Sets->value(#Category"/"#Param, Default).toString(); \
        } \
        return  m_##Param; \
    }

#define M_PARAM_IMPLEMENT_INT(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(int theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    int MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else \
                m_##Param = Sets->value(#Category"/"#Param, Default).toInt(); \
        } \
        return  m_##Param; \
    }
#define M_PARAM_IMPLEMENT_INT_DELAYED(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(int theValue) \
    { \
        m_##Param = theValue; \
    } \
    void MerkaartorPreferences::save##Param() \
    { \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, m_##Param); \
        } \
    } \
    int MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else \
                m_##Param = Sets->value(#Category"/"#Param, Default).toInt(); \
        } \
        return  m_##Param; \
    }

#define M_PARAM_IMPLEMENT_DOUBLE(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(double theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    double MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else \
                m_##Param = Sets->value(#Category"/"#Param, Default).toDouble(); \
        } \
        return  m_##Param; \
    }

#define M_PARAM_IMPLEMENT_COLOR(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(QColor theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, QVariant(theValue)); \
        } \
    } \
    QColor MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = Default; \
            else { \
                QString sColor = Sets->value(#Category"/"#Param, "").toString(); \
                if (sColor.isEmpty() || !QColor(sColor).isValid()) \
                    m_##Param = Default; \
                else \
                    m_##Param = Sets->value(#Category"/"#Param).value<QColor>(); \
            } \
        } \
        return  m_##Param; \
    }

/***************************/

MerkaartorPreferences* MerkaartorPreferences::m_prefInstance = 0;
MerkaartorPreferences* MerkaartorPreferences::instance() {
    if (!m_prefInstance) {
        m_prefInstance = new MerkaartorPreferences;
    }

    return m_prefInstance;
}


IPaintStyle* MerkaartorPreferences::m_EPSInstance = 0;
IPaintStyle* MerkaartorPreferences::styleinstance() {
    if (!m_EPSInstance) {
        m_EPSInstance = new MasPaintStyle;
    }

    return m_EPSInstance;
}

Tool::Tool(QString Name, QString Path)
    : ToolName(Name), ToolPath(Path)
{
}

Tool::Tool()
    : ToolName(""), ToolPath("")
{
}

/* MekaartorPreferences */

MerkaartorPreferences::MerkaartorPreferences()
    : Sets(0)
{
    if (!g_Merk_Ignore_Preferences) {
        if (!g_Merk_Portable) {
            Sets = new QSettings();

            QSettings oldSettings("BartVanhauwaert", "Merkaartor");
            QStringList oldKeys = oldSettings.allKeys();
            foreach(QString k, oldKeys) {
                Sets->setValue(k, oldSettings.value(k));
                Sets->sync();
                oldSettings.remove(k);
            }
            oldSettings.clear();
        } else {
            Sets = new QSettings(qApp->applicationDirPath() + "/merkaartor.ini", QSettings::IniFormat);
        }
        version = Sets->value("version/version", "0").toString();
    }

    theToolList = new ToolList();

    connect(&httpRequest, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(on_responseHeaderReceived(const QHttpResponseHeader &)));
    connect(&httpRequest,SIGNAL(requestFinished(int, bool)),this,SLOT(on_requestFinished(int, bool)));

#ifdef USE_LIBPROXY
    // Initialise libproxy
    proxyFactory = px_proxy_factory_new();

    // Map libproxy URL schemes to QNetworkProxy types
    proxyTypeMap["direct"] = QNetworkProxy::NoProxy;
    proxyTypeMap["socks" ] = QNetworkProxy::Socks5Proxy;
    proxyTypeMap["socks5"] = QNetworkProxy::Socks5Proxy;
    proxyTypeMap["http"  ] = QNetworkProxy::HttpCachingProxy;
#endif

    initialize();
}

MerkaartorPreferences::~MerkaartorPreferences()
{
    delete theToolList;
    delete Sets;
#ifdef USE_LIBPROXY
    px_proxy_factory_free(proxyFactory);
#endif
}

void MerkaartorPreferences::save(bool UserPwdChanged)
{
    if (g_Merk_Ignore_Preferences)
        return;

    Sets->setValue("version/version", QString("%1").arg(STRINGIFY(VERSION)));
    setWmsServers();
    setTmsServers();
    setTools();
    setAlphaList();

    saveProjections();
    saveFilters();
    saveWMSes();
    saveTMSes();
    saveBookmarks();
    saveOsmServers();

    saveTagListFirstColumnWidth();
    Sets->sync();

    if (UserPwdChanged)
        fromOsmPref();
    else
        toOsmPref();
}

void MerkaartorPreferences::toOsmPref()
{
    qDebug() << "MerkaartorPreferences::toOsmPref";
    if (getOfflineMode()) return;

    if (getOsmUser().isEmpty() || getOsmPassword().isEmpty()) return;

    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorLists");
    theXmlDoc.appendChild(root);

    theProjectionsList.toXml(root);
    theBookmarkList.toXml(root);
    theTmsServerList.toXml(root);
    theWmsServerList.toXml(root);
    theFiltersList.toXml(root);

    QByteArray ba = qCompress(theXmlDoc.toString().toUtf8());
    QByteArray PrefsXML = ba.toBase64();

    QStringList slicedPrefs;
    for (int i=0; i<PrefsXML.size(); i+=254) {
        QString s = PrefsXML.mid(i, 254);
        slicedPrefs.append(s);
        if (s.size() < 254)
            break;
    }

    QMap<QString, QString> OsmPref;

    QString k = QString("MerkaartorSizePrefsXML");
    QString v = QString::number(slicedPrefs.size());
    OsmPref.insert(k, v);

    for (int i=0; i<slicedPrefs.size(); i++) {
        k = QString("MerkaartorPrefsXML%1").arg(i, 3, 10, QLatin1Char('0'));
        v = slicedPrefs[i];
        OsmPref.insert(k, v);
    }

    QMapIterator<QString, QString> it(OsmPref);
    while(it.hasNext()) {
        it.next();
        putOsmPref(it.key(), it.value());
    }

}

void MerkaartorPreferences::fromOsmPref()
{
    if (getOfflineMode()) return;

    if (getOsmUser().isEmpty() || getOsmPassword().isEmpty()) return;

    QUrl osmWeb(getOsmWebsite());
    if (osmWeb.port() == -1)
        osmWeb.setPort(80);

    httpRequest.setHost(osmWeb.host(), osmWeb.port());

    QHttpRequestHeader Header("GET", osmWeb.path() + "/user/preferences/");
    if (osmWeb.port() == 80)
        Header.setValue("Host",osmWeb.host());
    else
        Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
    Header.setValue("User-Agent", USER_AGENT);

    QString auth = QString("%1:%2").arg(getOsmUser()).arg(getOsmPassword());
    QByteArray ba_auth = auth.toUtf8().toBase64();
    Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

    httpRequest.setProxy(getProxy(osmWeb));
    OsmPrefLoadId = httpRequest.request(Header, NULL, &OsmPrefContent);
}

void MerkaartorPreferences::on_requestFinished ( int id, bool error )
{
    if (id != OsmPrefLoadId || error)
        return;

    QMap<QString, QString> OsmPref;

    QDomDocument aOsmPrefDoc;
    aOsmPrefDoc.setContent(OsmPrefContent.buffer(), false);

    QDomNodeList prefList = aOsmPrefDoc.elementsByTagName("preference");

    int sz = 0;
    for (int i=0; i < prefList.size(); ++i) {
        QDomElement e = prefList.at(i).toElement();
        if (e.attribute("k").startsWith("MerkaartorSizePrefsXML")) {
            sz = e.attribute("v").toInt();
            break;
        }
    }

    if (!sz)
        return;

    QVector<QString> slicedPrefs(sz);
    QString k, v;
    for (int i=0; i < prefList.size(); ++i) {
        QDomElement e = prefList.at(i).toElement();
        k = e.attribute("k");
        v = e.attribute("v");
        if (k.startsWith("MerkaartorPrefsXML")) {
            int idx = k.right(3).toInt();
            if (idx < sz)
                slicedPrefs[idx] = v;
        }
    }

    QByteArray PrefsXML;
    for (int i=0; i<sz; i++)
        PrefsXML.append(slicedPrefs[i].toAscii());

    //qDebug() << "Size: " << PrefsXML.size();

    QDomDocument theXmlDoc;
    QByteArray ba = QByteArray::fromBase64(PrefsXML);
    if (!theXmlDoc.setContent(qUncompress(ba))) {
        qDebug() << "Invalid OSM Prefs XML";
        return;
    }

    QDomElement docElem = theXmlDoc.documentElement();
    if (docElem.tagName() != "MerkaartorLists") {
        qDebug() << "Invalid OSM Prefs XML root element: " << docElem.tagName();
        return;
    }

    //qDebug() << theXmlDoc.toString();

    QDomElement c = docElem.firstChildElement();
    while(!c.isNull()) {
        if (c.tagName() == "Projections") {
            ProjectionsList aProjList = ProjectionsList::fromXml(c);
            theProjectionsList.add(aProjList);
        } else
            if (c.tagName() == "Bookmarks") {
            BookmarksList aBkList = BookmarksList::fromXml(c);
            theBookmarkList.add(aBkList);
        } else
            if (c.tagName() == "TmsServers") {
            TmsServersList aTmsList = TmsServersList::fromXml(c);
            theTmsServerList.add(aTmsList);
        } else
            if (c.tagName() == "WmsServers") {
            WmsServersList aWmsList = WmsServersList::fromXml(c);
            theWmsServerList.add(aWmsList);
        } else
            if (c.tagName() == "Filters") {
            FiltersList aFiltList = FiltersList::fromXml(c);
            theFiltersList.add(aFiltList);
        }

        c = c.nextSiblingElement();
    }
}


void MerkaartorPreferences::putOsmPref(const QString& k, const QString& v)
{
    QUrl osmWeb(getOsmWebsite());
    if (osmWeb.port() == -1)
        osmWeb.setPort(80);

    QByteArray ba(v.toUtf8());
    QBuffer Buf(&ba);

    httpRequest.setHost(osmWeb.host(), osmWeb.port());

    QHttpRequestHeader Header("PUT", osmWeb.path() + QString("/user/preferences/%1").arg(k));
    if (osmWeb.port() == 80)
        Header.setValue("Host",osmWeb.host());
    else
        Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
    Header.setValue("User-Agent", USER_AGENT);

    QString auth = QString("%1:%2").arg(getOsmUser()).arg(getOsmPassword());
    QByteArray ba_auth = auth.toUtf8().toBase64();
    Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

    httpRequest.setProxy(getProxy(osmWeb));
    OsmPrefSaveId = httpRequest.request(Header,ba);
}

void MerkaartorPreferences::deleteOsmPref(const QString& k)
{
    QUrl osmWeb(getOsmWebsite());
    if (osmWeb.port() == -1)
        osmWeb.setPort(80);

    httpRequest.setHost(osmWeb.host(), osmWeb.port());

    QHttpRequestHeader Header("DELETE", osmWeb.path() + QString("/user/preferences/%1").arg(k));
    if (osmWeb.port() == 80)
        Header.setValue("Host",osmWeb.host());
    else
        Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
    Header.setValue("User-Agent", USER_AGENT);

    QString auth = QString("%1:%2").arg(getOsmUser()).arg(getOsmPassword());
    QByteArray ba_auth = auth.toUtf8().toBase64();
    Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

    httpRequest.setProxy(getProxy(osmWeb));
    httpRequest.request(Header);
}

void MerkaartorPreferences::on_responseHeaderReceived(const QHttpResponseHeader & hdr)
{
    switch (hdr.statusCode()) {
        case 200:
            break;
        case 406:
            QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","Duplicate key"));
            break;
        case 413:
            QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","More than 150 preferences"));
            break;
        default:
            qDebug() << "MerkaartorPreferences::on_responseHeaderReceived :" << hdr.statusCode() << hdr.reasonPhrase();
            break;
    }
}

void MerkaartorPreferences::initialize()
{
//	Use06Api = Sets->value("osm/use06api", "true").toBool();
    Use06Api = true;

    loadProjections();
    loadFilters();
    loadWMSes();
    loadTMSes();
    loadBookmarks();
    loadOsmServers();

    fromOsmPref();

    QStringList sl;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        sl = Sets->value("downloadosm/bookmarks").toStringList();
        if (sl.size()) {
            for (int i=0; i<sl.size(); i+=5) {
                Bookmark B(sl[i], CoordBox(Coord(angToCoord(sl[i+1].toDouble()),angToCoord(sl[i+2].toDouble())),
                                           Coord(angToCoord(sl[i+3].toDouble()),angToCoord(sl[i+4].toDouble()))));
                theBookmarkList.addBookmark(B);
            }
            save();
            Sets->remove("downloadosm/bookmarks");
        }
    }

    QStringList alphaList = getAlphaList();
    if (alphaList.size() == 0) {
        alpha["Low"] = 0.33;
        alpha["High"] = 0.66;
        alpha["Opaque"] = 1.0;
    } else {
        for (int i=0; i<alphaList.size(); i+=2) {
            alpha[alphaList[i]] = alphaList[i+1].toDouble();
        }
    }

    QStringList tl;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        tl = Sets->value("Tools/list").toStringList();
        for (int i=0; i<tl.size(); i+=TOOL_FIELD_SIZE) {
            Tool t(tl[i], tl[i+1]);
            theToolList->insert(tl[i], t);
        }
    }
    if (!theToolList->contains("Inkscape")) {
        Tool t("Inkscape", "");
        theToolList->insert("Inkscape", t);
    }

    QStringList Servers;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        Servers = Sets->value("WSM/servers").toStringList();
        if (Servers.size()) {
            for (int i=0; i<Servers.size(); i+=7) {
                WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6], "", "");
                theWmsServerList.addServer(S);
            }
            save();
            Sets->remove("WSM/servers");
        }

        Servers = Sets->value("TMS/servers").toStringList();
        if (Servers.size()) {
            for (int i=0; i<Servers.size(); i+=6) {
                TmsServer S(Servers[i], Servers[i+1], Servers[i+2], "EPSG:900913", Servers[i+3].toInt(), Servers[i+4].toInt(), Servers[i+5].toInt(), "", "");
                theTmsServerList.addServer(S);
            }
            save();
            Sets->remove("TMS/servers");
        }
    }

    // PRoxy upgrade
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        if (Sets->contains("proxy/Use")) {
            bool b = Sets->value("proxy/Use").toBool();
            QString h = Sets->value("proxy/Host").toString();
            int p = Sets->value("proxy/Port").toInt();

            Sets->remove("proxy");

            setProxyUse(b);
            setProxyHost(h);
            setProxyPort(p);
        }
    }
    // Set global proxy
    QNetworkProxy::setApplicationProxy(getProxy(QUrl("http://merkaartor.be")));

    parentDashes << 1 << 5;

    //Ensure we have a CacheDir value in QSettings
    Sets->setValue("backgroundImage/CacheDir", Sets->value("backgroundImage/CacheDir", HOMEDIR + "/BackgroundCache"));
}

const QVector<qreal> MerkaartorPreferences::getParentDashes() const
{
    return parentDashes;
}

double MerkaartorPreferences::apiVersionNum() const
{
    if (Use06Api)
        return 0.6;
    else
        return 0.5;
}

const QString MerkaartorPreferences::apiVersion() const
{
    if (Use06Api)
        return "0.6";
    else
        return "0.5";
}

void MerkaartorPreferences::setUse06Api(bool b)
{
    // ATTENTION this does not update Use06API member on purpose to force a
    // restart before it takes effect. Mixing 0.5 api and 0.6 api in one session
    //  is dangerous!!!
    Sets->setValue("osm/use06api", b);
}

M_PARAM_IMPLEMENT_BOOL(rightsidedriving, roadstructure, true);
M_PARAM_IMPLEMENT_DOUBLE(doubleroaddistance, roadstructure, 20.);
M_PARAM_IMPLEMENT_STRING(workingdir, general, "");

BookmarkList* MerkaartorPreferences::getBookmarks()
{
    //return Sets->value("downloadosm/bookmarks").toStringList();
    return theBookmarkList.getBookmarks();
}

/* WMS */

WmsServerList* MerkaartorPreferences::getWmsServers()
{
//	return Sets->value("WSM/servers").toStringList();
    return theWmsServerList.getServers();
}

void MerkaartorPreferences::setWmsServers()
{
    //QStringList Servers;
    //WmsServerListIterator i(theWmsServerList);
    //while (i.hasNext()) {
    //	i.next();
    //	WmsServer S = i.value();
    //	Servers.append(S.WmsName);
    //	Servers.append(S.WmsAdress);
    //	Servers.append(S.WmsPath);
    //	Servers.append(S.WmsLayers);
    //	Servers.append(S.WmsProjections);
    //	Servers.append(S.WmsStyles);
    //	Servers.append(S.WmsImgFormat);
    //}
    //Sets->setValue("WSM/servers", Servers);
}

OsmServerList* MerkaartorPreferences::getOsmServers()
{
    return &theOsmServers;
}

/* TMS */

TmsServerList* MerkaartorPreferences::getTmsServers()
{
//	return Sets->value("WSM/servers").toStringList();
    return theTmsServerList.getServers();
}

void MerkaartorPreferences::setTmsServers()
{
    //QStringList Servers;
    //TmsServerListIterator i(theTmsServerList);
    //while (i.hasNext()) {
    //	i.next();
    //	TmsServer S = i.value();
    //	Servers.append(S.TmsName);
    //	Servers.append(S.TmsAdress);
    //	Servers.append(S.TmsPath);
    //	Servers.append(QString::number(S.TmsTileSize));
    //	Servers.append(QString::number(S.TmsMinZoom));
    //	Servers.append(QString::number(S.TmsMaxZoom));
    //}
    //Sets->setValue("TMS/servers", Servers);
}

/* */

M_PARAM_IMPLEMENT_STRING(SelectedServer, backgroundImage, "");

bool MerkaartorPreferences::getBgVisible() const
{
    if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences)
        return false;
    else
        return Sets->value("backgroundImage/Visible", false).toBool();
}

void MerkaartorPreferences::setBgVisible(bool theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("backgroundImage/Visible", theValue);
}

/* Plugins */

void MerkaartorPreferences::addBackgroundPlugin(IMapAdapterFactory* aPlugin)
{
    mBackgroundPlugins.insert(aPlugin->getId(), aPlugin);
}

IMapAdapterFactory* MerkaartorPreferences::getBackgroundPlugin(const QUuid& anAdapterUid)
{
    if (mBackgroundPlugins.contains(anAdapterUid))
        return mBackgroundPlugins[anAdapterUid];
    else
        return NULL;
}

void MerkaartorPreferences::setBackgroundPlugin(const QUuid & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("backgroundImage/BackgroundPlugin", theValue.toString());
}

QUuid MerkaartorPreferences::getBackgroundPlugin() const
{
    QString s;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        s = Sets->value("backgroundImage/BackgroundPlugin", "").toString();
    }
    return QUuid(s);
}

QMap<QUuid, IMapAdapterFactory *> MerkaartorPreferences::getBackgroundPlugins()
{
    return mBackgroundPlugins;
}

M_PARAM_IMPLEMENT_STRING(CacheDir, backgroundImage, HOMEDIR + "/BackgroundCache");
M_PARAM_IMPLEMENT_INT(CacheSize, backgroundImage, 0);

/* Search */
M_PARAM_IMPLEMENT_INT(LastMaxSearchResults, search, 100);
M_PARAM_IMPLEMENT_STRING(LastSearchName, search, "");
M_PARAM_IMPLEMENT_STRING(LastSearchKey, search, "");
M_PARAM_IMPLEMENT_STRING(LastSearchValue, search, "");
M_PARAM_IMPLEMENT_STRING(LastSearchTagSelector, search, "");
/* Visuals */

void MerkaartorPreferences::saveMainWindowState(const MainWindow * mainWindow)
{
    if (!g_Merk_Ignore_Preferences) {
        //    Sets->setValue("MainWindow/Position", mainWindow->pos());
        //    Sets->setValue("MainWindow/Size", mainWindow->size());
        Sets->setValue("MainWindow/Geometry", mainWindow->saveGeometry());
        Sets->setValue("MainWindow/State", mainWindow->saveState());
        Sets->setValue("MainWindow/Fullscreen", mainWindow->ui->windowShowAllAction->isEnabled());
        Sets->setValue("MainWindow/FullscreenState", mainWindow->fullscreenState);
    }
}

void MerkaartorPreferences::restoreMainWindowState(MainWindow * mainWindow) const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        //    if (Sets->contains("MainWindow/Position"))
        //        mainWindow->move( Sets->value("MainWindow/Position").toPoint());
        //
        //    if (Sets->contains("MainWindow/Size"))
        //        mainWindow->resize( Sets->value("MainWindow/Size").toSize());

        if (Sets->contains("MainWindow/Geometry"))
            mainWindow->restoreGeometry(Sets->value("MainWindow/Geometry").toByteArray() );

        if (Sets->contains("MainWindow/State"))
            mainWindow->restoreState( Sets->value("MainWindow/State").toByteArray() );

        if (Sets->contains("MainWindow/FullscreenState"))
            mainWindow->fullscreenState = Sets->value("MainWindow/FullscreenState").toByteArray();

        if (Sets->value("MainWindow/Fullscreen", false).toBool()) {
            mainWindow->ui->windowHideAllAction->setEnabled(false);
            mainWindow->ui->windowHideAllAction->setVisible(false);
            mainWindow->ui->windowShowAllAction->setEnabled(true);
            mainWindow->ui->windowShowAllAction->setVisible(true);
        } else {
            mainWindow->ui->windowHideAllAction->setEnabled(true);
            mainWindow->ui->windowHideAllAction->setVisible(true);
            mainWindow->ui->windowShowAllAction->setEnabled(false);
            mainWindow->ui->windowShowAllAction->setVisible(false);
        }
    }
}

void MerkaartorPreferences::setInitialPosition(MapView* vw)
{
    if (!g_Merk_Ignore_Preferences) {
        QStringList ip;
        CoordBox cb = vw->viewport();
        ip.append(QString::number(cb.bottomLeft().lat(), 'f', 8));
        ip.append(QString::number(cb.bottomLeft().lon(), 'f', 8));
        ip.append(QString::number(cb.topRight().lat(), 'f', 8));
        ip.append(QString::number(cb.topRight().lon(), 'f', 8));

        Sets->setValue("MainWindow/InitialPosition", ip);
        //    Sets->setValue("MainWindow/ViewRect", vw->rect());
    }
}

void MerkaartorPreferences::initialPosition(MapView* vw)
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        if (!Sets->contains("MainWindow/InitialPosition")) {
            vw->setViewport(WORLD_COORDBOX, vw->rect());
            return;
        }

        const QStringList & ip = Sets->value("MainWindow/InitialPosition").toStringList();

        const Coord bottomLeft(ip[0].toDouble(), ip[1].toDouble());
        const Coord topRight(ip[2].toDouble(),ip[3].toDouble());

        vw->setViewport(CoordBox(bottomLeft, topRight), vw->rect());
        //    if (!Sets->contains("MainWindow/ViewRect"))
        //        vw->setViewport(CoordBox(bottomLeft, topRight), vw->rect());
        //    else {
        //        QRect rt = Sets->value("MainWindow/ViewRect").toRect();
        //        vw->setViewport(CoordBox(bottomLeft, topRight), rt);
        //    }
    }
}

#ifndef _MOBILE
void MerkaartorPreferences::setProjectionType(QString theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("projection/Type", theValue);
}

QString MerkaartorPreferences::getProjectionType()
{
    //    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
    //        return Sets->value("projection/Type", "Mercator").toString();
    //    else
    return "EPSG:3857";
}

ProjectionsList* MerkaartorPreferences::getProjectionsList()
{
    return &theProjectionsList;
}

ProjectionItem MerkaartorPreferences::getProjection(QString aProj)
{
    return theProjectionsList.getProjection(aProj);
}
#endif

void MerkaartorPreferences::setCurrentFilter(FilterType theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("filter/Type", theValue);
}

QString MerkaartorPreferences::getCurrentFilter()
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("filter/Type", "").toString();
    else
        return "";
}

FiltersList* MerkaartorPreferences::getFiltersList()
{
    return &theFiltersList;
}

FilterItem MerkaartorPreferences::getFilter(QString aFilter)
{
    if (aFilter.isEmpty())
        return FilterItem();
    return theFiltersList.getFilter(aFilter);
}

qreal MerkaartorPreferences::getAlpha(QString lvl)
{
    return alpha[lvl];
}

QStringList MerkaartorPreferences::getAlphaList() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("visual/alpha").toStringList();
    else
        return QStringList();
}

void MerkaartorPreferences::setAlphaList()
{
    if (!g_Merk_Ignore_Preferences) {
        QStringList alphaList;
        QHashIterator<QString, qreal> i(alpha);
        while (i.hasNext()) {
            i.next();
            alphaList << i.key() << QString().setNum(i.value());
        }
        Sets->setValue("visual/alpha", alphaList);
    }
}

M_PARAM_IMPLEMENT_INT(HoverWidth, visual, 1);
M_PARAM_IMPLEMENT_INT(HighlightWidth, visual, 1);
M_PARAM_IMPLEMENT_INT(DirtyWidth, visual, 2);
M_PARAM_IMPLEMENT_INT(FocusWidth, visual, 3);
M_PARAM_IMPLEMENT_INT(RelationsWidth, visual, 3);
M_PARAM_IMPLEMENT_INT(GpxTrackWidth, visual, 3);

M_PARAM_IMPLEMENT_COLOR(BgColor, visual, Qt::white)
M_PARAM_IMPLEMENT_COLOR(WaterColor, visual, QColor(181, 208, 208))
M_PARAM_IMPLEMENT_COLOR(FocusColor, visual, Qt::blue);
M_PARAM_IMPLEMENT_COLOR(HoverColor, visual, Qt::magenta);
M_PARAM_IMPLEMENT_COLOR(HighlightColor, visual, Qt::darkCyan);
M_PARAM_IMPLEMENT_COLOR(DirtyColor, visual, QColor(255, 85, 0));
M_PARAM_IMPLEMENT_COLOR(RelationsColor, visual, QColor(0, 170, 0));
M_PARAM_IMPLEMENT_COLOR(GpxTrackColor, visual, QColor(50, 220, 220));

QHash< QString, qreal > * MerkaartorPreferences::getAlphaPtr()
{
    return &alpha;
}

bool MerkaartorPreferences::getDrawTileBoundary()
{
    return false;
}

/* DATA */

QString MerkaartorPreferences::getOsmWebsite() const
{
    QString s;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        s = Sets->value("osm/Website", "www.openstreetmap.org").toString();
    else
        s = "www.openstreetmap.org";

#if QT_VERSION >= 0x040600
    QUrl u = QUrl::fromUserInput(s);
#else
    // convenience for creating a valid URL
    // fails miserably if QString s already contains a schema
    QString h = s; // intermediate host
    QString p; // intermediate path

    int slashpos = s.indexOf('/');
    if (slashpos >= 1) // there's a path element in s
    {
        h = s.left(slashpos);
        p = s.right(s.size() - 1 - slashpos);
    }

    QUrl u;
    u.setHost(h);
    u.setScheme("http");
    u.setPath(p);
#endif
    if (u.path().isEmpty())
        u.setPath("/api/" + apiVersion());

    return u.toString();
}

void MerkaartorPreferences::setOsmWebsite(const QString & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("osm/Website", theValue);
}

M_PARAM_IMPLEMENT_STRING(XapiUrl, osm, "http://www.informationfreeway.org/api/0.6/")
M_PARAM_IMPLEMENT_STRING(NominatimUrl, osm, "http://nominatim.openstreetmap.org/search")
M_PARAM_IMPLEMENT_BOOL(AutoHistoryCleanup, data, true);

QString MerkaartorPreferences::getOsmUser() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("osm/User").toString();
    else
        return "";
}

void MerkaartorPreferences::setOsmUser(const QString & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("osm/User", theValue);
}

QString MerkaartorPreferences::getOsmPassword() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("osm/Password").toString();
    else
        return "";
}

void MerkaartorPreferences::setOsmPassword(const QString & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("osm/Password", theValue);
}

M_PARAM_IMPLEMENT_DOUBLE(MaxDistNodes, data, 0.0);

M_PARAM_IMPLEMENT_BOOL(AutoSaveDoc, data, false);
M_PARAM_IMPLEMENT_BOOL(AutoExtractTracks, data, false);

M_PARAM_IMPLEMENT_INT(DirectionalArrowsVisible, visual, 1);

RendererOptions MerkaartorPreferences::getRenderOptions()
{
    RendererOptions opt;

    if (getBackgroundVisible()) opt.options |= RendererOptions::BackgroundVisible; else opt.options &= ~RendererOptions::BackgroundVisible;
    if (getForegroundVisible()) opt.options |= RendererOptions::ForegroundVisible; else opt.options &= ~RendererOptions::ForegroundVisible;
    if (getTouchupVisible()) opt.options |= RendererOptions::TouchupVisible; else opt.options &= ~RendererOptions::TouchupVisible;
    if (getNamesVisible()) opt.options |= RendererOptions::NamesVisible; else opt.options &= ~RendererOptions::NamesVisible;
    if (getPhotosVisible()) opt.options |= RendererOptions::PhotosVisible; else opt.options &= ~RendererOptions::PhotosVisible;
    if (getVirtualNodesVisible()) opt.options |= RendererOptions::VirtualNodesVisible; else opt.options &= ~RendererOptions::VirtualNodesVisible;
    if (getTrackPointsVisible()) opt.options |= RendererOptions::NodesVisible; else opt.options &= ~RendererOptions::NodesVisible;
    if (getTrackSegmentsVisible()) opt.options |= RendererOptions::TrackSegmentVisible; else opt.options &= ~RendererOptions::TrackSegmentVisible;
    if (getRelationsVisible()) opt.options |= RendererOptions::RelationsVisible; else opt.options &= ~RendererOptions::RelationsVisible;
    if (getDownloadedVisible()) opt.options |= RendererOptions::DownloadedVisible; else opt.options &= ~RendererOptions::DownloadedVisible;
    if (getScaleVisible()) opt.options |= RendererOptions::ScaleVisible; else opt.options &= ~RendererOptions::ScaleVisible;
    if (getLatLonGridVisible()) opt.options |= RendererOptions::LatLonGridVisible; else opt.options &= ~RendererOptions::LatLonGridVisible;
    if (getZoomBoris()) opt.options |= RendererOptions::LockZoom; else opt.options &= ~RendererOptions::LockZoom;
    opt.arrowOptions &= getDirectionalArrowsVisible();

    return opt;
}

/* Export Type */
void MerkaartorPreferences::setExportType(ExportType theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("export/Type", theValue);
}

ExportType MerkaartorPreferences::getExportType() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return (ExportType)Sets->value("export/Type", 0).toInt();
    else
        return (ExportType)0;
}

/* Tools */
ToolList* MerkaartorPreferences::getTools() const
{
    return theToolList;
}

void MerkaartorPreferences::setTools()
{
    if (!g_Merk_Ignore_Preferences) {
        QStringList tl;
        ToolListIterator i(*theToolList);
        while (i.hasNext()) {
            i.next();
            Tool t = i.value();
            tl.append(t.ToolName);
            tl.append(t.ToolPath);
        }
        Sets->setValue("Tools/list", tl);
    }
}

Tool MerkaartorPreferences::getTool(QString toolName) const
{
    Tool ret;

    ToolListIterator i(*theToolList);
    while (i.hasNext()) {
        i.next();
        if (i.key() == toolName) {
            ret = i.value();
        }
    }
    return ret;
}

/* Recent */
QStringList MerkaartorPreferences::getRecentOpen() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("recent/open").toStringList();
    else
        return QStringList();
}

void MerkaartorPreferences::setRecentOpen(const QStringList & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("recent/open", theValue);
}

void MerkaartorPreferences::addRecentOpen(const QString & theValue)
{
    QStringList RecentOpen = getRecentOpen();
    int idx = RecentOpen.indexOf(theValue);
    if (idx  >= 0) {
        RecentOpen.move(idx, 0);
    } else {
        if (RecentOpen.size() == 4)
            RecentOpen.removeLast();

        RecentOpen.insert(0, theValue);
    }
    setRecentOpen(RecentOpen);
}

QStringList MerkaartorPreferences::getRecentImport() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("recent/import").toStringList();
    else
        return QStringList();
}

void MerkaartorPreferences::setRecentImport(const QStringList & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("recent/import", theValue);
}

void MerkaartorPreferences::addRecentImport(const QString & theValue)
{
    QStringList RecentImport = getRecentImport();
    int idx = RecentImport.indexOf(theValue);
    if (idx  >= 0) {
        RecentImport.move(idx, 0);
    } else {
        if (RecentImport.size() == 4)
            RecentImport.removeLast();

        RecentImport.insert(0, theValue);
    }
    setRecentImport(RecentImport);
}

QStringList MerkaartorPreferences::getShortcuts() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("Tools/shortcuts").toStringList();
    else
        return QStringList();
}

void MerkaartorPreferences::setShortcuts(const QStringList & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("Tools/shortcuts", theValue);
}

/* Styles */
M_PARAM_IMPLEMENT_STRING(DefaultStyle, style, ":/Styles/Mapnik.mas")
M_PARAM_IMPLEMENT_STRING(CustomStyle, style, "")
M_PARAM_IMPLEMENT_BOOL(DisableStyleForTracks, style, true)

/* Zoom */
M_PARAM_IMPLEMENT_INT(ZoomIn, zoom, 133)
M_PARAM_IMPLEMENT_INT(ZoomOut, zoom, 75)
M_PARAM_IMPLEMENT_BOOL(ZoomBoris, zoom, false)

/* Visual */
M_PARAM_IMPLEMENT_BOOL(BackgroundOverwriteStyle, visual, false)
M_PARAM_IMPLEMENT_INT(AreaOpacity, visual, 100)
M_PARAM_IMPLEMENT_BOOL(UseShapefileForBackground, visual, false)
M_PARAM_IMPLEMENT_BOOL(DrawingHack, visual, true)
M_PARAM_IMPLEMENT_BOOL(SimpleGpxTrack, visual, false)
M_PARAM_IMPLEMENT_BOOL(UseVirtualNodes, visual, true)
M_PARAM_IMPLEMENT_BOOL(RelationsSelectableWhenHidden, visual, true)
M_PARAM_IMPLEMENT_DOUBLE(LocalZoom, visual, 0.5)
M_PARAM_IMPLEMENT_DOUBLE(RegionalZoom, visual, 0.01)
M_PARAM_IMPLEMENT_INT(NodeSize, visual, 8)

M_PARAM_IMPLEMENT_BOOL(DownloadedVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(ScaleVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(LatLonGridVisible, visual, false)
M_PARAM_IMPLEMENT_BOOL(BackgroundVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(ForegroundVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(TouchupVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(NamesVisible, visual, false)
M_PARAM_IMPLEMENT_BOOL(TrackPointsVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(TrackSegmentsVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(RelationsVisible, visual, false)
M_PARAM_IMPLEMENT_BOOL(PhotosVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(VirtualNodesVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(DirtyVisible, visual, true)

/* Templates */
M_PARAM_IMPLEMENT_STRING(DefaultTemplate, templates, ":/Templates/default.mat")
M_PARAM_IMPLEMENT_STRING(CustomTemplate, templates, "")

/* GPS */
#ifdef Q_OS_WIN
M_PARAM_IMPLEMENT_BOOL(GpsUseGpsd, gps, false)
M_PARAM_IMPLEMENT_STRING(GpsPort, gps, "COM1")
#else
M_PARAM_IMPLEMENT_BOOL(GpsUseGpsd, gps, true)
M_PARAM_IMPLEMENT_STRING(GpsPort, gps, "/dev/rfcomm0")
#endif
M_PARAM_IMPLEMENT_STRING(GpsdHost, gps, "localhost")
M_PARAM_IMPLEMENT_INT(GpsdPort, gps, 2947)
M_PARAM_IMPLEMENT_BOOL(GpsSaveLog, gps, false)
M_PARAM_IMPLEMENT_BOOL(GpsMapCenter, gps, false)
M_PARAM_IMPLEMENT_STRING(GpsLogDir, gps, "")
M_PARAM_IMPLEMENT_BOOL(GpsSyncTime, gps, false)

M_PARAM_IMPLEMENT_BOOL(ResolveRelations, downloadosm, false)
M_PARAM_IMPLEMENT_BOOL(DeleteIncompleteRelations, downloadosm, false)

M_PARAM_IMPLEMENT_BOOL(MapTooltip, visual, false)
M_PARAM_IMPLEMENT_BOOL(InfoOnHover, visual, true)
M_PARAM_IMPLEMENT_BOOL(ShowParents, visual, true)

M_PARAM_IMPLEMENT_INT_DELAYED(TagListFirstColumnWidth, visual, 0)
M_PARAM_IMPLEMENT_BOOL(TranslateTags, locale, true)

/* World OSB manager */
M_PARAM_IMPLEMENT_DOUBLE(TileToRegionThreshold, WOSB, 0.03)
M_PARAM_IMPLEMENT_DOUBLE(RegionTo0Threshold, WOSB, 1.)

M_PARAM_IMPLEMENT_STRING(WorldOsbUri, WOSB, "")
M_PARAM_IMPLEMENT_BOOL(WorldOsbAutoload, WOSB, false)
M_PARAM_IMPLEMENT_BOOL(WorldOsbAutoshow, WOSB, false)

/* Background */
M_PARAM_IMPLEMENT_BOOL(AutoSourceTag, backgroundImage, true)

/* Data */
M_PARAM_IMPLEMENT_STRING(OpenStreetBugsUrl, data, "http://openstreetbugs.schokokeks.org/api/0.1/")
M_PARAM_IMPLEMENT_BOOL(HasAutoLoadDocument, data, false)
M_PARAM_IMPLEMENT_STRING(AutoLoadDocumentFilename, data, "")

/* Mouse bevaviour */
#ifdef _MOBILE
    M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, true)
#else
    M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, false)
#endif
M_PARAM_IMPLEMENT_BOOL(SeparateMoveMode, Mouse, true)
M_PARAM_IMPLEMENT_BOOL(SelectModeCreation, Mouse, false)

// Geotag
M_PARAM_IMPLEMENT_INT(MaxGeoPicWidth, geotag, 160)

/* Custom Style */
M_PARAM_IMPLEMENT_BOOL(MerkaartorStyle, visual, false)
M_PARAM_IMPLEMENT_STRING(MerkaartorStyleString, visual, "skulpture")

/* Network */
M_PARAM_IMPLEMENT_BOOL(OfflineMode, Network, false)
M_PARAM_IMPLEMENT_BOOL(LocalServer, Network, false)
M_PARAM_IMPLEMENT_INT(NetworkTimeout, Network, 10000)

/* Proxy */

QNetworkProxy MerkaartorPreferences::getProxy(const QUrl & requestUrl)
{
    QNetworkProxy theProxy;

    if ( getProxyUse() )
    {
        theProxy.setType(QNetworkProxy::HttpProxy);
        theProxy.setHostName(getProxyHost());
        theProxy.setPort(getProxyPort());
        theProxy.setUser(getProxyUser());
        theProxy.setPassword(getProxyPassword());
    }
    else
    {
#ifdef USE_LIBPROXY
        // Ask libproxy for the system proxy
        if (proxyFactory) {
            // get proxy URL(s) from libproxy, see http://code.google.com/p/libproxy/wiki/HowTo
            char **proxies = px_proxy_factory_get_proxies(proxyFactory, requestUrl.toString().toUtf8().data());

            // Iterate through the list until we find a proxy scheme QNetworkProxy supports
            for (int i=0 ; proxies[i] ; i++) {
                QUrl proxyUrl(proxies[i]);
                if (proxyTypeMap.contains(proxyUrl.scheme())) {
                    theProxy.setType(proxyTypeMap.value(proxyUrl.scheme()));
                    theProxy.setHostName(proxyUrl.host());
                    theProxy.setPort(proxyUrl.port());
                    theProxy.setUser(proxyUrl.userName());
                    theProxy.setPassword(proxyUrl.password());
                    //qDebug() << "Using proxy " << proxyUrl << " from libproxy for " << requestUrl;
                }
            }
            for (int i=0 ; proxies[i] ; i++) {
                free(proxies[i]);
            }
            return theProxy;
        }
#endif
#if QT_VERSION >= 0x040500
        // Ask Qt for the system proxy (Qt >= 4.5.0), libproxy is preferred if available since QNetworkProxyFactory
        // doesn't yet support auto-config (PAC) on MacOS or system settings on linux while libproxy does
        QList<QNetworkProxy> systemProxies = QNetworkProxyFactory::systemProxyForQuery(
            QNetworkProxyQuery(requestUrl, QNetworkProxyQuery::UrlRequest)
        );
        return systemProxies[0];
#else
        // Otherwise no proxy
        theProxy.setType(QNetworkProxy::NoProxy);
#endif
    }

    return theProxy;
}

M_PARAM_IMPLEMENT_BOOL(ProxyUse, proxy, false)
M_PARAM_IMPLEMENT_STRING(ProxyHost, proxy, "")
M_PARAM_IMPLEMENT_INT(ProxyPort, proxy, 8080)
M_PARAM_IMPLEMENT_STRING(ProxyUser, proxy, "")
M_PARAM_IMPLEMENT_STRING(ProxyPassword, proxy, "")

/* Track */
M_PARAM_IMPLEMENT_BOOL(ReadonlyTracksDefault, data, false)

/* FeaturesDock */
M_PARAM_IMPLEMENT_BOOL(FeaturesWithin, FeaturesDock, true)

/* Projections */
void MerkaartorPreferences::loadProjection(QString fn)
{
    if (QDir::isRelativePath(fn))
        fn = QCoreApplication::applicationDirPath() + "/" + fn;

    qDebug() << "loadProjection " << fn;
    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
        return;
    }

    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc->documentElement();
    ProjectionsList aProjList = ProjectionsList::fromXml(docElem.firstChildElement());
    theProjectionsList.add(aProjList);

    delete theXmlDoc;
}

void MerkaartorPreferences::loadProjections()
{
    QString fn;

    fn = HOMEDIR + "/Projections.xml";
    loadProjection(fn);

#if defined(Q_OS_MAC)
    {
        QDir resources = QDir(QCoreApplication::applicationDirPath());
        resources.cdUp();
        resources.cd("Resources");
        fn = resources.absolutePath() + "/Projections.xml";
    }
#else
    fn = QString(SHAREDIR) + "/Projections.xml";
#endif
    loadProjection(fn);

    fn = ":/Projections.xml";
    loadProjection(fn);
}

void MerkaartorPreferences::saveProjections()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorList");
    theXmlDoc.appendChild(root);
    theProjectionsList.toXml(root);

    QFile file(HOMEDIR + "/Projections.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //QMessageBox::critical(this, tr("Unable to open save projections file"), tr("%1 could not be opened for writing.").arg(HOMEDIR + "/Projections.xml"));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

/* Filters */
void MerkaartorPreferences::loadFilter(QString fn)
{
    if (QDir::isRelativePath(fn))
        fn = QCoreApplication::applicationDirPath() + "/" + fn;

    qDebug() << "loadFilter " << fn;
    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc->documentElement();
    FiltersList aFilterList = FiltersList::fromXml(docElem.firstChildElement());
    theFiltersList.add(aFilterList);

    delete theXmlDoc;
}

void MerkaartorPreferences::loadFilters()
{
    QString fn;

    fn = HOMEDIR + "/Filters.xml";
    loadFilter(fn);

#if defined(Q_OS_MAC)
    {
        QDir resources = QDir(QCoreApplication::applicationDirPath());
        resources.cdUp();
        resources.cd("Resources");
        fn = resources.absolutePath() + "/Filters.xml";
    }
#else
    fn = QString(SHAREDIR) + "/Filters.xml";
#endif
    loadFilter(fn);

    fn = ":/Filters.xml";
    loadFilter(fn);
}

void MerkaartorPreferences::saveFilters()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorList");
    theXmlDoc.appendChild(root);
    theFiltersList.toXml(root);

    QFile file(HOMEDIR + "/Filters.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}


/* WMS Servers */
void MerkaartorPreferences::loadWMS(QString fn)
{
    if (QDir::isRelativePath(fn))
        fn = QCoreApplication::applicationDirPath() + "/" + fn;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
        return;
    }

    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc->documentElement();
    WmsServersList aWmsList = WmsServersList::fromXml(docElem.firstChildElement());
    theWmsServerList.add(aWmsList);

    delete theXmlDoc;
}

void MerkaartorPreferences::loadWMSes()
{
    QString fn;

    fn = HOMEDIR + "/WmsServersList.xml";
    loadWMS(fn);

    fn = QString(SHAREDIR) + "/WmsServersList.xml";
    loadWMS(fn);

    fn = ":/WmsServersList.xml";
    loadWMS(fn);
}

void MerkaartorPreferences::saveWMSes()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorList");
    theXmlDoc.appendChild(root);
    theWmsServerList.toXml(root);

    QFile file(HOMEDIR + "/WmsServersList.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //QMessageBox::critical(this, tr("Unable to open save projections file"), tr("%1 could not be opened for writing.").arg(HOMEDIR + "/Projections.xml"));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

/* TMS Servers */
void MerkaartorPreferences::loadTMS(QString fn)
{
    if (QDir::isRelativePath(fn))
        fn = QCoreApplication::applicationDirPath() + "/" + fn;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
        return;
    }

    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc->documentElement();
    TmsServersList aTmsList = TmsServersList::fromXml(docElem.firstChildElement());
    theTmsServerList.add(aTmsList);

    delete theXmlDoc;
}

void MerkaartorPreferences::loadTMSes()
{
    QString fn;

    fn = HOMEDIR + "/TmsServersList.xml";
    loadTMS(fn);

    fn = QString(SHAREDIR) + "/TmsServersList.xml";
    loadTMS(fn);

    fn = ":/TmsServersList.xml";
    loadTMS(fn);
}

void MerkaartorPreferences::saveTMSes()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorList");
    theXmlDoc.appendChild(root);
    theTmsServerList.toXml(root);

    QFile file(HOMEDIR + "/TmsServersList.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //QMessageBox::critical(this, tr("Unable to open save projections file"), tr("%1 could not be opened for writing.").arg(HOMEDIR + "/Projections.xml"));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

/* Bookmarks */
void MerkaartorPreferences::loadBookmark(QString fn)
{
    if (QDir::isRelativePath(fn))
        fn = QCoreApplication::applicationDirPath() + "/" + fn;

    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
        return;
    }

    QDomDocument* theXmlDoc = new QDomDocument();
    if (!theXmlDoc->setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc->documentElement();
    BookmarksList aBkList = BookmarksList::fromXml(docElem.firstChildElement());
    theBookmarkList.add(aBkList);

    delete theXmlDoc;
}

void MerkaartorPreferences::loadBookmarks()
{
    QString fn;

    fn = HOMEDIR + "/BookmarksList.xml";
    loadBookmark(fn);

    fn = QString(SHAREDIR) + "/BookmarksList.xml";
    loadBookmark(fn);

    fn = ":/BookmarksList.xml";
    loadBookmark(fn);
}

void MerkaartorPreferences::saveBookmarks()
{
    QDomDocument theXmlDoc;

    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    QDomElement root = theXmlDoc.createElement("MerkaartorList");
    theXmlDoc.appendChild(root);
    theBookmarkList.toXml(root);

    QFile file(HOMEDIR + "/BookmarksList.xml");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        //QMessageBox::critical(this, tr("Unable to open save bookmarks file"), tr("%1 could not be opened for writing.").arg(HOMEDIR + "/BookmarksList.xml"));
        return;
    }
    file.write(theXmlDoc.toString().toUtf8());
    file.close();
}

/* OSM Servers */

void MerkaartorPreferences::loadOsmServers()
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        int size = Sets->beginReadArray("OsmServers");
        for (int i = 0; i < size; ++i) {
            Sets->setArrayIndex(i);
            OsmServer server;
            server.Selected = Sets->value("selected").toBool();
            server.Url = Sets->value("url").toString();
            server.User = Sets->value("user").toString();
            server.Password = Sets->value("password").toString();
            theOsmServers.append(server);
        }
        Sets->endArray();
    }
}

void MerkaartorPreferences::saveOsmServers()
{
    if (!g_Merk_Ignore_Preferences) {
        Sets->beginWriteArray("OsmServers");
        for (int i = 0; i < theOsmServers.size(); ++i) {
            Sets->setArrayIndex(i);
            Sets->setValue("selected", theOsmServers.at(i).Selected);
            Sets->setValue("url", theOsmServers.at(i).Url);
            Sets->setValue("user", theOsmServers.at(i).User);
            Sets->setValue("password", theOsmServers.at(i).Password);
        }
        Sets->endArray();
    }
}


/* */

QString getDefaultLanguage()
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        QSettings Sets;
        QString lang = Sets.value("locale/language").toString();
        if (lang == "")
            lang = QLocale::system().name().split("_")[0];
        return lang;
    } else
        return QLocale::system().name().split("_")[0];
}

void setDefaultLanguage(const QString& theValue)
{
    if (!g_Merk_Ignore_Preferences) {
        QSettings Sets;
        Sets.setValue("locale/language", theValue);
    }
}
