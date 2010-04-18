#include "MainWindow.h"

#include "LayerDock.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "DirtyDock.h"
#include "StyleDock.h"
#include "FeaturesDock.h"
#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "ImportExportOsmBin.h"
#include "ImportExportOSC.h"
#include "ExportGPX.h"
#include "ImportExportKML.h"
#include "CreateAreaInteraction.h"
#include "CreateDoubleWayInteraction.h"
#include "CreateNodeInteraction.h"
#include "CreateRoundaboutInteraction.h"
#include "CreatePolygonInteraction.h"
#include "CreateSingleWayInteraction.h"
#include "EditInteraction.h"
#include "MoveNodeInteraction.h"
#include "RotateInteraction.h"
#include "ZoomInteraction.h"
#include "Maps/Coord.h"
#include "DownloadOSM.h"
#include "ImportGPX.h"
#include "ImportNGT.h"
#include "ImportOSM.h"
#include "Document.h"
#include "Layer.h"
#include "ImageMapLayer.h"
#include "Features.h"
#include "Maps/FeatureManipulations.h"
#include "LayerIterator.h"
#include "PaintStyle/MasPaintStyle.h"
#include "PaintStyle/PaintStyleEditor.h"
#include "Sync/SyncOSM.h"

#include <ui_MainWindow.h>
#include <ui_AboutDialog.h>
#include <ui_UploadMapDialog.h>
#include <ui_SelectionDialog.h>
#include <ui_ExportDialog.h>

#include "Preferences/PreferencesDialog.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Preferences/ProjectionsList.h"
#include "Preferences/WMSPreferencesDialog.h"
#include "Preferences/TMSPreferencesDialog.h"
#include "Preferences/ProjPreferencesDialog.h"
#include "Utils/SelectionDialog.h"
#include "Utils/MDiscardableDialog.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
    #include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/mapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#include "Tools/WorldOsbManager.h"
#include "Tools/ActionsDialog.h"
#include "GotoDialog.h"

#ifndef RELEASE
#include "revision.h"
#endif
#include <boost/version.hpp>

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
            , projActgrp(0)
        {
            title = QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION));
        }
        int lastPrefTabIndex;
        QString defStyle;
        StyleDock* theStyle;
        FeaturesDock* theFeats;
        QString title;
        QActionGroup* projActgrp;
};

MainWindow::MainWindow(QWidget *parent)
        : QMainWindow(parent)
        , ui(new Ui::MainWindow)
        , fileName("")
        , theDocument(0)
        , gpsRecLayer(0)
        , curGpsTrackSegment(0)
        , qtTranslator(0)
        , merkaartorTranslator(0)
{
    p = new MainWindowPrivate;

    theProgressDialog = NULL;
    theProgressBar = NULL;
    theProgressLabel = NULL;

    p->defStyle = QApplication::style()->objectName();
    QString qVer = QString(qVersion()).replace(".", "");
    int iQVer = qVer.toInt();
    if (iQVer < 451) {
        QApplication::setStyle(QStyleFactory::create("skulpture"));
    } else {
        if (M_PREFS->getMerkaartorStyle())
            QApplication::setStyle(QStyleFactory::create(M_PREFS->getMerkaartorStyleString()));
    }

    ui->setupUi(this);
    M_STYLE->loadPainters(MerkaartorPreferences::instance()->getDefaultStyle());

    blockSignals(true);

    ViewportStatusLabel = new QLabel(this);
    pbImages = new QProgressBar(this);
    statusBar()->addPermanentWidget(ViewportStatusLabel);
    statusBar()->addPermanentWidget(pbImages);
#ifndef NDEBUG
    PaintTimeLabel = new QLabel(this);
    PaintTimeLabel->setMinimumWidth(23);
    statusBar()->addPermanentWidget(PaintTimeLabel);
#endif

    QList<QAction*> actions = findChildren<QAction*>();
    for (int i=0; i<actions.size(); i++) {
        shortcutsDefault[actions[i]->objectName()] = actions[i]->shortcut().toString();
    }
    updateLanguage();

    ViewportStatusLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    pbImages->setMaximumWidth(200);
    pbImages->setFormat(tr("tile %v / %m"));
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

    p->theFeats = new FeaturesDock(this);
    connect(p->theFeats, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
    connect(theView, SIGNAL(viewportChanged()), p->theFeats, SLOT(on_Viewport_changed()), Qt::QueuedConnection);

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
    connect (ui->menuBookmarks, SIGNAL(triggered(QAction *)), this, SLOT(bookmarkTriggered(QAction *)));

    updateRecentOpenMenu();
    connect (ui->menuRecentOpen, SIGNAL(triggered(QAction *)), this, SLOT(recentOpenTriggered(QAction *)));

    updateRecentImportMenu();
    connect (ui->menuRecentImport, SIGNAL(triggered(QAction *)), this, SLOT(recentImportTriggered(QAction *)));

    updateStyleMenu();
    connect (ui->menuStyles, SIGNAL(triggered(QAction *)), this, SLOT(styleTriggered(QAction *)));

    ui->viewDownloadedAction->setChecked(MerkaartorPreferences::instance()->getDownloadedVisible());
    ui->viewScaleAction->setChecked(M_PREFS->getScaleVisible());
    ui->viewPhotosAction->setChecked(M_PREFS->getPhotosVisible());
    ui->viewShowLatLonGridAction->setChecked(M_PREFS->getLatLonGridVisible());
    ui->viewStyleBackgroundAction->setChecked(M_PREFS->getBackgroundVisible());
    ui->viewStyleForegroundAction->setChecked(M_PREFS->getForegroundVisible());
    ui->viewStyleTouchupAction->setChecked(M_PREFS->getTouchupVisible());
    ui->viewNamesAction->setChecked(M_PREFS->getNamesVisible());
    ui->viewTrackPointsAction->setChecked(M_PREFS->getTrackPointsVisible());
    ui->viewTrackSegmentsAction->setChecked(M_PREFS->getTrackSegmentsVisible());
    ui->viewRelationsAction->setChecked(M_PREFS->getRelationsVisible());
    ui->viewVirtualNodesAction->setChecked(M_PREFS->getVirtualNodesVisible());
    ui->viewLockZoomAction->setChecked(M_PREFS->getZoomBoris());

    updateMenu();

    QActionGroup* actgrpArrows = new QActionGroup(this);
    actgrpArrows->addAction(ui->viewArrowsNeverAction);
    actgrpArrows->addAction(ui->viewArrowsOnewayAction);
    actgrpArrows->addAction(ui->viewArrowsAlwaysAction);
    switch (M_PREFS->getDirectionalArrowsVisible()) {
        case DirectionalArrows_Never:
            ui->viewArrowsNeverAction->setChecked(true);
            break;
        case DirectionalArrows_Oneway:
            ui->viewArrowsOnewayAction->setChecked(true);
            break;
        case DirectionalArrows_Always:
            ui->viewArrowsAlwaysAction->setChecked(true);
            break;
    }

    ui->gpsCenterAction->setChecked(M_PREFS->getGpsMapCenter());

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));

    setWindowTitle(QString("untitled - %1").arg(p->title));

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

    p->theFeats->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, p->theFeats);

    theGPS->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, theGPS);

    ui->mobileToolBar->setVisible(false);

