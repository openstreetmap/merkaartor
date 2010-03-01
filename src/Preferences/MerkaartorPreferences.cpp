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

#include "MerkaartorPreferences.h"

#include <QApplication>
#include <QByteArray>
#include <QMessageBox>


#include "MainWindow.h"
#include <ui_MainWindow.h>
#include "MapView.h"

#include "IMapAdapter.h"

#define M_PARAM_IMPLEMENT_BOOL(Param, Category, Default) \
	bool mb_##Param = false; \
	void MerkaartorPreferences::set##Param(bool theValue) \
	{ \
		m_##Param = theValue; \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	bool MerkaartorPreferences::get##Param() \
	{ \
		if (!::mb_##Param) { \
			::mb_##Param = true; \
			m_##Param = Sets->value(#Category"/"#Param, Default).toBool(); \
		} \
		return  m_##Param; \
	}

#define M_PARAM_IMPLEMENT_STRING(Param, Category, Default) \
	bool mb_##Param = false; \
	void MerkaartorPreferences::set##Param(QString theValue) \
	{ \
		m_##Param = theValue; \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	QString MerkaartorPreferences::get##Param() \
	{ \
		if (!::mb_##Param) { \
			::mb_##Param = true; \
			m_##Param = Sets->value(#Category"/"#Param, Default).toString(); \
		} \
		return  m_##Param; \
	}

#define M_PARAM_IMPLEMENT_INT(Param, Category, Default) \
	bool mb_##Param = false; \
	void MerkaartorPreferences::set##Param(int theValue) \
	{ \
		m_##Param = theValue; \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	int MerkaartorPreferences::get##Param() \
	{ \
		if (!::mb_##Param) { \
			::mb_##Param = true; \
			m_##Param = Sets->value(#Category"/"#Param, Default).toInt(); \
		} \
		return  m_##Param; \
	}

#define M_PARAM_IMPLEMENT_DOUBLE(Param, Category, Default) \
	bool mb_##Param = false; \
	void MerkaartorPreferences::set##Param(double theValue) \
	{ \
		m_##Param = theValue; \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	double MerkaartorPreferences::get##Param() \
	{ \
		if (!::mb_##Param) { \
			::mb_##Param = true; \
			m_##Param = Sets->value(#Category"/"#Param, Default).toDouble(); \
		} \
		return  m_##Param; \
	}

/***************************/

MerkaartorPreferences* MerkaartorPreferences::m_prefInstance = 0;

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
{
	Sets = new QSettings();
	theToolList = new ToolList();

	version = Sets->value("version/version", "0").toString();

	connect(&httpRequest, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(on_responseHeaderReceived(const QHttpResponseHeader &)));
	connect(&httpRequest,SIGNAL(requestFinished(int, bool)),this,SLOT(on_requestFinished(int, bool)));

	initialize();
}

MerkaartorPreferences::~MerkaartorPreferences()
{
	delete theToolList;
	delete Sets;
}

void MerkaartorPreferences::save(bool UserPwdChanged)
{
	Sets->setValue("version/version", QString("%1").arg(STRINGIFY(VERSION)));
	setWmsServers();
	setTmsServers();
	setTools();
	setAlphaList();
	Sets->sync();

	saveProjections();
	saveWMSes();
	saveTMSes();
	saveBookmarks();

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

	QUrl osmWeb;
	osmWeb.setScheme("http");
	osmWeb.setAuthority(getOsmWebsite());
	if (osmWeb.port() == -1)
		osmWeb.setPort(80);

	httpRequest.setHost(osmWeb.host(), osmWeb.port());

	QHttpRequestHeader Header("GET", QString("/api/%1/user/preferences/").arg(apiVersion()));
	qDebug() << "MerkaartorPreferences::fromOsmPref :" <<  QString("GET /api/%1/user/preferences/").arg(apiVersion());
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));

	QString auth = QString("%1:%2").arg(getOsmUser()).arg(getOsmPassword());
	QByteArray ba_auth = auth.toUtf8().toBase64();
	Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

	httpRequest.setProxy(getProxy(osmWeb));
	OsmPrefLoadId = httpRequest.request(Header, NULL, &OsmPrefContent);
}

void MerkaartorPreferences::on_requestFinished ( int id, bool error )
{
	if (id == OsmPrefLoadId && !error) {
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
				//ProjectionsList aProjList = ProjectionsList::fromXml(c);
				//theProjectionsList.add(aProjList);
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
			}

			c = c.nextSiblingElement();
		}
	}
}


