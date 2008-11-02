//
// C++ Interface: MerkaartorPreferences
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, bvh, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef MERKAARTORPREFERENCES_H
#define MERKAARTORPREFERENCES_H

#include <QtCore>
#include <QtCore/QSettings>
#include <QColor>

#include "Map/Coord.h"

class MainWindow;
class MapView;

//#define WORLD_COORDBOX CoordBox(Coord(1.3, -1.3), Coord(-1.3, 1.3))
#define WORLD_COORDBOX CoordBox(Coord(INT_MAX/2, -INT_MAX/2), Coord(-INT_MAX/2, INT_MAX/2))
#define BUILTIN_STYLES_DIR ":/Styles"
#define BUILTIN_TEMPLATES_DIR ":/Templates"
#define M_PREFS MerkaartorPreferences::instance()

#define M_PARAM_DECLARE_BOOL(Param) \
	void set##Param(bool theValue); \
	bool get##Param() const; 
#define M_PARAM_DECLARE_STRING(Param) \
	void set##Param(const QString & theValue); \
	QString get##Param() const; 

#define SAFE_DELETE(x) {delete (x); x = NULL;}

/**
	@author cbro <cbro@semperpax.com>
*/

enum ImageBackgroundType {
	Bg_None,
	Bg_Wms,
 	Bg_Tms
#ifdef YAHOO
 	, Bg_Yahoo
#endif
#ifdef YAHOO_ILLEGAL
	, Bg_Yahoo_illegal
#endif
#ifdef GOOGLE_ILLEGAL
	, Bg_Google_illegal
#endif
#ifdef MSLIVEMAP_ILLEGAL
	, Bg_MsVirtualEarth_illegal
#endif
};

enum ProjectionType {
 	Proj_Background,
	Proj_Merkaartor
};

enum ExportType {
	Export_All,
	Export_Viewport,
	Export_Selected
};

class WmsServer
{
	public:
		WmsServer();
		WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat);

	public:
		QString WmsName;
		QString WmsAdress;
		QString WmsPath;
		QString WmsLayers;
		QString WmsProjections;
		QString WmsStyles;
		QString WmsImgFormat;
};
typedef QMap<QString, WmsServer> WmsServerList;
typedef QMapIterator<QString, WmsServer> WmsServerListIterator;

class TmsServer
{
	public:
		TmsServer();
		TmsServer(QString Name, QString Adress, QString Path, int tileSize, int minZoom, int maxZoom);

	public:
		QString TmsName;
		QString TmsAdress;
		QString TmsPath;
		int TmsTileSize;
		int TmsMinZoom;
		int TmsMaxZoom;
};
typedef QMap<QString, TmsServer> TmsServerList;
typedef QMapIterator<QString, TmsServer> TmsServerListIterator;

#define TOOL_FIELD_SIZE 2
class Tool
{
	public:
		Tool();
		Tool(QString Name, QString Path);

	public:
		QString ToolName;
		QString ToolPath;
};
typedef QMap<QString, Tool> ToolList;
typedef QMapIterator<QString, Tool> ToolListIterator;


// Outside of merkaartorpreferences, because initializing it will need translations
// Classic chicken & egg problem.
QString getDefaultLanguage();
void setDefaultLanguage(const QString& L);

class MerkaartorPreferences : public QObject
{
Q_OBJECT
public:
	MerkaartorPreferences();
	~MerkaartorPreferences();

	static MerkaartorPreferences* instance() {
		if (!m_prefInstance) {
			m_prefInstance = new MerkaartorPreferences;
		}

		return m_prefInstance;
	}

	void save();

	bool use06Api() const;
	void setUse06Api(bool b);

	void setRightSideDriving(bool theValue);
	bool getRightSideDriving() const;

	void setDoubleRoadDistance(double theValue);
	double getDoubleRoadDistance() const;

	void setWorkingDir(const QString & theValue);
	QString getWorkingDir() const;

	void setProxyUse(bool theValue);
	bool getProxyUse() const;

	void setProxyHost(const QString & theValue);
	QString getProxyHost() const;

	void setBookmarks(const QStringList & theValue);
	QStringList getBookmarks() const;

	void setSelectedWmsServer(const QString & theValue);
	QString getSelectedWmsServer() const;
	WmsServerList* getWmsServers() const;

	void setSelectedTmsServer(const QString & theValue);
	QString getSelectedTmsServer() const;
	TmsServerList* getTmsServers() const;

	void setProxyPort(int theValue);
	int getProxyPort() const;

	void setBgVisible(bool theValue);
	bool getBgVisible() const;

	void setBgType(ImageBackgroundType theValue);
	ImageBackgroundType getBgType() const;
	QStringList getBgTypes();

	/* Tile Cache */
	void setCacheDir(const QString & theValue);
	QString getCacheDir() const;
	void setCacheSize(int theValue);
	int getCacheSize() const;

	/* Search */
	void setLastMaxSearchResults(int theValue);
	int getLastMaxSearchResults() const;
	void setLastSearchName(const QString & theValue);
	QString getLastSearchName() const;
	void setLastSearchKey(const QString & theValue);
	QString getLastSearchKey() const;
	void setLastSearchValue(const QString & theValue);
	QString getLastSearchValue() const;