#else
    theProperties->setVisible(false);
    theInfo->setVisible(false);
    theLayers->setVisible(false);
    theDirty->setVisible(false);
    theFeats->setVisible(false);
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
    ui->renderSVGAction->setVisible(false);
#endif

#ifndef GEOIMAGE
        ui->windowGeoimageAction->setVisible(false);
        ui->viewPhotosAction->setVisible(false);
#endif

    ui->viewStyleBackgroundAction->setVisible(false);
    ui->viewStyleForegroundAction->setVisible(false);
    ui->viewStyleTouchupAction->setVisible(false);

    MerkaartorPreferences::instance()->restoreMainWindowState( this );
#ifndef _MOBILE
    if (!M_PREFS->getProjectionsList()->getProjections()->size()) {
        QMessageBox::critical(this, tr("Cannot load Projections file"), tr("\"Projections.xml\" could not be opened anywhere. Aborting."));
        exit(1);
    }
#endif
    on_fileNewAction_triggered();

#define NUMOP 3
    static const char *opStr[NUMOP] = {
        QT_TR_NOOP("Low"), QT_TR_NOOP("High"), QT_TR_NOOP("Opaque")};

    int o = M_PREFS->getAreaOpacity();
    QActionGroup* actgrp = new QActionGroup(this);
    for (int i=0; i<NUMOP; i++) {
        QAction* act = new QAction(tr(opStr[i]), ui->mnuAreaOpacity);
        actgrp->addAction(act);
        qreal a = M_PREFS->getAlpha(opStr[i]);
        act->setData(a);
        act->setCheckable(true);
        ui->mnuAreaOpacity->addAction(act);
        if (o == int(a*100))
            act->setChecked(true);
    }
    connect(ui->mnuAreaOpacity, SIGNAL(triggered(QAction*)), this, SLOT(setAreaOpacity(QAction*)));

    blockSignals(false);

    M_PREFS->initialPosition(theView);

    QTimer::singleShot( 0, this, SLOT(delayedInit()) );
}

void MainWindow::delayedInit()
{
    updateProjectionMenu();
    updateWindowMenu();
}


