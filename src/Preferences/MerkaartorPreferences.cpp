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
#include "IFeature.h"
#include "MerkaartorPreferences.h"

#include <QApplication>
#include <QByteArray>
#include <QMessageBox>


#include "MainWindow.h"
#ifndef _MOBILE
#include <ui_MainWindow.h>
#endif
#include "MapView.h"

#include "IMapAdapterFactory.h"
#include "IPaintStyle.h"
#include "MasPaintStyle.h"


// TODO: Replace 'g_Merk_Ignore_Preferences' by having two implementations
// of the "preferences API": one that behaves as if
// g_Merk_Ignore_Preferences == false, and one that ignores writes
// and always returns default settings.

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
    void MerkaartorPreferences::set##Param(const QString& theValue) \
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

#define M_PARAM_IMPLEMENT_STRINGLIST(Param, Category, Default) \
    bool mb_##Param = false; \
    void MerkaartorPreferences::set##Param(const QStringList& theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    QStringList& MerkaartorPreferences::get##Param() \
    { \
        if (!::mb_##Param) { \
            ::mb_##Param = true; \
            if (g_Merk_Ignore_Preferences || g_Merk_Reset_Preferences) \
                m_##Param = QString(Default).split("#"); \
            else \
                m_##Param = Sets->value(#Category"/"#Param, QVariant(QString(Default).split("#"))).toStringList(); \
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
    void MerkaartorPreferences::set##Param(qreal theValue) \
    { \
        m_##Param = theValue; \
        if (!g_Merk_Ignore_Preferences) { \
            Sets->setValue(#Category"/"#Param, theValue); \
        } \
    } \
    qreal MerkaartorPreferences::get##Param() \
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
    void MerkaartorPreferences::set##Param(const QColor& theValue) \
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
                QString sColor = Sets->value(#Category"/"#Param, QString()).toString(); \
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
{
}

/* MekaartorPreferences */

namespace {

QSettings* getSettings() {
    if (!g_Merk_Portable) {
        return new QSettings();
    } else {
        return new QSettings(qApp->applicationDirPath() + "/merkaartor.ini", QSettings::IniFormat);
    }
}

}  // namespace

MerkaartorPreferences::MerkaartorPreferences()
{
    if (!g_Merk_Ignore_Preferences) {
        Sets = getSettings();

        QSettings oldSettings("BartVanhauwaert", "Merkaartor");
        QStringList oldKeys = oldSettings.allKeys();
        foreach(QString k, oldKeys) {
            Sets->setValue(k, oldSettings.value(k));
            Sets->sync();
            oldSettings.remove(k);
        }
        oldSettings.clear();
        version = Sets->value("version/version", "0").toString();
    }

    connect(&httpRequest, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), this, SLOT(on_authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(&httpRequest, SIGNAL(finished(QNetworkReply*)),this,SLOT(on_requestFinished(QNetworkReply*)));
    connect(&httpRequest, SIGNAL(sslErrors(QNetworkReply *, const QList<QSslError>&)), this, SLOT(on_sslErrors(QNetworkReply*, const QList<QSslError>&)));

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
    delete Sets;
#ifdef USE_LIBPROXY
    px_proxy_factory_free(proxyFactory);
#endif
}

void MerkaartorPreferences::save(bool UserPwdChanged)
{
    if (g_Merk_Ignore_Preferences || !saveOnline)
        return;

    Sets->setValue("version/version", QString("%1").arg(STRINGIFY(VERSION)));
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

    /* If OSM login info has been changed, it might be a good idea to load new
     * preferences from that user account. */
    if (UserPwdChanged)
        fromOsmPref();

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

    // TODO: Why is it required to load from PrefsXML in chunk of 254?
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

    qDebug() << "Requesting preferences from OSM server.";

    if (getOsmUser().isEmpty() || getOsmPassword().isEmpty()) return;

    QUrl osmWeb(getOsmApiUrl()+"/user/preferences");

    QNetworkRequest req(osmWeb);
    req.setRawHeader(QByteArray("User-Agent"), USER_AGENT.toLatin1());

    httpRequest.setProxy(getProxy(osmWeb));
    OsmPrefLoadReply = httpRequest.get(req);
}

void MerkaartorPreferences::on_authenticationRequired( QNetworkReply *reply, QAuthenticator *auth ) {
    static QNetworkReply *lastReply = NULL;

    /* Only provide authentication the first time we see this reply, to avoid
     * infinite loop providing the same credentials. */
    if (lastReply != reply) {
        lastReply = reply;
        qDebug() << "Authentication required and provided.";
        auth->setUser(getOsmUser());
        auth->setPassword(getOsmPassword());
    }
}

void MerkaartorPreferences::on_sslErrors(QNetworkReply *reply, const QList<QSslError>& errors) {
    Q_UNUSED(reply);
    qDebug() << "We stumbled upon some SSL errors: ";
    foreach ( QSslError error, errors ) {
        qDebug() << "1:";
        qDebug() << error.errorString();
    }
}

void MerkaartorPreferences::on_requestFinished ( QNetworkReply *reply )
{
    int error = reply->error();
    if (error != QNetworkReply::NoError) {
        //qDebug() << "Received response with code " << error << "(" << reply->errorString() << ")";
        switch (error) {
            case QNetworkReply::HostNotFoundError:
                qWarning() << "MerkaartorPreferences: Host not found, preferences won't be synchronized with your profile.";
                /* We don't want to save local changes online, and possibly corrupt the store */
                saveOnline = false;
                break;
            case 406:
                QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","Duplicate key"));
                return;
            case 413:
                QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","More than 150 preferences"));
                return;
            default:
                QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences communication failed"), QApplication::translate("MerkaartorPreferences", "Communication error")+":\n"+reply->errorString());
                return;
        }
    }

    if (reply != OsmPrefLoadReply)
        return;

    qDebug() << "Reading preferences from online profile.";

    QDomDocument aOsmPrefDoc;
    aOsmPrefDoc.setContent(reply, false);

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
        PrefsXML.append(slicedPrefs[i].toLatin1());

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

    reply->deleteLater();
}


void MerkaartorPreferences::putOsmPref(const QString& k, const QString& v)
{
    qDebug() << "Saving OSM preference online: " << k << "=" << v;
    QUrl osmWeb(getOsmApiUrl()+QString("/user/preferences/%1").arg(k));

    QByteArray ba(v.toUtf8());
    QBuffer Buf(&ba);

    QNetworkRequest req(osmWeb);

    httpRequest.setProxy(getProxy(osmWeb));
    OsmPrefSaveReply = httpRequest.put(req, ba);
}

void MerkaartorPreferences::deleteOsmPref(const QString& k)
{
    qDebug() << "Deleting OSM preference online: " << k;

    QUrl osmWeb(getOsmApiUrl()+QString("/user/preferences/%1").arg(k));

    QNetworkRequest req(osmWeb);

    httpRequest.setProxy(getProxy(osmWeb));
    OsmPrefSaveReply = httpRequest.sendCustomRequest(req,"DELETE");
}

void MerkaartorPreferences::initialize()
{
//  Use06Api = Sets->value("osm/use06api", "true").toBool();
    Use06Api = true;
    saveOnline = true;

    // Proxy upgrade
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
                Bookmark B(sl[i], CoordBox(Coord(sl[i+2].toDouble(),sl[i+1].toDouble()),
                                           Coord(sl[i+4].toDouble(),sl[i+3].toDouble())));
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
            theToolList.insert(tl[i], t);
        }
    }
    if (!theToolList.contains("Inkscape")) {
        Tool t("Inkscape", QString());
        theToolList.insert("Inkscape", t);
    }

    QStringList Servers;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        Servers = Sets->value("WSM/servers").toStringList();
	// TODO: Apparently WMS/servers is a list, and every 7 consecutive
	// items describe a single server. There should be some documentation
	// about what do the fields mean. Same with TMS/servers.
        if (Servers.size()) {
            for (int i=0; i<Servers.size(); i+=7) {
                WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6], QString(), QString());
                theWmsServerList.addServer(S);
            }
            save();
            Sets->remove("WSM/servers");
        }

        Servers = Sets->value("TMS/servers").toStringList();
        if (Servers.size()) {
            for (int i=0; i<Servers.size(); i+=6) {
                TmsServer S(Servers[i], Servers[i+1], Servers[i+2], "EPSG:900913", Servers[i+3].toInt(), Servers[i+4].toInt(), Servers[i+5].toInt(), QString(), QString());
                theTmsServerList.addServer(S);
            }
            save();
            Sets->remove("TMS/servers");
        }
    }

    parentDashes << 1 << 5;

    //Ensure we have a CacheDir value in QSettings
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("backgroundImage/CacheDir", Sets->value("backgroundImage/CacheDir", HOMEDIR + "/BackgroundCache"));
}

