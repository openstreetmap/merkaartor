#ifndef MERKATOR_MAINWINDOW_H_
#define MERKATOR_MAINWINDOW_H_

#include <ui_MainWindow.h>

#include <QtGui/QMainWindow>
#include <QtXml>
#include <QProgressBar>
#include <QLabel>

class MainWindowPrivate;
class LayerDock;
class MapDocument;
class MapLayer;
class MapView;
class MapFeature;
class PropertiesDock;
class InfoDock;
class DirtyDock;
class QGPS;
class FeaturePainter;
class TrackMapLayer;
class TrackSegment;

#ifdef GEOIMAGE
class GeoImageDock;
#endif

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

	friend class ActionsDialog;

	public:
		MainWindow(void);
	public:
		virtual ~MainWindow(void);

	public slots:
		virtual void on_createRoundaboutAction_triggered();
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
		virtual void on_editCopyAction_triggered();
		virtual void on_editPasteOverwriteAction_triggered();
		virtual void on_editPasteMergeAction_triggered();
		virtual void on_editPasteFeaturesAction_triggered();
		
		virtual void on_fileNewAction_triggered();
		virtual void on_fileDownloadAction_triggered();
		virtual void on_fileDownloadMoreAction_triggered();
		virtual void on_fileUploadAction_triggered();
		virtual void on_fileImportAction_triggered();
		virtual void on_fileOpenAction_triggered();
		virtual void on_fileSaveAsAction_triggered();
		virtual void on_fileSaveAction_triggered();
		virtual void on_fileWorkOfflineAction_triggered();
		
		virtual void on_layersAddImageAction_triggered();
		virtual void on_helpAboutAction_triggered();
		virtual void on_viewZoomAllAction_triggered();
		virtual void on_viewZoomInAction_triggered();
		virtual void on_viewZoomOutAction_triggered();
		virtual void on_viewZoomWindowAction_triggered();
		virtual void on_viewDownloadedAction_triggered();
		virtual void on_viewScaleAction_triggered();
		virtual void on_viewStyleBackgroundAction_triggered();
		virtual void on_viewStyleForegroundAction_triggered();
		virtual void on_viewStyleTouchupAction_triggered();
		virtual void on_viewNamesAction_triggered();
		virtual void on_viewTrackPointsAction_triggered();
		virtual void on_viewTrackSegmentsAction_triggered();
		virtual void on_viewRelationsAction_triggered();
		virtual void on_viewGotoAction_triggered();
		virtual void on_viewArrowsNeverAction_triggered(bool checked);
		virtual void on_viewArrowsOnewayAction_triggered(bool checked);
		virtual void on_viewArrowsAlwaysAction_triggered(bool checked);

		virtual void on_editRemoveAction_triggered();
		virtual void on_editMoveAction_triggered();
		virtual void on_editReverseAction_triggered();

		virtual void on_roadSplitAction_triggered();
		virtual void on_roadBreakAction_triggered();
		virtual void on_roadJoinAction_triggered();
		virtual void on_featureCommitAction_triggered();
		virtual void on_nodeAlignAction_triggered();
		virtual void on_nodeMergeAction_triggered();
		virtual void on_nodeDetachAction_triggered();
		virtual void on_relationAddMemberAction_triggered();
		virtual void on_relationRemoveMemberAction_triggered();

		virtual void on_mapStyleSaveAction_triggered();
		virtual void on_mapStyleLoadAction_triggered();
		virtual void on_exportOSMAction_triggered();
		virtual void on_exportOSMBinAction_triggered();
		virtual void on_exportGPXAction_triggered();
		virtual void on_exportKMLAction_triggered();
		virtual void on_renderNativeAction_triggered();
		virtual void on_renderSVGAction_triggered();
		virtual void on_editSelectAction_triggered();
		virtual void on_bookmarkAddAction_triggered();
		virtual void on_bookmarkRemoveAction_triggered();

		virtual void on_toolsPreferencesAction_triggered() {toolsPreferencesAction_triggered();}
		virtual void on_toolsWorldOsbAction_triggered();
		virtual void on_toolsShortcutsAction_triggered();
		
		virtual void on_windowPropertiesAction_triggered();
		virtual void on_windowLayersAction_triggered();
		virtual void on_windowInfoAction_triggered();
		virtual void on_windowDirtyAction_triggered();
		virtual void on_windowToolbarAction_triggered();
		virtual void on_windowGPSAction_triggered();
#ifdef GEOIMAGE
		virtual void on_windowGeoimageAction_triggered();
#endif
		virtual void on_windowHideAllAction_triggered();
		virtual void on_windowShowAllAction_triggered();
		virtual void on_gpsConnectAction_triggered();
		virtual void on_gpsReplayAction_triggered();
		virtual void on_gpsRecordAction_triggered();
		virtual void on_gpsPauseAction_triggered();
		virtual void on_gpsDisconnectAction_triggered();
		virtual void on_gpsCenterAction_triggered();
		virtual void preferencesChanged();
		virtual void clipboardChanged();
		virtual void toolsPreferencesAction_triggered(bool focusData=false);

		virtual void on_toolTemplatesSaveAction_triggered();
		virtual void on_toolTemplatesMergeAction_triggered();
		virtual void on_toolTemplatesLoadAction_triggered();

	signals:
		void remove_triggered();
		void move_triggered();
		void add_triggered();
		void reverse_triggered();

	public:
		MainWindowPrivate* p;

		QString fileName;
		PropertiesDock* properties();
		InfoDock* info();
		QGPS* gps();
		#ifdef GEOIMAGE
		GeoImageDock* geoImage();
		#endif
		MapDocument* document();
		//MapLayer* activeLayer();
		MapView* view();
		QByteArray fullscreenState;

		QProgressBar* pbImages;
		QString StatusMessage;
		QLabel* ViewportStatusLabel;
		QLabel* PaintTimeLabel;

	public slots:
		void adjustLayers(bool adjustViewport);
		void bookmarkTriggered(QAction* anAction);
		void recentOpenTriggered(QAction* anAction);
		void recentImportTriggered(QAction* anAction);
		void projectionTriggered(QAction* anAction);
        void updateGpsPosition(float latitude, float longitude, QDateTime time, float altitude, float speed, float heading);
		void applyStyles(QVector<FeaturePainter>* thePainters);

	public:
		void invalidateView(bool UpdateDock = true);
		bool importFiles(MapDocument * mapDocument, const QStringList & filesNames, QStringList * importedFileNames = NULL);
		void loadFiles(const QStringList & fileNames);
		void loadDocument(QString fn);
        void saveDocument();
        void updateLanguage();


    private:
		MapView* theView;
		MapDocument* theDocument;
		PropertiesDock* theProperties;
		InfoDock* theInfo;
		DirtyDock* theDirty;
		LayerDock* theLayers;
		#ifdef GEOIMAGE
		GeoImageDock* theGeoImage;
		#endif
		QGPS* theGPS;
		QDomDocument* theXmlDoc;

		TrackMapLayer* gpsRecLayer;
		TrackSegment* curGpsTrackSegment;
		QHash<QString, QString> shortcutsDefault;

		QTranslator* qtTranslator;
		QTranslator* merkaartorTranslator;

	private slots:
		void setAreaOpacity(QAction*);
		void updateBookmarksMenu();

	private:
		void updateMenu();
		void updateRecentOpenMenu();
		void updateRecentImportMenu();
		void updateProjectionMenu();
		MapDocument* getDocumentFromClipboard();
		bool selectExportedFeatures(QVector<MapFeature*>& theFeatures);

	protected:
		void closeEvent(QCloseEvent * event);
};

#endif


