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

#include <QtGui/QApplication>
#include <QtGui/QMainWindow>

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
	theWmsServerList = new WmsServerList();
	theTmsServerList = new TmsServerList();
	theToolList = new ToolList();

	version = Sets->value("version/version", "0").toString();
	initialize();
}

MerkaartorPreferences::~MerkaartorPreferences()
{
	delete theWmsServerList;
	delete theTmsServerList;
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

	QStringList sl = getBookmarks();
	if (sl.size() == 0) {
		QStringList DefaultBookmarks;
		DefaultBookmarks << "London" << "51.47" << "-0.20" << "51.51" << "-0.08";
	//	DefaultBookmarks << "Rotterdam" << "51.89" << "4.43" << "51.93" << "4.52";
		setBookmarks(DefaultBookmarks);
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
				theWmsServerList->insert(Servers[(i*6)], S);
				if (i == selI)
					selS = Servers[(i*6)];
			}
			setSelectedWmsServer(selS);
			if (!theWmsServerList->contains("OpenAerialMap")) {
				WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
					"world", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("OpenAerialMap", oam);
				WmsServer otm("OpenTopoMap", "opentopomap.org", "/wms/?",
					"world", "EPSG:4326", ",", "image/jpeg");
				theWmsServerList->insert("OpenTopoMap", otm);
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
	}
	QStringList Servers = Sets->value("WSM/servers").toStringList();
	if (Servers.size() == 0) {
		WmsServer demis("Demis", "www2.demis.nl", "/wms/wms.asp?wms=WorldMap&",
						"Countries,Borders,Highways,Roads,Cities", "EPSG:4326", ",", "image/png");
		theWmsServerList->insert("Demis", demis);
		WmsServer oam("OpenAerialMap", "openaerialmap.org", "/wms/?",
						"world", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList->insert("OpenAerialMap", oam);
		WmsServer otm("OpenTopoMap", "opentopomap.org", "/wms/?",
						"world", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList->insert("OpenTopoMap", otm);
		WmsServer tu("Terraservice_Urban", "terraservice.net", "/ogcmap.ashx?",
						"urbanarea", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList->insert("Terraservice_Urban", tu);
		WmsServer tg("Terraservice_DRG", "terraservice.net", "/ogcmap.ashx?",
						"drg", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList->insert("Terraservice_DRG", tg);
		WmsServer tq("Terraservice_DOQ", "terraservice.net", "/ogcmap.ashx?",
						"doq", "EPSG:4326", ",", "image/jpeg");
		theWmsServerList->insert("Terraservice_DOQ", tq);
		setSelectedWmsServer("OpenAerialMap");
		save();
	}
	for (int i=0; i<Servers.size(); i+=7) {
		WmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3], Servers[i+4], Servers[i+5], Servers[i+6]);
		theWmsServerList->insert(Servers[i], S);
	}
	Servers = Sets->value("TMS/servers").toStringList();
	if (Servers.size() == 0) {
		TmsServer osmmapnik("OSM Mapnik", "tile.openstreetmap.org", "/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList->insert("OSM Mapnik", osmmapnik);
		TmsServer osmth("OSM T@H", "tah.openstreetmap.org", "/Tiles/tile/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList->insert("OSM T@H", osmth);
		TmsServer cycle("Cycle Map", "andy.sandbox.cloudmade.com", "/tiles/cycle/%1/%2/%3.png", 256, 0, 14);
		theTmsServerList->insert("Gravitystorm Cycle", cycle);
		TmsServer oam("OpenAerialMap", "tile.openaerialmap.org", "/tiles/1.0.0/openaerialmap-900913/%1/%2/%3.png", 256, 0, 17);
		theTmsServerList->insert("OpenAerialMap", oam);
		setSelectedTmsServer("OSM Mapnik");
		save();
	}
	for (int i=0; i<Servers.size(); i+=6) {
		TmsServer S(Servers[i], Servers[i+1], Servers[i+2], Servers[i+3].toInt(), Servers[i+4].toInt(), Servers[i+5].toInt());
		theTmsServerList->insert(Servers[i], S);
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

QStringList MerkaartorPreferences::getBookmarks() const
{
	return Sets->value("downloadosm/bookmarks").toStringList();
}

void MerkaartorPreferences::setBookmarks(const QStringList & theValue)
{
	Sets->setValue("downloadosm/bookmarks", theValue);
}

/* WMS */

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

/* TMS */

TmsServerList* MerkaartorPreferences::getTmsServers() const
{
//	return Sets->value("WSM/servers").toStringList();
	return theTmsServerList;
}

void MerkaartorPreferences::setTmsServers()
{
	QStringList Servers;
	TmsServerListIterator i(*theTmsServerList);
	while (i.hasNext()) {
		i.next();
		TmsServer S = i.value();
		Servers.append(S.TmsName);
		Servers.append(S.TmsAdress);
		Servers.append(S.TmsPath);
		Servers.append(QString().setNum(S.TmsTileSize));
		Servers.append(QString().setNum(S.TmsMinZoom));
		Servers.append(QString().setNum(S.TmsMaxZoom));
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

QString MerkaartorPreferences::getCustomStyle() const
{
	return Sets->value("style/Custom","").toString();
}

void MerkaartorPreferences::setCustomStyle(const QString& S)
{
	Sets->setValue("style/Custom",S);
}

QString MerkaartorPreferences::getDefaultStyle() const
{
	return Sets->value("style/Default",":/Styles/Classic.mas").toString();
}

void MerkaartorPreferences::setDefaultStyle(const QString& S)
{
	Sets->setValue("style/Default",S);
}


void MerkaartorPreferences::setDisableStyleForTracks(bool theValue)
{
	Sets->setValue("style/DisableStyleForTracks", theValue);
}

bool MerkaartorPreferences::getDisableStyleForTracks() const
{
	return Sets->value("style/DisableStyleForTracks", true).toBool();
}

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

void MerkaartorPreferences::saveMainWindowState(const QMainWindow * mainWindow)
{
	Sets->setValue("MainWindow/Position", mainWindow->pos());
	Sets->setValue("MainWindow/Size", mainWindow->size());
	Sets->setValue("MainWindow/State", mainWindow->saveState());
}

void MerkaartorPreferences::restoreMainWindowState(QMainWindow * mainWindow) const
{
	if (Sets->contains("MainWindow/Position"))
		mainWindow->move( Sets->value("MainWindow/Position").toPoint());

	if (Sets->contains("MainWindow/Size"))
		mainWindow->resize( Sets->value("MainWindow/Size").toSize());

	if (Sets->contains("MainWindow/State"))
		mainWindow->restoreState( Sets->value("MainWindow/State").toByteArray() );
}

void MerkaartorPreferences::setInitialPosition(const CoordBox & cb)
{
	QStringList ip;
	ip.append(QString::number(intToAng(cb.bottomLeft().lat())));
	ip.append(QString::number(intToAng(cb.bottomLeft().lon())));
	ip.append(QString::number(intToAng(cb.topRight().lat())));
	ip.append(QString::number(intToAng(cb.topRight().lon())));

	Sets->setValue("MainWindow/InitialPosition", ip);
}

CoordBox MerkaartorPreferences::getInitialPosition()
{
	if (Sets->contains("MainWindow/InitialPosition") == false)
		return WORLD_COORDBOX;

	const QStringList & ip = Sets->value("MainWindow/InitialPosition").toStringList();

	const Coord bottomLeft(angToInt(ip[0].toDouble()), angToInt(ip[1].toDouble()));
	const Coord topRight(angToInt(ip[2].toDouble()),angToInt(ip[3].toDouble()));

	return CoordBox(bottomLeft, topRight);
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

QStringList MerkaartorPreferences::getRecentImport() const
{
	return Sets->value("recent/import").toStringList();
}

void MerkaartorPreferences::setRecentImport(const QStringList & theValue)
{
	Sets->setValue("recent/import", theValue);
}

QStringList MerkaartorPreferences::getShortcuts() const
{
	return Sets->value("Tools/shortcuts").toStringList();
}

void MerkaartorPreferences::setShortcuts(const QStringList & theValue)
{
	Sets->setValue("Tools/shortcuts", theValue);
}

M_PARAM_IMPLEMENT_BOOL(GpsSaveLog, gps, false)
M_PARAM_IMPLEMENT_BOOL(GpsMapCenter, gps, false)
M_PARAM_IMPLEMENT_STRING(GpsLogDir, gps, "")
M_PARAM_IMPLEMENT_BOOL(GpsSyncTime, gps, false)

M_PARAM_IMPLEMENT_BOOL(ResolveRelations, downloadosm, false)
