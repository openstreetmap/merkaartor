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
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/RoadManipulations.h"
#include "Map/TrackPoint.h"
#include "Sync/SyncOSM.h"
#include "GeneratedFiles/ui_AboutDialog.h"
#include "GeneratedFiles/ui_UploadMapDialog.h"
#include "GeneratedFiles/ui_SetPositionDialog.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QDialog>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QMouseEvent>

#define MAJORVERSION "0"
#define MINORVERSION "09"


MainWindow::MainWindow(void)
: theDocument(0)
{
	setupUi(this);
	theView = new MapView(this);
	setCentralWidget(theView);
	theDocument = new MapDocument;
	theView->setDocument(theDocument);
	theDocument->history().setActions(editUndoAction,editRedoAction);

	theLayers = new LayerDock(this);
	theLayers->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::LeftDockWidgetArea, theLayers);

	theProperties = new PropertiesDock(this);
	theProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	addDockWidget(Qt::RightDockWidgetArea, theProperties);
	on_editPropertiesAction_triggered();
	QTimer::singleShot(0,this,SLOT(initViewport()));
	QSettings Sets;
	if (Sets.contains("general/workingdir"))
		QDir::setCurrent(Sets.value("general/workingdir").toString());
}

void MainWindow::initViewport()
{
	theView->projection().setViewport(CoordBox(
		Coord(0.90608700309350998,0.077357771651368701),
		Coord(0.90611773551427466,0.077409749255956645)),theView->rect());
}

MainWindow::~MainWindow(void)
{
	QSettings Sets;
	Sets.setValue("general/workingdir",QDir::currentPath());
	delete theDocument;
}

void MainWindow::invalidateView(bool UpdateDock)
{
	theView->invalidate();
	theLayers->updateContent();
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
}


#define FILTER_LOAD_SUPPORTED \
	"Supported formats (*.gpx *.osm *.ngt)\n" \
	"GPS Exchange format (*.gpx)\n" \
	"OpenStreetMap format (*.osm)\n" \
	"Noni GPSPlot format (*.ngt)\n" \
	"All Files (*)"

void MainWindow::on_fileImportAction_triggered()
{
	QString s = QFileDialog::getOpenFileName(
		this,
		tr("Open track file"),
		"", tr(FILTER_LOAD_SUPPORTED));
	if (!s.isNull())
	{
		changeCurrentDirToFile(s);
		bool OK = false;
		MapLayer* NewLayer = new MapLayer(tr("Import %1").arg(s.right(s.length()-s.lastIndexOf('/')-1)));
		if (s.right(4).toLower() == ".gpx")
		{
			OK = importGPX(this, s, theDocument, NewLayer);
			if (OK)
				theDocument->add(NewLayer);
		}
		else if (s.right(4).toLower() == ".osm")
		{
			view()->setUpdatesEnabled(false);
			OK = importOSM(this, s, theDocument, NewLayer);
			view()->setUpdatesEnabled(true);
		}
		else if (s.right(4).toLower() == ".ngt")
		{
			view()->setUpdatesEnabled(false);
			OK = importNGT(this, s, theDocument, NewLayer);
			view()->setUpdatesEnabled(true);
		}
		if (OK)
		{
			on_viewZoomAllAction_triggered();
			on_editPropertiesAction_triggered();
			theDocument->history().setActions(editUndoAction,editRedoAction);
		}
		else
		{
			delete NewLayer;
			QMessageBox::warning(this,tr("Not a valid file"),tr("The file could not be opened"));
		}
	}
}

static bool mayDiscardUnsavedChanges(QWidget* aWidget)
{
	return QMessageBox::question(aWidget,MainWindow::tr("Unsaved changes"),
		MainWindow::tr("The current map contains unsaved changes that will be lost when starting a new one.\n"
			"Do you want to cancel starting a new map or continue and discard the old changes?"),
			QMessageBox::Discard|QMessageBox::Cancel,QMessageBox::Cancel) == QMessageBox::Discard;
}