void MerkaartorPreferences::putOsmPref(const QString& k, const QString& v)
{
	QUrl osmWeb;
	osmWeb.setScheme("http");
	osmWeb.setAuthority(getOsmWebsite());
	if (osmWeb.port() == -1)
		osmWeb.setPort(80);

	QByteArray ba(v.toUtf8());
	QBuffer Buf(&ba);

	httpRequest.setHost(osmWeb.host(), osmWeb.port());

	QHttpRequestHeader Header("PUT", QString("/api/%1/user/preferences/%2").arg(apiVersion()).arg(k));
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));

	QString auth = QString("%1:%2").arg(getOsmUser()).arg(getOsmPassword());
	QByteArray ba_auth = auth.toUtf8().toBase64();
	Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

	httpRequest.setProxy(getProxy(osmWeb));
	OsmPrefSaveId = httpRequest.request(Header,ba);
}

void MerkaartorPreferences::deleteOsmPref(const QString& k)
{
	QUrl osmWeb;
	osmWeb.setScheme("http");
	osmWeb.setAuthority(getOsmWebsite());
	if (osmWeb.port() == -1)
		osmWeb.setPort(80);

	httpRequest.setHost(osmWeb.host(), osmWeb.port());

	QHttpRequestHeader Header("DELETE", QString("/api/%1/user/preferences/%2").arg(apiVersion()).arg(k));
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));

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
	loadWMSes();
	loadTMSes();
	loadBookmarks();

	fromOsmPref();

	QStringList sl = Sets->value("downloadosm/bookmarks").toStringList();
	if (sl.size()) {
		for (int i=0; i<sl.size(); i+=5) {
			Bookmark B(sl[i], CoordBox(Coord(angToInt(sl[i+1].toDouble()),angToInt(sl[i+2].toDouble())),
									Coord(angToInt(sl[i+3].toDouble()),angToInt(sl[i+4].toDouble()))));
			theBookmarkList.addBookmark(B);
		}
		save();
		Sets->remove("downloadosm/bookmarks");
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

	QStringList tl = Sets->value("Tools/list").toStringList();
	for (int i=0; i<tl.size(); i+=TOOL_FIELD_SIZE) {
		Tool t(tl[i], tl[i+1]);
		theToolList->insert(tl[i], t);
	}
	if (!theToolList->contains("Inkscape")) {
		Tool t("Inkscape", "");
		theToolList->insert("Inkscape", t);
	}

	QStringList Servers = Sets->value("WSM/servers").toStringList();
	if (Servers.size()) {
		for (int i=0; i<Servers.size(); i+=7) {
			WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6]);
			theWmsServerList.addServer(S);
		}
		save();
		Sets->remove("WSM/servers");
	}
	Servers = Sets->value("TMS/servers").toStringList();
	if (Servers.size()) {
		for (int i=0; i<Servers.size(); i+=6) {
			TmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3].toInt(), Servers[i+4].toInt(), Servers[i+5].toInt());
			theTmsServerList.addServer(S);
		}
		save();
		Sets->remove("TMS/servers");
	}
	//if (Servers.size() == 0) {
	//	TmsServer osmmapnik("OSM Mapnik", "tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17);
	//	theTmsServerList.insert("OSM Mapnik", osmmapnik);
	//	TmsServer osmth("OSM T@H", "tah.openstreetmap.org", "/Tiles/tile/%1/%2/%3.png", 256, 0, 17);
	//	theTmsServerList.insert("OSM T@H", osmth);
	//	TmsServer cycle("Cycle Map", "andy.sandbox.cloudmade.com", "/tiles/cycle/%1/%2/%3.png", 256, 0, 17);
	//	theTmsServerList.insert("Gravitystorm Cycle", cycle);
	//	TmsServer oam("OpenAerialMap", "tile.openaerialmap.org", "/tiles/1.0.0/openaerialmap-900913/%1/%2/%3.png", 256, 0, 17);
	//	theTmsServerList.insert("OpenAerialMap", oam);
	//	TmsServer npe("New Popular Edition (NPE)", "npe.openstreetmap.org", "/%1/%2/%3.png", 256, 6, 15);
	//	theTmsServerList.insert("New Popular Edition (NPE)", npe);
	//	TmsServer osmmaplint("OSM Maplint", "tah.openstreetmap.org", "/Tiles/maplint/%1/%2/%3.png", 256, 12, 16);
	//	theTmsServerList.insert("OSM Maplint", osmmaplint);
	//	setSelectedTmsServer("OSM Mapnik");
	//	save();
	//}

	// PRoxy upgrade
	if (Sets->contains("proxy/Use")) {
		bool b = Sets->value("proxy/Use").toBool();
		QString h = Sets->value("proxy/Host").toString();
		int p = Sets->value("proxy/Port").toInt();

		Sets->remove("proxy");

		setProxyUse(b);
		setProxyHost(h);
		setProxyPort(p);
	}

	parentDashes << 1 << 5;
}

