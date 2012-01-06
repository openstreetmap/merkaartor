#ifndef MERKATOR_MAINWINDOW_H_
#define MERKATOR_MAINWINDOW_H_

#include "IMerkMainWindow.h"
#include "IProgressWindow.h"

#include <QtGui/QMainWindow>
#include <QtXml>
#include <QProgressBar>
#include <QLabel>

namespace Ui {
    class MainWindow;
}

class MainWindowPrivate;
class LayerDock;
class Document;
class Layer;
class MapView;
class Feature;
class PropertiesDock;
class InfoDock;
class DirtyDock;
class FeaturesDock;
class QGPS;
class GlobalPainter;
class Painter;
class TrackLayer;
class TrackSegment;
class Interaction;
class QtToolBarManager;
class PreferencesDialog;

#ifdef GEOIMAGE
class GeoImageDock;
#endif

class MainWindow : public QMainWindow, public IMerkMainWindow, public IProgressWindow
{
    Q_OBJECT

    friend class ActionsDialog;
    friend class MerkaartorPreferences;

public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow();

public slots:
    void invalidateView(bool UpdateDock = true);

    virtual void on_createRoundaboutAction_triggered();
    virtual void on_createPolygonAction_triggered();
    virtual void on_createRectangleAction_triggered();
    virtual void on_createDoubleWayAction_triggered();
    virtual void on_createNodeAction_triggered();
    virtual void on_createRoadAction_triggered();
    virtual void on_createCurvedRoadAction_triggered();
    virtual void on_createRelationAction_triggered();
    virtual void on_createAreaAction_triggered();
    virtual void on_editPropertiesAction_triggered();
    virtual void on_editUndoAction_triggered();
    virtual void on_editRedoAction_triggered();
    virtual void on_editMapStyleAction_triggered();
    virtual void on_editCutAction_triggered();
    virtual void on_editCopyAction_triggered();
    virtual void on_editPasteFeatureAction_triggered();
    virtual void on_editPasteOverwriteAction_triggered();
    virtual void on_editPasteMergeAction_triggered();

    virtual void on_layersNewImageAction_triggered();
    virtual void on_layersNewDrawingAction_triggered();
    virtual void on_layersNewFilterAction_triggered();
    virtual void on_layersOpenstreetbugsAction_triggered();
    virtual void on_layersMapdustAction_triggered();

    virtual void on_fileNewAction_triggered();
    virtual void on_fileDownloadAction_triggered();
    virtual void on_fileDownloadMoreAction_triggered();
    virtual void on_fileUploadAction_triggered();
    virtual void on_fileImportAction_triggered();
    virtual void on_fileOpenAction_triggered();
    virtual void on_fileSaveAsAction_triggered();
    virtual void on_fileSaveAsTemplateAction_triggered();
    virtual void on_fileSaveAction_triggered();
    virtual void on_fileWorkOfflineAction_triggered();
    virtual void on_filePrintAction_triggered();
    virtual void on_filePropertiesAction_triggered();

    virtual void on_helpAboutAction_triggered();
    virtual void on_viewZoomAllAction_triggered();
    virtual void on_viewZoomInAction_triggered();
    virtual void on_viewZoomOutAction_triggered();
    virtual void on_viewZoomWindowAction_triggered();
    virtual void on_viewLockZoomAction_triggered();
    virtual void on_viewDownloadedAction_triggered();
    virtual void on_viewDirtyAction_triggered();
    virtual void on_viewScaleAction_triggered();
    virtual void on_viewPhotosAction_triggered();
    virtual void on_viewShowLatLonGridAction_triggered();
    virtual void on_viewStyleBackgroundAction_triggered();
    virtual void on_viewStyleForegroundAction_triggered();
    virtual void on_viewStyleTouchupAction_triggered();
    virtual void on_viewNamesAction_triggered();
    virtual void on_viewVirtualNodesAction_triggered();
    virtual void on_viewTrackPointsAction_triggered();
    virtual void on_viewTrackSegmentsAction_triggered();
    virtual void on_viewRelationsAction_triggered();
    virtual void on_viewGotoAction_triggered();
    virtual void on_viewArrowsNeverAction_triggered(bool checked);
    virtual void on_viewArrowsOnewayAction_triggered(bool checked);
    virtual void on_viewArrowsAlwaysAction_triggered(bool checked);

