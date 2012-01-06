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

#ifdef USE_LIBPROXY
#include <proxy.h>
#endif

#include "Coord.h"
#include "WmsServersList.h"
#include "TmsServersList.h"
#include "ProjectionsList.h"
#include "BookmarksList.h"
#include "FilterList.h"

#include "IRenderer.h"

class MainWindow;
class MapView;
class IMapAdapterFactory;
class IPaintStyle;

#ifdef Q_WS_X11
#define PLATFORM "X11"
#else
#ifdef Q_WS_WIN
#define PLATFORM "Windows"
#else
#ifdef Q_WS_MACX
#define PLATFORM "Macintosh"
#else
#define PLATFORM "Other"
#endif
#endif
#endif
#define USER_AGENT (QString("%1/%2%3 (%4;%5)").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(PLATFORM).arg(STRINGIFY(SVNREV)))

#define WORLD_COORDBOX CoordBox(Coord(-COORD_MAX*.80, COORD_MAX*.80/2), Coord(COORD_MAX*.80, -COORD_MAX*.80/2))
#define BUILTIN_STYLES_DIR ":/Styles"
#define BUILTIN_TEMPLATES_DIR ":/Templates"

#define M_PREFS MerkaartorPreferences::instance()
#define M_STYLE MerkaartorPreferences::styleinstance()

#define HOMEDIR (g_Merk_Portable ? qApp->applicationDirPath() : QDir::homePath() + "/.merkaartor")
#define SHAREDIR (g_Merk_Portable ? qApp->applicationDirPath() : STRINGIFY(SHARE_DIR))
#define TEMPLATE_DOCUMENT (HOMEDIR + "/Startup.mdc")

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
        void set##Param(const QString& theValue); \
        QString get##Param();
#define M_PARAM_DECLARE_STRINGList(Param) \
    private: \
        QStringList m_##Param; \
    public: \
        void set##Param(const QStringList& theValue); \
        QStringList& get##Param();
#define M_PARAM_DECLARE_INT(Param) \
    private: \
        int m_##Param; \
    public: \
        void set##Param(int theValue); \
        int get##Param();
#define M_PARAM_DECLARE_INT_DELAYED(Param) \
    private: \
        int m_##Param; \
    public: \
        void set##Param(int theValue); \
        void save##Param(); \
        int get##Param();
#define M_PARAM_DECLARE_DOUBLE(Param) \
    private: \
        qreal m_##Param; \
    public: \
        void set##Param(qreal theValue); \
        qreal get##Param();
#define M_PARAM_DECLARE_COLOR(Param) \
    private: \
        QColor m_##Param; \
    public: \
        void set##Param(const QColor& theValue); \
        QColor get##Param();

#define SAFE_DELETE(x) {delete (x); x = NULL;}
#define STRINGIFY(x) XSTRINGIFY(x)
#define XSTRINGIFY(x) #x

/**
    @author cbro <cbro@semperpax.com>
*/

typedef QString FilterType;

enum ExportType {
    Export_All,
    Export_Viewport,
    Export_Selected
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

struct OsmServer
{
    bool Selected;
    QString Url;
    QString User;
    QString Password;
};
typedef QList<OsmServer> OsmServerList;
typedef QListIterator<OsmServer> OsmServerIterator;

// Outside of merkaartorpreferences, because initializing it will need translations
// Classic chicken & egg problem.
QString getDefaultLanguage(bool returnDefault=true);
void setDefaultLanguage(const QString& L);

class MerkaartorPreferences : public QObject
{
Q_OBJECT
public:
    static MerkaartorPreferences* instance();
    static IPaintStyle* styleinstance();

    MerkaartorPreferences();
    ~MerkaartorPreferences();

    QSettings* getQSettings() const { return Sets; }
    void save(bool UserPwdChanged = false);

    void toOsmPref();
    void fromOsmPref();
    void putOsmPref(const QString& k, const QString& v);
    void deleteOsmPref(const QString& k);

    const QVector<qreal> getParentDashes() const;

    //bool use06Api() const;
    void setUse06Api(bool b);
    const QString apiVersion() const;
    qreal apiVersionNum() const;

    M_PARAM_DECLARE_BOOL(rightsidedriving);
    M_PARAM_DECLARE_DOUBLE(doubleroaddistance);
    M_PARAM_DECLARE_STRING(workingdir);

    void setBgVisible(bool theValue);
    bool getBgVisible() const;

    /* Tile Cache */
    M_PARAM_DECLARE_STRING(CacheDir);
    M_PARAM_DECLARE_INT(CacheSize);