const QVector<qreal> MerkaartorPreferences::getParentDashes() const
{
	return parentDashes;
}

bool MerkaartorPreferences::getRightSideDriving() const
{
	return Sets->value("roadstructure/rightsidedriving",true).toBool();
}

void MerkaartorPreferences::setRightSideDriving(bool theValue)
{
	Sets->setValue("roadstructure/rightsidedriving", theValue);
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

double MerkaartorPreferences::getDoubleRoadDistance() const
{
	return Sets->value("roadstructure/doubleroaddistance","20").toDouble();
}

void MerkaartorPreferences::setDoubleRoadDistance(double theValue)
{
	Sets->setValue("roadstructure/doubleroaddistance", theValue);
}

QString MerkaartorPreferences::getWorkingDir() const
{
	return Sets->value("general/workingdir", "").toString();
}

void MerkaartorPreferences::setWorkingDir(const QString & theValue)
{
	Sets->setValue("general/workingdir", theValue);
}

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

QString MerkaartorPreferences::getSelectedServer() const
{
	return Sets->value("backgroundImage/SelectedServer").toString();
}

void MerkaartorPreferences::setSelectedServer(const QString & theValue)
{
	Sets->setValue("backgroundImage/SelectedServer", theValue);
}


bool MerkaartorPreferences::getBgVisible() const
{
	return Sets->value("backgroundImage/Visible", false).toBool();
}

void MerkaartorPreferences::setBgVisible(bool theValue)
{
	Sets->setValue("backgroundImage/Visible", theValue);
}

/* Plugins */

void MerkaartorPreferences::addBackgroundPlugin(IMapAdapter* aPlugin)
{
	mBackgroundPlugins.insert(aPlugin->getId(), aPlugin);
}

IMapAdapter* MerkaartorPreferences::getBackgroundPlugin(const QUuid& anAdapterUid)
{
	if (mBackgroundPlugins.contains(anAdapterUid))
		return mBackgroundPlugins[anAdapterUid];
	else
		return NULL;
}

void MerkaartorPreferences::setBackgroundPlugin(const QUuid & theValue)
{
	Sets->setValue("backgroundImage/BackgroundPlugin", theValue.toString());
}

QUuid MerkaartorPreferences::getBackgroundPlugin() const
{
	QString s = Sets->value("backgroundImage/BackgroundPlugin", "").toString();
	return QUuid(s);
}

QMap<QUuid, IMapAdapter *> MerkaartorPreferences::getBackgroundPlugins()
{
	return mBackgroundPlugins;
}

void MerkaartorPreferences::setCacheDir(const QString & theValue)
{
	Sets->setValue("backgroundImage/CacheDir", theValue);
}

QString MerkaartorPreferences::getCacheDir() const
{
	return Sets->value("backgroundImage/CacheDir", HOMEDIR + "/BackgroundCache").toString();
}

int MerkaartorPreferences::getCacheSize() const
{
	return Sets->value("backgroundImage/CacheSize", 0).toInt();
}

void MerkaartorPreferences::setCacheSize(int theValue)
{
	Sets->setValue("backgroundImage/CacheSize", theValue);
}

/* Search */
void MerkaartorPreferences::setLastMaxSearchResults(int theValue)
{
	Sets->setValue("search/LastMax", theValue);
}

int MerkaartorPreferences::getLastMaxSearchResults() const
{
	return Sets->value("search/LastMax", 100).toInt();
}

void MerkaartorPreferences::setLastSearchName(const QString & theValue)
{
	Sets->setValue("search/LastName", theValue);
}

QString MerkaartorPreferences::getLastSearchName() const
{
	return Sets->value("search/LastName").toString();
}

void MerkaartorPreferences::setLastSearchKey(const QString & theValue)
{
	Sets->setValue("search/LastKey", theValue);
}

QString MerkaartorPreferences::getLastSearchKey() const
{
	return Sets->value("search/LastKey").toString();
}

void MerkaartorPreferences::setLastSearchValue(const QString & theValue)
{
	Sets->setValue("search/LastValue", theValue);
}

QString MerkaartorPreferences::getLastSearchValue() const
{
	return Sets->value("search/LastValue").toString();
}

/* Visuals */

void MerkaartorPreferences::saveMainWindowState(const MainWindow * mainWindow)
{
	Sets->setValue("MainWindow/Position", mainWindow->pos());
	Sets->setValue("MainWindow/Size", mainWindow->size());
	Sets->setValue("MainWindow/State", mainWindow->saveState());
	Sets->setValue("MainWindow/Fullscreen", mainWindow->ui->windowShowAllAction->isEnabled());
	Sets->setValue("MainWindow/FullscreenState", mainWindow->fullscreenState);
}

void MerkaartorPreferences::restoreMainWindowState(MainWindow * mainWindow) const
{
	if (Sets->contains("MainWindow/Position"))
		mainWindow->move( Sets->value("MainWindow/Position").toPoint());

	if (Sets->contains("MainWindow/Size"))
		mainWindow->resize( Sets->value("MainWindow/Size").toSize());

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

void MerkaartorPreferences::setInitialPosition(MapView* vw)
{
	QStringList ip;
	CoordBox cb = vw->viewport();
	ip.append(QString::number(cb.bottomLeft().lat()));
	ip.append(QString::number(cb.bottomLeft().lon()));
	ip.append(QString::number(cb.topRight().lat()));
	ip.append(QString::number(cb.topRight().lon()));

	Sets->setValue("MainWindow/InitialPosition", ip);
	Sets->setValue("MainWindow/ViewRect", vw->rect());
}

void MerkaartorPreferences::initialPosition(MapView* vw)
{
	if (!Sets->contains("MainWindow/InitialPosition")) {
		vw->setViewport(CoordBox(Coord(313646971, -120391031), Coord(793005387, 444097188)), vw->rect());
		return;
	}

	const QStringList & ip = Sets->value("MainWindow/InitialPosition").toStringList();

	const Coord bottomLeft(ip[0].toInt(), ip[1].toInt());
	const Coord topRight(ip[2].toInt(),ip[3].toInt());

	if (!Sets->contains("MainWindow/ViewRect"))
		vw->setViewport(CoordBox(bottomLeft, topRight), vw->rect());
	else {
		QRect rt = Sets->value("MainWindow/ViewRect").toRect();
		vw->setViewport(CoordBox(bottomLeft, topRight), rt);
	}
}

#ifndef _MOBILE
void MerkaartorPreferences::setProjectionType(ProjectionType theValue)
{
	Sets->setValue("projection/Type", theValue);
}

ProjectionType MerkaartorPreferences::getProjectionType()
{
	if (getZoomBoris())
		return (ProjectionType)QString("Mercator");
	else
		return (ProjectionType)Sets->value("projection/Type", "Mercator").toString();
}

ProjectionsList MerkaartorPreferences::getProjectionsList()
{
	return theProjectionsList;
}

ProjectionItem MerkaartorPreferences::getProjection(QString aProj)
{
	return theProjectionsList.getProjection(aProj);
}
#endif

qreal MerkaartorPreferences::getAlpha(QString lvl)
{
	return alpha[lvl];
}

QStringList MerkaartorPreferences::getAlphaList() const
{
	return Sets->value("visual/alpha").toStringList();
}

void MerkaartorPreferences::setAlphaList()
{
	QStringList alphaList;
	QHashIterator<QString, qreal> i(alpha);
	while (i.hasNext()) {
		i.next();
		alphaList << i.key() << QString().setNum(i.value());
	}
	Sets->setValue("visual/alpha", alphaList);
}

int MerkaartorPreferences::getHoverWidth() const
{
	return Sets->value("visual/HoverWidth",1).toInt();
}

int MerkaartorPreferences::getHighlightWidth() const
{
	return Sets->value("visual/HighlightWidth",1).toInt();
}

int MerkaartorPreferences::getFocusWidth() const
{
	return Sets->value("visual/FocusWidth",3).toInt();
}

int MerkaartorPreferences::getRelationsWidth() const
{
	return Sets->value("visual/RelationsWidth",3).toInt();
}

int MerkaartorPreferences::getGpxTrackWidth() const
{
	return Sets->value("visual/GpxTrackWidth",3).toInt();
}

QColor mb_BgColor;
QColor MerkaartorPreferences::getBgColor() const
{
	if (!::mb_BgColor.isValid()) {
		QString sColor = Sets->value("visual/BgColor").toString();
		if (sColor.isEmpty())
			::mb_BgColor = Qt::white;
		else
			::mb_BgColor = Sets->value("visual/BgColor").value<QColor>();
	}
	return ::mb_BgColor;
}

void MerkaartorPreferences::setBgColor(const QColor theValue)
{
	::mb_BgColor = theValue;
	Sets->setValue("visual/BgColor", QVariant(theValue));
}


QColor mb_WaterColor;
QColor MerkaartorPreferences::getWaterColor() const
{
	if (!::mb_BgColor.isValid()) {
		QString sColor = Sets->value("visual/WaterColor").toString();
		if (sColor.isEmpty())
			::mb_WaterColor = QColor(181, 208, 208);
		else
			mb_WaterColor = Sets->value("visual/WaterColor").value<QColor>();
	}
	return ::mb_WaterColor;
}

void MerkaartorPreferences::setWaterColor(const QColor theValue)
{
	::mb_WaterColor = theValue;
	Sets->setValue("visual/WaterColor", QVariant(theValue));
}


QColor MerkaartorPreferences::getFocusColor() const
{
	QString sColor = Sets->value("visual/FocusColor").toString();
	if (sColor.isEmpty())
		return Qt::blue;
	return Sets->value("visual/FocusColor").value<QColor>();
}

QColor MerkaartorPreferences::getHoverColor() const
{
	QString sColor = Sets->value("visual/HoverColor").toString();
	if (sColor.isEmpty())
		return Qt::magenta;
	return Sets->value("visual/HoverColor").value<QColor>();
}

QColor MerkaartorPreferences::getHighlightColor() const
{
	QString sColor = Sets->value("visual/HighlightColor").toString();
	if (sColor.isEmpty())
		return Qt::darkCyan;
	return Sets->value("visual/HighlightColor").value<QColor>();
}

QColor MerkaartorPreferences::getRelationsColor() const
{
	QString sColor = Sets->value("visual/RelationsColor").toString();
	if (sColor.isEmpty())
		return QColor(0, 170, 0);
	return Sets->value("visual/RelationsColor").value<QColor>();
}

QColor MerkaartorPreferences::getGpxTrackColor() const
{
	QString sColor = Sets->value("visual/GpxTrackColor").toString();
	if (sColor.isEmpty())
		return QColor(50, 220, 220);
	return Sets->value("visual/GpxTrackColor").value<QColor>();
}

void MerkaartorPreferences::setHoverColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/HoverColor", QVariant(theValue));
	Sets->setValue("visual/HoverWidth", Width);
}

void MerkaartorPreferences::setHighlightColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/HighlightColor", QVariant(theValue));
	Sets->setValue("visual/HighlightWidth", Width);
}

