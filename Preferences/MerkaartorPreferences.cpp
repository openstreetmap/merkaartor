//
// C++ Implementation: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "MerkaartorPreferences.h"

MerkaartorPreferences* MerkaartorPreferences::m_prefInstance = 0;

WmsServer::WmsServer()
{
	WmsServer("New Server", "", "", "", "", "", "");
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat)
	: WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles), WmsImgFormat(ImgFormat)
{
	if (Name == "") {
		WmsName = "New Server";
	}
}

MerkaartorPreferences::MerkaartorPreferences()
{
	Sets = new QSettings();
	theWmsServerList = new WmsServerList();

	version = Sets->value("version/version", "0").toString();
	initialize();
}

MerkaartorPreferences::~MerkaartorPreferences()
{
	delete Sets;
}

void MerkaartorPreferences::save()
{
	Sets->setValue("version/version", QString("%1.%2.%3").arg(MAJORVERSION).arg(MINORVERSION).arg(REVISION));
	setWmsServers();
 	Sets->sync();
}

void MerkaartorPreferences::initialize()
{
	bgTypes.insert(Bg_None, "None");
	bgTypes.insert(Bg_Wms, "WMS adapter");
	bgTypes.insert(Bg_OSM, "OSM adapter");
#ifdef yahoo_illegal
	bgTypes.insert(Bg_Yahoo_illegal, "Illegal Yahoo adapter");
#endif
#ifdef google_illegal
	bgTypes.insert(Bg_Google_illegal, "Illegal Google adapter");
#endif

	QStringList sl = getBookmarks();
	if (sl.size() == 0) {
		QStringList DefaultBookmarks;
		DefaultBookmarks << "London" << "51.47" << "-0.20" << "51.51" << "-0.08";
	//	DefaultBookmarks << "Rotterdam" << "51.89" << "4.43" << "51.93" << "4.52";
		setBookmarks(DefaultBookmarks);
	}

	if (version == "0") {
		QStringList Servers = Sets->value("WSM/servers").toStringList();
		if (Servers.size() == 0) {
			WmsServer demis("Demis", "www2.demis.nl", "/wms/wms.asp?wms=WorldMap&",
				"Countries,Borders,Highways,Roads,Cities", "EPSG:4326", ",", "image/png");
			theWmsServerList->insert("Demis", demis);
			WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
				"world", "EPSG:4326", ",", "image/jpeg");
			theWmsServerList->insert("OpenAerialMap", oam);
			WmsServer tu("Terraservice_Urban", "terraservice.net", "/ogcmap.ashx?",
				"urbanarea", "EPSG:4326", ",", "image/jpeg");
			theWmsServerList->insert("Terraservice_Urban", tu);
			WmsServer tg("Terraservice_DRG", "terraservice.net", "/ogcmap.ashx?",
				"drg", "EPSG:4326", ",", "image/jpeg");
			theWmsServerList->insert("Terraservice_DRG", tg);
			WmsServer tq("Terraservice_DOQ", "terraservice.net", "/ogcmap.ashx?",
				"doq", "EPSG:4326", ",", "image/jpeg");
			theWmsServerList->insert("Terraservice_DOQ", tq);
			MerkaartorPreferences::instance()->setSelectedWmsServer("OpenAerialMap");
		} else {
			int selI = Sets->value("WSM/selected", "0").toInt();
			QString selS;
			for (int i=0; i<Servers.size()/6; i++) {
				WmsServer S(Servers[(i*6)], Servers[(i*6)+1], Servers[(i*6)+2], Servers[(i*6)+3], Servers[(i*6)+4], Servers[(i*6)+5], "image/png");
				theWmsServerList->insert(Servers[(i*6)], S);
				if (i == selI)
					selS = Servers[(i*6)];
			}
			setSelectedWmsServer(selS);
			if (!theWmsServerList->contains("OpenAerialMap")) {
				WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
					"world", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("OpenAerialMap", oam);
				WmsServer tu("Terraservice_Urban", "terraservice.net", "/ogcmap.ashx?",
					"urbanarea", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("Terraservice_Urban", tu);
				WmsServer tg("Terraservice_DRG", "terraservice.net", "/ogcmap.ashx?",
					"drg", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("Terraservice_DRG", tg);
				WmsServer tq("Terraservice_DOQ", "terraservice.net", "/ogcmap.ashx?",
					"doq", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("Terraservice_DOQ", tq);
			}
		}
		save();
	} else {
		QStringList Servers = Sets->value("WSM/servers").toStringList();
		for (int i=0; i<Servers.size(); i+=7) {
			WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6]);
			theWmsServerList->insert(Servers[i], S);
		}
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

QString MerkaartorPreferences::getOsmWebsite() const
{
	return Sets->value("osm/Website", "www.openstreetmap.org").toString();
}

void MerkaartorPreferences::setOsmWebsite(const QString & theValue)
{
	Sets->setValue("osm/Website", theValue);
}

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

QStringList MerkaartorPreferences::getBookmarks() const
{
	return Sets->value("downloadosm/bookmarks").toStringList();
}

void MerkaartorPreferences::setBookmarks(const QStringList & theValue)
{
	Sets->setValue("downloadosm/bookmarks", theValue);
}

WmsServerList* MerkaartorPreferences::getWmsServers() const
{
//	return Sets->value("WSM/servers").toStringList();
	return theWmsServerList;
}

void MerkaartorPreferences::setWmsServers()
{
	QStringList Servers;
	WmsServerListIterator i(*theWmsServerList);
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

QString MerkaartorPreferences::getDefaultStyle() const
{
	return Sets->value("style/Default",":/Styles/Classic.mas").toString();
}

void MerkaartorPreferences::setDefaultStyle(const QString& S)
{
	Sets->setValue("style/Default",S);
}