	/* Visual */
	void setZoomInPerc(int theValue);
	int getZoomInPerc() const;
	void setZoomOutPerc(int theValue);
	int getZoomOutPerc() const;

	void setProjectionType(ProjectionType theValue);
	ProjectionType getProjectionType() const;
	QStringList getProjectionTypes();

	QStringList getAlphaList() const;
	void setAlphaList();
	qreal getAlpha(QString lvl);
	QHash<QString, qreal>* getAlphaPtr();

	QColor getBgColor() const;
	void setBgColor(const QColor theValue);
	QColor getFocusColor() const;
	int getFocusWidth() const;
	void setFocusColor(const QColor theValue, int width);
	QColor getHoverColor() const;
	int getHoverWidth() const;
	void setHoverColor(const QColor theValue, int width);
	QColor getRelationsColor() const;
	int getRelationsWidth() const;
	void setRelationsColor(const QColor theValue, int width);

	void setDownloadedVisible(bool theValue);
	bool getDownloadedVisible() const;

	void setScaleVisible(bool theValue);
	bool getScaleVisible() const;

	bool getStyleBackgroundVisible() const;
	void setStyleBackgroundVisible(bool theValue);
	bool getStyleForegroundVisible() const;
	void setStyleForegroundVisible(bool theValue);
	bool getStyleTouchupVisible() const;
	void setStyleTouchupVisible(bool theValue);
	void setNamesVisible(bool theValue);
	bool getNamesVisible() const;

	void setTrackPointsVisible(bool theValue);
	bool getTrackPointsVisible() const;

	void setTrackSegmentsVisible(bool theValue);
	bool getTrackSegmentsVisible() const;

	void setRelationsVisible(bool theValue);
	bool getRelationsVisible() const;

	/* MainWindow state */
	void saveMainWindowState(const class MainWindow * mainWindow);
	void restoreMainWindowState(class MainWindow * mainWindow) const;

	void setInitialPosition(MapView* vw);
	void initialPosition(MapView* vw);

	bool getDrawTileBoundary();

	/* Data */
	void setOsmWebsite(const QString & theValue);
	QString getOsmWebsite() const;

	void setOsmUser(const QString & theValue);
	QString getOsmUser() const;

	void setOsmPassword(const QString & theValue);
	QString getOsmPassword() const;

	void setGpsPort(const QString & theValue);
	QString getGpsPort() const;

	void setMaxDistNodes(double theValue);
	double getMaxDistNodes() const;

	void setAutoSaveDoc(bool theValue);
	bool getAutoSaveDoc() const;

	void setAutoExtractTracks(bool theValue);
	bool getAutoExtractTracks() const;

	/* Export Type */
	void setExportType(ExportType theValue);
	ExportType getExportType() const;

	/* Tools */
	ToolList* getTools() const;
	Tool getTool(QString toolName) const;

	QStringList getShortcuts() const;
	void setShortcuts(const QStringList & theValue);

	/* Recent */
	void setRecentOpen(const QStringList & theValue);
	QStringList getRecentOpen() const;
	void addRecentOpen(const QString & theValue);

	void setRecentImport(const QStringList & theValue);
	QStringList getRecentImport() const;
	void addRecentImport(const QString & theValue);

	/* Styles */
	M_PARAM_DECLARE_STRING(DefaultStyle)
	M_PARAM_DECLARE_STRING(CustomStyle)
	M_PARAM_DECLARE_BOOL(DisableStyleForTracks)

	/* Templates */
	M_PARAM_DECLARE_STRING(DefaultTemplate)
	M_PARAM_DECLARE_STRING(CustomTemplate)

	M_PARAM_DECLARE_BOOL(GpsSaveLog)
	M_PARAM_DECLARE_BOOL(GpsMapCenter)
	M_PARAM_DECLARE_STRING(GpsLogDir)
	M_PARAM_DECLARE_BOOL(GpsSyncTime)

	M_PARAM_DECLARE_BOOL(ResolveRelations)
	M_PARAM_DECLARE_BOOL(TranslateTags)

	M_PARAM_DECLARE_BOOL(MapTooltip)
	M_PARAM_DECLARE_BOOL(InfoOnHover)
	M_PARAM_DECLARE_BOOL(ShowParents)

protected:
	bool Use06Api;
	QString version;
	bool RightSideDriving;
	double DoubleRoadDistance;
	QString WorkingDir;
	QString OsmWebsite;
	QString OsmUser;
	QString OsmPassword;
	bool ProxyUse;
	QString ProxyHost;
	int ProxyPort;
	QStringList Bookmarks;

	void setWmsServers();
	void setTmsServers();
	void setTools();
	void initialize();

private:
	QHash<QString, qreal> alpha;
	WmsServerList* theWmsServerList;
	TmsServerList* theTmsServerList;
	ToolList* theToolList;
	QSettings * Sets;
	QStringList bgTypes;
	QStringList projTypes;
	static MerkaartorPreferences* m_prefInstance;
};

#endif
