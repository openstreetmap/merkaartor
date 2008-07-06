#include "MainWindow.h"

#include "LayerDock.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "DirtyDock.h"
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
#include "Interaction/CreateSingleWayInteraction.h"
#include "Interaction/EditInteraction.h"
#include "Interaction/MoveTrackPointInteraction.h"
#include "Interaction/ZoomInteraction.h"
#include "Map/Coord.h"
#include "Map/DownloadOSM.h"
#include "Map/ImportGPX.h"
#include "Map/ImportNGT.h"
#include "Map/ImportOSM.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/RoadManipulations.h"
#include "Map/TrackPoint.h"
#include "PaintStyle/EditPaintStyle.h"
#include "PaintStyle/PaintStyleEditor.h"
#include "Sync/SyncOSM.h"
#include <ui_AboutDialog.h>
#include <ui_UploadMapDialog.h>
#include <ui_SetPositionDialog.h>
#include <ui_SelectionDialog.h>
#include <ui_ExportDialog.h>
#include "Preferences/PreferencesDialog.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Utils/SelectionDialog.h"
#include "QMapControl/imagemanager.h"
#include "QMapControl/mapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#ifdef OSMARENDER
	#include "Render/OsmaRender.h"
#endif

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

MainWindow::MainWindow(void)
		: fileName(""), theDocument(0), theXmlDoc(0)
{
	setupUi(this);
	loadPainters(MerkaartorPreferences::instance()->getDefaultStyle());

	theView = new MapView(this);
	setCentralWidget(theView);

	theLayers = new LayerDock(this);
	theLayers->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, theLayers);

	theDocument = new MapDocument(theLayers);
	theView->setDocument(theDocument);
	addAction(viewMoveLeftAction);
	addAction(viewMoveRightAction);
	addAction(viewMoveUpAction);
	addAction(viewMoveDownAction);
	theDocument->history().setActions(editUndoAction, editRedoAction);


	theProperties = new PropertiesDock(this);
	theProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theProperties);
	on_editPropertiesAction_triggered();

	theInfo = new InfoDock(this);
	theInfo->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theInfo);

	theDirty = new DirtyDock(this);
	theDirty->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theDirty);

	connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
	connect (theLayers, SIGNAL(layersChanged(bool)), this, SLOT(adjustLayers(bool)));

	updateBookmarksMenu();
	updateProjectionMenu();
	connect (menuBookmarks, SIGNAL(triggered(QAction *)), this, SLOT(bookmarkTriggered(QAction *)));

	MerkaartorPreferences::instance()->restoreMainWindowState( this );
	CoordBox initialPosition = MerkaartorPreferences::instance()->getInitialPosition();
	theView->projection().setViewport(initialPosition, theView->rect());
	viewDownloadedAction->setChecked(MerkaartorPreferences::instance()->getDownloadedVisible());

	connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));

#ifndef OSMARENDER
	//TODO Osmarender rendering
	renderAction->setVisible(false);
#endif
}

MainWindow::~MainWindow(void)
{
	MerkaartorPreferences::instance()->setWorkingDir(QDir::currentPath());
	delete MerkaartorPreferences::instance();
	delete theDocument;
	delete theView;
}


void MainWindow::adjustLayers(bool adjustViewport)
{
	if (adjustViewport) {
		if (MerkaartorPreferences::instance()->getProjectionType() == Proj_Background)
			theView->projection().setViewport(theView->projection().viewport(), theView->rect());
		else
			theView->projection().zoom(1, QPoint(theView->width() / 2, theView->height() / 2), theView->rect());
	}
	invalidateView(true);
}