void MerkaartorPreferences::setFocusColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/FocusColor", QVariant(theValue));
	Sets->setValue("visual/FocusWidth", Width);
}

void MerkaartorPreferences::setRelationsColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/RelationsColor", QVariant(theValue));
	Sets->setValue("visual/RelationsWidth", Width);
}

void MerkaartorPreferences::setGpxTrackColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/GpxTrackColor", QVariant(theValue));
	Sets->setValue("visual/GpxTrackWidth", Width);
}

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
	return Sets->value("osm/Website", "www.openstreetmap.org").toString();
}

void MerkaartorPreferences::setOsmWebsite(const QString & theValue)
{
	Sets->setValue("osm/Website", theValue);
}

M_PARAM_IMPLEMENT_STRING(XapiWebSite, osm, "www.informationfreeway.org")
M_PARAM_IMPLEMENT_BOOL(AutoHistoryCleanup, data, true);

QString MerkaartorPreferences::getOsmUser() const
{
	return Sets->value("osm/User").toString();
}

void MerkaartorPreferences::setOsmUser(const QString & theValue)
{
	Sets->setValue("osm/User", theValue);
}

QString MerkaartorPreferences::getOsmPassword() const
{
	return Sets->value("osm/Password").toString();
}

void MerkaartorPreferences::setOsmPassword(const QString & theValue)
{
	Sets->setValue("osm/Password", theValue);
}

