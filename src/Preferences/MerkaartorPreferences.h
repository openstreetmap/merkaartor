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
#include <QHttp>
#include <QBuffer>
#include <QUuid>
#include <QNetworkProxy>

#include "Maps/Coord.h"
#include "Preferences/WmsServersList.h"
#include "Preferences/TmsServersList.h"
#include "Preferences/ProjectionsList.h"
#include "Preferences/BookmarksList.h"

class MainWindow;
class MapView;
class IMapAdapter;

//#define WORLD_COORDBOX CoordBox(Coord(1.3, -1.3), Coord(-1.3, 1.3))
#define WORLD_COORDBOX CoordBox(Coord(INT_MAX/4, -INT_MAX/4), Coord(-INT_MAX/4, INT_MAX/4))
#define BUILTIN_STYLES_DIR ":/Styles"
#define BUILTIN_TEMPLATES_DIR ":/Templates"
#define M_PREFS MerkaartorPreferences::instance()

#define NONE_ADAPTER_UUID QUuid("{8F5D3625-F987-45c5-A50B-17D88384F97D}")
#define SHAPE_ADAPTER_UUID QUuid("{AFB0324E-34D0-4267-BB8A-CF56CD2D7012}")
#define WMS_ADAPTER_UUID QUuid("{E238750A-AC27-429e-995C-A60C17B9A1E0}")
#define TMS_ADAPTER_UUID QUuid("{CA8A07EC-A466-462b-929F-3805BC9DEC95}")

#define HOMEDIR QDir::homePath() + "/.merkaartor"

#define M_PARAM_DECLARE_BOOL(Param) \
	private: \
		bool m_##Param; \
	public: \
		void set##Param(bool theValue); \
		bool get##Param();
#define M_PARAM_DECLARE_STRING(Param) \
	private: \
		QString m_##Param; \
	public: \
		void set##Param(QString theValue); \
		QString get##Param();
#define M_PARAM_DECLARE_INT(Param) \
	private: \
		int m_##Param; \
	public: \
		void set##Param(int theValue); \
		int get##Param();
#define M_PARAM_DECLARE_DOUBLE(Param) \
	private: \
		double m_##Param; \
	public: \
		void set##Param(double theValue); \
		double get##Param();

#define SAFE_DELETE(x) {delete (x); x = NULL;}
#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

/**
	@author cbro <cbro@semperpax.com>
*/

#ifndef _MOBILE
typedef QString ProjectionType;
#endif

enum ExportType {
	Export_All,
	Export_Viewport,
	Export_Selected
};