const QVector<qreal> MerkaartorPreferences::getParentDashes() const
{
    return parentDashes;
}

qreal MerkaartorPreferences::apiVersionNum() const
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
M_PARAM_IMPLEMENT_DOUBLE(RoundaboutPrecision, roadstructure, 10.);
M_PARAM_IMPLEMENT_INT(RoundaboutType, misc, 0);
M_PARAM_IMPLEMENT_STRING(workingdir, general, QString());

BookmarkList* MerkaartorPreferences::getBookmarks()
{
    //return Sets->value("downloadosm/bookmarks").toStringList();
    return theBookmarkList.getBookmarks();
}

/* WMS */

WmsServerList* MerkaartorPreferences::getWmsServers()
{
//  return Sets->value("WSM/servers").toStringList();
    return theWmsServerList.getServers();
}

OsmServerList* MerkaartorPreferences::getOsmServers()
{
    return &theOsmServers;
}

/* TMS */

TmsServerList* MerkaartorPreferences::getTmsServers()
{
//  return Sets->value("WSM/servers").toStringList();
    return theTmsServerList.getServers();
}

/* */

M_PARAM_IMPLEMENT_STRING(SelectedServer, backgroundImage, QString());

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
        s = Sets->value("backgroundImage/BackgroundPlugin", QString()).toString();
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
M_PARAM_IMPLEMENT_INT(LastMaxSearchResults, search, 999);
M_PARAM_IMPLEMENT_STRING(LastSearchName, search, QString());
M_PARAM_IMPLEMENT_STRING(LastSearchKey, search, QString());
M_PARAM_IMPLEMENT_STRING(LastSearchValue, search, QString());
M_PARAM_IMPLEMENT_STRING(LastSearchTagSelector, search, QString());
/* Visuals */