void MainWindow::invalidateView(bool UpdateDock)
{
	theView->invalidate();
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

MapDocument* MainWindow::document()
{
	return theDocument;
}

MapDocument* MainWindow::getDocumentFromClipboard()
{
	QClipboard *clipboard = QApplication::clipboard();
	QDomDocument* theXmlDoc = new QDomDocument();
	if (!theXmlDoc->setContent(clipboard->text())) {
		QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard is not valid XML."));
		return NULL;
	}

	QDomElement c = theXmlDoc->documentElement();

	if (c.tagName() != "osm") {
		QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard do not contain valid OSM."));
		return NULL;
	}

	MapDocument* NewDoc = new MapDocument(NULL);
	MapLayer* l = NewDoc->getDirtyLayer();

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

	return NewDoc;
}

void MainWindow::on_editCopyAction_triggered()
{
	QClipboard *clipboard = QApplication::clipboard();
	clipboard->setText(theDocument->exportOSM(view()->properties()->selection()));
	invalidateView();
}

void MainWindow::on_editPasteOverwriteAction_triggered()
{
	QVector<MapFeature*> sel = properties()->selection();
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
	QVector<MapFeature*> sel = properties()->selection();
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
	QDomDocument* theXmlDoc = new QDomDocument();
	if (!theXmlDoc->setContent(clipboard->text())) {
		return;
	}

	QDomElement c = theXmlDoc->documentElement();

	if (c.tagName() != "osm") {
		return;
	}

	editPasteFeaturesAction->setEnabled(true);
	editPasteMergeAction->setEnabled(true);
	editPasteOverwriteAction->setEnabled(true);
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
	theProperties->setSelection(0);
	invalidateView();
	theView->launch(new EditInteraction(theView));
}

void MainWindow::on_editRemoveAction_triggered()
{
	emit remove_triggered();
}

void MainWindow::on_editMoveAction_triggered()
{
	view()->launch(new MoveTrackPointInteraction(view()));
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


#define FILTER_OPEN_SUPPORTED \
	tr("Supported formats (*.mdc *.gpx *.osm *.osb *.ngt *.nmea *.nme)\n" \
	"Merkaartor document (*.mdc)\n" \
	"GPS Exchange format (*.gpx)\n" \
	"OpenStreetMap format (*.osm)\n" \
	"OpenStreetMap binary format (*.osb)\n" \
	"Noni GPSPlot format (*.ngt)\n" \
	"NMEA GPS log format (*.nmea *.nme)\n" \
	"All Files (*)")
#define FILTER_IMPORT_SUPPORTED \
	tr("Supported formats (*.gpx *.osm *.osb *.ngt *.nmea *.nme)\n" \
	"GPS Exchange format (*.gpx)\n" \
	"OpenStreetMap format (*.osm)\n" \
	"OpenStreetMap binary format (*.osb)\n" \
	"Noni GPSPlot format (*.ngt)\n" \
	"NMEA GPS log format (*.nmea *.nme)\n" \
	"All Files (*)")

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

	importFiles(theDocument, fileNames);

	view()->setUpdatesEnabled(true);
	theLayers->setUpdatesEnabled(true);

	on_editPropertiesAction_triggered();
	theDocument->history().setActions(editUndoAction, editRedoAction);
}

static bool mayDiscardUnsavedChanges(QWidget* aWidget)
{
	return QMessageBox::question(aWidget, MainWindow::tr("Unsaved changes"),
								 MainWindow::tr("The current map contains unsaved changes that will be lost when starting a new one.\n"
												"Do you want to cancel starting a new map or continue and discard the old changes?"),
								 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
}

bool MainWindow::importFiles(MapDocument * mapDocument, const QStringList & fileNames)
{
	QApplication::setOverrideCursor(Qt::WaitCursor);

	bool foundImport = false;

	QStringListIterator it(fileNames);
	while (it.hasNext())
	{
		const QString & fn = it.next();
		changeCurrentDirToFile(fn);

		QString baseFileName = fn.section('/', - 1);
		MapLayer* newLayer = NULL;

		bool importOK = false;

		if (fn.endsWith(".gpx")) {
			newLayer = new TrackMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = importGPX(this, baseFileName, mapDocument, newLayer);
			if (importOK & MerkaartorPreferences::instance()->getAutoExtractTracks()) {
				((TrackMapLayer *)newLayer)->extractLayer();
			}
		}
		else if (fn.endsWith(".osm")) {
			newLayer = new DrawingMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = importOSM(this, baseFileName, mapDocument, newLayer);
		}
		else if (fn.endsWith(".osb")) {
			newLayer = new DrawingMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = mapDocument->importOSB(baseFileName, (DrawingMapLayer *)newLayer);
		}
		else if (fn.endsWith(".ngt")) {
			newLayer = new TrackMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = importNGT(this, baseFileName, mapDocument, newLayer);
			if (importOK & MerkaartorPreferences::instance()->getAutoExtractTracks()) {
				((TrackMapLayer *)newLayer)->extractLayer();
			}
		}
		else if (fn.endsWith(".nmea") || (fn.endsWith(".nme"))) {
			newLayer = new TrackMapLayer( baseFileName );
			mapDocument->add(newLayer);
			importOK = mapDocument->importNMEA(baseFileName, (TrackMapLayer *)newLayer);
			if (importOK & MerkaartorPreferences::instance()->getAutoExtractTracks()) {
				((TrackMapLayer *)newLayer)->extractLayer();
			}
		}

		if (!importOK && newLayer)
			mapDocument->remove(newLayer);

		if (importOK)
		{
			foundImport = true;
		}
		else
		{
 			delete newLayer;
			QMessageBox::warning(this, tr("No valid file"), tr("%1 could not be opened.").arg(fn));
		}
	}
	QApplication::restoreOverrideCursor();

	return foundImport;
}

void MainWindow::loadFiles(const QStringList & fileList)
{
	if (fileList.isEmpty())
		return;

	QStringList fileNames(fileList);

	QApplication::setOverrideCursor(Qt::WaitCursor);
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
	if (foundDocument == false)
		newDoc = new MapDocument(theLayers);

	bool foundImport = importFiles(newDoc, fileNames);

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
	theDocument->history().setActions(editUndoAction, editRedoAction);

	theLayers->setUpdatesEnabled(true);
	view()->setUpdatesEnabled(true);
	QApplication::restoreOverrideCursor();
}

void MainWindow::on_fileOpenAction_triggered()
{
	if (hasUnsavedChanges(*theDocument) && !mayDiscardUnsavedChanges(this))
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
			toolsPreferencesAction_triggered(1);
		} else
			return;
	}
	on_editPropertiesAction_triggered();
	syncOSM(this, p->getOsmWebsite(), p->getOsmUser(), p->getOsmPassword(), p->getProxyUse(),
		p->getProxyHost(), p->getProxyPort());

	theDocument->history().updateActions();
	theDirty->updateList();
}

