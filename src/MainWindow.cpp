#include "MainWindow.h"

#include "LayerDock.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "DirtyDock.h"
#include "StyleDock.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "ImportExport/ImportExportOsmBin.h"
#include "ImportExport/ExportGPX.h"
#include "ImportExport/ImportExportKML.h"
#include "Interaction/CreateAreaInteraction.h"
#include "Interaction/CreateDoubleWayInteraction.h"
#include "Interaction/CreateNodeInteraction.h"
#include "Interaction/CreateRoundaboutInteraction.h"
#include "Interaction/CreatePolygonInteraction.h"
#include "Interaction/CreateSingleWayInteraction.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/MoveTrackPointInteraction.h"
#include "Interaction/RotateInteraction.h"
#include "Interaction/ZoomInteraction.h"
#include "Maps/Coord.h"
#include "Maps/DownloadOSM.h"
#include "Maps/ImportGPX.h"
#include "Maps/ImportNGT.h"
#include "Maps/ImportOSM.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"
#include "Maps/ImageMapLayer.h"
#include "Maps/MapFeature.h"
#include "Maps/Relation.h"
#include "Maps/Road.h"
#include "Maps/FeatureManipulations.h"
#include "Maps/TrackPoint.h"
#include "Maps/LayerIterator.h"
#include "PaintStyle/EditPaintStyle.h"
#include "PaintStyle/PaintStyleEditor.h"
#include "Sync/SyncOSM.h"
#include <ui_AboutDialog.h>
#include <ui_UploadMapDialog.h>
#include <ui_SelectionDialog.h>
#include <ui_ExportDialog.h>
#include "Preferences/PreferencesDialog.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Preferences/ProjectionsList.h"
#include "Preferences/WMSPreferencesDialog.h"
#include "Preferences/TMSPreferencesDialog.h"
#include "Utils/SelectionDialog.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
	#include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/mapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#include "Tools/WorldOsbManager.h"
#include "Tools/ActionsDialog.h"
#include "GotoDialog.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#include "Render/NativeRenderDialog.h"
#ifdef OSMARENDER
	#include "Render/OsmaRenderDialog.h"
#endif

#include "qgps.h"
#include "qgpsdevice.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QtGlobal>
#include <QtCore/QTimer>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>
#include <QInputDialog>
#include <QClipboard>
#include <QProgressDialog>
#include <QMenuBar>
#include <QTranslator>
#include <QLocale>
#include <QMessageBox>
#include <QStyleFactory>
#include <QMenu>

SlippyMapCache* SlippyMapWidget::theSlippyCache = 0;

class MainWindowPrivate
{
	public:
		MainWindowPrivate()
			: lastPrefTabIndex(0)
		{
		}
		int lastPrefTabIndex;
		QString defStyle;
        StyleDock* theStyle;
};

MainWindow::MainWindow(void)
		: fileName(""), theDocument(0),
		gpsRecLayer(0),curGpsTrackSegment(0),
		qtTranslator(0), merkaartorTranslator(0)
{
	p = new MainWindowPrivate;
	
	theProgressDialog = NULL;
	theProgressBar = NULL;
	theProgressLabel = NULL;

	p->defStyle = QApplication::style()->objectName();
#ifndef FORCED_CUSTOM_STYLE
	if (M_PREFS->getMerkaartorStyle())
		QApplication::setStyle(QStyleFactory::create(M_PREFS->getMerkaartorStyleString()));
#else
		QApplication::setStyle(QStyleFactory::create("skulpture"));
#endif

	setupUi(this);
	M_STYLE->loadPainters(MerkaartorPreferences::instance()->getDefaultStyle());

	blockSignals(true);

	ViewportStatusLabel = new QLabel(this);
	pbImages = new QProgressBar(this);
	PaintTimeLabel = new QLabel(this);
	statusBar()->addPermanentWidget(ViewportStatusLabel);
	statusBar()->addPermanentWidget(pbImages);
	statusBar()->addPermanentWidget(PaintTimeLabel);

	QList<QAction*> actions = findChildren<QAction*>();
	for (int i=0; i<actions.size(); i++) {
		shortcutsDefault[actions[i]->objectName()] = actions[i]->shortcut().toString();
	}
	updateLanguage();

	ViewportStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
	pbImages->setMaximumWidth(200);
	pbImages->setFormat(tr("tile %v / %m"));
	//PaintTimeLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));
	PaintTimeLabel->setMinimumWidth(23);
#ifdef _MOBILE
	pbImages->setVisible(false);
#endif

	SlippyMapWidget::theSlippyCache = new SlippyMapCache;

	theView = new MapView(this);
	setCentralWidget(theView);
	connect (theView, SIGNAL(interactionChanged(Interaction*)), this, SLOT(mapView_interactionChanged(Interaction*)));

	theInfo = new InfoDock(this);
    connect(theInfo, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

	theLayers = new LayerDock(this);
    connect(theLayers, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

	theProperties = new PropertiesDock(this);
    connect(theProperties, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
	on_editPropertiesAction_triggered();

    theDirty = new DirtyDock(this);
    connect(theDirty, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

    p->theStyle = new StyleDock(this);
    connect(p->theStyle, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

    theGPS = new QGPS(this);
    connect(theGPS, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

#ifdef GEOIMAGE
	theGeoImage = new GeoImageDock(this);
	theGeoImage->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theGeoImage);
    connect(theGeoImage, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
#endif

	connect (theLayers, SIGNAL(layersChanged(bool)), this, SLOT(adjustLayers(bool)));

	connect (MerkaartorPreferences::instance(), SIGNAL(bookmarkChanged()), this, SLOT(updateBookmarksMenu()));
	updateBookmarksMenu();
	connect (menuBookmarks, SIGNAL(triggered(QAction *)), this, SLOT(bookmarkTriggered(QAction *)));

	updateRecentOpenMenu();
	connect (menuRecentOpen, SIGNAL(triggered(QAction *)), this, SLOT(recentOpenTriggered(QAction *)));

	updateRecentImportMenu();
	connect (menuRecentImport, SIGNAL(triggered(QAction *)), this, SLOT(recentImportTriggered(QAction *)));

	updateProjectionMenu();

	updateStyleMenu();
	connect (menuStyles, SIGNAL(triggered(QAction *)), this, SLOT(styleTriggered(QAction *)));

	viewDownloadedAction->setChecked(MerkaartorPreferences::instance()->getDownloadedVisible());
	viewScaleAction->setChecked(M_PREFS->getScaleVisible());
	viewStyleBackgroundAction->setChecked(M_PREFS->getStyleBackgroundVisible());
	viewStyleForegroundAction->setChecked(M_PREFS->getStyleForegroundVisible());
	viewStyleTouchupAction->setChecked(M_PREFS->getStyleTouchupVisible());
	viewNamesAction->setChecked(M_PREFS->getNamesVisible());
	viewTrackPointsAction->setChecked(M_PREFS->getTrackPointsVisible());
	viewTrackSegmentsAction->setChecked(M_PREFS->getTrackSegmentsVisible());
	viewRelationsAction->setChecked(M_PREFS->getRelationsVisible());

	updateMenu();

	QActionGroup* actgrpArrows = new QActionGroup(this);
	actgrpArrows->addAction(viewArrowsNeverAction);
	actgrpArrows->addAction(viewArrowsOnewayAction);
	actgrpArrows->addAction(viewArrowsAlwaysAction);
	switch (M_PREFS->getDirectionalArrowsVisible()) {
		case DirectionalArrows_Never:
			viewArrowsNeverAction->setChecked(true);
			break;
		case DirectionalArrows_Oneway:
			viewArrowsOnewayAction->setChecked(true);
			break;
		case DirectionalArrows_Always:
			viewArrowsAlwaysAction->setChecked(true);
			break;
	}

	gpsCenterAction->setChecked(M_PREFS->getGpsMapCenter());

	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));

	setWindowTitle(QString("Merkaartor - untitled"));

#ifndef _MOBILE
	theLayers->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, theLayers);

	theProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, theProperties);

	theInfo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theInfo);

	theDirty->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theDirty);

    p->theStyle->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, p->theStyle);

    theGPS->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theGPS);

	mobileToolBar->setVisible(false);

#else
	theProperties->setVisible(false);
	theInfo->setVisible(false);
	theLayers->setVisible(false);
	theDirty->setVisible(false);
	theGPS->setVisible(false);

	toolBar->setVisible(false);
	mobileToolBar->setVisible(true);

	gpsPopupAction->setMenu(menuGps);
	dynamic_cast<QToolButton*>(mobileToolBar->widgetForAction(gpsPopupAction))->setPopupMode(QToolButton::InstantPopup);

#ifdef _WINCE
	windowPropertiesAction->setText(tr("Properties..."));
	menuBar()->setDefaultAction(windowPropertiesAction);
#endif // _WINCE
#endif // _MOBILE

#ifndef OSMARENDER
	renderSVGAction->setVisible(false);
#endif

#ifndef GEOIMAGE
	windowGeoimageAction->setVisible(false);
#endif

//#ifdef NDEBUG
	viewStyleBackgroundAction->setVisible(false);
	viewStyleForegroundAction->setVisible(false);
	viewStyleTouchupAction->setVisible(false);
//#endif
	
	MerkaartorPreferences::instance()->restoreMainWindowState( this );
#ifndef _MOBILE
	if (!M_PREFS->getProjectionsList().getProjections().size()) {
		QMessageBox::critical(this, tr("Cannot load Projections file"), tr("\"Projections.xml\" could not be opened anywhere. Aborting."));
		exit(1);
	}
#endif
	on_fileNewAction_triggered();
	M_PREFS->initialPosition(theView);

#define NUMOP 3
	static const char *opStr[NUMOP] = {
		QT_TR_NOOP("Low"), QT_TR_NOOP("High"), QT_TR_NOOP("Opaque")};

	int o = M_PREFS->getAreaOpacity();
	QActionGroup* actgrp = new QActionGroup(this);
	for (int i=0; i<NUMOP; i++) {
		QAction* act = new QAction(tr(opStr[i]), mnuAreaOpacity);
		actgrp->addAction(act);
		qreal a = M_PREFS->getAlpha(opStr[i]);
		act->setData(a);
		act->setCheckable(true);
		mnuAreaOpacity->addAction(act);
		if (o == int(a*100))
			act->setChecked(true);
	}
	connect(mnuAreaOpacity, SIGNAL(triggered(QAction*)), this, SLOT(setAreaOpacity(QAction*)));

	blockSignals(false);

	QTimer::singleShot( 0, this, SLOT(delayedInit()) );

}