void MerkaartorPreferences::saveMainWindowState(const MainWindow * mainWindow)
{
#ifndef _MOBILE

    if (!g_Merk_Ignore_Preferences) {
        //    Sets->setValue("MainWindow/Position", mainWindow->pos());
        //    Sets->setValue("MainWindow/Size", mainWindow->size());
        Sets->setValue("MainWindow/Geometry", mainWindow->saveGeometry());
        Sets->setValue("MainWindow/State", mainWindow->saveState());
        Sets->setValue("MainWindow/Fullscreen", mainWindow->ui->windowShowAllAction->isEnabled());
        Sets->setValue("MainWindow/FullscreenState", mainWindow->fullscreenState);
    }
#endif
}

void MerkaartorPreferences::restoreMainWindowState(MainWindow * mainWindow) const
{
#ifndef _MOBILE
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
#endif
}

void MerkaartorPreferences::setInitialPosition(MapView* vw)
{
    if (!g_Merk_Ignore_Preferences) {
        QStringList ip;
        CoordBox cb = vw->viewport();
        ip.append(QString::number(cb.bottomLeft().y(), 'f', 8));
        ip.append(QString::number(cb.bottomLeft().x(), 'f', 8));
        ip.append(QString::number(cb.topRight().y(), 'f', 8));
        ip.append(QString::number(cb.topRight().x(), 'f', 8));

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

QString MerkaartorPreferences::getCurrentFilter()
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("filter/Type", QString()).toString();
    else
        return QString();
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

M_PARAM_IMPLEMENT_BOOL(HideToolbarLabels, interface, false)

/* DATA */

QString MerkaartorPreferences::getOsmWebsite() const
{
    QString s;
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        s = Sets->value("osm/Website", "www.openstreetmap.org").toString();
    else
        s = "www.openstreetmap.org";

#if QT_VERSION >= 0x040600 && !defined(FORCE_46)
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

    if (!u.path().isEmpty())
        u.setPath(QString());

    return u.toString();
}


QString MerkaartorPreferences::getOsmApiUrl() const
{
    QUrl u(getOsmWebsite());
    if (u.path().isEmpty())
        u.setPath("/api/" + apiVersion());

    return u.toString();
}

void MerkaartorPreferences::setOsmWebsite(const QString & theValue)
{
    if (!g_Merk_Ignore_Preferences)
        Sets->setValue("osm/Website", theValue);
}

M_PARAM_IMPLEMENT_STRING(XapiUrl, osm, "http://www.overpass-api.de/api/xapi_meta?")
M_PARAM_IMPLEMENT_STRING(NominatimUrl, osm, "http://nominatim.openstreetmap.org/search")
M_PARAM_IMPLEMENT_BOOL(AutoHistoryCleanup, data, true);

QString MerkaartorPreferences::getOsmUser() const
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
        return Sets->value("osm/User").toString();
    else
        return QString();
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
        return QString();
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
ToolList* MerkaartorPreferences::getTools()
{
    return &theToolList;
}

void MerkaartorPreferences::setTools()
{
    if (!g_Merk_Ignore_Preferences) {
        QStringList tl;
        ToolListIterator i(theToolList);
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

    ToolListIterator i(theToolList);
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

M_PARAM_IMPLEMENT_INT(PolygonSides, Tools, 3)

/* Rendering */
M_PARAM_IMPLEMENT_BOOL(UseAntiAlias, style, true)
M_PARAM_IMPLEMENT_BOOL(AntiAliasWhilePanning, style, false)
M_PARAM_IMPLEMENT_BOOL(UseStyledWireframe, style, false)
M_PARAM_IMPLEMENT_STRING(DefaultStyle, style, ":/Styles/Mapnik.mas")
M_PARAM_IMPLEMENT_STRING(CustomStyle, style, QString())
M_PARAM_IMPLEMENT_BOOL(DisableStyleForTracks, style, true)
M_PARAM_IMPLEMENT_STRINGLIST(TechnicalTags, style, TECHNICAL_TAGS)
M_PARAM_IMPLEMENT_INT(EditRendering, style, 0)

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
M_PARAM_IMPLEMENT_BOOL(WireframeView, visual, false)

/* Templates */
M_PARAM_IMPLEMENT_STRING(DefaultTemplate, templates, ":/Templates/default.mat")
M_PARAM_IMPLEMENT_STRING(CustomTemplate, templates, QString())

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
M_PARAM_IMPLEMENT_STRING(GpsLogDir, gps, QString())
M_PARAM_IMPLEMENT_BOOL(GpsSyncTime, gps, false)

M_PARAM_IMPLEMENT_BOOL(ResolveRelations, downloadosm, false)
M_PARAM_IMPLEMENT_BOOL(DeleteIncompleteRelations, downloadosm, false)

M_PARAM_IMPLEMENT_BOOL(MapTooltip, visual, false)
M_PARAM_IMPLEMENT_BOOL(InfoOnHover, visual, true)
M_PARAM_IMPLEMENT_BOOL(ShowParents, visual, true)

M_PARAM_IMPLEMENT_INT_DELAYED(TagListFirstColumnWidth, visual, 20)
M_PARAM_IMPLEMENT_BOOL(TranslateTags, locale, true)

/* Background */
M_PARAM_IMPLEMENT_BOOL(AutoSourceTag, backgroundImage, false)

/* Data */
M_PARAM_IMPLEMENT_STRING(MapdustUrl, data, "http://www.mapdust.com/feed?lang=en&ft=wrong_turn,bad_routing,oneway_road,blocked_street,missing_street,wrong_roundabout,missing_speedlimit,other&fd=1&minR=&maxR=")
M_PARAM_IMPLEMENT_BOOL(GdalConfirmProjection, data, true)
M_PARAM_IMPLEMENT_BOOL(HasAutoLoadDocument, data, false)
M_PARAM_IMPLEMENT_STRING(AutoLoadDocumentFilename, data, QString())

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
        return QNetworkProxy(QNetworkProxy::HttpProxy, getProxyHost(), getProxyPort(), getProxyUser(), getProxyPassword());
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
M_PARAM_IMPLEMENT_STRING(ProxyHost, proxy, QString())
M_PARAM_IMPLEMENT_INT(ProxyPort, proxy, 8080)
M_PARAM_IMPLEMENT_STRING(ProxyUser, proxy, QString())
M_PARAM_IMPLEMENT_STRING(ProxyPassword, proxy, QString())

/* Track */
M_PARAM_IMPLEMENT_BOOL(ReadonlyTracksDefault, data, false)

/* FeaturesDock */
M_PARAM_IMPLEMENT_BOOL(FeaturesWithin, FeaturesDock, true)
M_PARAM_IMPLEMENT_BOOL(FeaturesSelectionFilter, FeaturesDock, true)

namespace {

// Preference XMLs may be stored in several directories depending
// on the platform. This method returns the list of directories to load
// preference XMLs from.
QStringList getPreferenceDirectories() {
    QStringList directories;
    directories << HOMEDIR;
    // TODO: Some files are loaded without this override for Q_OS_MAC. Why?
#if defined(Q_OS_MAC)
    {
        QDir resources = QDir(QCoreApplication::applicationDirPath());
        resources.cdUp();
        resources.cd("Resources");
        directories << resources.absolutePath();
    }
#else
    directories << QString(SHAREDIR);
#endif
    directories << ":";
    return directories;
}

// Returns the list of all alternative locations of the given preference
// file.
QStringList getPreferenceFilePaths(QString fileName) {
    QStringList paths;
    const QStringList directories = getPreferenceDirectories();
    for (QStringList::const_iterator i = directories.begin(); i != directories.end(); ++i) {
	paths << (*i) + "/" + fileName;
    }
    return paths;
}

}  // namespace

/* Projections */
void MerkaartorPreferences::loadProjectionsFromFile(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QCoreApplication::applicationDirPath() + "/" + fileName;

    qDebug() << "loadProjection " << fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
//      QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fileName));
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fileName));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    ProjectionsList aProjList = ProjectionsList::fromXml(docElem.firstChildElement());
    theProjectionsList.add(aProjList);
}

void MerkaartorPreferences::loadProjections()
{
    const QStringList paths = getPreferenceFilePaths("Projections.xml");
    for (QStringList::const_iterator i = paths.begin(); i != paths.end(); ++i) {
	loadProjectionsFromFile(*i);
    }
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
void MerkaartorPreferences::loadFiltersFromFile(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QCoreApplication::applicationDirPath() + "/" + fileName;

    qDebug() << "loadFiltersFromFile " << fileName;
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    FiltersList aFilterList = FiltersList::fromXml(docElem.firstChildElement());
    theFiltersList.add(aFilterList);
}

void MerkaartorPreferences::loadFilters()
{
    const QStringList paths = getPreferenceFilePaths("Filters.xml");
    for (QStringList::const_iterator i = paths.begin(); i != paths.end(); ++i) {
        loadFiltersFromFile(*i);
    }
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
void MerkaartorPreferences::loadWMSesFromFile(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QCoreApplication::applicationDirPath() + "/" + fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
//      QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fileName));
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fileName));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    WmsServersList aWmsList = WmsServersList::fromXml(docElem.firstChildElement());
    theWmsServerList.add(aWmsList);
}

void MerkaartorPreferences::loadWMSes()
{
    loadWMSesFromFile(HOMEDIR + "/WmsServersList.xml");
    // TODO: Why is the Q_OS_MAC override in getPreferenceDirectories()
    // missing here? Is that a bug, or an intention?
    loadWMSesFromFile(QString(SHAREDIR) + "/WmsServersList.xml");
    loadWMSesFromFile(":/WmsServersList.xml");
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
void MerkaartorPreferences::loadTMSesFromFile(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QCoreApplication::applicationDirPath() + "/" + fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
//      QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fileName));
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fileName));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    TmsServersList aTmsList = TmsServersList::fromXml(docElem.firstChildElement());
    theTmsServerList.add(aTmsList);
}

void MerkaartorPreferences::loadTMSes()
{
    loadTMSesFromFile(HOMEDIR + "/TmsServersList.xml");
    // TODO: Why is the Q_OS_MAC override in getPreferenceDirectories()
    // missing here? Is that a bug, or an intention?
    loadTMSesFromFile(QString(SHAREDIR) + "/TmsServersList.xml");
    loadTMSesFromFile(":/TmsServersList.xml");
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
void MerkaartorPreferences::loadBookmarksFromFile(QString fileName)
{
    if (QDir::isRelativePath(fileName))
        fileName = QCoreApplication::applicationDirPath() + "/" + fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
//      QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fileName));
        return;
    }

    QDomDocument theXmlDoc;
    if (!theXmlDoc.setContent(&file)) {
//		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fileName));
        file.close();
        return;
    }
    file.close();

    QDomElement docElem = theXmlDoc.documentElement();
    BookmarksList aBkList = BookmarksList::fromXml(docElem.firstChildElement());
    theBookmarkList.add(aBkList);
}

void MerkaartorPreferences::loadBookmarks()
{
    loadBookmarksFromFile(HOMEDIR + "/BookmarksList.xml");
    // TODO: Why is the Q_OS_MAC override in getPreferenceDirectories()
    // missing here? Is that a bug, or an intention?
    loadBookmarksFromFile(QString(SHAREDIR) + "/BookmarksList.xml");
    loadBookmarksFromFile(":/BookmarksList.xml");
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

QString getDefaultLanguage(bool returnDefault)
{
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences) {
        QSettings* sets = getSettings();
        QString lang = sets->value("locale/language").toString();
        delete sets;
        if (lang.isEmpty())
            if (returnDefault)
                lang = QLocale::system().name().split("_")[0];
        return lang;
    } else {
        if (returnDefault)
            return QLocale::system().name().split("_")[0];
        else
            return QString();
    }
}

void setDefaultLanguage(const QString& theValue)
{
    if (!g_Merk_Ignore_Preferences) {
        QSettings* sets = getSettings();
        sets->setValue("locale/language", theValue);
        // TODO: 'sets' memory leak?
    }
}