MainWindow::~MainWindow(void)
{
    theProperties->setSelection(NULL);

    if (MasPaintStyle::instance())
        delete MasPaintStyle::instance();
    MerkaartorPreferences::instance()->setWorkingDir(QDir::currentPath());
    delete theDocument;
    delete theView;
    delete theProperties;

    delete qtTranslator;
    delete merkaartorTranslator;

    delete SlippyMapWidget::theSlippyCache;

    delete p;

    delete MerkaartorPreferences::instance();
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

void MainWindow::deleteProgressDialog()
{
    SAFE_DELETE(theProgressDialog)
    theProgressBar = NULL;
    theProgressLabel = NULL;
}

void MainWindow::setAreaOpacity(QAction *act)
{
    qreal a = act->data().toDouble();
    M_PREFS->setAreaOpacity(int(a*100));

    theView->invalidate(true, false);
}

void MainWindow::adjustLayers(bool adjustViewport)
{
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled())
            theView->projection().setProjectionType(l->projection());
    }
    CoordBox theVp;
    if (adjustViewport) {
        theVp = theView->viewport();
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

FeaturesDock* MainWindow::features()
{
    return p->theFeats;
}

Document* MainWindow::document()
{
    return theDocument;
}

Document* MainWindow::getDocumentFromClipboard()
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
        Document* NewDoc = new Document(NULL);
        DrawingLayer* l = new DrawingLayer("Dummy");
        NewDoc->add(l);

        c = c.firstChildElement();
        while(!c.isNull()) {
            if (c.tagName() == "bound") {
            } else
            if (c.tagName() == "way") {
                Way::fromXML(NewDoc, l, c);
            } else
            if (c.tagName() == "relation") {
                Relation::fromXML(NewDoc, l, c);
            } else
            if (c.tagName() == "node") {
                Node::fromXML(NewDoc, l, c);
            }

            c = c.nextSiblingElement();
        }

        delete theXmlDoc;
        return NewDoc;
    } else
    if (c.tagName() == "kml") {
        Document* NewDoc = new Document(NULL);
        DrawingLayer* l = new DrawingLayer("Dummy");
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
    Document* doc;
    if (!(doc = getDocumentFromClipboard()))
        return;

    CommandList* theList = new CommandList();
    theList->setDescription("Paste Features");

    QList<Feature*> theFeats;
    for (FeatureIterator k(doc); !k.isEnd(); ++k) {
        theFeats.push_back(k.get());
    }
    for (int i=0; i<theFeats.size(); ++i) {
        Feature*F = theFeats.at(i);
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
    QList<Feature*> sel = properties()->selection();
    if (!sel.size())
        return;

    Document* doc;
    if (!(doc = getDocumentFromClipboard()))
        return;

    CommandList* theList = new CommandList();
    theList->setDescription("Paste tags (overwrite)");

    for(int i=0; i < sel.size(); ++i) {
        theList->add(new ClearTagsCommand(sel[i], theDocument->getDirtyOrOriginLayer(sel[i]->layer())));
        for (FeatureIterator k(doc); !k.isEnd(); ++k) {
            if (k.get()->getClass() == sel[i]->getClass())
                Feature::mergeTags(theDocument, theList, sel[i], k.get());
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
    QList<Feature*> sel = properties()->selection();
    if (!sel.size())
        return;

    Document* doc;
    if (!(doc = getDocumentFromClipboard()))
        return;

    CommandList* theList = new CommandList();
    theList->setDescription("Paste tags (merge)");

    for(int i=0; i < sel.size(); ++i) {
        for (FeatureIterator k(doc); !k.isEnd(); ++k) {
            if (k.get()->getClass() == sel[i]->getClass())
                Feature::mergeTags(theDocument, theList, sel[i], k.get());
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
    ui->editPasteFeaturesAction->setEnabled(false);
    ui->editPasteMergeAction->setEnabled(false);
    ui->editPasteOverwriteAction->setEnabled(false);

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

    ui->editPasteFeaturesAction->setEnabled(true);
    ui->editPasteMergeAction->setEnabled(true);
    ui->editPasteOverwriteAction->setEnabled(true);

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
        view()->launch(new MoveNodeInteraction(view()));
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
    tr("Supported formats")+" (*.mdc *.gpx *.osm *.osb *.osc *.ngt *.nmea *.nma *.kml *.shp *.csv)\n" \
    +tr("Merkaartor document (*.mdc)\n") \
    +tr("GPS Exchange format (*.gpx)\n") \
    +tr("OpenStreetMap format (*.osm)\n") \
    +tr("OpenStreetMap binary format (*.osb)\n") \
    +tr("OpenStreetMap change format (*.osc)\n") \
    +tr("Noni GPSPlot format (*.ngt)\n") \
    +tr("NMEA GPS log format (*.nmea *.nma)\n") \
    +tr("KML file (*.kml)\n") \
    +tr("ESRI Shapefile (*.shp)\n") \
    +tr("Comma delimited format (*.csv)\n") \
    +tr("All Files (*)")
#else
#define FILTER_OPEN_SUPPORTED \
    tr("Supported formats")+" (*.mdc *.gpx *.osm *.osb *.osc *.ngt *.nmea *.nma *.kml *.shp *.csv *.jpg)\n" \
    +tr("Merkaartor document (*.mdc)\n") \
    +tr("GPS Exchange format (*.gpx)\n") \
    +tr("OpenStreetMap format (*.osm)\n") \
    +tr("OpenStreetMap binary format (*.osb)\n") \
    +tr("OpenStreetMap change format (*.osc)\n") \
    +tr("Noni GPSPlot format (*.ngt)\n") \
    +tr("NMEA GPS log format (*.nmea *.nma)\n") \
    +tr("KML file (*.kml)\n") \
    +tr("ESRI Shapefile (*.shp)\n") \
    +tr("Comma delimited format (*.csv)\n") \
    +tr("Geotagged images (*.jpg)\n") \
    +tr("All Files (*)")
#endif
#define FILTER_IMPORT_SUPPORTED \
    tr("Supported formats")+" (*.gpx *.osm *.osb *.osc *.ngt *.nmea *.nma *.kml *.shp *.csv)\n" \
    +tr("GPS Exchange format (*.gpx)\n") \
    +tr("OpenStreetMap format (*.osm)\n") \
    +tr("OpenStreetMap binary format (*.osb)\n") \
    +tr("OpenStreetMap change format (*.osc)\n") \
    +tr("Noni GPSPlot format (*.ngt)\n") \
    +tr("NMEA GPS log format (*.nmea *.nma)\n") \
    +tr("KML file (*.kml)\n") \
    +tr("ESRI Shapefile (*.shp)\n") \
    +tr("Comma delimited format (*.csv)\n") \
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
    theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
}

static bool mayDiscardUnsavedChanges(QWidget* aWidget)
{
    return QMessageBox::question(aWidget, MainWindow::tr("Unsaved changes"),
                                 MainWindow::tr("The current map contains unsaved changes that will be lost when starting a new one.\n"
                                                "Do you want to cancel starting a new map or continue and discard the old changes?"),
                                 QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Cancel) == QMessageBox::Discard;
}

bool MainWindow::importFiles(Document * mapDocument, const QStringList & fileNames, QStringList * importedFileNames )
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
        Layer* newLayer = NULL;

        bool importOK = false;
        bool importAborted = false;

        if (fn.toLower().endsWith(".gpx")) {
            QList<TrackLayer*> theTracklayers;
            TrackLayer* newLayer = new TrackLayer( baseFileName + " - " + tr("Waypoints"), baseFileName);
            mapDocument->add(newLayer);
            theTracklayers.append(newLayer);
            newLayer->blockIndexing(true);
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
                } else {
                    newLayer->blockIndexing(false);
                    newLayer->reIndex();
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
        else if (fn.toLower().endsWith(".osm")) {
            newLayer = new DrawingLayer( baseFileName );
            newLayer->blockIndexing(true);
            mapDocument->add(newLayer);
            importOK = importOSM(this, baseFileName, mapDocument, newLayer);
        }
        else if (fn.toLower().endsWith(".osc")) {
            newLayer = mapDocument->getDirtyOrOriginLayer();
            newLayer->blockIndexing(true);
            importOK = mapDocument->importOSC(fn, (DirtyLayer *)newLayer);
        }
        else if (fn.toLower().endsWith(".osb")) {
            newLayer = new OsbLayer( baseFileName, fn );
            newLayer->blockIndexing(true);
            mapDocument->add(newLayer);
            importOK = mapDocument->importOSB(fn, (DrawingLayer *)newLayer);
        }
        else if (fn.toLower().endsWith(".ngt")) {
            newLayer = new TrackLayer( baseFileName );
            newLayer->blockIndexing(true);
            mapDocument->add(newLayer);
            importOK = importNGT(this, baseFileName, mapDocument, newLayer);
            if (importOK && MerkaartorPreferences::instance()->getAutoExtractTracks()) {
                ((TrackLayer *)newLayer)->extractLayer();
            }
        }
        else if (fn.toLower().endsWith(".nmea") || (fn.toLower().endsWith(".nma"))) {
            newLayer = new TrackLayer( baseFileName );
            newLayer->blockIndexing(true);
            mapDocument->add(newLayer);
            importOK = mapDocument->importNMEA(baseFileName, (TrackLayer *)newLayer);
            if (importOK && MerkaartorPreferences::instance()->getAutoExtractTracks()) {
                ((TrackLayer *)newLayer)->extractLayer();
            }
        }
        else if (fn.toLower().endsWith(".kml")) {
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
                newLayer = new DrawingLayer( baseFileName );
                newLayer->blockIndexing(true);
                newLayer->setUploadable(false);
                mapDocument->add(newLayer);
                importOK = mapDocument->importKML(baseFileName, (TrackLayer *)newLayer);
            } else
                importAborted = true;
        }
        else if (fn.toLower().endsWith(".csv")) {
#ifndef Q_OS_SYMBIAN
            QApplication::restoreOverrideCursor();
#endif
            newLayer = new DrawingLayer( baseFileName );
            newLayer->setUploadable(false);
            mapDocument->add(newLayer);
            importOK = mapDocument->importCSV(baseFileName, (DrawingLayer*)newLayer);
        }
#ifdef USE_GDAL
        else if (fn.toLower().endsWith(".shp")) {
            newLayer = new DrawingLayer( baseFileName );
            newLayer->setUploadable(false);
            mapDocument->add(newLayer);
            importOK = mapDocument->importSHP(baseFileName, (DrawingLayer*)newLayer);
        }
#endif

        if (!importOK && newLayer)
            mapDocument->remove(newLayer);

        if (importOK)
        {
            foundImport = true;

            if (importedFileNames)
                importedFileNames->append(fn);

            if (newLayer) {
                newLayer->blockIndexing(false);
                newLayer->reIndex();
            }
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
    deleteProgressDialog();

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
    theLayers->setUpdatesEnabled(false);
    view()->setUpdatesEnabled(false);

        // Load only the first merkaartor document
    bool foundDocument = false;
    QMutableStringListIterator it(fileNames);
    while (it.hasNext())
    {
        const QString & fn = it.next();

        if (fn.toLower().endsWith(".mdc") == false)
            continue;

        if (foundDocument == false)
        {
            changeCurrentDirToFile(fn);
            loadDocument(fn);
            foundDocument = true;
        }

        it.remove();
    }

    Document* newDoc = theDocument;
    if (foundDocument == false) {
        newDoc = new Document(theLayers);
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
    theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);

    theLayers->setUpdatesEnabled(true);
    view()->setUpdatesEnabled(true);
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

    deleteProgressDialog();

    updateBookmarksMenu();
}

void MainWindow::on_fileDownloadMoreAction_triggered()
{
    createProgressDialog();

    if (!downloadMoreOSM(this, theView->viewport(), theDocument)) {
        QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
    }

    deleteProgressDialog();
}

void MainWindow::downloadFeatures(const QList<Feature*>& aDownloadList)
{
    createProgressDialog();

    if (!::downloadFeatures(this, aDownloadList, theDocument)) {
        QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
    }

    deleteProgressDialog();
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
    dlg.setWindowFlags(dlg.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dlg.setWindowFlags(dlg.windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
    About.Version->setText(About.Version->text().arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)));
    About.QTVersion->setText(About.QTVersion->text().arg(qVersion()).arg(QT_VERSION_STR));
    int boostMajVer = BOOST_VERSION / 100000;
    int boostMinVer = BOOST_VERSION / 100 % 1000;
    About.BoostVersion->setText(About.BoostVersion->text().arg(QString::number(boostMajVer)+"."+QString::number(boostMinVer)));

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
    theView->zoom(M_PREFS->getZoomIn()/100., theView->rect().center());
    invalidateView();
}

void MainWindow::on_viewZoomOutAction_triggered()
{
    theView->zoom(M_PREFS->getZoomOut()/100., theView->rect().center());
    invalidateView();
}

void MainWindow::on_viewZoomWindowAction_triggered()
{
    theView->launch(new ZoomInteraction(theView));
}

void MainWindow::on_viewLockZoomAction_triggered()
{
    M_PREFS->setZoomBoris(!M_PREFS->getZoomBoris());
    ui->viewLockZoomAction->setChecked(M_PREFS->getZoomBoris());
    ImageMapLayer* l = NULL;
    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
        l = ImgIt.get();
        break;
    }
    if (l && l->isTiled()) {
        theView->projection().setProjectionType(l->projection());
        M_PREFS->setProjectionType(l->projection());
        theView->setViewport(theView->viewport(), theView->rect());
    }
    theView->adjustZoomToBoris();
    updateProjectionMenu();
    invalidateView();
}

void MainWindow::on_viewDownloadedAction_triggered()
{
    M_PREFS->setDownloadedVisible(!M_PREFS->getDownloadedVisible());
    ui->viewDownloadedAction->setChecked(M_PREFS->getDownloadedVisible());
    invalidateView();
}

void MainWindow::on_viewScaleAction_triggered()
{
    M_PREFS->setScaleVisible(!M_PREFS->getScaleVisible());
    ui->viewScaleAction->setChecked(M_PREFS->getScaleVisible());
    invalidateView();
}

void MainWindow::on_viewPhotosAction_triggered()
{
    M_PREFS->setPhotosVisible(!M_PREFS->getPhotosVisible());
    ui->viewPhotosAction->setChecked(M_PREFS->getPhotosVisible());
    invalidateView();
}

void MainWindow::on_viewShowLatLonGridAction_triggered()
{
    M_PREFS->setLatLonGridVisible(!M_PREFS->getLatLonGridVisible());
    ui->viewShowLatLonGridAction->setChecked(M_PREFS->getLatLonGridVisible());
    invalidateView();
}

void MainWindow::on_viewStyleBackgroundAction_triggered()
{
    M_PREFS->setBackgroundVisible(!M_PREFS->getBackgroundVisible());
    ui->viewStyleBackgroundAction->setChecked(M_PREFS->getBackgroundVisible());
    invalidateView();
}

void MainWindow::on_viewStyleForegroundAction_triggered()
{
    M_PREFS->setForegroundVisible(!M_PREFS->getForegroundVisible());
    ui->viewStyleForegroundAction->setChecked(M_PREFS->getForegroundVisible());
    invalidateView();
}

void MainWindow::on_viewStyleTouchupAction_triggered()
{
    M_PREFS->setTouchupVisible(!M_PREFS->getTouchupVisible());
    ui->viewStyleTouchupAction->setChecked(M_PREFS->getTouchupVisible());
    invalidateView();
}

void MainWindow::on_viewNamesAction_triggered()
{
    M_PREFS->setNamesVisible(!M_PREFS->getNamesVisible());
    ui->viewNamesAction->setChecked(M_PREFS->getNamesVisible());
    invalidateView();
}

void MainWindow::on_viewVirtualNodesAction_triggered()
{
    M_PREFS->setVirtualNodesVisible(!M_PREFS->getVirtualNodesVisible());
    ui->viewVirtualNodesAction->setChecked(M_PREFS->getVirtualNodesVisible());
    invalidateView();
}

void MainWindow::on_viewTrackPointsAction_triggered()
{
    M_PREFS->setTrackPointsVisible(!M_PREFS->getTrackPointsVisible());
    ui->viewTrackPointsAction->setChecked(M_PREFS->getTrackPointsVisible());
    invalidateView();
}

void MainWindow::on_viewTrackSegmentsAction_triggered()
{
    M_PREFS->setTrackSegmentsVisible(!M_PREFS->getTrackSegmentsVisible());
    ui->viewTrackSegmentsAction->setChecked(M_PREFS->getTrackSegmentsVisible());
    invalidateView();
}

void MainWindow::on_viewRelationsAction_triggered()
{
    M_PREFS->setRelationsVisible(!M_PREFS->getRelationsVisible());
    ui->viewRelationsAction->setChecked(M_PREFS->getRelationsVisible());
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
        M_PREFS->cleanupBackgroundPlugins();
        delete theDocument;
        theDocument = new Document(theLayers);
        theDocument->addDefaultLayers();
        if (M_PREFS->getWorldOsbAutoload() && !M_PREFS->getWorldOsbUri().isEmpty()) {
            Layer* newLayer = new OsbLayer( "World", M_PREFS->getWorldOsbUri() + "/world.osb" );
            if (M_PREFS->getWorldOsbAutoshow())
                newLayer->setVisible(true);
            else
                newLayer->setVisible(false);
            newLayer->setReadonly(true);

            theDocument->add(newLayer);
        }

        theView->setDocument(theDocument);
        theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
        connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
        theDirty->updateList();

        fileName = "";
        setWindowTitle(QString("untitled - %1").arg(p->title));

        on_editPropertiesAction_triggered();
        adjustLayers(true);
    }
#ifdef GEOIMAGE
    if (theGeoImage)
        theGeoImage->clear();
#endif
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
    Node * firstPoint = NULL;

    if (theProperties->size() == 1)
    {
        Feature * feature = theProperties->selection(0);
        firstPoint = dynamic_cast<Node*>(feature);
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

void MainWindow::on_featureDeleteAction_triggered()
{
    Feature* F = theProperties->selection(0);
    if (!F)
        return;

    while (F->sizeParents()) {
        F->getParent(0)->remove(F);
    }
    F->layer()->deleteFeature(F);
    theProperties->setSelection(0);
    invalidateView();
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
    int n = createJunction(theDocument, theList, theProperties, false);
    if (n > 1) {
        MDiscardableMessage dlg(view(),
            MainWindow::tr("Multiple intersection."),
            MainWindow::tr("Those roads have multiple intersections.\nDo you still want to create a junction for each one (Unwanted junctions can still be deleted afterhand)?"));
        if (dlg.check() != QDialog::Accepted)
            return;
    }
    createJunction(theDocument, theList, theProperties, true);

    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_roadAddStreetNumbersAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Add Street Numbers"), NULL);

    addStreetNumbers(theDocument, theList, theProperties);

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
    Feature* F = theProperties->selection(0);
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
    Feature* F = theProperties->selection(0);
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
    QString f = QFileDialog::getSaveFileName(this, tr("Save map style"), M_PREFS->getCustomStyle(), tr("Merkaartor map style (*.mas)"));
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

void MainWindow::on_toolsProjectionsAction_triggered()
{
    ProjPreferencesDialog* prefDlg;
    prefDlg = new ProjPreferencesDialog();
    if (prefDlg->exec() == QDialog::Accepted) {
        updateProjectionMenu();
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
    theActions.append(ui->fileNewAction);
    theActions.append(ui->fileOpenAction);
    theActions.append(ui->fileImportAction);
    theActions.append(ui->fileSaveAction);
    theActions.append(ui->fileSaveAsAction);
    theActions.append(ui->fileDownloadAction);
    theActions.append(ui->fileDownloadMoreAction);
    theActions.append(ui->fileUploadAction);
    theActions.append(ui->fileQuitAction);
    theActions.append(ui->editUndoAction);
    theActions.append(ui->editRedoAction);
    theActions.append(ui->editCopyAction);
    theActions.append(ui->editPasteFeatureAction);
    theActions.append(ui->editPasteMergeAction);
    theActions.append(ui->editPasteOverwriteAction);
    theActions.append(ui->editRemoveAction);
    theActions.append(ui->editPropertiesAction);
    theActions.append(ui->editMoveAction);
    theActions.append(ui->editRotateAction);
    theActions.append(ui->editSelectAction);
    theActions.append(ui->viewZoomAllAction);
    theActions.append(ui->viewZoomWindowAction);
    theActions.append(ui->viewZoomOutAction);
    theActions.append(ui->viewZoomInAction);
    theActions.append(ui->viewDownloadedAction);
    theActions.append(ui->viewRelationsAction);
    theActions.append(ui->viewScaleAction);
    theActions.append(ui->viewNamesAction);
    theActions.append(ui->viewTrackPointsAction);
    theActions.append(ui->viewTrackSegmentsAction);
    theActions.append(ui->viewGotoAction);
    theActions.append(ui->gpsConnectAction);
    theActions.append(ui->gpsReplayAction);
    theActions.append(ui->gpsRecordAction);
    theActions.append(ui->gpsPauseAction);
    theActions.append(ui->gpsDisconnectAction);
    theActions.append(ui->gpsCenterAction);
    theActions.append(ui->createNodeAction);
    theActions.append(ui->createRoadAction);
    theActions.append(ui->createDoubleWayAction);
    theActions.append(ui->createRoundaboutAction);
    theActions.append(ui->createAreaAction);
    theActions.append(ui->createRelationAction);
    theActions.append(ui->createRectangleAction);
    theActions.append(ui->createPolygonAction);
    theActions.append(ui->featureDeleteAction);
    theActions.append(ui->featureCommitAction);
    theActions.append(ui->roadSplitAction);
    theActions.append(ui->roadBreakAction);
    theActions.append(ui->roadJoinAction);
    theActions.append(ui->editReverseAction);
    theActions.append(ui->nodeMergeAction);
    theActions.append(ui->nodeAlignAction);
    theActions.append(ui->toolsPreferencesAction);
    theActions.append(ui->toolsShortcutsAction);
    theActions.append(ui->toolsWorldOsbAction);

    theActions.append(ui->windowHideAllAction);
    theActions.append(ui->windowPropertiesAction);
    theActions.append(ui->windowLayersAction);
    theActions.append(ui->windowInfoAction);
    theActions.append(ui->windowDirtyAction);
    theActions.append(ui->windowGPSAction);
    theActions.append(ui->windowGeoimageAction);
    theActions.append(ui->windowStylesAction);

    theActions.append(ui->helpAboutAction);

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
    QString qVer = QString(qVersion()).replace(".", "");
    int iQVer = qVer.toInt();
    if (iQVer < 451) {
        QApplication::setStyle(QStyleFactory::create("skulpture"));
    } else {
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
    }
    ui->mnuProjections->menuAction()->setEnabled(true);
    view()->projection().setProjectionType(M_PREFS->getProjectionType());
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled()) {
            ui->mnuProjections->menuAction()->setEnabled(false);
            view()->projection().setProjectionType(l->projection());
            view()->zoom(0.99, view()->rect().center());
        }
    }

    updateStyleMenu();
    updateMenu();
    theView->launch(new EditInteraction(theView));
    invalidateView(false);
}

void MainWindow::on_fileSaveAsAction_triggered()
{
    fileName = QFileDialog::getSaveFileName(this,
        tr("Save Merkaartor document"), QString("%1/%2.mdc").arg(MerkaartorPreferences::instance()->getWorkingDir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)"));

    if (fileName != "") {
        saveDocument();
        M_PREFS->addRecentOpen(fileName);
        updateRecentOpenMenu();
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
#ifndef Q_OS_SYMBIAN
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

    QDomElement root;
    QDomDocument* theXmlDoc;

    theXmlDoc = new QDomDocument();
    theXmlDoc->appendChild(theXmlDoc->createProcessingInstruction("xml", "version=\"1.0\""));
    root = theXmlDoc->createElement("MerkaartorDocument");
    root.setAttribute("version", "1.1");
    root.setAttribute("creator", QString("%1").arg(p->title));

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

    setWindowTitle(QString("%1 - %2").arg(fileName).arg(p->title));

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
            Document* newDoc = Document::fromXML(e, version, theLayers, progress);

            if (progress.wasCanceled())
                break;

            if (newDoc) {
                theProperties->setSelection(0);
                delete theDocument;
                theDocument = newDoc;
                theView->setDocument(theDocument);
                on_editPropertiesAction_triggered();
                theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
                connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
                theDirty->updateList();
                fileName = fn;
                setWindowTitle(QString("%1 - %2").arg(fileName).arg(p->title));
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

#ifdef GEOIMAGE
    if (theGeoImage)
        theGeoImage->clear();
#endif

    M_PREFS->addRecentOpen(fn);
    updateRecentOpenMenu();
}

void MainWindow::on_exportOSMAction_triggered()
{
    QList<Feature*> theFeatures;
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
    QList<Feature*> theFeatures;
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

void MainWindow::on_exportOSCAction_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
        tr("Export osmChange"), MerkaartorPreferences::instance()->getWorkingDir() + "/untitled.osc", tr("osmChange Files (*.osc)"));

    if (fileName != "") {
#ifndef Q_OS_SYMBIAN
        QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

        ImportExportOSC osc(document());
        if (osc.saveFile(fileName)) {
            osc.export_();
        }

#ifndef Q_OS_SYMBIAN
        QApplication::restoreOverrideCursor();
#endif
    }
}


void MainWindow::on_exportGPXAction_triggered()
{
    QList<Feature*> theFeatures;
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
    QList<Feature*> theFeatures;
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

bool MainWindow::selectExportedFeatures(QList<Feature*>& theFeatures)
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
                if (Node* P = dynamic_cast<Node*>(i.get())) {
                    if (aCoordBox.contains(P->position())) {
                        theFeatures.append(P);
                    }
                } else
                    if (Way* G = dynamic_cast<Way*>(i.get())) {
                        if (aCoordBox.intersects(G->boundingBox())) {
                            for (int j=0; j < G->size(); j++) {
                                if (Node* P = dynamic_cast<Node*>(G->get(j)))
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
                                    if (Way* R = dynamic_cast<Way*>(G->get(j))) {
                                        if (!aCoordBox.contains(R->boundingBox())) {
                                            for (int k=0; k < R->size(); k++) {
                                                if (Node* P = dynamic_cast<Node*>(R->get(k)))
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

        QList <Feature *> selection;
        int added = 0;
        for (VisibleFeatureIterator i(theDocument); !i.isEnd() && added < selMaxResult; ++i) {
            Feature* F = i.get();
            int ok = false;

            if (selName.indexIn(F->description()) == -1) {
                continue;
            }
            if (F->id().indexOf(Sel->edID->text(), theSensitivity) == -1) {
                continue;
            }
            if (Sel->cbKey->currentText().isEmpty() && Sel->cbValue->currentText().isEmpty())
                ok = true;
            else
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
    for(int i=ui->menuBookmarks->actions().count()-1; i > 2 ; i--) {
        ui->menuBookmarks->removeAction(ui->menuBookmarks->actions()[3]);
    }

    BookmarkListIterator it(*(M_PREFS->getBookmarks()));
    while (it.hasNext()) {
        it.next();
        if (it.value().deleted == false) {
            QAction* a = new QAction(it.key(), ui->menuBookmarks);
            ui->menuBookmarks->addAction(a);
        }
    }
}

void MainWindow::updateRecentOpenMenu()
{
    for(int i=ui->menuRecentOpen->actions().count()-1; i >= 0; i--) {
        ui->menuRecentOpen->removeAction(ui->menuRecentOpen->actions()[0]);
    }

    if (!M_PREFS->getRecentOpen().size()) {
        ui->menuRecentOpen->setEnabled(false);
        return;
    }

    ui->menuRecentOpen->setEnabled(true);
    QStringList RecentOpen = M_PREFS->getRecentOpen();
    for (int i=0; i<RecentOpen.size(); i++) {
        QAction* a = new QAction(RecentOpen[i], ui->menuRecentOpen);
        ui->menuRecentOpen->addAction(a);
    }
}

void MainWindow::updateRecentImportMenu()
{
    for(int i=ui->menuRecentImport->actions().count()-1; i >= 0; i--) {
        ui->menuRecentImport->removeAction(ui->menuRecentImport->actions()[0]);
    }

    if (!M_PREFS->getRecentImport().size()) {
        ui->menuRecentImport->setEnabled(false);
        return;
    }

    ui->menuRecentImport->setEnabled(true);
    QStringList RecentImport = M_PREFS->getRecentImport();
    for (int i=0; i<RecentImport.size(); i++) {
        QAction* a = new QAction(RecentImport[i], ui->menuRecentImport);
        ui->menuRecentImport->addAction(a);
    }
}

void MainWindow::updateProjectionMenu()
{
#ifndef _MOBILE
    SAFE_DELETE(p->projActgrp)
    p->projActgrp = new QActionGroup(this);
    foreach (ProjectionItem it, *M_PREFS->getProjectionsList()->getProjections()) {
        if (it.deleted)
            continue;
        QAction* a = new QAction(it.name, p->projActgrp);
        a->setCheckable (true);
        if (it.name.contains(M_PREFS->getProjectionType(), Qt::CaseInsensitive))
            a->setChecked(true);
        ui->mnuProjections->addAction(a);
    }
    connect (ui->mnuProjections, SIGNAL(triggered(QAction *)), this, SLOT(projectionTriggered(QAction *)));
#endif
    ui->mnuProjections->menuAction()->setVisible(true);
    if (M_PREFS->getZoomBoris()) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled())
            ui->mnuProjections->menuAction()->setVisible(false);
    }
}

void MainWindow::updateStyleMenu()
{
    for(int i=ui->menuStyles->actions().count()-1; i > 4 ; i--) {
        ui->menuStyles->removeAction(ui->menuStyles->actions()[5]);
    }
    p->theStyle->clearItems();

    QActionGroup* actgrp = new QActionGroup(this);
    QDir intStyles(BUILTIN_STYLES_DIR);
    for (int i=0; i < intStyles.entryList().size(); ++i) {
        QAction* a = new QAction(QString(tr("%1 (int)")).arg(intStyles.entryList().at(i)), ui->menuStyles);
        actgrp->addAction(a);
        a->setCheckable(true);
        a->setData(QVariant(intStyles.entryInfoList().at(i).absoluteFilePath()));
        ui->menuStyles->addAction(a);
        if (intStyles.entryInfoList().at(i).absoluteFilePath() == M_PREFS->getDefaultStyle())
            a->setChecked(true);
        p->theStyle->addItem(a);
    }
    if (!M_PREFS->getCustomStyle().isEmpty()) {
        QDir customStyles(M_PREFS->getCustomStyle(), "*.mas");
        for (int i=0; i < customStyles.entryList().size(); ++i) {
            QAction* a = new QAction(customStyles.entryList().at(i), ui->menuStyles);
            actgrp->addAction(a);
            a->setCheckable(true);
            a->setData(QVariant(customStyles.entryInfoList().at(i).absoluteFilePath()));
            ui->menuStyles->addAction(a);
            if (customStyles.entryInfoList().at(i).absoluteFilePath() == M_PREFS->getDefaultStyle())
                a->setChecked(true);
            p->theStyle->addItem(a);
       }
    }
}

void MainWindow::updateWindowMenu(bool)
{
    ui->windowPropertiesAction->setChecked(theProperties->isVisible());
    ui->windowLayersAction->setChecked(theLayers->isVisible());
    ui->windowInfoAction->setChecked(theInfo->isVisible());
    ui->windowDirtyAction->setChecked(theDirty->isVisible());
    ui->windowFeatsAction->setChecked(p->theFeats->isVisible());
    ui->windowGPSAction->setChecked(theGPS->isVisible());
#ifdef GEOIMAGE
    ui->windowGeoimageAction->setChecked(theGeoImage->isVisible());
#endif
    ui->windowStylesAction->setChecked(p->theStyle->isVisible());
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
                    for(int i=2; i < ui->menuBookmarks->actions().count(); i++) {
                        if (ui->menuBookmarks->actions()[i]->text() == newBk) {
                            ui->menuBookmarks->removeAction(ui->menuBookmarks->actions()[i]);
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

        QAction* a = new QAction(text,ui-> menuBookmarks);
        ui->menuBookmarks->addAction(a);
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

        for(int i=2; i < ui->menuBookmarks->actions().count(); i++) {
            if (ui->menuBookmarks->actions()[i]->text() == item) {
                ui->menuBookmarks->removeAction(ui->menuBookmarks->actions()[i]);
                break;
            }
        }
    }
}

void MainWindow::bookmarkTriggered(QAction* anAction)
{
    if (anAction == ui->bookmarkAddAction || anAction == ui->bookmarkRemoveAction)
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
    theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
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
    ui->windowPropertiesAction->setChecked(theProperties->isVisible());
}

void MainWindow::on_windowLayersAction_triggered()
{
    theLayers->setVisible(!theLayers->isVisible());
    ui->windowLayersAction->setChecked(theLayers->isVisible());
}

void MainWindow::on_windowInfoAction_triggered()
{
    theInfo->setVisible(!theInfo->isVisible());
    ui->windowInfoAction->setChecked(theInfo->isVisible());
}

void MainWindow::on_windowDirtyAction_triggered()
{
    theDirty->setVisible(!theDirty->isVisible());
    ui->windowDirtyAction->setChecked(theDirty->isVisible());
}

void MainWindow::on_windowFeatsAction_triggered()
{
    p->theFeats->setVisible(!p->theFeats->isVisible());
    ui->windowFeatsAction->setChecked(p->theFeats->isVisible());
}

void MainWindow::on_windowToolbarAction_triggered()
{
    ui->toolBar->setVisible(!ui->toolBar->isVisible());
}

void MainWindow::on_windowGPSAction_triggered()
{
    theGPS->setVisible(!theGPS->isVisible());
    ui->windowGPSAction->setChecked(theGPS->isVisible());
}

#ifdef GEOIMAGE
void MainWindow::on_windowGeoimageAction_triggered()
{
    theGeoImage->setVisible(!theGeoImage->isVisible());
    ui->windowGeoimageAction->setChecked(theGeoImage->isVisible());
}
#endif

void MainWindow::on_windowStylesAction_triggered()
{
    p->theStyle->setVisible(!p->theStyle->isVisible());
    ui->windowStylesAction->setChecked(p->theStyle->isVisible());
}

void MainWindow::on_windowHideAllAction_triggered()
{
    fullscreenState = saveState(1);

    ui->windowHideAllAction->setEnabled(false);
    ui->windowHideAllAction->setVisible(false);
    ui->windowShowAllAction->setEnabled(true);
    ui->windowShowAllAction->setVisible(true);

//	ui->toolBar->setVisible(false);
    theInfo->setVisible(false);
    theDirty->setVisible(false);
    p->theFeats->setVisible(false);
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

    ui->windowHideAllAction->setEnabled(true);
    ui->windowHideAllAction->setVisible(true);
    ui->windowShowAllAction->setEnabled(false);
    ui->windowShowAllAction->setVisible(false);
}

void MainWindow::on_gpsConnectAction_triggered()
{
#ifndef Q_OS_SYMBIAN
    QGPSDevice* aGps;
    if (M_PREFS->getGpsUseGpsd())
        aGps = new QGPSDDevice("gpsd");
    else
        aGps = new QGPSComDevice(M_PREFS->getGpsPort());
#else
    QGPSS60Device* aGps = new QGPSS60Device();
#endif
    if (aGps->openDevice()) {
        connect(aGps, SIGNAL(updatePosition(float, float, QDateTime, float, float, float)),
            this, SLOT(updateGpsPosition(float, float, QDateTime, float, float, float)));

        ui->gpsConnectAction->setEnabled(false);
        ui->gpsReplayAction->setEnabled(false);
        ui->gpsDisconnectAction->setEnabled(true);
        ui->gpsRecordAction->setEnabled(true);
        ui->gpsPauseAction->setEnabled(true);
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

        ui->gpsConnectAction->setEnabled(false);
        ui->gpsReplayAction->setEnabled(false);
        ui->gpsDisconnectAction->setEnabled(true);
        ui->gpsRecordAction->setEnabled(true);
        ui->gpsPauseAction->setEnabled(true);

        theGPS->setGpsDevice(aGps);
        theGPS->resetGpsStatus();
        theGPS->startGps();
    }
}

void MainWindow::on_gpsDisconnectAction_triggered()
{
    ui->gpsConnectAction->setEnabled(true);
    ui->gpsReplayAction->setEnabled(true);
    ui->gpsDisconnectAction->setEnabled(false);
    ui->gpsRecordAction->setEnabled(false);
    ui->gpsPauseAction->setEnabled(false);
    ui->gpsRecordAction->setChecked(false);
    ui->gpsPauseAction->setChecked(false);

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
        if (M_PREFS->getGpsMapCenter()) {
            CoordBox vp = theView->viewport();
            int lonDiff = vp.lonDiff();
            int latDiff = vp.latDiff();
            QRect vpr = vp.toRect().adjusted(lonDiff / 4, latDiff / 4, -lonDiff / 4, -latDiff / 4);
            if (!vpr.contains(gpsCoord.toQPoint())) {
                theView->setCenter(gpsCoord, theView->rect());
                theView->invalidate(true, true);
            }
        }

        if (ui->gpsRecordAction->isChecked() && !ui->gpsPauseAction->isChecked()) {
            Node* pt = new Node(gpsCoord);
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
    ui->gpsCenterAction->setChecked(M_PREFS->getGpsMapCenter());
    invalidateView();
}

void MainWindow::on_gpsRecordAction_triggered()
{
    if (ui->gpsRecordAction->isChecked()) {
        if (theDocument) {
            QString fn = "log-" + QDateTime::currentDateTime().toString(Qt::ISODate);
            fn.replace(':', '-');

            gpsRecLayer = new TrackLayer();
            gpsRecLayer->setName(fn);
            theDocument->add(gpsRecLayer);

            curGpsTrackSegment = new TrackSegment();
            gpsRecLayer->add(curGpsTrackSegment);
        } else {
            ui->gpsRecordAction->setChecked(false);
        }
    } else {
        ui->gpsPauseAction->setChecked(false);
    }
}
void MainWindow::on_gpsPauseAction_triggered()
{
    if (ui->gpsPauseAction->isChecked()) {
        if (!ui->gpsRecordAction->isChecked()) {
            ui->gpsPauseAction->setChecked(false);
        }
    } else {
        if (theDocument && ui->gpsRecordAction->isChecked()) {
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
    #if defined(Q_OS_MAC)
        QDir resources = QDir(QCoreApplication::applicationDirPath());
        resources.cdUp();
        resources.cd("Resources");
    #endif
    #ifdef TRANSDIR_SYSTEM
        bool retQt;
        if (!QDir::isAbsolutePath(STRINGIFY(TRANSDIR_SYSTEM)))
            retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), QCoreApplication::applicationDirPath() + "/" + STRINGIFY(TRANSDIR_SYSTEM));
        else
            retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), STRINGIFY(TRANSDIR_SYSTEM));
    #else
        #if defined(Q_OS_MAC)
            bool retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), resources.absolutePath());
        #else
            bool retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        #endif

    #endif
        if (retQt)
            QCoreApplication::installTranslator(qtTranslator);

        merkaartorTranslator = new QTranslator;
        // Do not prevent Merkaartor translations to be loaded, even if there is no Qt translation for the language.

        // First, try in the app dir
    #if defined(Q_OS_MAC)
        bool retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, resources.absolutePath());
    #else
        bool retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, QCoreApplication::applicationDirPath());
    #endif
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
        else
            statusBar()->showMessage(tr("Warning! Could not load the Merkaartor translations for the \"%1\" language. Switching to default english.").arg(DefaultLanguage), 15000);
    }
    ui->retranslateUi(this);

    QStringList shortcuts = M_PREFS->getShortcuts();
    for (int i=0; i<shortcuts.size(); i+=2) {
        QAction* act = findChild<QAction*>(shortcuts[i]);
        act->setShortcut(QKeySequence(shortcuts[i+1]));
    }
}

void MainWindow::mapView_interactionChanged(Interaction* anInteraction)
{
    ui->editPropertiesAction->setChecked(dynamic_cast<EditInteraction*>(anInteraction) != NULL);
    ui->editMoveAction->setChecked(dynamic_cast<MoveNodeInteraction*>(anInteraction) != NULL);
    ui->editRotateAction->setChecked(dynamic_cast<RotateInteraction*>(anInteraction) != NULL);
    ui->createNodeAction->setChecked(dynamic_cast<CreateNodeInteraction*>(anInteraction) != NULL);
    ui->createRoadAction->setChecked(dynamic_cast<CreateSingleWayInteraction*>(anInteraction) != NULL);
    ui->createAreaAction->setChecked(dynamic_cast<CreateAreaInteraction*>(anInteraction) != NULL);
}

void MainWindow::updateMenu()
{
    if (M_PREFS->getOfflineMode()) {
        ui->fileWorkOfflineAction->setChecked(true);
        ui->fileDownloadAction->setEnabled(false);
        ui->fileDownloadMoreAction->setEnabled(false);
        ui->fileUploadAction->setEnabled(false);
    } else {
        ui->fileWorkOfflineAction->setChecked(false);
        ui->fileDownloadAction->setEnabled(true);
        ui->fileDownloadMoreAction->setEnabled(true);
        ui->fileUploadAction->setEnabled(true);
    }

    if (M_PREFS->getSeparateMoveMode())
        ui->editMoveAction->setVisible(true);
    else
        ui->editMoveAction->setVisible(false);
}

void MainWindow::on_layersNewImageAction_triggered()
{
    if (theDocument)
        theDocument->addImageLayer();
}