    virtual void on_editRemoveAction_triggered();
    virtual void on_editMoveAction_triggered();
    virtual void on_editRotateAction_triggered();
    virtual void on_editScaleAction_triggered();
    virtual void on_editReverseAction_triggered();

    virtual void on_roadSplitAction_triggered();
    virtual void on_roadBreakAction_triggered();
    virtual void on_roadJoinAction_triggered();
    virtual void on_roadSimplifyAction_triggered();
    virtual void on_roadCreateJunctionAction_triggered();
    virtual void on_roadAddStreetNumbersAction_triggered();
    virtual void on_roadSubdivideAction_triggered();
    virtual void on_roadAxisAlignAction_triggered();
    virtual void on_roadExtrudeAction_triggered();
    virtual void on_roadBingExtractAction_triggered();

    virtual void on_featureSelectChildrenAction_triggered();
    virtual void on_featureDeleteAction_triggered();
    virtual void on_featureCommitAction_triggered();
    virtual void on_featureOsbClose_triggered();

    virtual void on_nodeAlignAction_triggered();
    virtual void on_nodeSpreadAction_triggered();
    virtual void on_nodeMergeAction_triggered();
    virtual void on_nodeDetachAction_triggered();
    virtual void on_relationAddMemberAction_triggered();
    virtual void on_relationRemoveMemberAction_triggered();
    virtual void on_relationAddToMultipolygonAction_triggered();
    virtual void on_areaJoinAction_triggered();
    virtual void on_areaSplitAction_triggered();
    virtual void on_areaTerraceAction_triggered();

    virtual void on_mapStyleSaveAction_triggered();
    virtual void on_mapStyleSaveAsAction_triggered();
    virtual void on_mapStyleLoadAction_triggered();
    virtual void on_exportOSMAction_triggered();
    virtual void on_exportOSCAction_triggered();
    virtual void on_exportGPXAction_triggered();
    virtual void on_exportKMLAction_triggered();
    virtual void on_exportGDALAction_triggered();
    virtual void on_editSelectAction_triggered();
    virtual void on_bookmarkAddAction_triggered();
    virtual void on_bookmarkRemoveAction_triggered();

    virtual void on_toolsPreferencesAction_triggered() {toolsPreferencesAction_triggered();}
    virtual void on_toolsShortcutsAction_triggered();
    virtual void on_toolsToolbarsAction_triggered();
    virtual void on_toolsWMSServersAction_triggered();
    virtual void on_toolsTMSServersAction_triggered();
    virtual void on_toolsProjectionsAction_triggered();
    virtual void on_toolsFiltersAction_triggered();
    virtual void toolsPreferencesAction_triggered(bool focusData=false);
    virtual void on_toolsResetDiscardableAction_triggered();
    virtual void on_toolsRebuildHistoryAction_triggered();

    virtual void on_windowPropertiesAction_triggered();
    virtual void on_windowLayersAction_triggered();
    virtual void on_windowInfoAction_triggered();
    virtual void on_windowDirtyAction_triggered();
    virtual void on_windowFeatsAction_triggered();
    virtual void on_windowToolbarAction_triggered();
    virtual void on_windowGPSAction_triggered();
#ifdef GEOIMAGE
    virtual void on_windowGeoimageAction_triggered();
#endif
    virtual void on_windowStylesAction_triggered();