M_PARAM_IMPLEMENT_DOUBLE(MaxDistNodes, data, 0.0)

bool MerkaartorPreferences::getAutoSaveDoc() const
{
	return Sets->value("data/AutoSaveDoc", false).toBool();
}

void MerkaartorPreferences::setAutoSaveDoc(bool theValue)
{
	Sets->setValue("data/AutoSaveDoc", theValue);
}

void MerkaartorPreferences::setAutoExtractTracks(bool theValue)
{
	Sets->setValue("data/AutoExtractTracks", theValue);
}

bool MerkaartorPreferences::getAutoExtractTracks() const
{
	return Sets->value("data/AutoExtractTracks", false).toBool();
}


bool MerkaartorPreferences::getDownloadedVisible() const
{
	return Sets->value("visual/DownloadedVisible", true).toBool();
}

void MerkaartorPreferences::setDownloadedVisible(bool theValue)
{
	Sets->setValue("visual/DownloadedVisible", theValue);
}

bool MerkaartorPreferences::getScaleVisible() const
{
	return Sets->value("visual/ScaleVisible", true).toBool();
}

void MerkaartorPreferences::setScaleVisible(bool theValue)
{
	Sets->setValue("visual/ScaleVisible", theValue);
}

bool MerkaartorPreferences::getStyleBackgroundVisible() const
{
	return Sets->value("visual/BackgroundVisible", true).toBool();
}

