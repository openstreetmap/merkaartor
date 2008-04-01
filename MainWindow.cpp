#include "MainWindow.h"

#include "LayerDock.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Interaction/CreateAreaInteraction.h"
#include "Interaction/CreateDoubleWayInteraction.h"
#include "Interaction/CreateNodeInteraction.h"
#include "Interaction/CreateRoundaboutInteraction.h"
#include "Interaction/CreateSingleWayInteraction.h"
#include "Interaction/EditInteraction.h"
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
#include "GeneratedFiles/ui_AboutDialog.h"
#include "GeneratedFiles/ui_UploadMapDialog.h"
#include "GeneratedFiles/ui_SetPositionDialog.h"
#include "GeneratedFiles/ui_SelectionDialog.h"
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

MainWindow::MainWindow(void)
		: theDocument(0)
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
	theDocument->history().setActions(editUndoAction, editRedoAction);

	theProperties = new PropertiesDock(this);
	theProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theProperties);
	on_editPropertiesAction_triggered();
	QDir::setCurrent(MerkaartorPreferences::instance()->getWorkingDir());

	connect (theLayers, SIGNAL(layersChanged(bool)), this, SLOT(adjustLayers(bool)));

	updateBookmarksMenu();
	updateProjectionMenu();

	MerkaartorPreferences::instance()->restoreMainWindowState( this );

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
}