enum DirectionalArrowsShow {
	DirectionalArrows_Never,
	DirectionalArrows_Oneway,
	DirectionalArrows_Always
};

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

	void save(bool UserPwdChanged = false);

	void toOsmPref();
	void fromOsmPref();
	void putOsmPref(const QString& k, const QString& v);
	void deleteOsmPref(const QString& k);

	const QVector<qreal> getParentDashes() const;

	//bool use06Api() const;
	void setUse06Api(bool b);
	const QString apiVersion() const;
	double apiVersionNum() const;

	void setRightSideDriving(bool theValue);
	bool getRightSideDriving() const;

	void setDoubleRoadDistance(double theValue);
	double getDoubleRoadDistance() const;

	void setWorkingDir(const QString & theValue);
	QString getWorkingDir() const;

	BookmarkList*  getBookmarks();

	WmsServerList* getWmsServers();
	TmsServerList* getTmsServers();
	void setSelectedServer(const QString & theValue);
	QString getSelectedServer() const;

	void setBgVisible(bool theValue);
	bool getBgVisible() const;

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
	QStringList getAlphaList() const;
	void setAlphaList();
	qreal getAlpha(QString lvl);
	QHash<QString, qreal>* getAlphaPtr();

	QColor getBgColor() const;
	void setBgColor(const QColor theValue);
	QColor getWaterColor() const;
	void setWaterColor(const QColor theValue);
	QColor getFocusColor() const;
	int getFocusWidth() const;
	void setFocusColor(const QColor theValue, int width);
	QColor getHoverColor() const;
	int getHoverWidth() const;
	void setHoverColor(const QColor theValue, int width);
	QColor getHighlightColor() const;
	int getHighlightWidth() const;
	void setHighlightColor(const QColor theValue, int width);
	QColor getRelationsColor() const;
	int getRelationsWidth() const;
	void setRelationsColor(const QColor theValue, int width);
	QColor getGpxTrackColor() const;
	int getGpxTrackWidth() const;
	void setGpxTrackColor(const QColor theValue, int width);

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

	DirectionalArrowsShow getDirectionalArrowsVisible();
	void setDirectionalArrowsVisible(DirectionalArrowsShow theValue);

	void setTrackPointsVisible(bool theValue);
	bool getTrackPointsVisible() const;

	void setTrackSegmentsVisible(bool theValue);
	bool getTrackSegmentsVisible() const;

	void setRelationsVisible(bool theValue);
	bool getRelationsVisible() const;

	M_PARAM_DECLARE_INT(TagListFirstColumnWidth)

	/* MainWindow state */
	void saveMainWindowState(const class MainWindow * mainWindow);
	void restoreMainWindowState(class MainWindow * mainWindow) const;

	void setInitialPosition(MapView* vw);
	void initialPosition(MapView* vw);

	bool getDrawTileBoundary();

	/* Data */
	void setOsmWebsite(const QString & theValue);
	QString getOsmWebsite() const;

	M_PARAM_DECLARE_STRING(XapiWebSite)
	M_PARAM_DECLARE_BOOL(AutoHistoryCleanup)

	void setOsmUser(const QString & theValue);
	QString getOsmUser() const;

	void setOsmPassword(const QString & theValue);
	QString getOsmPassword() const;

	M_PARAM_DECLARE_DOUBLE(MaxDistNodes)

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

	/* Visual */
    M_PARAM_DECLARE_INT(ZoomIn)
    M_PARAM_DECLARE_INT(ZoomOut)
    M_PARAM_DECLARE_BOOL(ZoomBoris)
    M_PARAM_DECLARE_BOOL(BackgroundOverwriteStyle)
	M_PARAM_DECLARE_INT(AreaOpacity)
	M_PARAM_DECLARE_BOOL(UseShapefileForBackground)
	M_PARAM_DECLARE_BOOL(DrawingHack)
	M_PARAM_DECLARE_BOOL(SimpleGpxTrack)
	M_PARAM_DECLARE_BOOL(VirtualNodesVisible)
	M_PARAM_DECLARE_BOOL(UseVirtualNodes)
	M_PARAM_DECLARE_BOOL(RelationsSelectableWhenHidden)

	/* Templates */
	M_PARAM_DECLARE_STRING(DefaultTemplate)
	M_PARAM_DECLARE_STRING(CustomTemplate)

	/* GPS */
	M_PARAM_DECLARE_BOOL(GpsUseGpsd)
	M_PARAM_DECLARE_STRING(GpsPort)
	M_PARAM_DECLARE_STRING(GpsdHost)
	M_PARAM_DECLARE_INT(GpsdPort)
	M_PARAM_DECLARE_BOOL(GpsSaveLog)
	M_PARAM_DECLARE_BOOL(GpsMapCenter)
	M_PARAM_DECLARE_STRING(GpsLogDir)
	M_PARAM_DECLARE_BOOL(GpsSyncTime)

	M_PARAM_DECLARE_BOOL(ResolveRelations)
	M_PARAM_DECLARE_BOOL(DeleteIncompleteRelations)

	M_PARAM_DECLARE_BOOL(TranslateTags)

	M_PARAM_DECLARE_BOOL(MapTooltip)
	M_PARAM_DECLARE_BOOL(InfoOnHover)
	M_PARAM_DECLARE_BOOL(ShowParents)

	/* World OSB manager */
	M_PARAM_DECLARE_DOUBLE(TileToRegionThreshold)

	M_PARAM_DECLARE_STRING(WorldOsbUri)
	M_PARAM_DECLARE_BOOL(WorldOsbAutoload)
	M_PARAM_DECLARE_BOOL(WorldOsbAutoshow)

	/* Mouse bevaviour */
	M_PARAM_DECLARE_BOOL(MouseSingleButton)
	M_PARAM_DECLARE_BOOL(SeparateMoveMode)

	/* Custom Style */
	M_PARAM_DECLARE_BOOL(MerkaartorStyle)
	M_PARAM_DECLARE_STRING(MerkaartorStyleString)

	/* Network */
	M_PARAM_DECLARE_BOOL(OfflineMode)

	/* Proxy */
	QNetworkProxy getProxy(const QUrl & requestUrl);

	M_PARAM_DECLARE_BOOL(ProxyUse)
	M_PARAM_DECLARE_STRING(ProxyHost)
	M_PARAM_DECLARE_INT(ProxyPort)
	M_PARAM_DECLARE_STRING(ProxyUser)
	M_PARAM_DECLARE_STRING(ProxyPassword)

	/* Track */
	M_PARAM_DECLARE_BOOL(ReadonlyTracksDefault)

	/* FeaturesDock */
	M_PARAM_DECLARE_BOOL(FeaturesWithin)

	/* Plugins */
	void addBackgroundPlugin(IMapAdapter* aPlugin);
	void setBackgroundPlugin(const QUuid& theValue);
	QUuid getBackgroundPlugin() const;
	IMapAdapter* getBackgroundPlugin(const QUuid& anAdapterUid);
	QMap<QUuid, IMapAdapter *> getBackgroundPlugins();

	/* Projections */
	void loadProjection(QString fn);
	void loadProjections();
	void saveProjections();

	/* WMSes */
	void loadWMS(QString fn);
	void loadWMSes();
	void saveWMSes();

	/* TMSes */
	void loadTMS(QString fn);
	void loadTMSes();
	void saveTMSes();

	/* Bookmarks */
	void loadBookmark(QString fn);
	void loadBookmarks();
	void saveBookmarks();

#ifndef _MOBILE
	void setProjectionType(ProjectionType theValue);
    ProjectionType getProjectionType();
	ProjectionsList getProjectionsList();
	ProjectionItem getProjection(QString aProj);
#endif



protected:
	QVector<qreal> parentDashes;

	bool Use06Api;
	QString version;
	bool RightSideDriving;
	double DoubleRoadDistance;
	QString WorkingDir;
	QString OsmWebsite;
	QString OsmUser;
	QString OsmPassword;

	QHttp httpRequest;
	int OsmPrefLoadId;
	int OsmPrefSaveId;
	int OsmPrefDeleteId;
	QBuffer OsmPrefContent;
	QMap<QString, int> OsmPrefListsCount;

	void setWmsServers();
	void setTmsServers();
	void setTools();
	void initialize();

private:
	QHash<QString, qreal> alpha;
	ToolList* theToolList;
	QSettings * Sets;
	QStringList projTypes;
	QMap<QUuid, IMapAdapter *> mBackgroundPlugins;
	ProjectionsList theProjectionsList;
	WmsServersList theWmsServerList;
	TmsServersList theTmsServerList;
	BookmarksList theBookmarkList;

	static MerkaartorPreferences* m_prefInstance;

private slots:
	void on_responseHeaderReceived(const QHttpResponseHeader & hdr);
	void on_requestFinished ( int id, bool error );

signals:
	void bookmarkChanged();
};

#endif