    /* Search */
    M_PARAM_DECLARE_INT(LastMaxSearchResults);
    M_PARAM_DECLARE_STRING(LastSearchName);
    M_PARAM_DECLARE_STRING(LastSearchKey);
    M_PARAM_DECLARE_STRING(LastSearchValue);
    M_PARAM_DECLARE_STRING(LastSearchTagSelector);

    /* Visual */
    QStringList getAlphaList() const;
    void setAlphaList();
    qreal getAlpha(QString lvl);
    QHash<QString, qreal>* getAlphaPtr();

    M_PARAM_DECLARE_INT(HoverWidth);
    M_PARAM_DECLARE_INT(HighlightWidth);
    M_PARAM_DECLARE_INT(DirtyWidth);
    M_PARAM_DECLARE_INT(FocusWidth);
    M_PARAM_DECLARE_INT(RelationsWidth);
    M_PARAM_DECLARE_INT(GpxTrackWidth);

    M_PARAM_DECLARE_COLOR(BgColor)
    M_PARAM_DECLARE_COLOR(WaterColor)
    M_PARAM_DECLARE_COLOR(FocusColor);
    M_PARAM_DECLARE_COLOR(HoverColor);
    M_PARAM_DECLARE_COLOR(HighlightColor);
    M_PARAM_DECLARE_COLOR(DirtyColor);
    M_PARAM_DECLARE_COLOR(RelationsColor);
    M_PARAM_DECLARE_COLOR(GpxTrackColor);

    M_PARAM_DECLARE_INT(DirectionalArrowsVisible);
    RendererOptions getRenderOptions();

    M_PARAM_DECLARE_INT_DELAYED(TagListFirstColumnWidth)

    /* MainWindow state */
    void saveMainWindowState(const class MainWindow * mainWindow);
    void restoreMainWindowState(class MainWindow * mainWindow) const;

    void setInitialPosition(MapView* vw);
    void initialPosition(MapView* vw);

    bool getDrawTileBoundary();

    /* Data */
    void setOsmWebsite(const QString & theValue);
    QString getOsmWebsite() const;
    QString getOsmApiUrl() const;

    M_PARAM_DECLARE_STRING(XapiUrl)
    M_PARAM_DECLARE_STRING(NominatimUrl)
    M_PARAM_DECLARE_BOOL(AutoHistoryCleanup)

    void setOsmUser(const QString & theValue);
    QString getOsmUser() const;

    void setOsmPassword(const QString & theValue);
    QString getOsmPassword() const;

    M_PARAM_DECLARE_DOUBLE(MaxDistNodes);

    M_PARAM_DECLARE_BOOL(AutoSaveDoc);
    M_PARAM_DECLARE_BOOL(AutoExtractTracks);

    /* Export Type */
    void setExportType(ExportType theValue);
    ExportType getExportType() const;

    /* Tools */
    ToolList* getTools() const;
    Tool getTool(QString toolName) const;

    QStringList getShortcuts() const;
    void setShortcuts(const QStringList & theValue);

    M_PARAM_DECLARE_INT(PolygonSides)

    /* Recent */
    void setRecentOpen(const QStringList & theValue);
    QStringList getRecentOpen() const;
    void addRecentOpen(const QString & theValue);

    void setRecentImport(const QStringList & theValue);
    QStringList getRecentImport() const;
    void addRecentImport(const QString & theValue);

    /* Rendering */
    M_PARAM_DECLARE_BOOL(UseAntiAlias)
    M_PARAM_DECLARE_BOOL(AntiAliasWhilePanning)
    M_PARAM_DECLARE_STRING(DefaultStyle)
    M_PARAM_DECLARE_STRING(CustomStyle)
    M_PARAM_DECLARE_BOOL(DisableStyleForTracks)
    M_PARAM_DECLARE_STRINGList(TechnicalTags)

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
    M_PARAM_DECLARE_DOUBLE(LocalZoom)
    M_PARAM_DECLARE_DOUBLE(RegionalZoom)
    M_PARAM_DECLARE_INT(NodeSize)

    M_PARAM_DECLARE_BOOL(DownloadedVisible)
    M_PARAM_DECLARE_BOOL(ScaleVisible)
    M_PARAM_DECLARE_BOOL(LatLonGridVisible)
    M_PARAM_DECLARE_BOOL(BackgroundVisible)
    M_PARAM_DECLARE_BOOL(ForegroundVisible)
    M_PARAM_DECLARE_BOOL(TouchupVisible)
    M_PARAM_DECLARE_BOOL(NamesVisible)
    M_PARAM_DECLARE_BOOL(TrackPointsVisible)
    M_PARAM_DECLARE_BOOL(TrackSegmentsVisible)
    M_PARAM_DECLARE_BOOL(RelationsVisible)
    M_PARAM_DECLARE_BOOL(PhotosVisible)
    M_PARAM_DECLARE_BOOL(DirtyVisible)

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