void MainWindow::on_fileDownloadAction_triggered()
{
	if (downloadOSM(this, theView->projection().viewport(), theDocument)) {
		on_editPropertiesAction_triggered();
	} else
		QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));

	updateBookmarksMenu();
}

void MainWindow::on_fileDownloadMoreAction_triggered()
{
	if (downloadMoreOSM(this, theView->projection().viewport(), theDocument)) {
		on_editPropertiesAction_triggered();
	} else
		QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
}

void MainWindow::on_helpAboutAction_triggered()
{
	QDialog dlg(this);
	Ui::AboutDialog About;
	About.setupUi(&dlg);
	About.Version->setText(About.Version->text().arg(VERSION));
	About.QTVersion->setText(About.QTVersion->text().arg(qVersion()).arg(QT_VERSION_STR));
	dlg.exec();
}

void MainWindow::on_viewZoomAllAction_triggered()
{
	std::pair<bool, CoordBox> BBox(boundingBox(theDocument));
	if (BBox.first) {
		BBox.second.resize(1.01);
		theView->projection().setViewport(BBox.second, theView->rect());
		invalidateView();
	}
}

void MainWindow::on_viewZoomInAction_triggered()
{
	theView->projection().zoom(1.33333, theView->rect().center(), theView->rect());
	invalidateView();
}

