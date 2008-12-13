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
#include "MapView.h"

#define M_PARAM_IMPLEMENT_BOOL(Param, Category, Default) \
	void MerkaartorPreferences::set##Param(bool theValue) \
	{ \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	bool MerkaartorPreferences::get##Param() const \
	{ \
		return Sets->value(#Category"/"#Param, Default).toBool(); \
	}

#define M_PARAM_IMPLEMENT_STRING(Param, Category, Default) \
	void MerkaartorPreferences::set##Param(const QString & theValue) \
	{ \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	QString MerkaartorPreferences::get##Param() const \
	{ \
		return Sets->value(#Category"/"#Param, Default).toString(); \
	}

#define M_PARAM_IMPLEMENT_INT(Param, Category, Default) \
	void MerkaartorPreferences::set##Param(const int theValue) \
	{ \
		Sets->setValue(#Category"/"#Param, theValue); \
	} \
	int MerkaartorPreferences::get##Param() const \
	{ \
		return Sets->value(#Category"/"#Param, Default).toInt(); \
	}

/***************************/

MerkaartorPreferences* MerkaartorPreferences::m_prefInstance = 0;

WmsServer::WmsServer()
{
	WmsServer(QApplication::translate("MerkaartorPreferences","New Server"), "", "", "", "", "", "");
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat)
	: WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles), WmsImgFormat(ImgFormat)
{
	if (Name == "") {
		WmsName = QApplication::translate("MerkaartorPreferences","New Server");
	}
}

TmsServer::TmsServer()
{
	TmsServer(QApplication::translate("MerkaartorPreferences","New Server"), "", "", 256, 0, 17);
}

TmsServer::TmsServer(QString Name, QString Adress, QString Path, int tileSize, int minZoom, int maxZoom)
	: TmsName(Name), TmsAdress(Adress), TmsPath(Path), TmsTileSize(tileSize), TmsMinZoom(minZoom), TmsMaxZoom(maxZoom)
{
	if (Name == "") {
		TmsName = QApplication::translate("MerkaartorPreferences","New Server");
	}
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
{
	Sets = new QSettings();
	theToolList = new ToolList();

	version = Sets->value("version/version", "0").toString();

	connect(&httpRequest, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(on_responseHeaderReceived(const QHttpResponseHeader &)));
	connect(&httpRequest,SIGNAL(requestFinished(int, bool)),this,SLOT(on_requestFinished(int, bool)));
	if (getProxyUse())
		httpRequest.setProxy(getProxyHost(),getProxyPort());

	initialize();
	fromOsmPref();
}

MerkaartorPreferences::~MerkaartorPreferences()
{
	delete theToolList;
	delete Sets;
}

void MerkaartorPreferences::save()
{
	Sets->setValue("version/version", QString("%1").arg(VERSION));
	setWmsServers();
	setTmsServers();
	setTools();
	setAlphaList();
	Sets->sync();

	if (getProxyUse())
		httpRequest.setProxy(getProxyHost(),getProxyPort());
	else
		httpRequest.setProxy("",0);
	toOsmPref(true);
}

void MerkaartorPreferences::toOsmPref(bool clear)
{
	if (getOsmUser().isEmpty() || getOsmPassword().isEmpty()) return;

	QMap<QString, QString> OsmPref;

	int i=0;
	BookmarkListIterator bl(getBookmarks());
	while(bl.hasNext()) {
		bl.next();

		QString k = QString("MerkaartorBookmark%1").arg(i, 3, 10, QLatin1Char('0'));
		QString v = QString("%1;%2;%3;%4;%5")
						.arg(bl.key())
						.arg(intToAng(bl.value().bottomRight().lat()))
						.arg(intToAng(bl.value().bottomRight().lon()))
						.arg(intToAng(bl.value().topLeft().lat()))
						.arg(intToAng(bl.value().topLeft().lon()));
		OsmPref.insert(k, v);
		++i;
	}

	i=0;
	TmsServerListIterator tsl(getTmsServers());
	while(tsl.hasNext()) {
		tsl.next();

		QString k = QString("MerkaartorTmsServer%1").arg(i, 3, 10, QLatin1Char('0'));
		QString v;
		TmsServer S = tsl.value();
		v.append(S.TmsName + ";");
		v.append(S.TmsAdress + ";");
		v.append(S.TmsPath + ";");
		v.append(QString::number(S.TmsTileSize) + ";");
		v.append(QString::number(S.TmsMinZoom) + ";");
		v.append(QString::number(S.TmsMaxZoom));

		OsmPref.insert(k, v);
		++i;
	}

	i=0;
	WmsServerListIterator wsl(getWmsServers());
	while(wsl.hasNext()) {
		wsl.next();

		QString k = QString("MerkaartorWmsServer%1").arg(i, 3, 10, QLatin1Char('0'));
		QString v;
		WmsServer S = wsl.value();
		v.append(S.WmsName + ";");
		v.append(S.WmsAdress + ";");
		v.append(S.WmsPath + ";");
		v.append(S.WmsLayers + ";");
		v.append(S.WmsProjections + ";");
		v.append(S.WmsStyles + ";");
		v.append(S.WmsImgFormat);

		OsmPref.insert(k, v);
		++i;
	}

	QMapIterator<QString, QString> it(OsmPref);
	while(it.hasNext()) {
		it.next();
		putOsmPref(it.key(), it.value());
	}
	for (int i=getBookmarks().size(); i<OsmPrefListsCount["MerkaartorBookmark"]; ++i) {
		QString k = QString("MerkaartorBookmark%1").arg(i, 3, 10, QLatin1Char('0'));
		deleteOsmPref(k);
	}
	for (int i=getBookmarks().size(); i<OsmPrefListsCount["MerkaartorTmsServer"]; ++i) {
		QString k = QString("MerkaartorTmsServer%1").arg(i, 3, 10, QLatin1Char('0'));
		deleteOsmPref(k);
	}
	for (int i=getBookmarks().size(); i<OsmPrefListsCount["MerkaartorWmsServer"]; ++i) {
		QString k = QString("MerkaartorWmsServer%1").arg(i, 3, 10, QLatin1Char('0'));
		deleteOsmPref(k);
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
	httpRequest.setUser(getOsmUser().toUtf8(), getOsmPassword().toUtf8());

	QHttpRequestHeader Header("PUT", QString("/api/%1/user/preferences/%2").arg(apiVersion()).arg(k));
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
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
	httpRequest.setUser(getOsmUser().toUtf8(), getOsmPassword().toUtf8());

	QHttpRequestHeader Header("DELETE", QString("/api/%1/user/preferences/%2").arg(apiVersion()).arg(k));
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
	httpRequest.request(Header);
}

void MerkaartorPreferences::fromOsmPref()
{
	if (getOsmUser().isEmpty() || getOsmPassword().isEmpty()) return;

	QUrl osmWeb;
	osmWeb.setScheme("http");
	osmWeb.setAuthority(getOsmWebsite());
	if (osmWeb.port() == -1)
		osmWeb.setPort(80);

	httpRequest.setHost(osmWeb.host(), osmWeb.port());
	httpRequest.setUser(getOsmUser().toUtf8(), getOsmPassword().toUtf8());

	QHttpRequestHeader Header("GET", QString("/api/%1/user/preferences/").arg(apiVersion()));
	if (osmWeb.port() == 80)
		Header.setValue("Host",osmWeb.host());
	else
		Header.setValue("Host",osmWeb.host() + ':' + QString::number(osmWeb.port()));
	OsmPrefLoadId = httpRequest.request(Header, NULL, &OsmPrefContent);
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
			qDebug() << hdr.statusCode();
			qDebug() << hdr.reasonPhrase();
			break;
	}
}

void MerkaartorPreferences::on_requestFinished ( int id, bool error )
{
	if (id == OsmPrefLoadId && !error) {
		QMap<QString, QString> OsmPref;
		bool ChangedBookmark = false;

		QDomDocument aOsmPrefDoc;
		aOsmPrefDoc.setContent(OsmPrefContent.buffer(), false);

		OsmPrefListsCount["MerkaartorBookmark"] = 0;
		OsmPrefListsCount["MerkaartorTmsServer"] = 0;
		OsmPrefListsCount["MerkaartorWmsServer"] = 0;

		QDomNodeList prefList = aOsmPrefDoc.elementsByTagName("preference");
		for (int i=0; i < prefList.size(); ++i) {
			QDomElement e = prefList.at(i).toElement();

			if (e.attribute("k").startsWith("MerkaartorBookmark")) {
				OsmPrefListsCount["MerkaartorBookmark"]++;
				QStringList sl = e.attribute("v").split(";");
				if (!theBookmarks.contains(sl[0])) {
					ChangedBookmark = true;
					theBookmarks[sl[0]] = CoordBox(Coord(angToInt(sl[1].toDouble()),angToInt(sl[2].toDouble())),
											Coord(angToInt(sl[3].toDouble()),angToInt(sl[4].toDouble())));
				}
			}
			if (e.attribute("k").startsWith("MerkaartorTmsServer")) {
				OsmPrefListsCount["MerkaartorTmsServer"]++;
				QStringList sl = e.attribute("v").split(";");
				if (!theTmsServerList.contains(sl[0])) {
					TmsServer S(sl[0], sl[1], sl[2], sl[3].toInt(), sl[4].toInt(), sl[5].toInt());
					theTmsServerList[sl[0]] = S;
				}
			}
			if (e.attribute("k").startsWith("MerkaartorWmsServer")) {
				OsmPrefListsCount["MerkaartorWmsServer"]++;
				QStringList sl = e.attribute("v").split(";");
				if (!theWmsServerList.contains(sl[0])) {
					WmsServer S(sl[0], sl[1], sl[2], sl[3], sl[4], sl[5], sl[6]);
					theWmsServerList[sl[0]] = S;
				}
			}
		}
		if (ChangedBookmark) {
			setBookmarks();	
			emit(bookmarkChanged());
		}
	}
}

void MerkaartorPreferences::initialize()
{
	Use06Api = Sets->value("osm/use06api", "").toBool();
	bgTypes.insert(Bg_None, tr("None"));
	bgTypes.insert(Bg_Wms, tr("WMS adapter"));
	bgTypes.insert(Bg_Tms, tr("TMS adapter"));
#ifdef YAHOO
	bgTypes.insert(Bg_Yahoo, tr("Yahoo adapter"));
#endif
#ifdef YAHOO_ILLEGAL
	bgTypes.insert(Bg_Yahoo_illegal, tr("Illegal Yahoo adapter"));
#endif
#ifdef GOOGLE_ILLEGAL
	bgTypes.insert(Bg_Google_illegal, tr("Illegal Google adapter"));
#endif
#ifdef MSLIVEMAP_ILLEGAL
	bgTypes.insert(Bg_MsVirtualEarth_illegal, tr("Illegal Ms Virtual Earth adapter"));
#endif

	projTypes.insert(Proj_Merkaartor, tr("Merkaartor"));
	projTypes.insert(Proj_Background, tr("Background"));

	QStringList sl = Sets->value("downloadosm/bookmarks").toStringList();
	if (sl.size() == 0) {
		QStringList DefaultBookmarks;
		DefaultBookmarks << "London" << "51.47" << "-0.20" << "51.51" << "-0.08";
	//	DefaultBookmarks << "Rotterdam" << "51.89" << "4.43" << "51.93" << "4.52";
		Sets->setValue("downloadosm/bookmarks", DefaultBookmarks);
		sl = DefaultBookmarks;
	}
	for (int i=0; i<sl.size(); i+=5) {
		theBookmarks[sl[i]] = CoordBox(Coord(angToInt(sl[i+1].toDouble()),angToInt(sl[i+2].toDouble())),
								Coord(angToInt(sl[i+3].toDouble()),angToInt(sl[i+4].toDouble())));
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

	if (version == "0") {
		QStringList Servers = Sets->value("WSM/servers").toStringList();
		if (Servers.size() > 0) {
			int selI = Sets->value("WSM/selected", "0").toInt();
			QString selS;
			for (int i=0; i<Servers.size()/6; i++) {
				WmsServer S(Servers[(i*6)], Servers[(i*6)+1], Servers[(i*6)+2], Servers[(i*6)+3], Servers[(i*6)+4], Servers[(i*6)+5], "image/png");
				theWmsServerList.insert(Servers[(i*6)], S);
				if (i == selI)
					selS = Servers[(i*6)];
			}
			setSelectedWmsServer(selS);
			if (!theWmsServerList.contains("OpenAerialMap")) {
				WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
					"world", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList.insert("OpenAerialMap", oam);
				WmsServer otm("OpenTopoMap", "opentopomap.org", "/wms/?",
					"world", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList.insert("OpenTopoMap", otm);
				WmsServer tu("Terraservice_Urban", "terraservice.net", "/ogcmap.ashx?",
					"urbanarea", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList.insert("Terraservice_Urban", tu);
				WmsServer tg("Terraservice_DRG", "terraservice.net", "/ogcmap.ashx?",
					"drg", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList.insert("Terraservice_DRG", tg);
				WmsServer tq("Terraservice_DOQ", "terraservice.net", "/ogcmap.ashx?",
					"doq", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList.insert("Terraservice_DOQ", tq);
				WmsServer op("Oberpfalz Germany", "oberpfalz.geofabrik.de", "/wms/?",
					"DOP20", "EPSG:4326", ",", "image/png");
				theWmsServerList.insert("Oberpfalz_DOP20", op);
			}
		}
		save();
	}
	QStringList Servers = Sets->value("WSM/servers").toStringList();
	if (Servers.size() == 0) {
		WmsServer demis("Demis", "www2.demis.nl", "/wms/wms.asp?wms=WorldMap&",
						"Countries,Borders,Highways,Roads,Cities", "EPSG:4326", ",", "image/png");
		theWmsServerList.insert("Demis", demis);
		WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
						"world", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList.insert("OpenAerialMap", oam);
		WmsServer otm("OpenTopoMap", "opentopomap.org", "/wms/?",
						"world", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList.insert("OpenTopoMap", otm);
		WmsServer tu("Terraservice_Urban", "terraservice.net", "/ogcmap.ashx?",
						"urbanarea", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList.insert("Terraservice_Urban", tu);
		WmsServer tg("Terraservice_DRG", "terraservice.net", "/ogcmap.ashx?",
						"drg", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList.insert("Terraservice_DRG", tg);
		WmsServer tq("Terraservice_DOQ", "terraservice.net", "/ogcmap.ashx?",
						"doq", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList.insert("Terraservice_DOQ", tq);
		WmsServer op("Oberpfalz_DOP20", "oberpfalz.geofabrik.de", "/wms?",
						"DOP20", "EPSG:4326", ",", "image/png");
		theWmsServerList.insert("Oberpfalz_DOP20", op);
		setSelectedWmsServer("OpenAerialMap");
		save();
	}
	for (int i=0; i<Servers.size(); i+=7) {
		WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6]);
		theWmsServerList.insert(Servers[i], S);
	}
	Servers = Sets->value("TMS/servers").toStringList();
	if (Servers.size() == 0) {
		TmsServer osmmapnik("OSM Mapnik", "tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList.insert("OSM Mapnik", osmmapnik);
		TmsServer osmth("OSM T@H", "tah.openstreetmap.org", "/Tiles/tile/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList.insert("OSM T@H", osmth);
		TmsServer cycle("Cycle Map", "andy.sandbox.cloudmade.com", "/tiles/cycle/%1/%2/%3.png", 256, 0, 14);
		theTmsServerList.insert("Gravitystorm Cycle", cycle);
		TmsServer oam("OpenAerialMap", "tile.openaerialmap.org", "/tiles/1.0.0/openaerialmap-900913/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList.insert("OpenAerialMap", oam);
		TmsServer npe("New Popular Edition (NPE) at zoom 14", "richard.dev.openstreetmap.org", "/npe/%1/%2/%3.jpg", 256, 14, 14);
		theTmsServerList.insert("New Popular Edition (NPE) at zoom 14", npe);
		TmsServer osmmaplint("OSM Maplint", "tah.openstreetmap.org", "/Tiles/maplint/%1/%2/%3.png", 256, 12, 16);
		theTmsServerList.insert("OSM Maplint", osmmaplint);
		setSelectedTmsServer("OSM Mapnik");
		save();
	}
	for (int i=0; i<Servers.size(); i+=6) {
		TmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3].toInt(), Servers[i+4].toInt(), Servers[i+5].toInt());
		theTmsServerList.insert(Servers[i], S);
	}
}

bool MerkaartorPreferences::getRightSideDriving() const
{
	return Sets->value("roadstructure/rightsidedriving",true).toBool();
}

void MerkaartorPreferences::setRightSideDriving(bool theValue)
{
	Sets->setValue("roadstructure/rightsidedriving", theValue);
}

bool MerkaartorPreferences::use06Api() const
{
	return Use06Api;
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

bool MerkaartorPreferences::getProxyUse() const
{
	return Sets->value("proxy/Use").toBool();
}

void MerkaartorPreferences::setProxyUse(bool theValue)
{
	Sets->setValue("proxy/Use", theValue);
}

QString MerkaartorPreferences::getProxyHost() const
{
	return Sets->value("proxy/Host").toString();
}

void MerkaartorPreferences::setProxyHost(const QString & theValue)
{
	Sets->setValue("proxy/Host", theValue);
}

int MerkaartorPreferences::getProxyPort() const
{
	return Sets->value("proxy/Port").toInt();
}

void MerkaartorPreferences::setProxyPort(int theValue)
{
	Sets->setValue("proxy/Port", theValue);
}

BookmarkList& MerkaartorPreferences::getBookmarks()
{
	//return Sets->value("downloadosm/bookmarks").toStringList();
	return theBookmarks;
}

void MerkaartorPreferences::setBookmarks()
{
	QStringList bk;
	QMapIterator<QString, CoordBox> i(theBookmarks);
	while (i.hasNext()) {
		i.next();
		bk.append(i.key());
		bk.append(QString::number(intToAng(i.value().bottomLeft().lat()), 'f', 4));
		bk.append(QString::number(intToAng(i.value().bottomLeft().lon()), 'f', 4));
		bk.append(QString::number(intToAng(i.value().topRight().lat()), 'f', 4));
		bk.append(QString::number(intToAng(i.value().topRight().lon()), 'f', 4));
	}
	Sets->setValue("downloadosm/bookmarks", bk);
	toOsmPref(true);
}

/* WMS */

WmsServerList& MerkaartorPreferences::getWmsServers()
{
//	return Sets->value("WSM/servers").toStringList();
	return theWmsServerList;
}

void MerkaartorPreferences::setWmsServers()
{
	QStringList Servers;
	WmsServerListIterator i(theWmsServerList);
	while (i.hasNext()) {
		i.next();
		WmsServer S = i.value();
		Servers.append(S.WmsName);
		Servers.append(S.WmsAdress);
		Servers.append(S.WmsPath);
		Servers.append(S.WmsLayers);
		Servers.append(S.WmsProjections);
		Servers.append(S.WmsStyles);
		Servers.append(S.WmsImgFormat);
	}
	Sets->setValue("WSM/servers", Servers);
}

QString MerkaartorPreferences::getSelectedWmsServer() const
{
	return Sets->value("WSM/selected").toString();
}

void MerkaartorPreferences::setSelectedWmsServer(const QString & theValue)
{
	Sets->setValue("WSM/selected", theValue);
}

/* TMS */

TmsServerList& MerkaartorPreferences::getTmsServers()
{
//	return Sets->value("WSM/servers").toStringList();
	return theTmsServerList;
}

void MerkaartorPreferences::setTmsServers()
{
	QStringList Servers;
	TmsServerListIterator i(theTmsServerList);
	while (i.hasNext()) {
		i.next();
		TmsServer S = i.value();
		Servers.append(S.TmsName);
		Servers.append(S.TmsAdress);
		Servers.append(S.TmsPath);
		Servers.append(QString::number(S.TmsTileSize));
		Servers.append(QString::number(S.TmsMinZoom));
		Servers.append(QString::number(S.TmsMaxZoom));
	}
	Sets->setValue("TMS/servers", Servers);
}

QString MerkaartorPreferences::getSelectedTmsServer() const
{
	return Sets->value("TMS/selected").toString();
}

void MerkaartorPreferences::setSelectedTmsServer(const QString & theValue)
{
	Sets->setValue("TMS/selected", theValue);
}

/* */

bool MerkaartorPreferences::getBgVisible() const
{
	return Sets->value("backgroundImage/Visible", false).toBool();
}

void MerkaartorPreferences::setBgVisible(bool theValue)
{
	Sets->setValue("backgroundImage/Visible", theValue);
}

void MerkaartorPreferences::setBgType(ImageBackgroundType theValue)
{
	Sets->setValue("backgroundImage/Type", theValue);
}

ImageBackgroundType MerkaartorPreferences::getBgType() const
{
	return (ImageBackgroundType)Sets->value("backgroundImage/Type", 0).toInt();
}

QStringList MerkaartorPreferences::getBgTypes()
{
	return bgTypes;
}

void MerkaartorPreferences::setCacheDir(const QString & theValue)
{
	Sets->setValue("backgroundImage/CacheDir", theValue);
}

QString MerkaartorPreferences::getCacheDir() const
{
	return Sets->value("backgroundImage/CacheDir", QDir::homePath() + "/.QMapControlCache").toString();
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
	return (ImageBackgroundType)Sets->value("search/LastMax", 100).toInt();
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

	int MerkaartorPreferences::getZoomInPerc() const
{
	return Sets->value("zoom/zoomIn", "133").toInt();
}

void MerkaartorPreferences::setZoomInPerc(int theValue)
{
	Sets->setValue("zoom/zoomIn", theValue);
}

int MerkaartorPreferences::getZoomOutPerc() const
{
	return Sets->value("zoom/zoomOut", "75").toInt();
}

void MerkaartorPreferences::setZoomOutPerc(int theValue)
{
	Sets->setValue("zoom/zoomOut", theValue);
}

void MerkaartorPreferences::saveMainWindowState(const MainWindow * mainWindow)
{
	Sets->setValue("MainWindow/Position", mainWindow->pos());
	Sets->setValue("MainWindow/Size", mainWindow->size());
	Sets->setValue("MainWindow/State", mainWindow->saveState());
	Sets->setValue("MainWindow/Fullscreen", mainWindow->windowShowAllAction->isEnabled());
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
		mainWindow->windowHideAllAction->setEnabled(false);
		mainWindow->windowHideAllAction->setVisible(false);
		mainWindow->windowShowAllAction->setEnabled(true);
		mainWindow->windowShowAllAction->setVisible(true);
	} else {
		mainWindow->windowHideAllAction->setEnabled(true);
		mainWindow->windowHideAllAction->setVisible(true);
		mainWindow->windowShowAllAction->setEnabled(false);
		mainWindow->windowShowAllAction->setVisible(false);
	}
}

void MerkaartorPreferences::setInitialPosition(MapView* vw)
{
	QStringList ip;
	CoordBox cb = vw->projection().viewport();
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
		vw->projection().setViewport(WORLD_COORDBOX, vw->rect());
		return;
	}

	const QStringList & ip = Sets->value("MainWindow/InitialPosition").toStringList();

	const Coord bottomLeft(ip[0].toInt(), ip[1].toInt());
	const Coord topRight(ip[2].toInt(),ip[3].toInt());

	if (!Sets->contains("MainWindow/ViewSize"))
		vw->projection().setViewport(CoordBox(bottomLeft, topRight), vw->rect());
	else {
		QRect rt = Sets->value("MainWindow/ViewRect").toRect();
		vw->projection().setViewport(CoordBox(bottomLeft, topRight), rt);
	}

}

void MerkaartorPreferences::setProjectionType(ProjectionType theValue)
{
	Sets->setValue("projection/Type", theValue);
}

ProjectionType MerkaartorPreferences::getProjectionType() const
{
	return (ProjectionType)Sets->value("projection/Type", 0).toInt();
}

QStringList MerkaartorPreferences::getProjectionTypes()
{
	return projTypes;
}

bool MerkaartorPreferences::getTranslateTags() const
{
	return Sets->value("locale/translatetags", true).toBool();
}

void MerkaartorPreferences::setTranslateTags(bool b)
{
	Sets->setValue("locale/translatetags",b);
}

QString getDefaultLanguage()
{
	QSettings Sets;
	return Sets.value("locale/language").toString();
}

void setDefaultLanguage(const QString& theValue)
{
	QSettings Sets;
	Sets.setValue("locale/language", theValue);
}

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

int MerkaartorPreferences::getFocusWidth() const
{
	return Sets->value("visual/FocusWidth",3).toInt();
}

int MerkaartorPreferences::getRelationsWidth() const
{
	return Sets->value("visual/RelationsWidth",3).toInt();
}

QColor MerkaartorPreferences::getBgColor() const
{
	QString sColor = Sets->value("visual/BgColor").toString();
	if (sColor.isEmpty())
		return Qt::white;
	return Sets->value("visual/BgColor").value<QColor>();
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

QColor MerkaartorPreferences::getRelationsColor() const
{
	QString sColor = Sets->value("visual/RelationsColor").toString();
	if (sColor.isEmpty())
		return QColor(0, 170, 0);
	return Sets->value("visual/RelationsColor").value<QColor>();
}

void MerkaartorPreferences::setBgColor(const QColor theValue)
{
	Sets->setValue("visual/BgColor", QVariant(theValue));
}

void MerkaartorPreferences::setHoverColor(const QColor theValue, int Width)
{
	Sets->setValue("visual/HoverColor", QVariant(theValue));
	Sets->setValue("visual/HoverWidth", Width);
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

QString MerkaartorPreferences::getGpsPort() const
{
	return Sets->value("gps/port", "/dev/rfcomm0").toString();
}

void MerkaartorPreferences::setGpsPort(const QString & theValue)
{
	Sets->setValue("gps/port", theValue);
}

double MerkaartorPreferences::getMaxDistNodes() const
{
	return Sets->value("data/MaxDistNodes", 0.1).toDouble();
}

void MerkaartorPreferences::setMaxDistNodes(double theValue)
{
	Sets->setValue("data/MaxDistNodes", theValue);
}

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
M_PARAM_IMPLEMENT_STRING(DefaultStyle, style, ":/Styles/Classic.mas")
M_PARAM_IMPLEMENT_STRING(CustomStyle, style, "")
M_PARAM_IMPLEMENT_BOOL(DisableStyleForTracks, style, true)

/* Visual */
M_PARAM_IMPLEMENT_BOOL(BackgroundOverwriteStyle, visual, false)

/* Templates */
M_PARAM_IMPLEMENT_STRING(DefaultTemplate, templates, ":/Templates/default.mat")
M_PARAM_IMPLEMENT_STRING(CustomTemplate, templates, "")

M_PARAM_IMPLEMENT_BOOL(GpsSaveLog, gps, false)
M_PARAM_IMPLEMENT_BOOL(GpsMapCenter, gps, false)
M_PARAM_IMPLEMENT_STRING(GpsLogDir, gps, "")
M_PARAM_IMPLEMENT_BOOL(GpsSyncTime, gps, false)

M_PARAM_IMPLEMENT_BOOL(ResolveRelations, downloadosm, false)

M_PARAM_IMPLEMENT_BOOL(MapTooltip, visual, false)
M_PARAM_IMPLEMENT_BOOL(InfoOnHover, visual, true)
M_PARAM_IMPLEMENT_BOOL(ShowParents, visual, true)

/* World OSB manager */
M_PARAM_IMPLEMENT_STRING(LastWorldOsbDir, WOSB, "")

/* Mouse bevaviour */
#ifdef _MOBILE
	M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, true)
#else
	M_PARAM_IMPLEMENT_BOOL(MouseSingleButton, Mouse, false)
#endif

/* Custom Style */
#ifdef CUSTOM_STYLE
	M_PARAM_IMPLEMENT_BOOL(MerkaartorStyle, visual, true)
#else
	M_PARAM_IMPLEMENT_BOOL(MerkaartorStyle, visual, false)
#endif