void MerkaartorPreferences::setStyleBackgroundVisible(bool theValue)
{
	Sets->setValue("visual/BackgroundVisible", theValue);
}

bool MerkaartorPreferences::getStyleForegroundVisible() const
{
	return Sets->value("visual/ForegroundVisible", true).toBool();
}

void MerkaartorPreferences::setStyleForegroundVisible(bool theValue)
{
	Sets->setValue("visual/ForegroundVisible", theValue);
}

bool MerkaartorPreferences::getStyleTouchupVisible() const
{
	return Sets->value("visual/TouchupVisible", true).toBool();
}

void MerkaartorPreferences::setStyleTouchupVisible(bool theValue)
{
	Sets->setValue("visual/TouchupVisible", theValue);
}

bool MerkaartorPreferences::getNamesVisible() const
{
	return Sets->value("visual/NamesVisible", false).toBool();
}

void MerkaartorPreferences::setNamesVisible(bool theValue)
{
	Sets->setValue("visual/NamesVisible", theValue);
}

bool MerkaartorPreferences::getTrackPointsVisible() const
{
	return Sets->value("visual/TrackPointsVisible", true).toBool();
}

void MerkaartorPreferences::setTrackPointsVisible(bool theValue)
{
	Sets->setValue("visual/TrackPointsVisible", theValue);
}