void MainWindow::on_viewZoomOutAction_triggered()
{
	theView->projection().zoom(0.75, theView->rect().center(), theView->rect());
	invalidateView();
}

void MainWindow::on_viewMoveLeftAction_triggered()
{
	QPoint p(theView->rect().width()/4,0);
	theView->projection().panScreen(p,  theView->rect());
	invalidateView();
}
void MainWindow::on_viewMoveRightAction_triggered()
{
	QPoint p(-theView->rect().width()/4,0);
	theView->projection().panScreen(p,  theView->rect());
	invalidateView();
}

void MainWindow::on_viewMoveUpAction_triggered()
{
	QPoint p(0,theView->rect().height()/4);
	theView->projection().panScreen(p,  theView->rect());
	invalidateView();
}

void MainWindow::on_viewMoveDownAction_triggered()
{
	QPoint p(0,-theView->rect().height()/4);
	theView->projection().panScreen(p,  theView->rect());
	invalidateView();
}

void MainWindow::on_viewZoomWindowAction_triggered()
{
	theView->launch(new ZoomInteraction(theView));
}

void MainWindow::on_viewDownloadedAction_triggered()
{
	MerkaartorPreferences::instance()->setDownloadedVisible(!MerkaartorPreferences::instance()->getDownloadedVisible());
	viewDownloadedAction->setChecked(MerkaartorPreferences::instance()->getDownloadedVisible());
	invalidateView();
}


void MainWindow::on_viewSetCoordinatesAction_triggered()
{
	QDialog* Dlg = new QDialog(this);
	Ui::SetPositionDialog Data;
	Data.setupUi(Dlg);
	CoordBox B(theView->projection().viewport());
	Data.Longitude->setText(QString::number(radToAng(B.center().lon())));
	Data.Latitude->setText(QString::number(radToAng(B.center().lat())));
	Data.SpanLongitude->setText(QString::number(radToAng(B.lonDiff())));
	Data.SpanLatitude->setText(QString::number(radToAng(B.latDiff())));
	if (Dlg->exec() == QDialog::Accepted) {
		theView->projection().setViewport(CoordBox(
											   Coord(
												   angToRad(Data.Latitude->text().toDouble() - Data.SpanLatitude->text().toDouble() / 2),
												   angToRad(Data.Longitude->text().toDouble() - Data.SpanLongitude->text().toDouble() / 2)),
											   Coord(
												   angToRad(Data.Latitude->text().toDouble() + Data.SpanLatitude->text().toDouble() / 2),
												   angToRad(Data.Longitude->text().toDouble() + Data.SpanLongitude->text().toDouble() / 2))), theView->rect());
		invalidateView();
	}
	delete Dlg;
}

void MainWindow::on_fileNewAction_triggered()
{
	theView->launch(0);
	theProperties->setSelection(0);
	if (!hasUnsavedChanges(*theDocument) || mayDiscardUnsavedChanges(this)) {
		delete theDocument;
		theDocument = new MapDocument(theLayers);
		theView->setDocument(theDocument);
		theDocument->history().setActions(editUndoAction, editRedoAction);
		connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
		theDirty->updateList();
		invalidateView();
	}
}

void MainWindow::on_createDoubleWayAction_triggered()
{
	theView->launch(new CreateDoubleWayInteraction(this, theView));
}

void MainWindow::on_createRoundaboutAction_triggered()
{
	theView->launch(new CreateRoundaboutInteraction(this, theView));
}

void MainWindow::on_createRoadAction_triggered()
{
	TrackPoint * firstPoint = NULL;	

	if (theView->properties()->size() == 1)
	{
		MapFeature * feature = theView->properties()->selection(0);
		firstPoint = dynamic_cast<TrackPoint*>(feature);
	}
	
	theView->launch(new CreateSingleWayInteraction(this, theView, firstPoint, false));
}