void MainWindow::on_fileOpenAction_triggered()
{
	if (hasUnsavedChanges(*theDocument) && !mayDiscardUnsavedChanges(this))
		return;
	QString s = QFileDialog::getOpenFileName(
		this,
		tr("Open track file"),
		"", tr(FILTER_LOAD_SUPPORTED));
	if (!s.isNull())
	{
		changeCurrentDirToFile(s);
		MapDocument* NewDoc = new MapDocument;
		MapLayer* NewLayer = new MapLayer(tr("Open %1").arg(s.right(s.length()-s.lastIndexOf('/')-1)));
		bool OK = false;
		if (s.right(4).toLower() == ".gpx")
			OK = importGPX(this, s, NewDoc, NewLayer);
		else if (s.right(4).toLower() == ".osm")
			OK = importOSM(this, s, NewDoc, NewLayer);
		else if (s.right(4).toLower() == ".ngt")
			OK = importNGT(this, s, NewDoc, NewLayer);
		if (OK)
		{
			theProperties->setSelection(0);
			delete theDocument;
			theDocument = NewDoc;
			theDocument->add(NewLayer);
			theView->setDocument(theDocument);
			on_viewZoomAllAction_triggered();
			on_editPropertiesAction_triggered();
			theDocument->history().setActions(editUndoAction,editRedoAction);
		}
		else
		{
			delete NewDoc;
			delete NewLayer;
			QMessageBox::warning(this,tr("Not a valid file"),tr("The file could not be opened"));
		}
	}
}

void MainWindow::on_fileUploadAction_triggered()
{
	on_editPropertiesAction_triggered();
	QDialog * dlg = new QDialog(this);
	QSettings Sets;
	Sets.beginGroup("downloadosm");
	Ui::UploadMapDialog ui;
	ui.setupUi(dlg);
	ui.Website->setText("www.openstreetmap.org");
	ui.Username->setText(Sets.value("user").toString());
	ui.Password->setText(Sets.value("password").toString());
	ui.UseProxy->setChecked(Sets.value("useproxy").toBool());
	ui.ProxyHost->setText(Sets.value("proxyhost").toString());
	ui.ProxyPort->setText(Sets.value("proxyport").toString());
	if (dlg->exec() == QDialog::Accepted)
	{
		Sets.setValue("user",ui.Username->text());
		Sets.setValue("password",ui.Password->text());
		Sets.setValue("useproxy",ui.UseProxy->isChecked());
		Sets.setValue("proxyhost",ui.ProxyHost->text());
		Sets.setValue("proxyport",ui.ProxyPort->text());
		syncOSM(this,ui.Website->text(),ui.Username->text(),ui.Password->text(),ui.UseProxy->isChecked(), ui.ProxyHost->text(), ui.ProxyPort->text().toInt());
	}
	delete dlg;

}

void MainWindow::on_fileDownloadAction_triggered()
{
	if ( downloadOSM(this,theView->projection().viewport(),theDocument))
	{
		on_editPropertiesAction_triggered();
	}
	else
		QMessageBox::warning(this,tr("Error downloading"),tr("The map could not be downloaded"));
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
	if (BBox.first)
	{
		theView->projection().setViewport(BBox.second,theView->rect());
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
	if (Dlg->exec() == QDialog::Accepted)
	{
		theView->projection().setViewport(CoordBox(
			Coord(
				angToRad(Data.Latitude->text().toDouble()-Data.SpanLatitude->text().toDouble()/2),
				angToRad(Data.Longitude->text().toDouble()-Data.SpanLongitude->text().toDouble()/2)),
			Coord(
				angToRad(Data.Latitude->text().toDouble()+Data.SpanLatitude->text().toDouble()/2),
				angToRad(Data.Longitude->text().toDouble()+Data.SpanLongitude->text().toDouble()/2))), theView->rect());
		invalidateView();
	}
	delete Dlg;
}

void MainWindow::on_fileNewAction_triggered()
{
	theView->launch(0);
	theProperties->setSelection(0);
	if (!hasUnsavedChanges(*theDocument) || mayDiscardUnsavedChanges(this))
	{
		theDocument->clear();
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
	theView->launch(new CreateSingleWayInteraction(this, theView));
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
	joinRoads(theDocument,theList,theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->history().add(theList);
}

void MainWindow::on_roadSplitAction_triggered()
{
	CommandList* theList = new CommandList;
	splitRoads(activeLayer(),theList,theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->history().add(theList);
}

void MainWindow::on_roadBreakAction_triggered()
{
	CommandList* theList = new CommandList;
	breakRoads(activeLayer(),theList,theProperties);
	if (theList->empty())
		delete theList;
	else
		theDocument->history().add(theList);
}

void MainWindow::on_createRelationAction_triggered()
{
	Relation* R = new Relation;
	for (unsigned int i=0; i<theProperties->size(); ++i)
		R->add("",theProperties->selection(i));
	theDocument->history().add(
		new AddFeatureCommand(theLayers->activeLayer(),R,true));
	theProperties->setSelection(R);
}

MapLayer* MainWindow::activeLayer()
{
	return theLayers->activeLayer();
}

MapView* MainWindow::view()
{
	return theView;
}