    virtual void on_windowHideAllAction_triggered();
    virtual void on_windowShowAllAction_triggered();
    virtual void on_gpsConnectAction_triggered();
    virtual void on_gpsReplayAction_triggered();
    virtual void on_gpsRecordAction_triggered();
    virtual void on_gpsPauseAction_triggered();
    virtual void on_gpsDisconnectAction_triggered();
    virtual void on_gpsCenterAction_triggered();
    virtual void preferencesChanged(PreferencesDialog* prefs);
    virtual void clipboardChanged();

    virtual void on_toolTemplatesSaveAction_triggered();
    virtual void on_toolTemplatesMergeAction_triggered();
    virtual void on_toolTemplatesLoadAction_triggered();

signals:
    void remove_triggered();
    void move_triggered();
    void add_triggered();
    void reverse_triggered();

    void content_changed();

public:
    MainWindowPrivate* p;
    Ui::MainWindow *ui;

    QString fileName;
    PropertiesDock* properties();
    FeaturesDock* features();
    InfoDock* info();
    QGPS* gps();
#ifdef GEOIMAGE
    GeoImageDock* geoImage();
#endif
    Document* document();
    //MapLayer* activeLayer();
    MapView* view();
    QByteArray fullscreenState;

    QProgressBar* pbImages;
    QString StatusMessage;
    QLabel* ViewportStatusLabel;
    QLabel* PaintTimeLabel;
    QLabel* MeterPerPixelLabel;
    QLabel* AdjusmentMeterLabel;

public slots:
    void handleMessage(const QString& msg);

    void adjustLayers(bool adjustViewport);
    void bookmarkTriggered(QAction* anAction);
    void recentOpenTriggered(QAction* anAction);
    void recentImportTriggered(QAction* anAction);
#ifndef _MOBILE
    void projectionSet(const QString& prj);
    void projectionTriggered(QAction* anAction);
#endif
    void styleTriggered(QAction* anAction);
    void updateGpsPosition(qreal latitude, qreal longitude, QDateTime time, qreal altitude, qreal speed, qreal heading);
    void applyStyles(QString NewStyle);
    void applyPainters(GlobalPainter* theGlobalPainter, QList<Painter>* thePainters);

public:
    bool importFiles(Document * mapDocument, const QStringList & filesNames, QStringList * importedFileNames = NULL);
    void loadFiles(const QStringList & fileNames);
    void loadUrl(const QUrl& u);
    void loadDocument(QString fn);
    void loadTemplateDocument(QString fn);
    void saveDocument(const QString& fn);
    void saveTemplateDocument(const QString& fn);
    void downloadFeatures(const QList<Feature*>& aDownloadList);

    void createProgressDialog();
    void deleteProgressDialog();

    void createToolBarManager();

    void updateLanguage();

    bool hasUnsavedChanges();
    void syncOSM(const QString &aWeb, const QString &aUser, const QString &aPwd);

protected:
    MapView* theView;
    Document* theDocument;
    InfoDock* theInfo;
    DirtyDock* theDirty;
    LayerDock* theLayers;
#ifdef GEOIMAGE
    GeoImageDock* theGeoImage;
#endif
    QGPS* theGPS;

    TrackLayer* gpsRecLayer;
    TrackSegment* curGpsTrackSegment;
    QHash<QString, QString> shortcutsDefault;

    QTranslator* qtTranslator;
    QTranslator* merkaartorTranslator;

    QtToolBarManager *toolBarManager;

private slots:
    void delayedInit();
    void setAreaOpacity(QAction*);
    void updateBookmarksMenu();
    void updateWindowMenu(bool b=false);
    void mapView_interactionChanged(Interaction* anInteraction);
    void incomingLocalConnection();
    void readLocalConnection();

private:
    void updateMenu();
    void updateRecentOpenMenu();
    void updateRecentImportMenu();
    void updateProjectionMenu();
    void updateStyleMenu();
    bool selectExportedFeatures(QList<Feature*>& theFeatures);

    Document* doLoadDocument(QFile* file);
    void doSaveDocument(QFile* fn, bool asTemplate=false);

protected:
    void closeEvent(QCloseEvent * event);
};

#endif