void MainWindow::on_createCurvedRoadAction_triggered()
{
	theView->launch(new CreateSingleWayInteraction(this, theView, NULL, true));
}

void MainWindow::on_createAreaAction_triggered()
{
	theView->launch(new CreateAreaInteraction(this, theView));
}

void MainWindow::on_createNodeAction_triggered()
{
	theView->launch(new CreateNodeInteraction(theView));
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
		theDocument->addHistory(theList);
}

void MainWindow::on_roadBreakAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Break Roads"), NULL);
	breakRoads(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->addHistory(theList);
}

void MainWindow::on_featureCommitAction_triggered()
{
	CommandList* theList = new CommandList(MainWindow::tr("Commit Roads"), NULL);
	commitFeatures(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->addHistory(theList);
}

void MainWindow::on_createRelationAction_triggered()
{
	Relation* R = new Relation;
	for (unsigned int i = 0; i < theProperties->size(); ++i)
		R->add("", theProperties->selection(i));
	CommandList* theList = new CommandList(MainWindow::tr("Create Relation %1").arg(R->description()), R);
	theList->add(
		new AddFeatureCommand(document()->getDirtyLayer(), R, true));
	theDocument->addHistory(theList);
	theProperties->setSelection(R);
}

void MainWindow::on_editMapStyleAction_triggered()
{
	PaintStyleEditor* dlg = new PaintStyleEditor(this, EditPaintStyle::Painters);
	if (dlg->exec() == QDialog::Accepted) {
		EditPaintStyle::Painters = dlg->thePainters;
		for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			i.get()->invalidatePainter();
		invalidateView();
	}
	delete dlg;
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
	QString f = QFileDialog::getSaveFileName(this, tr("Save map style"), QString(), tr("Merkaartor map style (*.mas)"));
	if (!f.isNull())
		savePainters(f);
}

void MainWindow::on_mapStyleLoadAction_triggered()
{
	QString f = QFileDialog::getOpenFileName(this, tr("Load map style"), QString(), tr("Merkaartor map style (*.mas)"));
	if (!f.isNull()) {
		loadPainters(f);
		for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
			i.get()->invalidatePainter();
		invalidateView();
	}
}

void MainWindow::toolsPreferencesAction_triggered(unsigned int tabidx)
{
	PreferencesDialog* Pref = new PreferencesDialog(this);
	Pref->tabPref->setCurrentIndex(tabidx);
	connect (Pref, SIGNAL(preferencesChanged()), this, SLOT(preferencesChanged()));
	Pref->exec();
}

void MainWindow::preferencesChanged(void)
{
	theDocument->getImageLayer()->setMapAdapter(MerkaartorPreferences::instance()->getBgType());
	theDocument->getImageLayer()->updateWidget();
	adjustLayers(true);

	ImageManager::instance()->setCacheDir(MerkaartorPreferences::instance()->getCacheDir());
	ImageManager::instance()->setCacheMaxSize(MerkaartorPreferences::instance()->getCacheSize());
	if (MerkaartorPreferences::instance()->getProxyUse()) {
		ImageManager::instance()->setProxy(MerkaartorPreferences::instance()->getProxyHost(),
			MerkaartorPreferences::instance()->getProxyPort());
	} else {
		ImageManager::instance()->setProxy("",0);
	}
}

void MainWindow::on_fileSaveAsAction_triggered()
{
	fileName = QFileDialog::getSaveFileName(this,
		tr("Save Merkaartor document"), QString("%1/%2.mdc").arg(MerkaartorPreferences::instance()->getWorkingDir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)"));

	if (fileName != "") {
		saveDocument();
	}
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
	QFile file(fileName);
	if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
		return;

	QApplication::setOverrideCursor(Qt::WaitCursor);

	QDomElement root;
	if (!theXmlDoc) {
		theXmlDoc = new QDomDocument();
		theXmlDoc->appendChild(theXmlDoc->createProcessingInstruction("xml", "version=\"1.0\""));
		root = theXmlDoc->createElement("MerkaartorDocument");
		root.setAttribute("version", "1.1");
		root.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

		theXmlDoc->appendChild(root);
	} else {
		root = theXmlDoc->documentElement();
		root.setAttribute("version", "1.1");
	}

	theDocument->toXML(root);
	theView->toXML(root);

	QTextStream out(&file);
	out.setCodec("UTF-8");
	theXmlDoc->save(out,2);
	file.close();

	setWindowTitle(QString("Merkaartor - %1").arg(fileName));

	QApplication::restoreOverrideCursor();

}

void MainWindow::loadDocument(QString fn)
{
	theXmlDoc = new QDomDocument();
	QFile file(fn);
	if (!file.open(QIODevice::ReadOnly)) {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
		return;
	}
	if (!theXmlDoc->setContent(&file)) {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid XML file.").arg(fn));
		file.close();
		return;
	}
	file.close();

	QDomElement docElem = theXmlDoc->documentElement();
	if (docElem.tagName() != "MerkaartorDocument") {
		QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid Merkaartor document.").arg(fn));
		return;
	}
	double version = docElem.attribute("version").toDouble();

	QDomElement e = docElem.firstChildElement();
	while(!e.isNull()) {
		if (e.tagName() == "MapDocument") {
			MapDocument* newDoc = MapDocument::fromXML(e, version, theLayers);
			if (newDoc) {
				theProperties->setSelection(0);
				delete theDocument;
				theDocument = newDoc;
				theView->setDocument(theDocument);
				on_editPropertiesAction_triggered();
				theDocument->history().setActions(editUndoAction, editRedoAction);
				connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
				theDirty->updateList();
			} else {
				delete newDoc;
			}
		} else
		if (e.tagName() == "MapView") {
			view()->fromXML(e);
		}

		e = e.nextSiblingElement();
	}
	fileName = fn;
	setWindowTitle(QString("Merkaartor - %1").arg(fileName));
}

void MainWindow::on_exportOSMAction_triggered()
{
	QVector<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osm", tr("OSM Files (*.osm)"));

	if (fileName != "") {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);
		out << theDocument->exportOSM(theFeatures);
		file.close();
	}
}

void MainWindow::on_exportOSMBinAction_triggered()
{
	QVector<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export Binary OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osb", tr("OSM Binary Files (*.osb)"));

	if (fileName != "") {
		QApplication::setOverrideCursor(Qt::WaitCursor);
	
		ImportExportOsmBin osb(document());
		if (osb.saveFile(fileName)) {
			osb.export_(theFeatures);
		}
	
		QApplication::restoreOverrideCursor();
	}
}

void MainWindow::on_exportGPXAction_triggered()
{
	QVector<MapFeature*> theFeatures;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export GPX"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.gpx", tr("GPX Files (*.gpx)"));

	if (fileName != "") {
		QApplication::setOverrideCursor(Qt::WaitCursor);

		for (VisibleFeatureIterator i(document()); !i.isEnd(); ++i) {
			if (dynamic_cast<TrackMapLayer*>(i.get()->layer())) {
				theFeatures.push_back(i.get());
			}
		}

		ExportGPX gpx(document());
		if (gpx.saveFile(fileName)) {
			gpx.export_(theFeatures);
		}

		QApplication::restoreOverrideCursor();
	}
}