void MainWindow::adjustLayers(bool adjustViewport)
{
	if (adjustViewport)
		view()->projection().setViewport(view()->projection().viewport(), view()->rect());
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

MapDocument* MainWindow::document()
{
	return theDocument;
}

void MainWindow::on_editRedoAction_triggered()
{
	theDocument->history().redo();
	invalidateView();
}

void MainWindow::on_editUndoAction_triggered()
{
	theDocument->history().undo();
	invalidateView();
}

void MainWindow::on_editPropertiesAction_triggered()
{
	theProperties->setSelection(0);
	invalidateView();
	//TODO: Fix memleak
	theView->launch(new EditInteraction(theView));
}

void MainWindow::on_editRemoveAction_triggered()
{
	emit remove_triggered();
}

void MainWindow::on_editMoveAction_triggered()
{
	emit move_triggered();
}

void MainWindow::on_editAddAction_triggered()
{
	emit add_triggered();
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


#define FILTER_LOAD_SUPPORTED \
	"Supported formats (*.gpx *.osm *.ngt *.nmea *.nme)\n" \
	"GPS Exchange format (*.gpx)\n" \
	"OpenStreetMap format (*.osm)\n" \
	"Noni GPSPlot format (*.ngt)\n" \
	"NMEA GPS log format (*.nmea *.nme)\n" \
	"All Files (*)"

void MainWindow::on_fileImportAction_triggered()
{
	QString s = QFileDialog::getOpenFileName(
					this,
					tr("Open track file"),
					"", tr(FILTER_LOAD_SUPPORTED));
	if (!s.isNull()) {
		changeCurrentDirToFile(s);
		bool OK = false;
		TrackMapLayer* NewLayer = new TrackMapLayer(tr("Import %1").arg(s.right(s.length() - s.lastIndexOf('/') - 1)));
		theDocument->add(NewLayer);
		if (s.right(4).toLower() == ".gpx") {
			OK = importGPX(this, s, theDocument, NewLayer);
		} else
			if (s.right(4).toLower() == ".osm") {
				view()->setUpdatesEnabled(false);
				OK = importOSM(this, s, theDocument, NewLayer);
				view()->setUpdatesEnabled(true);
			} else
				if (s.right(4).toLower() == ".ngt") {
					view()->setUpdatesEnabled(false);
					OK = importNGT(this, s, theDocument, NewLayer);
					view()->setUpdatesEnabled(true);
				} else
					if ((s.right(5).toLower() == ".nmea") || (s.right(4).toLower() == ".nme")) {
						view()->setUpdatesEnabled(false);
						if (theDocument->importNMEA(s))
							OK = true;
						view()->setUpdatesEnabled(true);
					}
		if (OK) {
			on_viewZoomAllAction_triggered();
			on_editPropertiesAction_triggered();
			theDocument->history().setActions(editUndoAction, editRedoAction);
		} else {
			theDocument->remove(NewLayer);
			delete NewLayer;
			QMessageBox::warning(this, tr("Not a valid file"), tr("The file could not be opened"));
		}
	}
}

static bool mayDiscardUnsavedChanges(QWidget* aWidget)
{
	return QMessageBox::question(aWidget, MainWindow::tr("Unsaved changes"),
								 MainWindow::tr("The current map contains unsaved changes that will be lost when starting a new one.\n"
												"Do you want to cancel starting a new map or continue and discard the old changes?"),
								 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
}

void MainWindow::loadFile(const QString & fn)
{
	if (fn.isNull())
		return;

	changeCurrentDirToFile(fn);

	theLayers->setUpdatesEnabled(false);
	MapDocument* NewDoc = new MapDocument(theLayers);

	QString NewLayerName = tr("Open %1").arg( fn.section('/', - 1));
	MapLayer* NewLayer = NULL;

	bool importOK = false;

	if (fn.endsWith(".gpx")) {
		NewLayer = new TrackMapLayer( NewLayerName );
		NewDoc->add(NewLayer);
		importOK = importGPX(this, fn, NewDoc, NewLayer);
	}
	else if (fn.endsWith(".osm")) {
		NewLayer = new DrawingMapLayer( NewLayerName );
		NewDoc->add(NewLayer);
		importOK = importOSM(this, fn, NewDoc, NewLayer);
	}
	else if (fn.endsWith(".ngt")) {
		NewLayer = new TrackMapLayer( NewLayerName );
		NewDoc->add(NewLayer);
		importOK = importNGT(this, fn, NewDoc, NewLayer);
	}
	else if (fn.endsWith(".nmea") || (fn.endsWith(".nme"))) {
		importOK = NewDoc->importNMEA(fn);
	}

	if (!importOK && NewLayer)
		NewDoc->remove(NewLayer);

	if (importOK == false) {
		delete NewDoc;
 		delete NewLayer;
		QMessageBox::warning(this, tr("No valid file"), tr("%1 could not be opened.").arg(fn));
		return;
	}

	theProperties->setSelection(0);
	delete theDocument;
	theDocument = NewDoc;
	theView->setDocument(theDocument);
	on_viewZoomAllAction_triggered();
	on_editPropertiesAction_triggered();
	theDocument->history().setActions(editUndoAction, editRedoAction);

	theLayers->setUpdatesEnabled(true);
}

void MainWindow::on_fileOpenAction_triggered()
{
	if (hasUnsavedChanges(*theDocument) && !mayDiscardUnsavedChanges(this))
		return;

	QString fileName = QFileDialog::getOpenFileName(
					this,
					tr("Open track file"),
					"", tr(FILTER_LOAD_SUPPORTED));

	loadFile(fileName);
}

void MainWindow::on_fileUploadAction_triggered()
{

	if (QString(qVersion()) < "4.3.3")
	{
		if (QMessageBox::question(this,
			"Old Qt version detected",
			QString("Your setup uses Qt %1, which contains various known errors in uploading "
			"data to Openstreetmap leading to 401 server response codes. Are you sure you want to continue (which is not "
			"recommended).\n"
			"For more information see http://wiki.openstreetmap.org/index.php/Problem_uploading_with_Merkaartor").arg(qVersion()),QMessageBox::Yes|QMessageBox::No) != QMessageBox::Yes)
			return;
	}

	MerkaartorPreferences* p = MerkaartorPreferences::instance();
	while (p->getOsmUser().isEmpty()) {
		int ret = QMessageBox::warning(this, tr("Upload OSM"), tr("You don't seem to have specified your\n"
			"Openstreetmap userid & password.\nDo you want to do this now?"), QMessageBox::Yes | QMessageBox::No);
		if (ret == QMessageBox::Yes) {
			on_toolsPreferencesAction_triggered(1);
		} else
			return;
	}
	on_editPropertiesAction_triggered();
	syncOSM(this, p->getOsmWebsite(), p->getOsmUser(), p->getOsmPassword(), p->getProxyUse(),
		p->getProxyHost(), p->getProxyPort());

}

void MainWindow::on_fileDownloadAction_triggered()
{
	if (downloadOSM(this, theView->projection().viewport(), theDocument)) {
		on_editPropertiesAction_triggered();
	} else
		QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
}

void MainWindow::on_helpAboutAction_triggered()
{
	QDialog dlg(this);
	Ui::AboutDialog About;
	About.setupUi(&dlg);
	About.Version->setText(About.Version->text().arg(MAJORVERSION).arg(MINORVERSION));
	dlg.exec();
}

void MainWindow::on_viewZoomAllAction_triggered()
{
	std::pair<bool, CoordBox> BBox(boundingBox(theDocument));
	if (BBox.first) {
		theView->projection().setViewport(BBox.second, theView->rect());
		theView->projection().zoom(0.99, theView->rect().center(), theView->rect());
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

void MainWindow::on_viewZoomWindowAction_triggered()
{
	theView->launch(new ZoomInteraction(theView));
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
	theView->launch(new CreateSingleWayInteraction(this, theView, false));
}

void MainWindow::on_createCurvedRoadAction_triggered()
{
	theView->launch(new CreateSingleWayInteraction(this, theView, true));
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
	CommandList* theList = new CommandList;
	joinRoads(theDocument, theList, theProperties);
	if (theList->empty())
		delete theList;
	else
	{
		theDocument->history().add(theList);
		invalidateView();
	}
}

void MainWindow::on_roadSplitAction_triggered()
{
	CommandList* theList = new CommandList;
	splitRoads(activeLayer(), theList, theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->history().add(theList);
}

void MainWindow::on_roadBreakAction_triggered()
{
	CommandList* theList = new CommandList;
	breakRoads(activeLayer(), theList, theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->history().add(theList);
}

void MainWindow::on_createRelationAction_triggered()
{
	Relation* R = new Relation;
	for (unsigned int i = 0; i < theProperties->size(); ++i)
		R->add("", theProperties->selection(i));
	theDocument->history().add(
		new AddFeatureCommand(theLayers->activeLayer(), R, true));
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

MapLayer* MainWindow::activeLayer()
{
	return theLayers->activeLayer();
}

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

void MainWindow::on_toolsPreferencesAction_triggered(unsigned int tabidx)
{
	PreferencesDialog* Pref = new PreferencesDialog();
	Pref->tabPref->setCurrentIndex(tabidx);

	if (Pref->exec() == QDialog::Accepted) {
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
		emit(preferencesChanged());
	}
}

void MainWindow::on_exportOSMAllAction_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osm", tr("OSM Files (*.osm)"));

	if (fileName != "") {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);
		out << theDocument->exportOSM();
		file.close();
	}
}

void MainWindow::on_exportOSMViewportAction_triggered()
{
	QString fileName = QFileDialog::getSaveFileName(this,
		tr("Export OSM"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osm", tr("OSM Files (*.osm)"));

	if (fileName != "") {
		QFile file(fileName);
		if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
			return;

		QTextStream out(&file);
		out << theDocument->exportOSM(view()->projection().viewport());
		file.close();
	}
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
	}

	MerkaartorPreferences::instance()->saveMainWindowState( this );

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
	QStringList Bookmarks = MerkaartorPreferences::instance()->getBookmarks();
	for (int i=0; i<Bookmarks.size(); i+=5) {
		QAction* a = new QAction(Bookmarks[i], menuBookmarks);
		menuBookmarks->addAction(a);
	}

	connect (menuBookmarks, SIGNAL(triggered(QAction *)), this, SLOT(bookmarkTriggered(QAction *)));
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

void MainWindow::on_nodeMergeAction_triggered()
{
}