bool MerkaartorPreferences::getTrackSegmentsVisible() const
{
	return Sets->value("visual/TrackSegmentsVisible", true).toBool();
}

void MerkaartorPreferences::setTrackSegmentsVisible(bool theValue)
{
	Sets->setValue("visual/TrackSegmentsVisible", theValue);
}

bool MerkaartorPreferences::getRelationsVisible() const
{
	return Sets->value("visual/RelationsVisible", false).toBool();
}

void MerkaartorPreferences::setRelationsVisible(bool theValue)
{
	Sets->setValue("visual/RelationsVisible", theValue);
}

DirectionalArrowsShow MerkaartorPreferences::getDirectionalArrowsVisible()
{
	return (DirectionalArrowsShow)Sets->value("visual/DirectionalArrowsVisible", DirectionalArrows_Oneway).toInt();
}

void MerkaartorPreferences::setDirectionalArrowsVisible(DirectionalArrowsShow theValue)
{
	Sets->setValue("visual/DirectionalArrowsVisible", theValue);
}

/* Export Type */
void MerkaartorPreferences::setExportType(ExportType theValue)
{
	Sets->setValue("export/Type", theValue);
}

ExportType MerkaartorPreferences::getExportType() const
{
	return (ExportType)Sets->value("export/Type", 0).toInt();
}

/* Tools */
ToolList* MerkaartorPreferences::getTools() const
{
	return theToolList;
}

void MerkaartorPreferences::setTools()
{
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
	return Sets->value("recent/open").toStringList();
}

void MerkaartorPreferences::setRecentOpen(const QStringList & theValue)
{
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
	return Sets->value("recent/import").toStringList();
}

void MerkaartorPreferences::setRecentImport(const QStringList & theValue)
{
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
	return Sets->value("Tools/shortcuts").toStringList();
}

void MerkaartorPreferences::setShortcuts(const QStringList & theValue)
{
	Sets->setValue("Tools/shortcuts", theValue);
}

/* Styles */
M_PARAM_IMPLEMENT_STRING(DefaultStyle, style, ":/Styles/Mapnik.mas")
M_PARAM_IMPLEMENT_STRING(CustomStyle, style, "")
M_PARAM_IMPLEMENT_BOOL(DisableStyleForTracks, style, true)