void MainWindow::on_exportKMLAction_triggered()
{
	QVector<MapFeature*> theFeatures;
	if (!selectExportedFeatures(theFeatures))
		return;

	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export KML"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.kml", tr("KML Files (*.kml)"));

	if (fileName != "") {
		QApplication::setOverrideCursor(Qt::WaitCursor);
	
		ImportExportKML kml(document());
		if (kml.saveFile(fileName)) {
			kml.export_(theFeatures);
		}
	
		QApplication::restoreOverrideCursor();
	}
}

bool MainWindow::selectExportedFeatures(QVector<MapFeature*>& theFeatures)
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
			CoordBox aCoordBox = view()->projection().viewport();

			theFeatures.clear();
			for (VisibleFeatureIterator i(document()); !i.isEnd(); ++i) {
				if (TrackPoint* P = dynamic_cast<TrackPoint*>(i.get())) {
					if (aCoordBox.contains(P->position())) {
						theFeatures.append(P);
					}
				} else
					if (Road* G = dynamic_cast<Road*>(i.get())) {
						if (aCoordBox.intersects(G->boundingBox())) {
							for (unsigned int j=0; j < G->size(); j++) {
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
								for (unsigned int j=0; j < G->size(); j++) {
									if (Road* R = dynamic_cast<Road*>(G->get(j))) {
										if (!aCoordBox.contains(R->boundingBox())) {
											for (unsigned int k=0; k < R->size(); k++) {
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
			theFeatures = view()->properties()->selection();
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

		QRegExp selName(Sel->edName->text(), Qt::CaseInsensitive, QRegExp::RegExp);
		QRegExp selKey(Sel->cbKey->currentText(), Qt::CaseInsensitive, QRegExp::RegExp);
		QRegExp selValue(Sel->cbValue->currentText(), Qt::CaseInsensitive, QRegExp::RegExp);
		int selMaxResult = Sel->sbMaxResult->value();

		std::vector <MapFeature *> selection;
		int added = 0;
		for (VisibleFeatureIterator i(theDocument); !i.isEnd() && added < selMaxResult; ++i) {
			MapFeature* F = i.get();
			if (selName.indexIn(F->description()) == -1) {
				continue;
			}
			int ok = false;
			for (unsigned int j=0; j < F->tagSize(); j++) {
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
	}
}

void MainWindow::closeEvent(QCloseEvent * event)
{
	if (hasUnsavedChanges(*theDocument) && !mayDiscardUnsavedChanges(this)) {
		event->ignore();
		return;
	}

	MerkaartorPreferences::instance()->saveMainWindowState( this );

	CoordBox currentPosition = view()->projection().viewport();
	MerkaartorPreferences::instance()->setInitialPosition( currentPosition );

	QMainWindow::closeEvent(event);
}

void MainWindow::on_renderAction_triggered()
{
#ifdef OSMARENDER
	OsmaRender osmR;
	osmR.render(this, theDocument, view()->projection().viewport());
#endif
}

void MainWindow::updateBookmarksMenu()
{
	for(int i=menuBookmarks->actions().count()-1; i > 2 ; i--) {
		menuBookmarks->removeAction(menuBookmarks->actions()[3]);
	}

	QStringList Bookmarks = MerkaartorPreferences::instance()->getBookmarks();
	for (int i=0; i<Bookmarks.size(); i+=5) {
		QAction* a = new QAction(Bookmarks[i], menuBookmarks);
		menuBookmarks->addAction(a);
	}
}

void MainWindow::updateProjectionMenu()
{
	QStringList Projections = MerkaartorPreferences::instance()->getProjectionTypes();
	QActionGroup* actgrp = new QActionGroup(this);
	for (int i=0; i<Projections.size(); i++) {
		QAction* a = new QAction(Projections[i], mnuProjections);
		actgrp->addAction(a);
		a->setCheckable (true);
		if (i == (int)MerkaartorPreferences::instance()->getProjectionType())
			a->setChecked(true);
		mnuProjections->addAction(a);
	}

	connect (mnuProjections, SIGNAL(triggered(QAction *)), this, SLOT(projectionTriggered(QAction *)));
}

void MainWindow::on_bookmarkAddAction_triggered()
{
	bool ok = true;
	QString text;

	QStringList Bookmarks = MerkaartorPreferences::instance()->getBookmarks();
	QStringList bkName;
	for (int i=0; i<Bookmarks.size(); i+=5) {
		bkName.append(Bookmarks[i]);
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
			if (Bookmarks.contains(text)) {
				QString newBk = QInputDialog::getText(this, MainWindow::tr("Warning: Bookmark name already exists"),
						MainWindow::tr("Enter a new one, keep the same to overwrite or cancel."), QLineEdit::Normal,
									   text, &ok);
				if (ok && Bookmarks.contains(newBk)) {
					int i = Bookmarks.indexOf(newBk);
					Bookmarks.removeAt(i);
					Bookmarks.removeAt(i);
					Bookmarks.removeAt(i);
					Bookmarks.removeAt(i);
					Bookmarks.removeAt(i);

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
		CoordBox Clip = view()->projection().viewport();
		int idx = Bookmarks.size();
		Bookmarks.append(text);
		Bookmarks.append(QString::number(radToAng(Clip.bottomLeft().lat())));
		Bookmarks.append(QString::number(radToAng(Clip.bottomLeft().lon())));
		Bookmarks.append(QString::number(radToAng(Clip.topRight().lat())));
		Bookmarks.append(QString::number(radToAng(Clip.topRight().lon())));
		MerkaartorPreferences::instance()->setBookmarks(Bookmarks);

		QAction* a = new QAction(Bookmarks[idx], menuBookmarks);
		a->setData(idx);
		menuBookmarks->addAction(a);
	}
}

void MainWindow::on_bookmarkRemoveAction_triggered()
{
	bool ok;

	QStringList Bookmarks = MerkaartorPreferences::instance()->getBookmarks();
	QStringList bkName;
	for (int i=0; i<Bookmarks.size(); i+=5) {
		bkName.append(Bookmarks[i]);
	}
	QString item = QInputDialog::getItem(this, MainWindow::tr("Remove Bookmark"),
						MainWindow::tr("Select the bookmark to remove."), bkName, 0, false, &ok);
	if (ok) {
		int i = Bookmarks.indexOf(item);
		Bookmarks.removeAt(i);
		Bookmarks.removeAt(i);
		Bookmarks.removeAt(i);
		Bookmarks.removeAt(i);
		Bookmarks.removeAt(i);
		MerkaartorPreferences::instance()->setBookmarks(Bookmarks);

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
	QStringList Bookmarks = MerkaartorPreferences::instance()->getBookmarks();
	int idx = Bookmarks.indexOf(anAction->text()) + 1;
	CoordBox Clip = CoordBox(Coord(angToRad(Bookmarks[idx].toDouble()),angToRad(Bookmarks[idx+1].toDouble())),
		Coord(angToRad(Bookmarks[idx+2].toDouble()),angToRad(Bookmarks[idx+3].toDouble())));
	theView->projection().setViewport(Clip, theView->rect());

	invalidateView();
}

void MainWindow::projectionTriggered(QAction* anAction)
{
	QStringList Projections = MerkaartorPreferences::instance()->getProjectionTypes();
	int idx = Projections.indexOf(anAction->text());
	MerkaartorPreferences::instance()->setProjectionType((ProjectionType)idx);
	theView->projection().setViewport(theView->projection().viewport(), theView->rect());
	invalidateView();
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
	MapFeature* F = theView->properties()->selection(0);
	CommandList* theList = new CommandList(MainWindow::tr("Merge Nodes into %1").arg(F->id()), F);
	mergeNodes(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->addHistory(theList);
		theView->properties()->setSelection(F);
		invalidateView();
	}
}

void MainWindow::on_windowPropertiesAction_triggered()
{
	theProperties->setVisible(!theProperties->isVisible());
}

void MainWindow::on_windowLayersAction_triggered()
{
	theLayers->setVisible(!theLayers->isVisible());
}

void MainWindow::on_windowInfoAction_triggered()
{
	theInfo->setVisible(!theInfo->isVisible());
}

void MainWindow::on_windowDirtyAction_triggered()
{
	theDirty->setVisible(!theDirty->isVisible());
}