void MainWindow::delayedInit()
{
    updateWindowMenu();
}


MainWindow::~MainWindow(void)
{
    theProperties->setSelection(NULL);

	if (EditPaintStyle::instance())
		delete EditPaintStyle::instance();
	MerkaartorPreferences::instance()->setWorkingDir(QDir::currentPath());
	delete MerkaartorPreferences::instance();
	delete theDocument;
	delete theView;
	delete theProperties;

    delete qtTranslator;
    delete merkaartorTranslator;

	delete SlippyMapWidget::theSlippyCache;

	delete p;
}

void MainWindow::createProgressDialog()
{
	theProgressDialog = new QProgressDialog(this);
	theProgressDialog->setWindowModality(Qt::ApplicationModal);
	theProgressDialog->setMinimumDuration(0);

	theProgressBar = new QProgressBar(theProgressDialog);
	theProgressBar->setTextVisible(false);
	theProgressDialog->setBar(theProgressBar);

	theProgressLabel = new QLabel();
	theProgressLabel->setAlignment(Qt::AlignCenter);
	theProgressDialog->setLabel(theProgressLabel);
	
	theProgressDialog->setMaximum(11);
}

void MainWindow::setAreaOpacity(QAction *act)
{
	qreal a = act->data().toDouble();
	M_PREFS->setAreaOpacity(int(a*100));

	theView->invalidate(true, false);
}

void MainWindow::adjustLayers(bool adjustViewport)
{
	CoordBox theVp;
	if (adjustViewport) {
		theVp = theView->viewport();
#ifndef _MOBILE
		////FIXME Remove when the image layers will be independent from the projection
		//for (int i=0; i<theDocument->getImageLayersSize(); ++i)
		//{
		//	ProjectionItem pi = M_PREFS->getProjection(theDocument->getImageLayer(i)->projection());
		//	if (theView->projection().setProjectionType(theDocument->getImageLayer(i)->projection())) {
		//		M_PREFS->setProjectionType(pi.name);
		//		// TODO Select the proper action in the Projection menu
		//	}
		//	break;
		//}
#endif
		theView->setViewport(theVp, theView->rect());
	}
	invalidateView(true);
}

void MainWindow::invalidateView(bool UpdateDock)
{
	theView->invalidate(true, true);
	//theLayers->updateContent();
	if (UpdateDock)
		theProperties->resetValues();
}

PropertiesDock* MainWindow::properties()
{
	return theProperties;
}

InfoDock* MainWindow::info()
{
	return theInfo;
}

#ifdef GEOIMAGE
GeoImageDock* MainWindow::geoImage()
{
	return theGeoImage;
}
#endif

MapDocument* MainWindow::document()
{
	return theDocument;
}

MapDocument* MainWindow::getDocumentFromClipboard()
{
	QClipboard *clipboard = QApplication::clipboard();
	QDomDocument* theXmlDoc = new QDomDocument();

	if (clipboard->mimeData()->hasFormat("application/x-openstreetmap+xml")) {
		if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/x-openstreetmap+xml"))) {
			delete theXmlDoc;
			return NULL;
		}
	} else
	if (clipboard->mimeData()->hasFormat("application/vnd.google-earth.kml+xml")) {
		if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/vnd.google-earth.kml+xml"))) {
			delete theXmlDoc;
			return NULL;
		}
	} else
	if (clipboard->mimeData()->hasText()) {
		if (!theXmlDoc->setContent(clipboard->text())) {
			delete theXmlDoc;
			return NULL;
		}
	} else {
		delete theXmlDoc;
		return NULL;
	}

	QDomElement c = theXmlDoc->documentElement();

	if (c.tagName() == "osm") {
		MapDocument* NewDoc = new MapDocument(NULL);
		DrawingMapLayer* l = new DrawingMapLayer("Dummy");
		NewDoc->add(l);

		c = c.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "bound") {
			} else
			if (c.tagName() == "way") {
				Road::fromXML(NewDoc, l, c);
			} else
			if (c.tagName() == "relation") {
				Relation::fromXML(NewDoc, l, c);
			} else
			if (c.tagName() == "node") {
				TrackPoint::fromXML(NewDoc, l, c);
			}

			c = c.nextSiblingElement();
		}

		delete theXmlDoc;
		return NewDoc;
	} else
	if (c.tagName() == "kml") {
		MapDocument* NewDoc = new MapDocument(NULL);
		DrawingMapLayer* l = new DrawingMapLayer("Dummy");
		NewDoc->add(l);

		ImportExportKML imp(NewDoc);
		QByteArray ba = clipboard->text().toUtf8();
		QBuffer kmlBuf(&ba);
		kmlBuf.open(QIODevice::ReadOnly);
		if (imp.setDevice(&kmlBuf))
			imp.import(l);

		delete theXmlDoc;
		return NewDoc;
	} else
	if (c.tagName() == "gpx") {
	}
	QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard do not contain valid data."));
	return NULL;
}

void MainWindow::on_editCopyAction_triggered()
{
	QClipboard *clipboard = QApplication::clipboard();
	QMimeData* md = new QMimeData();

	QString osm = theDocument->exportOSM(theProperties->selection());
	md->setText(osm);
	md->setData("application/x-openstreetmap+xml", osm.toUtf8()); 

	ImportExportKML kmlexp(theDocument);
	QBuffer kmlBuf;
	kmlBuf.open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (kmlexp.setDevice(&kmlBuf)) {
		kmlexp.export_(theProperties->selection());
		md->setData("application/vnd.google-earth.kml+xml", kmlBuf.data()); 
	}

	ExportGPX gpxexp(theDocument);
	QBuffer gpxBuf;
	gpxBuf.open(QIODevice::WriteOnly | QIODevice::Truncate);
	if (gpxexp.setDevice(&gpxBuf)) {
		gpxexp.export_(theProperties->selection());
		md->setData("application/gpx+xml", gpxBuf.data()); 
	}

	clipboard->setMimeData(md);
	invalidateView();
}

void MainWindow::on_editPasteFeatureAction_triggered()
{
	MapDocument* doc;
	if (!(doc = getDocumentFromClipboard()))
		return;

	CommandList* theList = new CommandList();
	theList->setDescription("Paste Features");

	QList<MapFeature*> theFeats;
	for (FeatureIterator k(doc); !k.isEnd(); ++k) {
		theFeats.push_back(k.get());
	}
	for (int i=0; i<theFeats.size(); ++i) {
		MapFeature*F = theFeats.at(i);
		if (theDocument->getFeature(F->id()))
			F->resetId();

//		if (TrackPoint* P = CAST_NODE(F)) {
//		} else
//		if (Road* R = CAST_WAY(F)) {
//		} else
//		if (Relation* RR = CAST_RELATION(F)) {
//		}
		F->layer()->remove(F);
		theList->add(new AddFeatureCommand(theDocument->getDirtyOrOriginLayer(), F, true));
	}

	if (theList->size())
		document()->addHistory(theList);
	else
		delete theList;

	delete doc;

	theProperties->setSelection(theFeats);
	invalidateView();
}

void MainWindow::on_editPasteOverwriteAction_triggered()
{
	QList<MapFeature*> sel = properties()->selection();
	if (!sel.size())
		return;

	MapDocument* doc;
	if (!(doc = getDocumentFromClipboard()))
		return;

	CommandList* theList = new CommandList();
	theList->setDescription("Paste tags (overwrite)");

	for(int i=0; i < sel.size(); ++i) {
		theList->add(new ClearTagsCommand(sel[i], theDocument->getDirtyOrOriginLayer(sel[i]->layer())));
		for (FeatureIterator k(doc); !k.isEnd(); ++k) {
			MapFeature::mergeTags(theDocument, theList, sel[i], k.get());
		}
	}

	if (theList->size())
		document()->addHistory(theList);
	else
		delete theList;

	delete doc;
	invalidateView();
}

void MainWindow::on_editPasteMergeAction_triggered()
{
	QList<MapFeature*> sel = properties()->selection();
	if (!sel.size())
		return;

	MapDocument* doc;
	if (!(doc = getDocumentFromClipboard()))
		return;

	CommandList* theList = new CommandList();
	theList->setDescription("Paste tags (merge)");

	for(int i=0; i < sel.size(); ++i) {
		for (FeatureIterator k(doc); !k.isEnd(); ++k) {
			MapFeature::mergeTags(theDocument, theList, sel[i], k.get());
		}
	}

	if (theList->size())
		document()->addHistory(theList);
	else
		delete theList;

	delete doc;
	invalidateView();
}

void MainWindow::on_editPasteFeaturesAction_triggered()
{
	invalidateView();
}

void MainWindow::clipboardChanged()
{
	editPasteFeaturesAction->setEnabled(false);
	editPasteMergeAction->setEnabled(false);
	editPasteOverwriteAction->setEnabled(false);

	QClipboard *clipboard = QApplication::clipboard();
	//qDebug() << "Clipboard mime: " << clipboard->mimeData()->formats();
	QDomDocument* theXmlDoc = new QDomDocument();
	if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/x-openstreetmap+xml")))
		if (!theXmlDoc->setContent(clipboard->text())) {
			delete theXmlDoc;
			return;
		}

	QDomElement c = theXmlDoc->documentElement();

	if (c.tagName() != "osm" && c.tagName() != "kml") {
		delete theXmlDoc;
		return;
	}

	editPasteFeaturesAction->setEnabled(true);
	editPasteMergeAction->setEnabled(true);
	editPasteOverwriteAction->setEnabled(true);

	delete theXmlDoc;
}

void MainWindow::on_editRedoAction_triggered()
{
	theDocument->redoHistory();
	invalidateView();
}

void MainWindow::on_editUndoAction_triggered()
{
	theDocument->undoHistory();
	invalidateView();
}