    /* Background */
    M_PARAM_DECLARE_BOOL(AutoSourceTag)

    /* Mouse bevaviour */
    M_PARAM_DECLARE_BOOL(MouseSingleButton)
    M_PARAM_DECLARE_BOOL(SeparateMoveMode)
    M_PARAM_DECLARE_BOOL(SelectModeCreation)

    // Geotag
    M_PARAM_DECLARE_INT(MaxGeoPicWidth)

    /* Custom Style */
    M_PARAM_DECLARE_BOOL(MerkaartorStyle)
    M_PARAM_DECLARE_STRING(MerkaartorStyleString)

    /* Network */
    M_PARAM_DECLARE_BOOL(OfflineMode)
    M_PARAM_DECLARE_BOOL(LocalServer)
    M_PARAM_DECLARE_INT(NetworkTimeout)

    /* Proxy */
    QNetworkProxy getProxy(const QUrl & requestUrl);

    M_PARAM_DECLARE_BOOL(ProxyUse)
    M_PARAM_DECLARE_STRING(ProxyHost)
    M_PARAM_DECLARE_INT(ProxyPort)
    M_PARAM_DECLARE_STRING(ProxyUser)
    M_PARAM_DECLARE_STRING(ProxyPassword)

    /* Track */
    M_PARAM_DECLARE_BOOL(ReadonlyTracksDefault)

    /* Data */
    M_PARAM_DECLARE_STRING(OpenStreetBugsUrl)
    M_PARAM_DECLARE_STRING(MapdustUrl)
    M_PARAM_DECLARE_BOOL(GdalConfirmProjection)
    M_PARAM_DECLARE_BOOL(HasAutoLoadDocument)
    M_PARAM_DECLARE_STRING(AutoLoadDocumentFilename)

    /* FeaturesDock */
    M_PARAM_DECLARE_BOOL(FeaturesWithin)

    /* Plugins */
    void addBackgroundPlugin(IMapAdapterFactory* aPlugin);
    void setBackgroundPlugin(const QUuid& theValue);
    QUuid getBackgroundPlugin() const;
    IMapAdapterFactory* getBackgroundPlugin(const QUuid& anAdapterUid);
    QMap<QUuid, IMapAdapterFactory *> getBackgroundPlugins();

    /* Projections */
    void loadProjection(QString fn);
    void loadProjections();
    void saveProjections();
#ifndef _MOBILE
    void setProjectionType(QString theValue);
    QString getProjectionType();
    ProjectionsList* getProjectionsList();
    ProjectionItem getProjection(QString aProj);
#endif

    /* Filters */
    void loadFilter(QString fn);
    void loadFilters();
    void saveFilters();
    void setCurrentFilter(FilterType theValue);
    FilterType getCurrentFilter();
    FiltersList* getFiltersList();
    FilterItem getFilter(QString aFilter);

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

    /* Servers */
    void loadOsmServers();
    void saveOsmServers();

    BookmarkList*  getBookmarks();
    WmsServerList* getWmsServers();
    TmsServerList* getTmsServers();
    OsmServerList* getOsmServers();

    M_PARAM_DECLARE_STRING(SelectedServer);

protected:
    QVector<qreal> parentDashes;

    bool Use06Api;
    QString version;
    bool RightSideDriving;
    qreal DoubleRoadDistance;
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
    QMap<QUuid, IMapAdapterFactory *> mBackgroundPlugins;
    ProjectionsList theProjectionsList;
    FiltersList theFiltersList;
    WmsServersList theWmsServerList;
    TmsServersList theTmsServerList;
    BookmarksList theBookmarkList;
    OsmServerList theOsmServers;

#ifdef USE_LIBPROXY
    pxProxyFactory                          *proxyFactory;
    QHash<QString,QNetworkProxy::ProxyType>  proxyTypeMap;
#endif

    static MerkaartorPreferences* m_prefInstance;
    static IPaintStyle* m_EPSInstance;

private slots:
    void on_responseHeaderReceived(const QHttpResponseHeader & hdr);
    void on_requestFinished ( int id, bool error );

signals:
    void bookmarkChanged();
};

#endif