/* Visual */
M_PARAM_IMPLEMENT_INT(ZoomIn, zoom, 133)
M_PARAM_IMPLEMENT_INT(ZoomOut, zoom, 75)
M_PARAM_IMPLEMENT_BOOL(ZoomBoris, zoom, false)
M_PARAM_IMPLEMENT_BOOL(BackgroundOverwriteStyle, visual, false)
M_PARAM_IMPLEMENT_INT(AreaOpacity, visual, 100)
M_PARAM_IMPLEMENT_BOOL(UseShapefileForBackground, visual, false)
M_PARAM_IMPLEMENT_BOOL(DrawingHack, visual, true)
M_PARAM_IMPLEMENT_BOOL(SimpleGpxTrack, visual, false)
M_PARAM_IMPLEMENT_BOOL(VirtualNodesVisible, visual, true)
M_PARAM_IMPLEMENT_BOOL(UseVirtualNodes, visual, true)
M_PARAM_IMPLEMENT_BOOL(RelationsSelectableWhenHidden, visual, true)
M_PARAM_IMPLEMENT_DOUBLE(LocalZoom, visual, 1.0)
M_PARAM_IMPLEMENT_DOUBLE(RegionalZoom, visual, 0.01)

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

M_PARAM_IMPLEMENT_INT(TagListFirstColumnWidth, visual, 0)
M_PARAM_IMPLEMENT_BOOL(TranslateTags, locale, true)

/* World OSB manager */
M_PARAM_IMPLEMENT_DOUBLE(TileToRegionThreshold, WOSB, 0.03)
M_PARAM_IMPLEMENT_DOUBLE(RegionTo0Threshold, WOSB, 1.)

M_PARAM_IMPLEMENT_STRING(WorldOsbUri, WOSB, "")
M_PARAM_IMPLEMENT_BOOL(WorldOsbAutoload, WOSB, false)
M_PARAM_IMPLEMENT_BOOL(WorldOsbAutoshow, WOSB, false)

/* Mouse bevaviour */
#ifdef _MOBILE
	M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, true)
#else
	M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, false)
#endif
M_PARAM_IMPLEMENT_BOOL(SeparateMoveMode, Mouse, true)

/* Custom Style */
M_PARAM_IMPLEMENT_BOOL(MerkaartorStyle, visual, false)
M_PARAM_IMPLEMENT_STRING(MerkaartorStyleString, visual, "skulpture")

/* Network */
M_PARAM_IMPLEMENT_BOOL(OfflineMode, Network, false)

/* Proxy */

QNetworkProxy MerkaartorPreferences::getProxy(const QUrl & /*requestUrl*/)
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
		theProxy.setType(QNetworkProxy::NoProxy);
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
	QString fn = ":/Projections.xml";
	loadProjection(fn);

	fn = QString(STRINGIFY(SHARE_DIR)) + "/Projections.xml";
	loadProjection(fn);

	fn = HOMEDIR + "/Projections.xml";
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
	QString fn = ":/WmsServersList.xml";
	loadWMS(fn);

	fn = QString(STRINGIFY(SHARE_DIR)) + "/WmsServersList.xml";
	loadWMS(fn);

	fn = HOMEDIR + "/WmsServersList.xml";
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
	QString fn = ":/TmsServersList.xml";
	loadTMS(fn);

	fn = QString(STRINGIFY(SHARE_DIR)) + "/TmsServersList.xml";
	loadTMS(fn);

	fn = HOMEDIR + "/TmsServersList.xml";
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
	QString fn = ":/BookmarksList.xml";
	loadBookmark(fn);

	fn = QString(STRINGIFY(SHARE_DIR)) + "/BookmarksList.xml";
	loadBookmark(fn);

	fn = HOMEDIR + "/BookmarksList.xml";
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

/* */

QString getDefaultLanguage()
{
	QSettings Sets;
	QString lang = Sets.value("locale/language").toString();
	if (lang == "")
		lang = QLocale::system().name().split("_")[0];
	return lang;
}

void setDefaultLanguage(const QString& theValue)
{
	QSettings Sets;
	Sets.setValue("locale/language", theValue);
}