void MainWindow::on_editPropertiesAction_triggered()
{
	if (theView->interaction() && dynamic_cast<EditInteraction*>(theView->interaction()))
		theProperties->setSelection(0);
	theView->unlockSelection();
	theView->invalidate(true, false);
	theView->launch(new EditInteraction(theView));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_editRemoveAction_triggered()
{
	emit remove_triggered();
}

void MainWindow::on_editMoveAction_triggered()
{
	if (M_PREFS->getSeparateMoveMode()) {
		view()->launch(new MoveTrackPointInteraction(view()));
		theInfo->setHtml(theView->interaction()->toHtml());
	}
}

void MainWindow::on_editRotateAction_triggered()
{
	view()->launch(new RotateInteraction(view()));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_editReverseAction_triggered()
{
	emit reverse_triggered();
}

static void changeCurrentDirToFile(const QString& s)
{
	QFileInfo info(s);
	QDir::setCurrent(info.absolutePath());
	MerkaartorPreferences::instance()->setWorkingDir(QDir::currentPath());
}

#ifndef GEOIMAGE
#define FILTER_OPEN_SUPPORTED \
	tr("Supported formats")+" (*.mdc *.gpx *.osm *.osb *.ngt *.nmea *.nma *.kml *.shp)\n" \
	+tr("Merkaartor document (*.mdc)\n") \
	+tr("GPS Exchange format (*.gpx)\n") \
	+tr("OpenStreetMap format (*.osm)\n") \
	+tr("OpenStreetMap binary format (*.osb)\n") \
	+tr("Noni GPSPlot format (*.ngt)\n") \
	+tr("NMEA GPS log format (*.nmea *.nma)\n") \
	+tr("KML file (*.kml)\n") \
	+tr("ESRI Shapefile (*.shp)\n") \
	+tr("All Files (*)")
#else
#define FILTER_OPEN_SUPPORTED \
	tr("Supported formats")+" (*.mdc *.gpx *.osm *.osb *.ngt *.nmea *.nma *.kml *.shp *.jpg)\n" \
	+tr("Merkaartor document (*.mdc)\n") \
	+tr("GPS Exchange format (*.gpx)\n") \
	+tr("OpenStreetMap format (*.osm)\n") \
	+tr("OpenStreetMap binary format (*.osb)\n") \
	+tr("Noni GPSPlot format (*.ngt)\n") \
	+tr("NMEA GPS log format (*.nmea *.nma)\n") \
	+tr("KML file (*.kml)\n") \
	+tr("Geotagged images (*.jpg)\n") \
	+tr("ESRI Shapefile (*.shp)\n") \
	+tr("All Files (*)")
#endif
#define FILTER_IMPORT_SUPPORTED \
	tr("Supported formats")+" (*.gpx *.osm *.osb *.ngt *.nmea *.nma *.kml *.shp)\n" \
	+tr("GPS Exchange format (*.gpx)\n") \
	+tr("OpenStreetMap format (*.osm)\n") \
	+tr("OpenStreetMap binary format (*.osb)\n") \
	+tr("Noni GPSPlot format (*.ngt)\n") \
	+tr("NMEA GPS log format (*.nmea *.nma)\n") \
	+tr("KML file (*.kml)\n") \
	+tr("ESRI Shapefile (*.shp)\n") \
	+tr("All Files (*)")

void MainWindow::on_fileImportAction_triggered()
{
	QStringList fileNames = QFileDialog::getOpenFileNames(
					this,
					tr("Open track file"),
					"", FILTER_IMPORT_SUPPORTED);

	if (fileNames.isEmpty())
		return;

	view()->setUpdatesEnabled(false);
	theLayers->setUpdatesEnabled(false);

	QStringList importedFiles;
	importFiles(theDocument, fileNames, &importedFiles);

	foreach (QString currentFileName, importedFiles)
		M_PREFS->addRecentImport(currentFileName);

	updateRecentImportMenu();

	view()->setUpdatesEnabled(true);
	theLayers->setUpdatesEnabled(true);

	on_editPropertiesAction_triggered();
	theDocument->history().setActions(editUndoAction, editRedoAction, fileUploadAction);
}

static bool mayDiscardUnsavedChanges(QWidget* aWidget)
{
	return QMessageBox::question(aWidget, MainWindow::tr("Unsaved changes"),
								 MainWindow::tr("The current map contains unsaved changes that will be lost when starting a new one.\n"
												"Do you want to cancel starting a new map or continue and discard the old changes?"),
								 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
}

bool MainWindow::importFiles(MapDocument * mapDocument, const QStringList & fileNames, QStringList * importedFileNames )
{
	createProgressDialog();
#ifndef Q_OS_SYMBIAN
	QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

	bool foundImport = false;

	QStringListIterator it(fileNames);
	while (it.hasNext())
	{
		const QString & fn = it.next();
		changeCurrentDirToFile(fn);

		QString baseFileName = fn.section('/', - 1);
		MapLayer* newLayer = NULL;

		bool importOK = false;
		bool importAborted = false;

		if (fn.endsWith(".gpx")) {
			QList<TrackMapLayer*> theTracklayers;
			TrackMapLayer* newLayer = new TrackMapLayer( baseFileName + " - " + tr("Waypoints"), baseFileName);
			mapDocument->add(newLayer);
			theTracklayers.append(newLayer);
			importOK = importGPX(this, baseFileName, mapDocument, theTracklayers);
			if (!importOK) {
				for (int i=0; i<theTracklayers.size(); i++) {
					mapDocument->remove(theTracklayers[i]);
					delete theTracklayers[i];
				}
			} else {
				if (!newLayer->size()) {
					mapDocument->remove(newLayer);
					delete newLayer;
				}
				for (int i=1; i<theTracklayers.size(); i++) {
					if (theTracklayers[i]->name().isEmpty())
						theTracklayers[i]->setName(QString(baseFileName + " - " + tr("Track %1").arg(i)));
					if (importOK && MerkaartorPreferences::instance()->getAutoExtractTracks()) {
						theTracklayers[i]->extractLayer();
					}
				}
			}
		}
		else if (fn.endsWith(".osm")) {
			newLayer = new DrawingMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = importOSM(this, baseFileName, mapDocument, newLayer);
		}
		else if (fn.endsWith(".osb")) {
			newLayer = new OsbMapLayer( baseFileName, fn );
			mapDocument->add(newLayer);
			importOK = mapDocument->importOSB(fn, (DrawingMapLayer *)newLayer);
		}
		else if (fn.endsWith(".ngt")) {
			newLayer = new TrackMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = importNGT(this, baseFileName, mapDocument, newLayer);
			if (importOK && MerkaartorPreferences::instance()->getAutoExtractTracks()) {
				((TrackMapLayer *)newLayer)->extractLayer();
			}
		}
		else if (fn.endsWith(".nmea") || (fn.endsWith(".nma"))) {
			newLayer = new TrackMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = mapDocument->importNMEA(baseFileName, (TrackMapLayer *)newLayer);
			if (importOK && MerkaartorPreferences::instance()->getAutoExtractTracks()) {
				((TrackMapLayer *)newLayer)->extractLayer();
			}
		}
		else if (fn.endsWith(".kml")) {
			if (QMessageBox::warning(this, MainWindow::tr("Big Fat Copyright Warning"),
					 MainWindow::tr(
					 "You are trying to import a KML file. Please be aware that:\n"
					 "\n"
					 " - You cannot import to OSM a KML file created from Google Earth. While you might\n"
					 "   think that nodes you created from GE are yours, they are not!\n"
					 "   They are still a derivative work from GE, and, as such, cannot be used in OSM.\n"
					 "\n"
					 " - If you downloaded it from the Internet, chances are that there is a copyright on it.\n"
					 "   Please be absolutely sure that using those data in OSM is permitted by the author, or\n"
					 "   that the data is public domain.\n"
					 "\n"
					 "If unsure, please seek advice on the \"legal\" or \"talk\" openstreetmap mailing lists.\n"
					 "\n"
					 "Are you absolutely sure this KML can legally be imported in OSM?"),
					 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::Yes)
			{
				newLayer = new DrawingMapLayer( baseFileName );
				newLayer->setUploadable(false);
				mapDocument->add(newLayer);
				importOK = mapDocument->importKML(baseFileName, (TrackMapLayer *)newLayer);
			} else
				importAborted = true;
		}
#ifdef USE_GDAL
		else if (fn.endsWith(".shp")) {
			newLayer = new DrawingMapLayer( baseFileName );
			newLayer->setUploadable(false);
			mapDocument->add(newLayer);
			importOK = mapDocument->importSHP(baseFileName, (DrawingMapLayer*)newLayer);
		}
#endif

		if (!importOK && newLayer)
			mapDocument->remove(newLayer);

		if (importOK)
		{
			foundImport = true;

			if (importedFileNames)
				importedFileNames->append(fn);
		}
		else
		if (!importAborted)
		{
 			delete newLayer;
			QMessageBox::warning(this, tr("No valid file"), tr("%1 could not be opened.").arg(fn));
		}
	}
#ifndef Q_OS_SYMBIAN
	QApplication::restoreOverrideCursor();
#endif
	SAFE_DELETE(theProgressDialog);

	return foundImport;
}

void MainWindow::loadFiles(const QStringList & fileList)
{

	QStringList fileNames(fileList);

#ifdef GEOIMAGE
	QStringList images = fileList.filter(".jpg", Qt::CaseInsensitive);
	if (!images.isEmpty()) {
		theGeoImage->loadImages(images);
		QString cur;
		foreach (cur, images)
			fileNames.removeAll(cur);
	}
#endif
	
	if (fileNames.isEmpty())
		return;
#ifndef Q_OS_SYMBIAN
	QApplication::setOverrideCursor(Qt::BusyCursor);
#endif
	theLayers->setUpdatesEnabled(false);
	view()->setUpdatesEnabled(false);

        // Load only the first merkaartor document
	bool foundDocument = false;
	QMutableStringListIterator it(fileNames);
	while (it.hasNext())
	{
		const QString & fn = it.next();

		if (fn.endsWith(".mdc") == false)
			continue;

		if (foundDocument == false)
		{
			changeCurrentDirToFile(fn);
			loadDocument(fn);
			foundDocument = true;
		}

		it.remove();
	}

	MapDocument* newDoc = theDocument;
	if (foundDocument == false) {
		newDoc = new MapDocument(theLayers);
		newDoc->addDefaultLayers();
	}

	QStringList openedFiles;
	bool foundImport = importFiles(newDoc, fileNames, &openedFiles);

	foreach (QString currentFileName, openedFiles)
		M_PREFS->addRecentOpen(currentFileName);

	updateRecentOpenMenu();

	theProperties->setSelection(0);

	if (foundDocument == false)
	{
 		if (foundImport)
		{
			// only imported some tracks
			delete theDocument;
			theDocument = newDoc;
			connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
			theDirty->updateList();
			theView->setDocument(theDocument);
			on_viewZoomAllAction_triggered();
		}
		else
		{
			// we didn't really open anything successfully
			delete newDoc;
		}
	} else {
		if (foundImport) {
			on_viewZoomAllAction_triggered();
		}
	}

	on_editPropertiesAction_triggered();
	theDocument->history().setActions(editUndoAction, editRedoAction, fileUploadAction);

	theLayers->setUpdatesEnabled(true);
	view()->setUpdatesEnabled(true);
#ifndef Q_OS_SYMBIAN
	QApplication::restoreOverrideCursor();
#endif
}

void MainWindow::on_fileOpenAction_triggered()
{
	if (theDocument->hasUnsavedChanges() && !mayDiscardUnsavedChanges(this))
		return;

	QStringList fileNames = QFileDialog::getOpenFileNames(
					this,
					tr("Open track files"),
					"", FILTER_OPEN_SUPPORTED);

	loadFiles(fileNames);
}

void MainWindow::on_fileUploadAction_triggered()
{

	if (QString(qVersion()) < "4.3.3")
	{
		if (QMessageBox::question(this,
			tr("Old Qt version detected"),
			tr("Your setup uses Qt %1, which contains various known errors in uploading "
			"data to OpenStreetMap leading to 401 server response codes. Are you sure you want to continue (which is not "
			"recommended).\n"
			"For more information see http://wiki.openstreetmap.org/index.php/Problem_uploading_with_Merkaartor").arg(qVersion()),QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
			return;
	}

	MerkaartorPreferences* p = MerkaartorPreferences::instance();
	while (p->getOsmUser().isEmpty()) {
		int ret = QMessageBox::warning(this, tr("Upload OSM"), tr("You don't seem to have specified your\n"
			"OpenStreetMap username and password.\nDo you want to do this now?"), QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			toolsPreferencesAction_triggered(true);
		} else
			return;
	}
	on_editPropertiesAction_triggered();
	syncOSM(this, p->getOsmWebsite(), p->getOsmUser(), p->getOsmPassword());

	theDocument->history().updateActions();
	theDirty->updateList();
}

void MainWindow::on_fileDownloadAction_triggered()
{
	createProgressDialog();

	if (downloadOSM(this, theView->viewport(), theDocument)) {
		on_editPropertiesAction_triggered();
	} else
		QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));

	SAFE_DELETE(theProgressDialog);
	theProgressBar = NULL;
	theProgressLabel = NULL;

	updateBookmarksMenu();
}

void MainWindow::on_fileDownloadMoreAction_triggered()
{
	createProgressDialog();

	if (!downloadMoreOSM(this, theView->viewport(), theDocument)) {
		QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
	}

	SAFE_DELETE(theProgressDialog);
	theProgressBar = NULL;
	theProgressLabel = NULL;
}

void MainWindow::on_fileWorkOfflineAction_triggered()
{
	M_PREFS->setOfflineMode(!M_PREFS->getOfflineMode());
	updateMenu();
}

void MainWindow::on_helpAboutAction_triggered()
{
	QDialog dlg(this);
	Ui::AboutDialog About;
	About.setupUi(&dlg);
	About.Version->setText(About.Version->text().arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
	About.QTVersion->setText(About.QTVersion->text().arg(qVersion()).arg(QT_VERSION_STR));
	QFile ct(":/Utils/CHANGELOG");
	ct.open(QIODevice::ReadOnly);
	QTextStream cl(&ct);
	About.txtChangelog->setPlainText(cl.readAll());
	QPixmap px(":/Utils/Merkaartor_About.png");
	About.pxIcon->setPixmap(px);
	About.lblUrl->setOpenExternalLinks(true);
	dlg.exec();
}

void MainWindow::on_viewZoomAllAction_triggered()
{
	QPair<bool, CoordBox> BBox(theDocument->boundingBox());
	if (BBox.first) {
		BBox.second.resize(1.01);
		theView->setViewport(BBox.second, theView->rect());
		invalidateView();
	}
}

void MainWindow::on_viewZoomInAction_triggered()
{
	theView->zoom(1.33333, theView->rect().center(), theView->rect());
	invalidateView();
}

void MainWindow::on_viewZoomOutAction_triggered()
{
	theView->zoom(0.75, theView->rect().center(), theView->rect());
	invalidateView();
}

void MainWindow::on_viewZoomWindowAction_triggered()
{
	theView->launch(new ZoomInteraction(theView));
}

void MainWindow::on_viewDownloadedAction_triggered()
{
	M_PREFS->setDownloadedVisible(!M_PREFS->getDownloadedVisible());
	viewDownloadedAction->setChecked(M_PREFS->getDownloadedVisible());
	invalidateView();
}

void MainWindow::on_viewScaleAction_triggered()
{
	M_PREFS->setScaleVisible(!M_PREFS->getScaleVisible());
	viewScaleAction->setChecked(M_PREFS->getScaleVisible());
	invalidateView();
}

void MainWindow::on_viewStyleBackgroundAction_triggered()
{
	M_PREFS->setStyleBackgroundVisible(!M_PREFS->getStyleBackgroundVisible());
	viewStyleBackgroundAction->setChecked(M_PREFS->getStyleBackgroundVisible());
	invalidateView();
}

void MainWindow::on_viewStyleForegroundAction_triggered()
{
	M_PREFS->setStyleForegroundVisible(!M_PREFS->getStyleForegroundVisible());
	viewStyleForegroundAction->setChecked(M_PREFS->getStyleForegroundVisible());
	invalidateView();
}

void MainWindow::on_viewStyleTouchupAction_triggered()
{
	M_PREFS->setStyleTouchupVisible(!M_PREFS->getStyleTouchupVisible());
	viewStyleTouchupAction->setChecked(M_PREFS->getStyleTouchupVisible());
	invalidateView();
}

void MainWindow::on_viewNamesAction_triggered()
{
	M_PREFS->setNamesVisible(!M_PREFS->getNamesVisible());
	viewNamesAction->setChecked(M_PREFS->getNamesVisible());
	invalidateView();
}

void MainWindow::on_viewTrackPointsAction_triggered()
{
	M_PREFS->setTrackPointsVisible(!M_PREFS->getTrackPointsVisible());
	viewTrackPointsAction->setChecked(M_PREFS->getTrackPointsVisible());
	invalidateView();
}

void MainWindow::on_viewTrackSegmentsAction_triggered()
{
	M_PREFS->setTrackSegmentsVisible(!M_PREFS->getTrackSegmentsVisible());
	viewTrackSegmentsAction->setChecked(M_PREFS->getTrackSegmentsVisible());
	invalidateView();
}

void MainWindow::on_viewRelationsAction_triggered()
{
	M_PREFS->setRelationsVisible(!M_PREFS->getRelationsVisible());
	viewRelationsAction->setChecked(M_PREFS->getRelationsVisible());
	invalidateView();
}

void MainWindow::on_viewGotoAction_triggered()
{
	GotoDialog* Dlg = new GotoDialog(*theView, this);
	if (Dlg->exec() == QDialog::Accepted) {
		if (!Dlg->newViewport().isNull()) {
			theView->setViewport(Dlg->newViewport(), theView->rect());
			invalidateView();
		}
	}
	delete Dlg;
}

void MainWindow::on_viewArrowsNeverAction_triggered(bool checked)
{
	if (checked) {
		M_PREFS->setDirectionalArrowsVisible(DirectionalArrows_Never);
		invalidateView();
	}
}

void MainWindow::on_viewArrowsOnewayAction_triggered(bool checked)
{
	if (checked) {
		M_PREFS->setDirectionalArrowsVisible(DirectionalArrows_Oneway);
		invalidateView();
	}
}

void MainWindow::on_viewArrowsAlwaysAction_triggered(bool checked)
{
	if (checked) {
		M_PREFS->setDirectionalArrowsVisible(DirectionalArrows_Always);
		invalidateView();
	}
}

void MainWindow::on_fileNewAction_triggered()
{
	theView->launch(0);
	theProperties->setSelection(0);
	if (!theDocument || !theDocument->hasUnsavedChanges() || mayDiscardUnsavedChanges(this)) {
		delete theDocument;
		theDocument = new MapDocument(theLayers);
		theDocument->addDefaultLayers();
		if (M_PREFS->getWorldOsbAutoload() && !M_PREFS->getWorldOsbUri().isEmpty()) {
			MapLayer* newLayer = new OsbMapLayer( "World", M_PREFS->getWorldOsbUri() + "/world.osb" );
			if (M_PREFS->getWorldOsbAutoshow())
				newLayer->setVisible(true);
			else
				newLayer->setVisible(false);
			newLayer->setReadonly(true);

			theDocument->add(newLayer);
		}

		theView->setDocument(theDocument);
		theDocument->history().setActions(editUndoAction, editRedoAction, fileUploadAction);
		connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
		theDirty->updateList();

		fileName = "";
		setWindowTitle(QString("Merkaartor - untitled"));

		on_editPropertiesAction_triggered();
		adjustLayers(true);
	}
}

void MainWindow::on_createDoubleWayAction_triggered()
{
	theView->launch(new CreateDoubleWayInteraction(this, theView));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createRoundaboutAction_triggered()
{
	theView->launch(new CreateRoundaboutInteraction(this, theView));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createPolygonAction_triggered()
{
	int Sides = QInputDialog::getInteger(this, MainWindow::tr("Create Polygon"), MainWindow::tr("Specify the number of sides"), 3, 3);
	theView->launch(new CreatePolygonInteraction(this, theView, Sides));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createRectangleAction_triggered()
{
	theView->launch(new CreatePolygonInteraction(this, theView, 4));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createRoadAction_triggered()
{
	TrackPoint * firstPoint = NULL;

	if (theProperties->size() == 1)
	{
		MapFeature * feature = theProperties->selection(0);
		firstPoint = dynamic_cast<TrackPoint*>(feature);
	}

	theView->launch(new CreateSingleWayInteraction(this, theView, firstPoint, false));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createCurvedRoadAction_triggered()
{
	theView->launch(new CreateSingleWayInteraction(this, theView, NULL, true));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createAreaAction_triggered()
{
	theView->launch(new CreateAreaInteraction(this, theView));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createNodeAction_triggered()
{
	theView->launch(new CreateNodeInteraction(theView));
	theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_roadJoinAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Join Roads"), NULL);
	joinRoads(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_roadSplitAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Split Roads"), NULL);
	splitRoads(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_roadBreakAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Break Roads"), NULL);
	breakRoads(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_featureCommitAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Force Feature upload"), NULL);
	commitFeatures(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_roadCreateJunctionAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Create Junction"), NULL);
	createJunction(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_nodeAlignAction_triggered()
{
	//MapFeature* F = theView->properties()->selection(0);
	CommandList* theList = new CommandList(MainWindow::tr("Align Nodes"), NULL);
	alignNodes(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
	//	theView->properties()->setSelection(F);
		invalidateView();
	}
}

void MainWindow::on_nodeMergeAction_triggered()
{
	MapFeature* F = theProperties->selection(0);
	CommandList* theList = new CommandList(MainWindow::tr("Merge Nodes into %1").arg(F->id()), F);
	mergeNodes(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		theProperties->setSelection(F);
		invalidateView();
	}
}

void MainWindow::on_nodeDetachAction_triggered()
{
	MapFeature* F = theProperties->selection(0);
	CommandList* theList = new CommandList(MainWindow::tr("Detach Node %1").arg(F->id()), F);
	detachNode(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		theProperties->setSelection(F);
		invalidateView();
	}
}

void MainWindow::on_relationAddMemberAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Add member to relation"), NULL);
	addRelationMember(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else {
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_relationRemoveMemberAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Remove member from relation"), NULL);
	removeRelationMember(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else {
		theDocument->addHistory(theList);
		invalidateView();
	}
}

void MainWindow::on_createRelationAction_triggered()
{
	Relation* R = new Relation;
	for (int i = 0; i < theProperties->size(); ++i)
		R->add("", theProperties->selection(i));
	CommandList* theList = new CommandList(MainWindow::tr("Create Relation %1").arg(R->description()), R);
	theList->add(
		new AddFeatureCommand(document()->getDirtyOrOriginLayer(), R, true));
	theDocument->addHistory(theList);
	theProperties->setSelection(R);
	invalidateView();
}

void MainWindow::on_editMapStyleAction_triggered()
{
	PaintStyleEditor* dlg = new PaintStyleEditor(this, M_STYLE->getGlobalPainter(), M_STYLE->getPainters());
	connect(dlg, SIGNAL(stylesApplied(QList<FeaturePainter>* )), this, SLOT(applyStyles(QList<FeaturePainter>* )));
	GlobalPainter saveGlobalPainter = M_STYLE->getGlobalPainter();
	QList<FeaturePainter> savePainters = M_STYLE->getPainters();
	if (dlg->exec() == QDialog::Accepted) {
		M_STYLE->setGlobalPainter(dlg->theGlobalPainter);
		M_STYLE->setPainters(dlg->thePainters);
	} else {
		M_STYLE->setGlobalPainter(saveGlobalPainter);
		M_STYLE->setPainters(savePainters);
	}

	for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
		i.get()->invalidatePainter();
	invalidateView();
	delete dlg;
}

void MainWindow::applyStyles(QList<FeaturePainter>* thePainters)
{
	M_STYLE->setPainters(*thePainters);
	for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
		i.get()->invalidatePainter();
	invalidateView();
}

//MapLayer* MainWindow::activeLayer()
//{
////	return theLayers->activeLayer();
//	//The "active" layer is always the dirty layer
//	return theDocument->getDirtyLayer();
//}

MapView* MainWindow::view()
{
	return theView;
}

void MainWindow::on_mapStyleSaveAction_triggered()
{
	QString f = QFileDialog::getSaveFileName(this, tr("Save map style"), M_PREFS->M_PREFS->getCustomStyle(), tr("Merkaartor map style (*.mas)"));
	if (!f.isNull()) {
		if (!f.endsWith(".mas"))
			f.append(".mas");
		M_STYLE->savePainters(f);
	}
	updateStyleMenu();
}

void MainWindow::on_mapStyleLoadAction_triggered()
{
	QString f = QFileDialog::getOpenFileName(this, tr("Load map style"), QString(), tr("Merkaartor map style (*.mas)"));
	if (!f.isNull()) {
		M_STYLE->loadPainters(f);
		for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			i.get()->invalidatePainter();
		invalidateView();
	}
}

void MainWindow::on_toolsWorldOsbAction_triggered()
{
	WorldOsbManager osbMgr(this);
	osbMgr.setViewport(theView->viewport().toQRectF());
	osbMgr.exec();
}

void MainWindow::on_toolsWMSServersAction_triggered()
{
	WMSPreferencesDialog* WMSPref;
	WMSPref = new WMSPreferencesDialog();
	if (WMSPref->exec() == QDialog::Accepted) {
		for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
			ImgIt.get()->updateWidget();
		adjustLayers(true);
	}
}

void MainWindow::on_toolsTMSServersAction_triggered()
{
	TMSPreferencesDialog* TMSPref;
	TMSPref = new TMSPreferencesDialog();
	if (TMSPref->exec() == QDialog::Accepted) {
		for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt)
			ImgIt.get()->updateWidget();
		adjustLayers(true);
	}
}

void MainWindow::on_toolsResetDiscardableAction_triggered()
{
	QSettings Sets;
	Sets.remove("DiscardableDialogs");
}

void MainWindow::on_toolsShortcutsAction_triggered()
{
	QList<QAction*> theActions;
	theActions.append(fileNewAction);
	theActions.append(fileOpenAction);
	theActions.append(fileImportAction);
	theActions.append(fileSaveAction);
	theActions.append(fileSaveAsAction);
	theActions.append(fileDownloadAction);
	theActions.append(fileDownloadMoreAction);
	theActions.append(fileUploadAction);
	theActions.append(fileQuitAction);
	theActions.append(editUndoAction);
	theActions.append(editRedoAction);
	theActions.append(editCopyAction);
	theActions.append(editPasteFeatureAction);
	theActions.append(editPasteMergeAction);
	theActions.append(editPasteOverwriteAction);
	theActions.append(editRemoveAction);
	theActions.append(editPropertiesAction);
	theActions.append(editMoveAction);
	theActions.append(editRotateAction);
	theActions.append(editSelectAction);
	theActions.append(viewZoomAllAction);
	theActions.append(viewZoomWindowAction);
	theActions.append(viewZoomOutAction);
	theActions.append(viewZoomInAction);
	theActions.append(viewDownloadedAction);
	theActions.append(viewRelationsAction);
	theActions.append(viewScaleAction);
	theActions.append(viewNamesAction);
	theActions.append(viewTrackPointsAction);
	theActions.append(viewTrackSegmentsAction);
	theActions.append(viewGotoAction);
	theActions.append(gpsConnectAction);
	theActions.append(gpsReplayAction);
	theActions.append(gpsRecordAction);
	theActions.append(gpsPauseAction);
	theActions.append(gpsDisconnectAction);
	theActions.append(gpsCenterAction);
	theActions.append(createNodeAction);
	theActions.append(createRoadAction);
	theActions.append(createDoubleWayAction);
	theActions.append(createRoundaboutAction);
	theActions.append(createAreaAction);
	theActions.append(createRelationAction);
	theActions.append(createRectangleAction);
	theActions.append(createPolygonAction);
	theActions.append(featureCommitAction);
	theActions.append(roadSplitAction);
	theActions.append(roadBreakAction);
	theActions.append(roadJoinAction);
	theActions.append(editReverseAction);
	theActions.append(nodeMergeAction);
	theActions.append(nodeAlignAction);
	theActions.append(toolsPreferencesAction);
	theActions.append(toolsShortcutsAction);
	theActions.append(toolsWorldOsbAction);
	
	theActions.append(windowHideAllAction);
	theActions.append(windowPropertiesAction);
	theActions.append(windowLayersAction);
	theActions.append(windowInfoAction);
	theActions.append(windowDirtyAction);
	theActions.append(windowGPSAction);
	theActions.append(windowGeoimageAction);
	theActions.append(windowStylesAction);
	
	theActions.append(helpAboutAction);

	ActionsDialog dlg(theActions, this);
	dlg.exec();
}

void MainWindow::toolsPreferencesAction_triggered(bool focusData)
{
	PreferencesDialog* Pref = new PreferencesDialog(this);
	if (focusData)
		Pref->tabPref->setCurrentWidget(Pref->tabData);
	else
		Pref->tabPref->setCurrentIndex(p->lastPrefTabIndex);
	connect (Pref, SIGNAL(preferencesChanged()), this, SLOT(preferencesChanged()));
	Pref->exec();
	p->lastPrefTabIndex = Pref->tabPref->currentIndex();
}

void MainWindow::preferencesChanged(void)
{
#ifndef FORCED_CUSTOM_STYLE
    if (!M_PREFS->getMerkaartorStyle())
    {
        if (QApplication::style()->objectName() != p->defStyle)
            QApplication::setStyle(p->defStyle);
    }
    else
    {
        if (QApplication::style()->objectName() != "")
			QApplication::setStyle(QStyleFactory::create(M_PREFS->getMerkaartorStyleString()));
    }
#else
	QApplication::setStyle(QStyleFactory::create("skulpture"));
#endif
	
	updateStyleMenu();
	updateMenu();
	invalidateView(false);
}

void MainWindow::on_fileSaveAsAction_triggered()
{
	fileName = QFileDialog::getSaveFileName(this,
		tr("Save Merkaartor document"), QString("%1/%2.mdc").arg(MerkaartorPreferences::instance()->getWorkingDir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)"));

	if (fileName != "") {
		saveDocument();
	}

	M_PREFS->addRecentOpen(fileName);
	updateRecentOpenMenu();
}

void MainWindow::on_fileSaveAction_triggered()
{
	if (fileName != "") {
		saveDocument();
	} else {
		on_fileSaveAsAction_triggered();
	}
}

void MainWindow::saveDocument()
{
#ifndef Q_OS_SYMBIAN
	QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

	QDomElement root;
	QDomDocument* theXmlDoc;

	theXmlDoc = new QDomDocument();
	theXmlDoc->appendChild(theXmlDoc->createProcessingInstruction("xml", "version=\"1.0\""));
	root = theXmlDoc->createElement("MerkaartorDocument");
	root.setAttribute("version", "1.1");
	root.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

	theXmlDoc->appendChild(root);

	QProgressDialog progress("Saving document...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);

	theDocument->toXML(root, progress);
	theView->toXML(root);

	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
		QMessageBox::critical(this, tr("Unable to open save file"), tr("%1 could not be opened for writing.").arg(fileName));
		return;
	}
	file.write(theXmlDoc->toString().toUtf8());
	file.close();
	delete theXmlDoc;

	progress.setValue(progress.maximum());

	setWindowTitle(QString("Merkaartor - %1").arg(fileName));

#ifndef Q_OS_SYMBIAN
	QApplication::restoreOverrideCursor();
#endif

}

void MainWindow::loadDocument(QString fn)
{
	QFile file(fn);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
		return;
	}

	QDomDocument* theXmlDoc = new QDomDocument();
	if (!theXmlDoc->setContent(&file)) {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
		file.close();
		delete theXmlDoc;
		theXmlDoc = NULL;
		return;
	}
	file.close();

	QDomElement docElem = theXmlDoc->documentElement();
	if (docElem.tagName() != "MerkaartorDocument") {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid Merkaartor document.").arg(fn));
		return;
	}
	double version = docElem.attribute("version").toDouble();

	QProgressDialog progress("Loading document...", "Cancel", 0, 0);
	progress.setWindowModality(Qt::WindowModal);

	progress.setMaximum(progress.maximum() + theXmlDoc->elementsByTagName("relation").count());
	progress.setMaximum(progress.maximum() + theXmlDoc->elementsByTagName("way").count());
	progress.setMaximum(progress.maximum() + theXmlDoc->elementsByTagName("node").count());
	progress.setMaximum(progress.maximum() + theXmlDoc->elementsByTagName("trkpt").count());
	progress.setMaximum(progress.maximum() + theXmlDoc->elementsByTagName("wpt").count());

	QDomElement e = docElem.firstChildElement();
	while(!e.isNull()) {
		if (e.tagName() == "MapDocument") {
			MapDocument* newDoc = MapDocument::fromXML(e, version, theLayers, progress);

			if (progress.wasCanceled())
				break;

			if (newDoc) {
				theProperties->setSelection(0);
				delete theDocument;
				theDocument = newDoc;
				theView->setDocument(theDocument);
				on_editPropertiesAction_triggered();
				theDocument->history().setActions(editUndoAction, editRedoAction, fileUploadAction);
				connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
				theDirty->updateList();
				fileName = fn;
				setWindowTitle(QString("Merkaartor - %1").arg(fileName));
			}
		} else
		if (e.tagName() == "MapView") {
			view()->fromXML(e);
		}

		if (progress.wasCanceled())
			break;

		e = e.nextSiblingElement();
	}
	progress.reset();
	delete theXmlDoc;

	M_PREFS->addRecentOpen(fn);
	updateRecentOpenMenu();
}

void MainWindow::on_exportOSMAction_triggered()
{
	QList<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osm", tr("OSM Files (*.osm)"));

	if (fileName != "") {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		file.write(theDocument->exportOSM(theFeatures).toUtf8());
		file.close();
	}
}

void MainWindow::on_exportOSMBinAction_triggered()
{
	QList<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export Binary OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osb", tr("OSM Binary Files (*.osb)"));

	if (fileName != "") {
#ifndef Q_OS_SYMBIAN
		QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

		ImportExportOsmBin osb(document());
		if (osb.saveFile(fileName)) {
			osb.export_(theFeatures);
		}

#ifndef Q_OS_SYMBIAN
		QApplication::restoreOverrideCursor();
#endif
	}
}

void MainWindow::on_exportGPXAction_triggered()
{
	QList<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export GPX"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.gpx", tr("GPX Files (*.gpx)"));

	if (fileName != "") {
#ifndef Q_OS_SYMBIAN
		QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

		ExportGPX gpx(document());
		if (gpx.saveFile(fileName)) {
			gpx.export_(theFeatures);
		}

#ifndef Q_OS_SYMBIAN
		QApplication::restoreOverrideCursor();
#endif
	}
}

void MainWindow::on_exportKMLAction_triggered()
{
	QList<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export KML"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.kml", tr("KML Files (*.kml)"));

	if (fileName != "") {
#ifndef Q_OS_SYMBIAN
		QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

		ImportExportKML kml(document());
		if (kml.saveFile(fileName)) {
			kml.export_(theFeatures);
		}

#ifndef Q_OS_SYMBIAN
		QApplication::restoreOverrideCursor();
#endif
	}
}

bool MainWindow::selectExportedFeatures(QList<MapFeature*>& theFeatures)
{
	QDialog dlg(this);
	Ui::ExportDialog dlgExport;
	dlgExport.setupUi(&dlg);
	switch(MerkaartorPreferences::instance()->getExportType()) {
		case Export_All:
			dlgExport.rbAll->setChecked(true);
			break;
		case Export_Viewport:
			dlgExport.rbViewport->setChecked(true);
			break;
		case Export_Selected:
			dlgExport.rbSelected->setChecked(true);
			break;
		default:
			return false;

	}
	if (dlg.exec()) {
		if (dlgExport.rbAll->isChecked()) {
			theFeatures = document()->getFeatures();
			MerkaartorPreferences::instance()->setExportType(Export_All);
			return true;
		}
		if (dlgExport.rbViewport->isChecked()) {
			CoordBox aCoordBox = view()->viewport();

			theFeatures.clear();
			for (VisibleFeatureIterator i(document()); !i.isEnd(); ++i) {
				if (TrackPoint* P = dynamic_cast<TrackPoint*>(i.get())) {
					if (aCoordBox.contains(P->position())) {
						theFeatures.append(P);
					}
				} else
					if (Road* G = dynamic_cast<Road*>(i.get())) {
						if (aCoordBox.intersects(G->boundingBox())) {
							for (int j=0; j < G->size(); j++) {
								if (TrackPoint* P = dynamic_cast<TrackPoint*>(G->get(j)))
									if (!aCoordBox.contains(P->position()))
										theFeatures.append(P);
							}
							theFeatures.append(G);
						}
					} else
						//FIXME Not working for relation (not made of point?)
						if (Relation* G = dynamic_cast<Relation*>(i.get())) {
							if (aCoordBox.intersects(G->boundingBox())) {
								for (int j=0; j < G->size(); j++) {
									if (Road* R = dynamic_cast<Road*>(G->get(j))) {
										if (!aCoordBox.contains(R->boundingBox())) {
											for (int k=0; k < R->size(); k++) {
												if (TrackPoint* P = dynamic_cast<TrackPoint*>(R->get(k)))
													if (!aCoordBox.contains(P->position()))
														theFeatures.append(P);
											}
											theFeatures.append(R);
										}
									}
								}
								theFeatures.append(G);
							}
						}
			}
			MerkaartorPreferences::instance()->setExportType(Export_Viewport);
			return true;
		}
		if (dlgExport.rbSelected->isChecked()) {
			theFeatures = theProperties->selection();
			MerkaartorPreferences::instance()->setExportType(Export_Selected);
			return true;
		}
	}
	return false;
}

void MainWindow::on_editSelectAction_triggered()
{
	SelectionDialog* Sel = new SelectionDialog(this);

	if (Sel->exec() == QDialog::Accepted) {
		MerkaartorPreferences::instance()->setLastSearchName(Sel->edName->text());
		MerkaartorPreferences::instance()->setLastSearchKey(Sel->cbKey->currentText());
		MerkaartorPreferences::instance()->setLastSearchValue(Sel->cbValue->currentText());
		MerkaartorPreferences::instance()->setLastMaxSearchResults(Sel->sbMaxResult->value());

		Qt::CaseSensitivity theSensitivity = Qt::CaseInsensitive;
		if (Sel->cbCaseSensitive->isChecked())
			theSensitivity = Qt::CaseSensitive;
		QRegExp selName(Sel->edName->text(), theSensitivity, QRegExp::RegExp);
		QRegExp selKey(Sel->cbKey->currentText(), theSensitivity, QRegExp::RegExp);
		QRegExp selValue(Sel->cbValue->currentText(), theSensitivity, QRegExp::RegExp);
		int selMaxResult = Sel->sbMaxResult->value();

		QList <MapFeature *> selection;
		int added = 0;
		for (VisibleFeatureIterator i(theDocument); !i.isEnd() && added < selMaxResult; ++i) {
			MapFeature* F = i.get();
			int ok = false;

			if (selName.indexIn(F->description()) == -1) {
				continue;
			}
			if (F->id().indexOf(Sel->edID->text(), theSensitivity) == -1) {
				continue;
			}
			for (int j=0; j < F->tagSize(); j++) {
				if ((selKey.indexIn(F->tagKey(j)) > -1) && (selValue.indexIn(F->tagValue(j)) > -1)) {
					ok = true;
					break;
				}
			}
			if (ok) {
				selection.push_back(F);
				++added;
			}
		}
		theProperties->setMultiSelection(selection);
		view()->properties()->checkMenuStatus();
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if (theDocument->hasUnsavedChanges() && !mayDiscardUnsavedChanges(this)) {
		event->ignore();
		return;
	}

	MerkaartorPreferences::instance()->saveMainWindowState( this );
	MerkaartorPreferences::instance()->setInitialPosition(theView);
//4.3237,50.8753,4.3378,50.8838
	QMainWindow::closeEvent(event);
}

void MainWindow::on_renderNativeAction_triggered()
{
	NativeRenderDialog osmR(theDocument, theView->viewport(), this);
	osmR.exec();
}

void MainWindow::updateBookmarksMenu()
{
	for(int i=menuBookmarks->actions().count()-1; i > 2 ; i--) {
		menuBookmarks->removeAction(menuBookmarks->actions()[3]);
	}

	BookmarkListIterator it(*(M_PREFS->getBookmarks()));
	while (it.hasNext()) {
		it.next();
		if (it.value().deleted == false) {
			QAction* a = new QAction(it.key(), menuBookmarks);
			menuBookmarks->addAction(a);
		}
	}
}

void MainWindow::updateRecentOpenMenu()
{
	for(int i=menuRecentOpen->actions().count()-1; i >= 0; i--) {
		menuRecentOpen->removeAction(menuRecentOpen->actions()[0]);
	}

	if (!M_PREFS->getRecentOpen().size()) {
		menuRecentOpen->setEnabled(false);
		return;
	}

	menuRecentOpen->setEnabled(true);
	QStringList RecentOpen = M_PREFS->getRecentOpen();
	for (int i=0; i<RecentOpen.size(); i++) {
		QAction* a = new QAction(RecentOpen[i], menuRecentOpen);
		menuRecentOpen->addAction(a);
	}
}

void MainWindow::updateRecentImportMenu()
{
	for(int i=menuRecentImport->actions().count()-1; i >= 0; i--) {
		menuRecentImport->removeAction(menuRecentImport->actions()[0]);
	}

	if (!M_PREFS->getRecentImport().size()) {
		menuRecentImport->setEnabled(false);
		return;
	}

	menuRecentImport->setEnabled(true);
	QStringList RecentImport = M_PREFS->getRecentImport();
	for (int i=0; i<RecentImport.size(); i++) {
		QAction* a = new QAction(RecentImport[i], menuRecentImport);
		menuRecentImport->addAction(a);
	}
}

void MainWindow::updateProjectionMenu()
{
#ifndef _MOBILE
	QActionGroup* actgrp = new QActionGroup(this);
	foreach (QString name, M_PREFS->getProjectionsList().getProjections().keys()) {
		QAction* a = new QAction(name, mnuProjections);
		actgrp->addAction(a);
		a->setCheckable (true);
		if (M_PREFS->getProjectionType() == name)
			a->setChecked(true);
		mnuProjections->addAction(a);
	}
	connect (mnuProjections, SIGNAL(triggered(QAction *)), this, SLOT(projectionTriggered(QAction *)));
#else
	mnuProjections->setVisible(false);
#endif
}

void MainWindow::updateStyleMenu()
{
	for(int i=menuStyles->actions().count()-1; i > 4 ; i--) {
		menuStyles->removeAction(menuStyles->actions()[5]);
	}
	p->theStyle->clearItems();

	QActionGroup* actgrp = new QActionGroup(this);
	QDir intStyles(BUILTIN_STYLES_DIR);
    for (int i=0; i < intStyles.entryList().size(); ++i) {
		QAction* a = new QAction(QString(tr("%1 (int)")).arg(intStyles.entryList().at(i)), menuStyles);
		actgrp->addAction(a);
		a->setCheckable(true);
		a->setData(QVariant(intStyles.entryInfoList().at(i).absoluteFilePath()));
		menuStyles->addAction(a);
		if (intStyles.entryInfoList().at(i).absoluteFilePath() == M_PREFS->getDefaultStyle())
			a->setChecked(true);
		p->theStyle->addItem(a);
	}
    if (!M_PREFS->getCustomStyle().isEmpty()) {
        QDir customStyles(M_PREFS->getCustomStyle(), "*.mas");
        for (int i=0; i < customStyles.entryList().size(); ++i) {
 			QAction* a = new QAction(customStyles.entryList().at(i), menuStyles);
			actgrp->addAction(a);
			a->setCheckable(true);
			a->setData(QVariant(customStyles.entryInfoList().at(i).absoluteFilePath()));
			menuStyles->addAction(a);
			if (customStyles.entryInfoList().at(i).absoluteFilePath() == M_PREFS->getDefaultStyle())
				a->setChecked(true);
			p->theStyle->addItem(a);
       }
    }
}

void MainWindow::updateWindowMenu(bool)
{
    windowPropertiesAction->setChecked(theProperties->isVisible());
    windowLayersAction->setChecked(theLayers->isVisible());
    windowInfoAction->setChecked(theInfo->isVisible());
    windowDirtyAction->setChecked(theDirty->isVisible());
    windowGPSAction->setChecked(theGPS->isVisible());
#ifdef GEOIMAGE
    windowGeoimageAction->setChecked(theGeoImage->isVisible());
#endif
    windowStylesAction->setChecked(p->theStyle->isVisible());
}

void MainWindow::on_bookmarkAddAction_triggered()
{
	bool ok = true;
	QString text;

	BookmarkList* Bookmarks = M_PREFS->getBookmarks();
	QStringList bkName;
	BookmarkListIterator i(*Bookmarks);
	while (i.hasNext()) {
		i.next();
		if (i.value().deleted == false)
			bkName.append(i.key());
	}
	while (ok) {
		text = QInputDialog::getItem(this, MainWindow::tr("Add Bookmark"),
						MainWindow::tr("Specify the name of the bookmark."), bkName, 0, true, &ok);
		if (ok) {
			if (text.isEmpty()) {
				QMessageBox::critical(this, tr("Invalid bookmark name"),
					tr("Bookmark cannot be blank."), QMessageBox::Ok);
				continue;
			}
			if (Bookmarks->contains(text)) {
				QString newBk = QInputDialog::getText(this, MainWindow::tr("Warning: Bookmark name already exists"),
						MainWindow::tr("Enter a new one, keep the same to overwrite or cancel."), QLineEdit::Normal,
									   text, &ok);
				if (ok && Bookmarks->contains(newBk)) {
					for(int i=2; i < menuBookmarks->actions().count(); i++) {
						if (menuBookmarks->actions()[i]->text() == newBk) {
							menuBookmarks->removeAction(menuBookmarks->actions()[i]);
							break;
						}
					}
				}
				text = newBk;
			}
			break;
		}
	}
	if (ok) {
		CoordBox Clip = view()->viewport();
		Bookmark B(text, Clip);
		Bookmarks->insert(text, B);
		M_PREFS->save();

		QAction* a = new QAction(text, menuBookmarks);
		menuBookmarks->addAction(a);
	}
}

void MainWindow::on_bookmarkRemoveAction_triggered()
{
	bool ok;

	BookmarkList* Bookmarks = M_PREFS->getBookmarks();
	QStringList bkName;
	BookmarkListIterator i(*Bookmarks);
	while (i.hasNext()) {
		i.next();
		if (i.value().deleted == false)
			bkName.append(i.key());
	}
	QString item = QInputDialog::getItem(this, MainWindow::tr("Remove Bookmark"),
						MainWindow::tr("Select the bookmark to remove."), bkName, 0, false, &ok);
	if (ok) {
		Bookmark B = Bookmarks->value(item);
		B.deleted = true;
		Bookmarks->insert(item, B);
		M_PREFS->save();

		for(int i=2; i < menuBookmarks->actions().count(); i++) {
			if (menuBookmarks->actions()[i]->text() == item) {
				menuBookmarks->removeAction(menuBookmarks->actions()[i]);
				break;
			}
		}
	}
}

void MainWindow::bookmarkTriggered(QAction* anAction)
{
	if (anAction == bookmarkAddAction || anAction == bookmarkRemoveAction)
		return;
	BookmarkList* Bookmarks = M_PREFS->getBookmarks();
	theView->setViewport(Bookmarks->value(anAction->text()).Coordinates, theView->rect());

	invalidateView();
}

void MainWindow::recentOpenTriggered(QAction* anAction)
{
	if (theDocument->hasUnsavedChanges() && !mayDiscardUnsavedChanges(this))
		return;

	QStringList fileNames(anAction->text());
	loadFiles(fileNames);
}

void MainWindow::recentImportTriggered(QAction* anAction)
{
	view()->setUpdatesEnabled(false);
	theLayers->setUpdatesEnabled(false);

	QStringList fileNames(anAction->text());
	QStringList importedFiles;
	importFiles(theDocument, fileNames, &importedFiles);

	foreach (QString currentFileName, importedFiles)
		M_PREFS->addRecentImport(currentFileName);

	updateRecentImportMenu();

	view()->setUpdatesEnabled(true);
	theLayers->setUpdatesEnabled(true);

	on_editPropertiesAction_triggered();
	theDocument->history().setActions(editUndoAction, editRedoAction, fileUploadAction);
}

#ifndef _MOBILE
void MainWindow::projectionTriggered(QAction* anAction)
{
	if(theView->projection().setProjectionType((ProjectionType)anAction->text()))
		M_PREFS->setProjectionType(anAction->text());
	else
		QMessageBox::critical(this, tr("Invalid projection"), tr("Unable to set projection \"%1\".").arg(anAction->text()));
	theView->setViewport(theView->viewport(), theView->rect());
	invalidateView();
}
#endif

void MainWindow::styleTriggered(QAction* anAction)
{
	if (!anAction->isCheckable())
		return;

	QString NewStyle = anAction->data().toString();
	if (NewStyle != M_PREFS->getDefaultStyle())
	{
		M_PREFS->setDefaultStyle(NewStyle);
		M_STYLE->loadPainters(M_PREFS->getDefaultStyle());
		for (FeatureIterator it(document()); !it.isEnd(); ++it)
		{
			it.get()->invalidatePainter();
		}
	}
	p->theStyle->setCurrent(anAction);

	invalidateView(false);
}

void MainWindow::on_windowPropertiesAction_triggered()
{
	theProperties->setVisible(!theProperties->isVisible());
	windowPropertiesAction->setChecked(theProperties->isVisible());
}

void MainWindow::on_windowLayersAction_triggered()
{
	theLayers->setVisible(!theLayers->isVisible());
	windowLayersAction->setChecked(theLayers->isVisible());
}

void MainWindow::on_windowInfoAction_triggered()
{
	theInfo->setVisible(!theInfo->isVisible());
	windowInfoAction->setChecked(theInfo->isVisible());
}

void MainWindow::on_windowDirtyAction_triggered()
{
	theDirty->setVisible(!theDirty->isVisible());
	windowDirtyAction->setChecked(theDirty->isVisible());
}

void MainWindow::on_windowToolbarAction_triggered()
{
	toolBar->setVisible(!toolBar->isVisible());
}

void MainWindow::on_windowGPSAction_triggered()
{
	theGPS->setVisible(!theGPS->isVisible());
	windowGPSAction->setChecked(theGPS->isVisible());
}

#ifdef GEOIMAGE
void MainWindow::on_windowGeoimageAction_triggered()
{
	theGeoImage->setVisible(!theGeoImage->isVisible());
	windowGeoimageAction->setChecked(theGeoImage->isVisible());
}
#endif

void MainWindow::on_windowStylesAction_triggered()
{
	p->theStyle->setVisible(!p->theStyle->isVisible());
	windowStylesAction->setChecked(p->theStyle->isVisible());
}

void MainWindow::on_windowHideAllAction_triggered()
{
	fullscreenState = saveState(1);

	windowHideAllAction->setEnabled(false);
	windowHideAllAction->setVisible(false);
	windowShowAllAction->setEnabled(true);
	windowShowAllAction->setVisible(true);

//	toolBar->setVisible(false);
	theInfo->setVisible(false);
	theDirty->setVisible(false);
	theLayers->setVisible(false);
	theProperties->setVisible(false);
	theGPS->setVisible(false);
	p->theStyle->setVisible(false);
#ifdef GEOIMAGE
	theGeoImage->setVisible(false);
#endif
}

void MainWindow::on_windowShowAllAction_triggered()
{
	restoreState(fullscreenState, 1);

	windowHideAllAction->setEnabled(true);
	windowHideAllAction->setVisible(true);
	windowShowAllAction->setEnabled(false);
	windowShowAllAction->setVisible(false);
}

void MainWindow::on_gpsConnectAction_triggered()
{
#ifndef Q_OS_SYMBIAN
#ifdef USEGPSD
	QGPSDDevice* aGps = new QGPSDDevice("gpsd");
#else
	QGPSComDevice* aGps = new QGPSComDevice(M_PREFS->getGpsPort());
#endif
#else
	QGPSS60Device* aGps = new QGPSS60Device();
#endif
	if (aGps->openDevice()) {
		connect(aGps, SIGNAL(updatePosition(float, float, QDateTime, float, float, float)), 
			this, SLOT(updateGpsPosition(float, float, QDateTime, float, float, float)));

		gpsConnectAction->setEnabled(false);
		gpsReplayAction->setEnabled(false);
		gpsDisconnectAction->setEnabled(true);
		gpsRecordAction->setEnabled(true);
		gpsPauseAction->setEnabled(true);
		theGPS->setGpsDevice(aGps);
		theGPS->resetGpsStatus();
		theGPS->startGps();
	} else {
		QMessageBox::critical(this, tr("GPS error"),
			tr("Unable to open GPS port."), QMessageBox::Ok);
		delete aGps;
	}
}

void MainWindow::on_gpsReplayAction_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(
					this,
					tr("Open NMEA log file"),
					"", "NMEA GPS log format (*.nmea *.nma)" );

	if (fileName.isEmpty())
		return;

	QGPSFileDevice* aGps = new QGPSFileDevice(fileName);
	if (aGps->openDevice()) {
		connect(aGps, SIGNAL(updatePosition(float, float, QDateTime, float, float, float)), 
			this, SLOT(updateGpsPosition(float, float, QDateTime, float, float, float)));

		gpsConnectAction->setEnabled(false);
		gpsReplayAction->setEnabled(false);
		gpsDisconnectAction->setEnabled(true);
		gpsRecordAction->setEnabled(true);
		gpsPauseAction->setEnabled(true);

		theGPS->setGpsDevice(aGps);
		theGPS->resetGpsStatus();
		theGPS->startGps();
	}
}

void MainWindow::on_gpsDisconnectAction_triggered()
{
	gpsConnectAction->setEnabled(true);
	gpsReplayAction->setEnabled(true);
	gpsDisconnectAction->setEnabled(false);
	gpsRecordAction->setEnabled(false);
	gpsPauseAction->setEnabled(false);
	gpsRecordAction->setChecked(false);
	gpsPauseAction->setChecked(false);

	disconnect(theGPS->getGpsDevice(), SIGNAL(updatePosition(float, float, QDateTime, float, float, float)), 
		this, SLOT(updateGpsPosition(float, float, QDateTime, float, float, float)));
	theGPS->stopGps();
	theGPS->resetGpsStatus();
}

void MainWindow::updateGpsPosition(float latitude, float longitude, QDateTime time, float altitude, float speed, float heading)
{
	Q_UNUSED(heading)
	if (theGPS->getGpsDevice()) {
		Coord gpsCoord(angToInt(latitude), angToInt(longitude));
		if (gpsCenterAction->isChecked()) {
			CoordBox vp = theView->viewport();
			QRect vpr = vp.toRect().adjusted(vp.lonDiff() / 4, vp.latDiff() / 4, -vp.lonDiff() / 4, -vp.latDiff() / 4);
			if (!vpr.contains(gpsCoord.toQPoint())) {
				theView->setCenter(gpsCoord, theView->rect());
				theView->invalidate(true, true);
			}
		}

		if (gpsRecordAction->isChecked() && !gpsPauseAction->isChecked()) {
			TrackPoint* pt = new TrackPoint(gpsCoord);
			pt->setTime(time);
			pt->setElevation(altitude);
			pt->setSpeed(speed);
			gpsRecLayer->add(pt);
			curGpsTrackSegment->add(pt);
			theView->invalidate(true, false);
		}
	}
	theView->update();
}

QGPS* MainWindow::gps()
{
	return theGPS;
}

void MainWindow::on_gpsCenterAction_triggered()
{
	M_PREFS->setGpsMapCenter(!M_PREFS->getGpsMapCenter());
	gpsCenterAction->setChecked(M_PREFS->getGpsMapCenter());
	invalidateView();
}

void MainWindow::on_gpsRecordAction_triggered()
{
	if (gpsRecordAction->isChecked()) {
		if (theDocument) {
			QString fn = "log-" + QDateTime::currentDateTime().toString(Qt::ISODate);
			fn.replace(':', '-');

			gpsRecLayer = new TrackMapLayer();
			gpsRecLayer->setName(fn);
			theDocument->add(gpsRecLayer);

			curGpsTrackSegment = new TrackSegment();
			gpsRecLayer->add(curGpsTrackSegment);
		} else {
			gpsRecordAction->setChecked(false);
		}
	} else {
		gpsPauseAction->setChecked(false);
	}
}
void MainWindow::on_gpsPauseAction_triggered()
{
	if (gpsPauseAction->isChecked()) {
		if (!gpsRecordAction->isChecked()) {
			gpsPauseAction->setChecked(false);
		}
	} else {
		if (theDocument && gpsRecordAction->isChecked()) {
			curGpsTrackSegment = new TrackSegment();
			gpsRecLayer->add(curGpsTrackSegment);
		}
	}
}

void MainWindow::on_toolTemplatesSaveAction_triggered()
{
	QString f = QFileDialog::getSaveFileName(this, tr("Save Tag Templates"), "", tr("Merkaartor tag templates (*.mat)"));
	if (!f.isNull()) {
		if (!f.endsWith(".mat"))
			f.append(".mat");
		theProperties->saveTemplates(f);
	}
}

void MainWindow::on_toolTemplatesMergeAction_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(
					this,
					tr("Open Tag Templates"),
					"", "Merkaartor tag templates (*.mat)" );

	if (fileName.isEmpty())
		return;

	theProperties->mergeTemplates(fileName);
	theProperties->resetValues();
}

void MainWindow::on_toolTemplatesLoadAction_triggered()
{
	QString fileName = QFileDialog::getOpenFileName(
					this,
					tr("Open Tag Templates"),
					"", "Merkaartor tag templates (*.mat)" );

	if (fileName.isEmpty())
		return;

	theProperties->loadTemplates(fileName);
	theProperties->resetValues();
}

void MainWindow::updateLanguage()
{
    if (qtTranslator) {
        QCoreApplication::removeTranslator(qtTranslator);
    }
    if (merkaartorTranslator) {
        QCoreApplication::removeTranslator(merkaartorTranslator);
    }
    QString DefaultLanguage = getDefaultLanguage();
	if (DefaultLanguage != "-" && DefaultLanguage != "en")
    {
        qtTranslator = new QTranslator;
    #ifdef TRANSDIR_SYSTEM
        bool retQt;
        if (!QDir::isAbsolutePath(STRINGIFY(TRANSDIR_SYSTEM)))
            retQt = qtTranslator->load("qt_" + DefaultLanguage, QCoreApplication::applicationDirPath() + "/" + STRINGIFY(TRANSDIR_SYSTEM));
        else
            retQt = qtTranslator->load("qt_" + DefaultLanguage, STRINGIFY(TRANSDIR_SYSTEM));
    #else
        bool retQt = qtTranslator->load("qt_" + DefaultLanguage, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    #endif
        if (retQt)
	        QCoreApplication::installTranslator(qtTranslator);

        merkaartorTranslator = new QTranslator;
		// Do not prevent Merkaartor translations to be loaded, even if there is no Qt translation for the language.

		// First, try in the app dir
        bool retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, QCoreApplication::applicationDirPath());
    #ifdef TRANSDIR_MERKAARTOR
        if (!retM) {
			// Next, try the TRANSDIR_MERKAARTOR, if defined
            if (!QDir::isAbsolutePath(STRINGIFY(TRANSDIR_MERKAARTOR)))
                retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, QCoreApplication::applicationDirPath() + "/" + STRINGIFY(TRANSDIR_MERKAARTOR));
            else
                retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, STRINGIFY(TRANSDIR_MERKAARTOR));
        }
    #endif

		if (retM)
			QCoreApplication::installTranslator(merkaartorTranslator);

		if (!retQt && !retM)
        {
			QMessageBox::warning(this, tr("Error"), tr("Could not load the selected language. Go to Tools, Preferences to select another language or check whether the translation file is missing."));
		} else
			if (!retQt)
				statusBar()->showMessage(tr("Warning! Could not load the Qt translations for the \"%1\" language.").arg(DefaultLanguage), 15000);
			else
				if (!retM)
					statusBar()->showMessage(tr("Warning! Could not load the Merkaartor translations for the \"%1\" language.").arg(DefaultLanguage), 15000);
    }
    retranslateUi(this);

	QStringList shortcuts = M_PREFS->getShortcuts();
	for (int i=0; i<shortcuts.size(); i+=2) {
		QAction* act = findChild<QAction*>(shortcuts[i]);
		act->setShortcut(QKeySequence(shortcuts[i+1]));
	}
}

void MainWindow::mapView_interactionChanged(Interaction* anInteraction)
{
	editPropertiesAction->setChecked(dynamic_cast<EditInteraction*>(anInteraction) != NULL);
	editMoveAction->setChecked(dynamic_cast<MoveTrackPointInteraction*>(anInteraction) != NULL);
	createNodeAction->setChecked(dynamic_cast<CreateNodeInteraction*>(anInteraction) != NULL);
	createRoadAction->setChecked(dynamic_cast<CreateSingleWayInteraction*>(anInteraction) != NULL);
	createAreaAction->setChecked(dynamic_cast<CreateAreaInteraction*>(anInteraction) != NULL);
}

void MainWindow::updateMenu()
{
	if (M_PREFS->getOfflineMode()) {
		fileWorkOfflineAction->setChecked(true);
		fileDownloadAction->setEnabled(false);
		fileDownloadMoreAction->setEnabled(false);
		fileUploadAction->setEnabled(false);
	} else {
		fileWorkOfflineAction->setChecked(false);
		fileDownloadAction->setEnabled(true);
		fileDownloadMoreAction->setEnabled(true);
		fileUploadAction->setEnabled(true);
	}
	
	if (M_PREFS->getSeparateMoveMode())
		editMoveAction->setVisible(true);
	else
		editMoveAction->setVisible(false);
}

void MainWindow::on_layersNewImageAction_triggered()
{
	if (theDocument)
		theDocument->addImageLayer();
}
