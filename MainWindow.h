#ifndef MERKATOR_MAINWINDOW_H_
#define MERKATOR_MAINWINDOW_H_

#include <ui_MainWindow.h>

#include <QtGui/QMainWindow>
#include <QtXml>

class LayerDock;
class MapDocument;
class MapLayer;
class MapView;
class PropertiesDock;
class InfoDock;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
	Q_OBJECT

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
		virtual void on_fileNewAction_triggered();
		virtual void on_fileDownloadAction_triggered();
		virtual void on_fileDownloadMoreAction_triggered();
		virtual void on_fileUploadAction_triggered();
		virtual void on_fileImportAction_triggered();
		virtual void on_fileOpenAction_triggered();
		virtual void on_fileSaveAsAction_triggered();
		virtual void on_fileSaveAction_triggered();
		virtual void on_helpAboutAction_triggered();
		virtual void on_viewZoomAllAction_triggered();
		virtual void on_viewZoomInAction_triggered();
		virtual void on_viewZoomOutAction_triggered();
		virtual void on_viewZoomWindowAction_triggered();
		virtual void on_viewSetCoordinatesAction_triggered();
		virtual void on_editRemoveAction_triggered();
		virtual void on_editMoveAction_triggered();
		virtual void on_editReverseAction_triggered();
		virtual void on_roadSplitAction_triggered();
		virtual void on_roadBreakAction_triggered();
		virtual void on_roadJoinAction_triggered();
		virtual void on_mapStyleSaveAction_triggered();
		virtual void on_mapStyleLoadAction_triggered();
		virtual void on_exportOSMAllAction_triggered();
		virtual void on_exportOSMViewportAction_triggered();
		virtual void on_editSelectAction_triggered();
		virtual void on_renderAction_triggered();
		virtual void on_bookmarkAddAction_triggered();
		virtual void on_bookmarkRemoveAction_triggered();
		virtual void on_nodeAlignAction_triggered();
		virtual void on_nodeMergeAction_triggered();
		virtual void on_toolsPreferencesAction_triggered() {toolsPreferencesAction_triggered();}
		virtual void on_windowPropertiesAction_triggered();
		virtual void on_windowLayersAction_triggered();
		virtual void on_windowInfoAction_triggered();

		virtual void preferencesChanged();
		virtual void toolsPreferencesAction_triggered(unsigned int tabIdx = 0);

	signals:
		void remove_triggered();
		void move_triggered();
		void add_triggered();
		void reverse_triggered();

	public:
		PropertiesDock* properties();
		InfoDock* info();
		MapDocument* document();
		MapLayer* activeLayer();
		MapView* view();

	public slots:
		void adjustLayers(bool adjustViewport);
		void bookmarkTriggered(QAction* anAction);
		void projectionTriggered(QAction* anAction);

	public:
		void invalidateView(bool UpdateDock = true);
		bool importFiles(MapDocument * mapDocument, const QStringList & filesNames);
		void loadFiles(const QStringList & fileNames);
		void loadDocument(QString fn);
		void saveDocument(QString fn);


	private:
		MapView* theView;
		MapDocument* theDocument;
		PropertiesDock* theProperties;
		InfoDock* theInfo;
		LayerDock* theLayers;
		QDomDocument* theXmlDoc;
		QString fileName;

	private:
		void updateBookmarksMenu();
		void updateProjectionMenu();

	protected:
		void closeEvent(QCloseEvent * event);
};

#endif


