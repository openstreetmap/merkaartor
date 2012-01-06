#include "Global.h"
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
#include "RelationCommands.h"
#include "ImportExportOSC.h"
#include "ExportGPX.h"
#include "ImportExportKML.h"
#ifdef USE_PROTOBUF
#include "ImportExportPBF.h"
#endif
#include "ImportExportGdal.h"
#include "CreateAreaInteraction.h"
#include "CreateDoubleWayInteraction.h"
#include "CreateNodeInteraction.h"
#include "CreateRoundaboutInteraction.h"
#include "CreatePolygonInteraction.h"
#include "CreateSingleWayInteraction.h"
#include "EditInteraction.h"
#include "MoveNodeInteraction.h"
#include "RotateInteraction.h"
#include "ScaleInteraction.h"
#include "ZoomInteraction.h"
#include "ExtrudeInteraction.h"
#include "Coord.h"
#include "DownloadOSM.h"
#include "ImportGPX.h"
#include "ImportNGT.h"
#include "ImportOSM.h"
#include "Document.h"
#include "Layer.h"
#include "ImageMapLayer.h"
#include "Features.h"
#include "FeatureManipulations.h"
#include "LayerIterator.h"
#include "MasPaintStyle.h"
#include "MapCSSPaintstyle.h"
#include "PaintStyleEditor.h"
#include "Utils/Utils.h"
#include "DirtyList.h"
#include "DirtyListExecutorOSC.h"

#include <ui_MainWindow.h>
#include <ui_AboutDialog.h>
#include <ui_UploadMapDialog.h>
#include <ui_SelectionDialog.h>
#include <ui_ExportDialog.h>
#include <ui_PropertiesDialog.h>

#include "Preferences/PreferencesDialog.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Preferences/ProjectionsList.h"
#include "Preferences/WMSPreferencesDialog.h"
#include "Preferences/TMSPreferencesDialog.h"
#include "Preferences/ProjPreferencesDialog.h"
#include "Preferences/FilterPreferencesDialog.h"
#include "Utils/SelectionDialog.h"
#include "Utils/MDiscardableDialog.h"
#include "QMapControl/imagemanager.h"
#ifdef USE_WEBKIT
    #include "QMapControl/browserimagemanager.h"
#endif
#include "QMapControl/mapadapter.h"
#include "QMapControl/wmsmapadapter.h"
#include "Tools/ActionsDialog.h"
#include "GotoDialog.h"
#include "TerraceDialog.h"

#include "revision.h"
#if QT_VERSION < 0x040700
#include <boost/version.hpp>
#endif

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#include "Render/NativeRenderDialog.h"

#include "qgps.h"
#include "qgpsdevice.h"

#include <QtCore>
#include <QtGui>
#include <QTcpServer>
#include <QTcpSocket>
#include <QXmlStreamReader>

#include "qttoolbardialog.h"

#include <locale.h>

//For About
#include "proj_api.h"
#include "gdal_version.h"

#include "Utils/SlippyMapWidget.h"

class MainWindowPrivate
{
    public:
        MainWindowPrivate()
            : lastPrefTabIndex(0)
            , projActgrp(0)
            , theListeningServer(0)
            , latSaveDirtyLevel(0)
        {
            title = QString("%1 v%2%3(%4)").arg(STRINGIFY(PRODUCT)).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
        }

        QString FILTER_OPEN_SUPPORTED;
        QString FILTER_IMPORT_SUPPORTED;
        int lastPrefTabIndex;
        QString defStyle;
        StyleDock* theStyle;
        FeaturesDock* theFeats;
        QString title;
        QActionGroup* projActgrp;
        QTcpServer* theListeningServer;
        PropertiesDock* theProperties;
        RendererOptions renderOptions;
        int latSaveDirtyLevel;
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
        , toolBarManager(0)
{
    setlocale(LC_NUMERIC, "C");  // impose decimal point separator
    qsrand(QDateTime::currentDateTime().toTime_t());  //initialize random generator

    p = new MainWindowPrivate;

    QString supported_import_formats("*.gpx *.osm *.osc *.ngt *.nmea *.nma *.kml *.csv");
#ifdef GEOIMAGE
    supported_import_formats += " *.jpg";
#endif
    supported_import_formats += " *.shp *.gml";
#ifdef USE_PROTOBUF
    supported_import_formats += " *.pbf";
#endif
    QString supported_import_formats_desc =
            tr("GPS Exchange format (*.gpx)\n") \
            +tr("OpenStreetMap format (*.osm)\n") \
            +tr("OpenStreetMap change format (*.osc)\n") \
            +tr("Noni GPSPlot format (*.ngt)\n") \
            +tr("NMEA GPS log format (*.nmea *.nma)\n") \
            +tr("KML file (*.kml)\n") \
            +tr("Comma delimited format (*.csv)\n");

#ifdef GEOIMAGE
    supported_import_formats_desc += tr("Geotagged images (*.jpg)\n");
#endif
    supported_import_formats_desc += tr("ESRI Shapefile (*.shp)\n") + tr("Geography Markup Language (*.gml)\n");
#ifdef USE_PROTOBUF
    supported_import_formats_desc += tr("Protobuf Binary Format (*.pbf)\n");
#endif

    p->FILTER_OPEN_SUPPORTED = QString(tr("Supported formats") + " (*.mdc %1)\n").arg(supported_import_formats);
    p->FILTER_OPEN_SUPPORTED += tr("Merkaartor document (*.mdc)\n") + supported_import_formats_desc;
    p->FILTER_OPEN_SUPPORTED += tr("All Files (*)");

    p->FILTER_IMPORT_SUPPORTED = QString(tr("Supported formats") + " (%1)\n").arg(supported_import_formats);
    p->FILTER_IMPORT_SUPPORTED += supported_import_formats_desc;
    p->FILTER_IMPORT_SUPPORTED += tr("All Files (*)");

    theProgressDialog = NULL;
    theProgressBar = NULL;
    theProgressLabel = NULL;

    if (M_PREFS->getMerkaartorStyle())
        QApplication::setStyle(QStyleFactory::create(M_PREFS->getMerkaartorStyleString()));

    ui->setupUi(this);
    M_STYLE->loadPainters(M_PREFS->getDefaultStyle());

    blockSignals(true);

    ViewportStatusLabel = new QLabel(this);
    MeterPerPixelLabel = new QLabel(this);
    AdjusmentMeterLabel = new QLabel(this);

    pbImages = new QProgressBar(this);
    statusBar()->addPermanentWidget(ViewportStatusLabel);
    statusBar()->addPermanentWidget(pbImages);
    statusBar()->addPermanentWidget(MeterPerPixelLabel);
    statusBar()->addPermanentWidget(AdjusmentMeterLabel);
#ifndef NDEBUG
    PaintTimeLabel = new QLabel(this);
    PaintTimeLabel->setMinimumWidth(23);
    statusBar()->addPermanentWidget(PaintTimeLabel);
#endif

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

    p->theProperties = new PropertiesDock(this);
    connect(p->theProperties, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
    on_editPropertiesAction_triggered();

    theDirty = new DirtyDock(this);
    connect(theDirty, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

    p->theStyle = new StyleDock(this);
    connect(p->theStyle, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

    p->theFeats = new FeaturesDock(this);
    connect(p->theFeats, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
    connect(theView, SIGNAL(viewportChanged()), p->theFeats, SLOT(on_Viewport_changed()), Qt::QueuedConnection);
    connect(this, SIGNAL(content_changed()), p->theFeats, SLOT(on_Viewport_changed()), Qt::QueuedConnection);
    connect(this, SIGNAL(content_changed()), p->theProperties, SLOT(adjustSelection()), Qt::QueuedConnection);

    theGPS = new QGPS(this);
    connect(theGPS, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));

#ifdef GEOIMAGE
    theGeoImage = new GeoImageDock(this);
    theGeoImage->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::RightDockWidgetArea, theGeoImage);
    connect(theGeoImage, SIGNAL(visibilityChanged(bool)), this, SLOT(updateWindowMenu(bool)));
#endif

    connect(theLayers, SIGNAL(layersChanged(bool)), this, SLOT(adjustLayers(bool)));
    connect(theLayers, SIGNAL(layersCleared()), this, SIGNAL(content_changed()));
    connect(theLayers, SIGNAL(layersClosed()), this, SIGNAL(content_changed()));
    connect(theLayers, SIGNAL(layersProjection(const QString&)), this, SLOT(projectionSet(const QString&)));

    connect (M_PREFS, SIGNAL(bookmarkChanged()), this, SLOT(updateBookmarksMenu()));
    updateBookmarksMenu();
    connect (ui->menuBookmarks, SIGNAL(triggered(QAction *)), this, SLOT(bookmarkTriggered(QAction *)));

    updateRecentOpenMenu();
    connect (ui->menuRecentOpen, SIGNAL(triggered(QAction *)), this, SLOT(recentOpenTriggered(QAction *)));

    updateRecentImportMenu();
    connect (ui->menuRecentImport, SIGNAL(triggered(QAction *)), this, SLOT(recentImportTriggered(QAction *)));

    updateStyleMenu();
    connect (ui->menuStyles, SIGNAL(triggered(QAction *)), this, SLOT(styleTriggered(QAction *)));

    ui->viewDownloadedAction->setChecked(M_PREFS->getDownloadedVisible());
    ui->viewDirtyAction->setChecked(M_PREFS->getDirtyVisible());
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

    if (M_PREFS->getBackgroundVisible()) p->renderOptions.options |= RendererOptions::BackgroundVisible; else p->renderOptions.options &= ~RendererOptions::BackgroundVisible;
    if (M_PREFS->getForegroundVisible()) p->renderOptions.options |= RendererOptions::ForegroundVisible; else p->renderOptions.options &= ~RendererOptions::ForegroundVisible;
    if (M_PREFS->getTouchupVisible()) p->renderOptions.options |= RendererOptions::TouchupVisible; else p->renderOptions.options &= ~RendererOptions::TouchupVisible;
    if (M_PREFS->getNamesVisible()) p->renderOptions.options |= RendererOptions::NamesVisible; else p->renderOptions.options &= ~RendererOptions::NamesVisible;
    if (M_PREFS->getPhotosVisible()) p->renderOptions.options |= RendererOptions::PhotosVisible; else p->renderOptions.options &= ~RendererOptions::PhotosVisible;
    if (M_PREFS->getVirtualNodesVisible()) p->renderOptions.options |= RendererOptions::VirtualNodesVisible; else p->renderOptions.options &= ~RendererOptions::VirtualNodesVisible;
    if (M_PREFS->getTrackPointsVisible()) p->renderOptions.options |= RendererOptions::NodesVisible; else p->renderOptions.options &= ~RendererOptions::NodesVisible;
    if (M_PREFS->getTrackSegmentsVisible()) p->renderOptions.options |= RendererOptions::TrackSegmentVisible; else p->renderOptions.options &= ~RendererOptions::TrackSegmentVisible;
    if (M_PREFS->getRelationsVisible()) p->renderOptions.options |= RendererOptions::RelationsVisible; else p->renderOptions.options &= ~RendererOptions::RelationsVisible;
    if (M_PREFS->getDownloadedVisible()) p->renderOptions.options |= RendererOptions::DownloadedVisible; else p->renderOptions.options &= ~RendererOptions::DownloadedVisible;
    if (M_PREFS->getScaleVisible()) p->renderOptions.options |= RendererOptions::ScaleVisible; else p->renderOptions.options &= ~RendererOptions::ScaleVisible;
    if (M_PREFS->getLatLonGridVisible()) p->renderOptions.options |= RendererOptions::LatLonGridVisible; else p->renderOptions.options &= ~RendererOptions::LatLonGridVisible;
    if (M_PREFS->getZoomBoris()) p->renderOptions.options |= RendererOptions::LockZoom; else p->renderOptions.options &= ~RendererOptions::LockZoom;

    updateMenu();

    QActionGroup* actgrpArrows = new QActionGroup(this);
    actgrpArrows->addAction(ui->viewArrowsNeverAction);
    actgrpArrows->addAction(ui->viewArrowsOnewayAction);
    actgrpArrows->addAction(ui->viewArrowsAlwaysAction);
    switch (M_PREFS->getDirectionalArrowsVisible()) {
        case RendererOptions::ArrowsNever:
            ui->viewArrowsNeverAction->setChecked(true);
            p->renderOptions.arrowOptions = RendererOptions::ArrowsNever;
            break;
        case RendererOptions::ArrowsOneway:
            ui->viewArrowsOnewayAction->setChecked(true);
            p->renderOptions.arrowOptions = RendererOptions::ArrowsOneway;
            break;
        case RendererOptions::ArrowsAlways:
            ui->viewArrowsAlwaysAction->setChecked(true);
            p->renderOptions.arrowOptions = RendererOptions::ArrowsAlways;
            break;
    }

    theView->setRenderOptions(p->renderOptions);

    ui->gpsCenterAction->setChecked(M_PREFS->getGpsMapCenter());

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));

#ifndef _MOBILE
    theLayers->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, theLayers);

    p->theProperties->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, p->theProperties);

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

#else
    p->theProperties->setVisible(false);
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

    createToolBarManager();  // has to be before restorestate
    M_PREFS->restoreMainWindowState( this );

#ifndef _MOBILE
    if (!M_PREFS->getProjectionsList()->getProjections()->size()) {
        QMessageBox::critical(this, tr("Cannot load Projections file"), tr("\"Projections.xml\" could not be opened anywhere. Aborting."));
        exit(1);
    }
#endif

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

    if (!g_Merk_Frisius) {
        ui->layersNewDrawingAction->setVisible(false);
    }

    blockSignals(false);

    QTimer::singleShot( 0, this, SLOT(delayedInit()) );
}

void MainWindow::delayedInit()
{
    QList<QAction*> actions = findChildren<QAction*>();
    for (int i=0; i<actions.size(); i++) {
        shortcutsDefault[actions[i]->objectName()] = actions[i]->shortcut().toString();
    }
    QStringList shortcuts = M_PREFS->getShortcuts();
    for (int i=0; i<shortcuts.size(); i+=2) {
        QAction* act = findChild<QAction*>(shortcuts[i]);
        if (act)
            act->setShortcut(QKeySequence(shortcuts[i+1]));
    }

    updateWindowMenu();

    if (M_PREFS->getLocalServer()) {
        p->theListeningServer = new QTcpServer(this);
        connect(p->theListeningServer, SIGNAL(newConnection()), this, SLOT(incomingLocalConnection()));
        if (!p->theListeningServer->listen(QHostAddress::LocalHost, 8111))
            qDebug() << "Remote control: Unable to listen on 8111";
    }

//    M_PREFS->initialPosition(theView);
    on_fileNewAction_triggered();
    invalidateView();
}

void MainWindow::handleMessage(const QString &msg)
{
    QStringList args = msg.split("$", QString::SkipEmptyParts);
    QStringList fileNames;
    for (int i=0; i < args.size(); ++i) {
        if (args[i] == "-l" || args[i] == "--log") {
            ++i;
        } else {
            QUrl u(args[i]);
            if (u.scheme() == "osm") {
                loadUrl(u);
                continue;
            }
            fileNames.append(args[i]);
        }
    }

    if (fileNames.isEmpty())
        QDir::setCurrent(M_PREFS->getworkingdir());
    else
        loadFiles(fileNames);
}

MainWindow::~MainWindow(void)
{
    p->theProperties->setSelection(NULL);

    delete M_STYLE;
    delete theDocument;
    delete theView;
    delete p->theProperties;

    delete qtTranslator;
    delete merkaartorTranslator;

    delete SlippyMapWidget::theSlippyCache;

    delete p;

    delete M_PREFS;
    delete ui;
}

void MainWindow::incomingLocalConnection()
{
    QTcpSocket *clientConnection = p->theListeningServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));
    connect(clientConnection, SIGNAL(readyRead()), this, SLOT(readLocalConnection()) );
}

void MainWindow::readLocalConnection()
{
    QTcpSocket* socket = (QTcpSocket*)sender();
    if ( socket->canReadLine() ) {
        QString ln = socket->readLine();
        QStringList tokens = ln.split( QRegExp("[ \r\n][ \r\n]*"), QString::SkipEmptyParts );
        if ( tokens[0] == "GET" ) {
            QTextStream resultStream(socket);
            resultStream << "HTTP/1.1 200 OK\r\n";
            resultStream << "Date: " << QDateTime::currentDateTime().toString(Qt::TextDate);
            resultStream << "Server: Merkaartor RemoteControl\r\n";
            resultStream << "Content-type: text/plain\r\n";
            resultStream << "Access-Control-Allow-Origin: *\r\n";
            resultStream << "Content-length: 4\r\n\r\n";
            resultStream << "OK\r\n";
            socket->disconnectFromHost();

            QUrl u = QUrl(tokens[1]);
            loadUrl(u);
        }
    }
}

void MainWindow::createToolBarManager()
{
    toolBarManager = new QtToolBarManager(this);
    toolBarManager->setMainWindow(this);

    foreach(QAction* a, ui->menuFile->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("File"));;
    }
    foreach(QAction* a, ui->menuEdit->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Edit"));;
    }
    foreach(QAction* a, ui->menuView->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("View"));;
    }
    foreach(QAction* a, ui->menu_Show->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Show"));;
    }
    foreach(QAction* a, ui->menuShow_directional_Arrows->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Directional Arrows"));;
    }
    foreach(QAction* a, ui->menuGps->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("GPS"));;
    }
    foreach(QAction* a, ui->menuLayers->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Layers"));;
    }
    foreach(QAction* a, ui->menuCreate->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Create"));;
    }
    foreach(QAction* a, ui->menu_Feature->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Feature"));;
    }
    foreach(QAction* a, ui->menuOpenStreetBugs->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("OpenStreetBugs"));;
    }
    foreach(QAction* a, ui->menu_Node->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Node"));;
    }
    foreach(QAction* a, ui->menuRoad->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Way"));;
    }
    foreach(QAction* a, ui->menuRelation->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Relation"));;
    }
    foreach(QAction* a, ui->menuTools->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Tools"));;
    }
    foreach(QAction* a, ui->menuWindow->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Windows"));;
    }
    foreach(QAction* a, ui->menuHelp->actions()) {
        if (!a->isSeparator() && !a->menu())
            toolBarManager->addAction(a, tr("Help"));;
    }

    toolBarManager->addToolBar(ui->toolBar, "");

    QSettings* Sets = M_PREFS->getQSettings();
    if (!g_Merk_Ignore_Preferences && !g_Merk_Reset_Preferences)
            if (Sets->contains("MainWindow/Toolbars"))
                toolBarManager->restoreState(Sets->value("MainWindow/Toolbars").toByteArray());
}

void MainWindow::on_toolsToolbarsAction_triggered()
{
    QtToolBarDialog dlg(this);
    dlg.setToolBarManager(toolBarManager);
    dlg.exec();

    QSettings* Sets = M_PREFS->getQSettings();
    Sets->setValue("MainWindow/Toolbars", toolBarManager->saveState());
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
        if (l && l->isTiled()) {
            theView->projection().setProjectionType(l->projection());
            updateProjectionMenu();
        }
    }
    if (adjustViewport) {
        CoordBox theVp;
        theVp = theView->viewport();
        theView->setViewport(theVp, theView->rect());
    }
    invalidateView(true);
}

void MainWindow::invalidateView(bool UpdateDock)
{
    theView->setRenderOptions(p->renderOptions);
    theView->invalidate(true, true);
    //theLayers->updateContent();
    if (UpdateDock)
        p->theProperties->resetValues();
}

PropertiesDock* MainWindow::properties()
{
    return p->theProperties;
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

void MainWindow::on_editCutAction_triggered()
{
    // Export
    QClipboard *clipboard = QApplication::clipboard();
    QMimeData* md = new QMimeData();

    QBuffer osmBuf;
    osmBuf.open(QIODevice::WriteOnly);

    QList<Feature*> exportedFeatures = document()->exportCoreOSM(p->theProperties->selection());
    theDocument->exportOSM(this, &osmBuf, exportedFeatures);
    md->setText(QString(osmBuf.data()));
    md->setData("application/x-openstreetmap+xml", osmBuf.data());

    ImportExportKML kmlexp(theDocument);
    QBuffer kmlBuf;
    kmlBuf.open(QIODevice::WriteOnly);
    if (kmlexp.setDevice(&kmlBuf)) {
        kmlexp.export_(p->theProperties->selection());
        md->setData("application/vnd.google-earth.kml+xml", kmlBuf.data());
    }

    ExportGPX gpxexp(theDocument);
    QBuffer gpxBuf;
    gpxBuf.open(QIODevice::WriteOnly);
    if (gpxexp.setDevice(&gpxBuf)) {
        gpxexp.export_(p->theProperties->selection());
        md->setData("application/gpx+xml", gpxBuf.data());
    }

    //Deletion
    QList<Feature*> Sel;
    for (int i=0; i<p->theProperties->size(); ++i)
        Sel.push_back(p->theProperties->selection(i));
    if (Sel.size() == 0) return;

    CommandList* theList  = new CommandList(MainWindow::tr("Cut Features"), Sel[0]);
    for (int i=0; i<Sel.size(); ++i) {
        QList<Feature*> Alternatives;
        theList->add(new RemoveFeatureCommand(document(), Sel[i], Alternatives));
    }

    if (theList->size()) {
        document()->addHistory(theList);
    }
    else {
        delete theList;
        return;
    }

    QString xml;
    QXmlStreamWriter stream(&xml);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);
    stream.writeStartDocument();
    stream.writeStartElement("MerkaartorUndo");
    stream.writeAttribute("documentid", theDocument->id());
    theList->toXML(stream);
    stream.writeEndElement();
    stream.writeEndDocument();
    md->setData("application/x-merkaartor-undo+xml", xml.toUtf8());
//    qDebug() << doc.toString(2);

    clipboard->setMimeData(md);

    properties()->setSelection(0);
    properties()->checkMenuStatus();
    view()->invalidate(true, false);
}

void MainWindow::on_editCopyAction_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    QMimeData* md = new QMimeData();

    QBuffer osmBuf;
    osmBuf.open(QIODevice::WriteOnly);

    QList<Feature*> exportedFeatures = document()->exportCoreOSM(p->theProperties->selection(), true);
    theDocument->exportOSM(this, &osmBuf, exportedFeatures);
    md->setText(QString(osmBuf.data()));
    md->setData("application/x-openstreetmap+xml", osmBuf.data());

    ImportExportKML kmlexp(theDocument);
    QBuffer kmlBuf;
    kmlBuf.open(QIODevice::WriteOnly);
    if (kmlexp.setDevice(&kmlBuf)) {
        kmlexp.export_(p->theProperties->selection());
        md->setData("application/vnd.google-earth.kml+xml", kmlBuf.data());
    }

    ExportGPX gpxexp(theDocument);
    QBuffer gpxBuf;
    gpxBuf.open(QIODevice::WriteOnly);
    if (gpxexp.setDevice(&gpxBuf)) {
        gpxexp.export_(p->theProperties->selection());
        md->setData("application/gpx+xml", gpxBuf.data());
    }

    clipboard->setMimeData(md);
    invalidateView();
}

void MainWindow::on_editPasteFeatureAction_triggered()
{
    DrawingLayer* l = dynamic_cast<DrawingLayer*>(theDocument->getDirtyOrOriginLayer());
    if (!l)
        return;

    Document* doc;

    QClipboard *clipboard = QApplication::clipboard();
    if (clipboard->mimeData()->hasFormat("application/x-merkaartor-undo+xml")) {
        QDomDocument* theXmlDoc = new QDomDocument();
        if (!theXmlDoc->setContent(clipboard->mimeData()->data("application/x-merkaartor-undo+xml"))) {
            delete theXmlDoc;
        } else {
            QDomElement root = theXmlDoc->firstChildElement("MerkaartorUndo");
            if (!root.isNull()) {
                QString docId = root.attribute("documentid");
                if (theDocument->id() == docId) {

                    QDomNodeList nl = theXmlDoc->elementsByTagName("RemoveFeatureCommand");
                    for (int i=0; i<nl.size(); ++i) {
                        nl.at(i).toElement().setAttribute("layer", l->id());
                    }

                    QString xml;
                    QTextStream tstr(&xml, QIODevice::ReadOnly);
                    root.firstChildElement("CommandList").save(tstr, 2);
                    QXmlStreamReader stream(xml);

                    CommandList* theList = CommandList::fromXML(theDocument, stream);
                    theList->setReversed(true);
                    theList->redo();
                    theList->setDescription("Paste Features");
                    theDocument->addHistory(theList);

                    view()->invalidate(true, false);

                    return;
                }
            }
        }
    }

    if (!(doc = Document::getDocumentFromClipboard())) {
        QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard do not contain valid data."));
        return;
    }

    CommandList* theList = new CommandList();
    theList->setDescription("Paste Features");
    QList<Feature*> theFeats = theDocument->mergeDocument(doc, theDocument->getDirtyOrOriginLayer(), theList);

    if (theList->size())
        document()->addHistory(theList);
    else
        delete theList;

    delete doc;

    p->theProperties->setSelection(theFeats);
    view()->invalidate(true, false);
}

void MainWindow::on_editPasteOverwriteAction_triggered()
{
    QList<Feature*> sel = properties()->selection();
    if (!sel.size())
        return;

    Document* doc;
    if (!(doc = Document::getDocumentFromClipboard())) {
        QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard do not contain valid data."));
        return;
    }

    CommandList* theList = new CommandList();
    theList->setDescription("Paste tags (overwrite)");

    for(int i=0; i < sel.size(); ++i) {
        theList->add(new ClearTagsCommand(sel[i], theDocument->getDirtyOrOriginLayer(sel[i]->layer())));
        for (FeatureIterator k(doc); !k.isEnd(); ++k) {
            // Allow any<->any pasting but only takes top level feature into account
            if (k.get()->sizeParents())
                continue;
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
    if (!(doc = Document::getDocumentFromClipboard())) {
        QMessageBox::critical(this, tr("Clipboard invalid"), tr("Clipboard do not contain valid data."));
        return;
    }

    CommandList* theList = new CommandList();
    theList->setDescription("Paste tags (merge)");

    for(int i=0; i < sel.size(); ++i) {
        for (FeatureIterator k(doc); !k.isEnd(); ++k) {
            // Allow any<->any pasting but only takes top level feature into account
            if (k.get()->sizeParents())
                continue;
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

void MainWindow::clipboardChanged()
{
    ui->editPasteFeatureAction->setEnabled(false);
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

    ui->editPasteFeatureAction->setEnabled(true);
    ui->editPasteMergeAction->setEnabled(true);
    ui->editPasteOverwriteAction->setEnabled(true);

    delete theXmlDoc;
}

void MainWindow::on_editRedoAction_triggered()
{
    theDocument->redoHistory();
    p->theProperties->adjustSelection();
    invalidateView();
}

void MainWindow::on_editUndoAction_triggered()
{
    theDocument->undoHistory();
    p->theProperties->adjustSelection();
    invalidateView();
}

void MainWindow::on_editPropertiesAction_triggered()
{
    if (theView->interaction() && dynamic_cast<EditInteraction*>(theView->interaction()))
        p->theProperties->setSelection(0);
    theView->unlockSelection();
    theView->invalidate(true, false);
    theView->launch(new EditInteraction(theView));
    theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_editRemoveAction_triggered()
{
    emit remove_triggered();
    emit content_changed();
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

void MainWindow::on_editScaleAction_triggered()
{
    view()->launch(new ScaleInteraction(view()));
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
    M_PREFS->setworkingdir(QDir::currentPath());
}


void MainWindow::on_fileImportAction_triggered()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
                    this,
                    tr("Import file"),
                    "", p->FILTER_IMPORT_SUPPORTED);

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

static bool mayDiscardStyleChanges(QWidget* aWidget)
{
    return QMessageBox::question(aWidget, MainWindow::tr("Unsaved Style changes"),
                                 MainWindow::tr("You have modified the current style.\n"
                                                "Do you want to save your changes?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::No;
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
                    if (importOK && M_PREFS->getAutoExtractTracks()) {
                        theTracklayers[i]->extractLayer();
                    }
                }
            }
        }
        else if (fn.toLower().endsWith(".osm")) {
            newLayer = new DrawingLayer( baseFileName );
            mapDocument->add(newLayer);
            importOK = importOSM(this, baseFileName, mapDocument, newLayer);
        }
#ifndef FRISIUS_BUILD
        else if (fn.toLower().endsWith(".osc")) {
            if (g_Merk_Frisius) {
                newLayer = new DrawingLayer( baseFileName );
                mapDocument->add(newLayer);
            } else {
                newLayer = mapDocument->getDirtyOrOriginLayer();
            }
            importOK = mapDocument->importOSC(fn, (DrawingLayer*)newLayer);
        }
#endif
        else if (fn.toLower().endsWith(".ngt")) {
            newLayer = new TrackLayer( baseFileName );
            newLayer->setUploadable(false);
            mapDocument->add(newLayer);
            importOK = importNGT(this, baseFileName, mapDocument, newLayer);
            if (importOK && M_PREFS->getAutoExtractTracks()) {
                ((TrackLayer *)newLayer)->extractLayer();
            }
        }
        else if (fn.toLower().endsWith(".nmea") || (fn.toLower().endsWith(".nma"))) {
            newLayer = new TrackLayer( baseFileName );
            newLayer->setUploadable(false);
            mapDocument->add(newLayer);
            importOK = mapDocument->importNMEA(baseFileName, (TrackLayer *)newLayer);
            if (importOK && M_PREFS->getAutoExtractTracks()) {
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
#ifdef USE_PROTOBUF
        else if (fn.toLower().endsWith(".pbf")) {
            newLayer = new DrawingLayer( baseFileName );
            mapDocument->add(newLayer);
            importOK = mapDocument->importPBF(baseFileName, (DrawingLayer*)newLayer);
        }
#endif
        else { // Fallback to GDAL
            qDebug() << "Trying GDAL";
            newLayer = new DrawingLayer( baseFileName );
            newLayer->setUploadable(false);
            mapDocument->add(newLayer);
            importOK = mapDocument->importGDAL(baseFileName, (DrawingLayer*)newLayer);
        }

        if (!importOK && newLayer)
            mapDocument->remove(newLayer);

        if (importOK)
        {
            foundImport = true;

            if (importedFileNames)
                importedFileNames->append(fn);

            emit content_changed();
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
    bool skipImport = false;
    QMutableStringListIterator it(fileNames);
    while (it.hasNext())
    {
        const QString & fn = it.next();

        if (fn.toLower().endsWith(".mdc") == false)
            continue;

        if (skipImport == false)
        {
            changeCurrentDirToFile(fn);
            loadDocument(fn);
            skipImport = true;
        }

        it.remove();
    }

    Document* newDoc = theDocument;
    if (skipImport == false) {
        newDoc = new Document(theLayers);
        newDoc->addDefaultLayers();
    }

    QStringList openedFiles;
    bool foundImport = importFiles(newDoc, fileNames, &openedFiles);

    foreach (QString currentFileName, openedFiles)
        M_PREFS->addRecentOpen(currentFileName);

    updateRecentOpenMenu();

    p->theProperties->setSelection(0);

    if (skipImport == false)
    {
        if (foundImport)
        {
            // only imported some tracks
            p->theFeats->invalidate();
            delete theDocument;
            theDocument = newDoc;
            connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
            connect (theDocument, SIGNAL(historyChanged()), this, SIGNAL(content_changed()));
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

    invalidateView(false);
}

void MainWindow::loadUrl(const QUrl& u)
{
    activateWindow();

    if (u.path() == "/load_and_zoom") {
        qreal t = u.queryItemValue("top").toDouble();
        qreal b = u.queryItemValue("bottom").toDouble();
        qreal r = u.queryItemValue("right").toDouble();
        qreal l = u.queryItemValue("left").toDouble();

        if (theView) {
            CoordBox vp(Coord(l,b), Coord(r,t));
            theView->setViewport(vp, theView->rect());
            on_fileDownloadMoreAction_triggered();
        }
        properties()->setSelection(0);

        Feature* F;
        IFeature::FId mId;
        QString sel = u.queryItemValue("select");
        if (!sel.isNull()) {
            QStringList sl = sel.split(",");
            foreach (QString f, sl) {
                if (f.startsWith("node")) {
                    f.remove("node");
                    mId.type = IFeature::Point;
                    mId.numId = f.toLongLong();
                } else if (f.startsWith("way")) {
                    f.remove("way");
                    mId.type = IFeature::LineString;
                    mId.numId = f.toLongLong();
                } else if (f.startsWith("relation")) {
                    f.remove("relation");
                    mId.type = IFeature::OsmRelation;
                    mId.numId = f.toLongLong();
                }
                F = theDocument->getFeature(mId);
                if (F)
                    properties()->addSelection(F);
            }
        }
    } else {
        QMessageBox::critical(this, tr("Incoming Remote control request"), tr("Unknow action url: %1").arg(u.toString()));
    }
}

void MainWindow::on_fileOpenAction_triggered()
{
    if (hasUnsavedChanges() && !mayDiscardUnsavedChanges(this))
        return;

    QStringList fileNames = QFileDialog::getOpenFileNames(
                    this,
                    tr("Open files"),
                    "", p->FILTER_OPEN_SUPPORTED);

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

    while (M_PREFS->getOsmUser().isEmpty()) {
        int ret = QMessageBox::warning(this, tr("Upload OSM"), tr("You don't seem to have specified your\n"
            "OpenStreetMap username and password.\nDo you want to do this now?"), QMessageBox::Yes | QMessageBox::No);
        if (ret == QMessageBox::Yes) {
            toolsPreferencesAction_triggered(true);
        } else
            return;
    }
    on_editPropertiesAction_triggered();
    syncOSM(M_PREFS->getOsmApiUrl(), M_PREFS->getOsmUser(), M_PREFS->getOsmPassword());

    theDocument->history().updateActions();
    theDirty->updateList();
    invalidateView();
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

    emit content_changed();
}

void MainWindow::on_fileDownloadMoreAction_triggered()
{
    createProgressDialog();

    if (!downloadMoreOSM(this, theView->viewport(), theDocument)) {
        QMessageBox::warning(this, tr("Error downloading"), tr("The map could not be downloaded"));
    }

    deleteProgressDialog();

    emit content_changed();
}

void MainWindow::on_layersOpenstreetbugsAction_triggered()
{
    SpecialLayer* sl = NULL;
    for (int i=0; i<theDocument->layerSize(); ++i) {
        if (theDocument->getLayer(i)->classType() == Layer::OsmBugsLayer) {
            sl = dynamic_cast<SpecialLayer*>(theDocument->getLayer(i));
            while (sl->size())
            {
                sl->deleteFeature(sl->get(0));
            }
        }
    }

    createProgressDialog();

    if (!::downloadOpenstreetbugs(this, theView->viewport(), theDocument, sl)) {
        QMessageBox::warning(this, tr("Error downloading OpenStreetBugs"), tr("The OpenStreetBugs could not be downloaded"));
    }

    deleteProgressDialog();
}

void MainWindow::on_layersMapdustAction_triggered()
{
    SpecialLayer* sl = NULL;
    for (int i=0; i<theDocument->layerSize(); ++i) {
        if (theDocument->getLayer(i)->classType() == Layer::MapDustLayer) {
            sl = dynamic_cast<SpecialLayer*>(theDocument->getLayer(i));
            while (sl->size())
            {
                sl->deleteFeature(sl->get(0));
            }
        }
    }

    createProgressDialog();

    if (!::downloadMapdust(this, theView->viewport(), theDocument, sl)) {
        QMessageBox::warning(this, tr("Error downloading MapDust"), tr("The MapDust bugs could not be downloaded"));
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

    emit content_changed();

}

void MainWindow::on_fileWorkOfflineAction_triggered()
{
    M_PREFS->setOfflineMode(!M_PREFS->getOfflineMode());
    updateMenu();
}

void MainWindow::on_filePrintAction_triggered()
{
    NativeRenderDialog osmR(theDocument, theView->viewport(), this);
    osmR.exec();
}

void MainWindow::on_filePropertiesAction_triggered()
{
    QDialog dlg(this);
    Ui::PropertiesDialog Prop;
    Prop.setupUi(&dlg);

    QString h;
    h += theView->toPropertiesHtml();
    h += "<br/>";
    h += theDocument->toPropertiesHtml();
    Prop.textBrowser->setHtml(h);

    dlg.exec();
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
#if QT_VERSION < 0x040700
    int boostMajVer = BOOST_VERSION / 100000;
    int boostMinVer = BOOST_VERSION / 100 % 1000;
    About.BoostVersion->setText(About.BoostVersion->text().arg(QString::number(boostMajVer)+"."+QString::number(boostMinVer)));
#else
    About.BoostVersion->setVisible(false);
#endif
    QString projVer = QString(STRINGIFY(PJ_VERSION));
    About.Proj4Version->setText(About.Proj4Version->text().arg(QString("%1.%2.%3").arg(projVer.left(1)).arg(projVer.mid(1, 1)).arg(projVer.right(1))));
    About.GdalVersion->setText(About.GdalVersion->text().arg(GDAL_RELEASE_NAME));

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
    theView->zoomIn();
}

void MainWindow::on_viewZoomOutAction_triggered()
{
    theView->zoomOut();
}

void MainWindow::on_viewZoomWindowAction_triggered()
{
    theView->launch(new ZoomInteraction(theView));
}

void MainWindow::on_viewLockZoomAction_triggered()
{
    M_PREFS->setZoomBoris(!M_PREFS->getZoomBoris());
    if (M_PREFS->getZoomBoris()) p->renderOptions.options |= RendererOptions::LockZoom; else p->renderOptions.options &= ~RendererOptions::LockZoom;
    ui->viewLockZoomAction->setChecked(M_PREFS->getZoomBoris());
    ImageMapLayer* l = NULL;
    for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
        l = ImgIt.get();
        break;
    }
    if (l && l->isTiled()) {
        theView->projection().setProjectionType(l->projection());
        theView->setViewport(theView->viewport(), theView->rect());
    }
    theView->adjustZoomToBoris();
    updateProjectionMenu();
    invalidateView();
}

void MainWindow::on_viewDownloadedAction_triggered()
{
    M_PREFS->setDownloadedVisible(!M_PREFS->getDownloadedVisible());
    if (M_PREFS->getDownloadedVisible()) p->renderOptions.options |= RendererOptions::DownloadedVisible; else p->renderOptions.options &= ~RendererOptions::DownloadedVisible;
    ui->viewDownloadedAction->setChecked(M_PREFS->getDownloadedVisible());
    invalidateView();
}

void MainWindow::on_viewDirtyAction_triggered()
{
    M_PREFS->setDirtyVisible(!M_PREFS->getDirtyVisible());
    if (M_PREFS->getDirtyVisible()) p->renderOptions.options |= RendererOptions::DirtyVisible; else p->renderOptions.options &= ~RendererOptions::DirtyVisible;
    ui->viewDirtyAction->setChecked(M_PREFS->getDirtyVisible());
    invalidateView();
}

void MainWindow::on_viewScaleAction_triggered()
{
    M_PREFS->setScaleVisible(!M_PREFS->getScaleVisible());
    if (M_PREFS->getScaleVisible()) p->renderOptions.options |= RendererOptions::ScaleVisible; else p->renderOptions.options &= ~RendererOptions::ScaleVisible;
    ui->viewScaleAction->setChecked(M_PREFS->getScaleVisible());
    invalidateView();
}

void MainWindow::on_viewPhotosAction_triggered()
{
    M_PREFS->setPhotosVisible(!M_PREFS->getPhotosVisible());
    if (M_PREFS->getPhotosVisible()) p->renderOptions.options |= RendererOptions::PhotosVisible; else p->renderOptions.options &= ~RendererOptions::PhotosVisible;
    ui->viewPhotosAction->setChecked(M_PREFS->getPhotosVisible());
    invalidateView();
}

void MainWindow::on_viewShowLatLonGridAction_triggered()
{
    M_PREFS->setLatLonGridVisible(!M_PREFS->getLatLonGridVisible());
    if (M_PREFS->getLatLonGridVisible()) p->renderOptions.options |= RendererOptions::LatLonGridVisible; else p->renderOptions.options &= ~RendererOptions::LatLonGridVisible;
    ui->viewShowLatLonGridAction->setChecked(M_PREFS->getLatLonGridVisible());
    invalidateView();
}

void MainWindow::on_viewStyleBackgroundAction_triggered()
{
    M_PREFS->setBackgroundVisible(!M_PREFS->getBackgroundVisible());
    if (M_PREFS->getBackgroundVisible()) p->renderOptions.options |= RendererOptions::BackgroundVisible; else p->renderOptions.options &= ~RendererOptions::BackgroundVisible;
    ui->viewStyleBackgroundAction->setChecked(M_PREFS->getBackgroundVisible());
    invalidateView();
}

void MainWindow::on_viewStyleForegroundAction_triggered()
{
    M_PREFS->setForegroundVisible(!M_PREFS->getForegroundVisible());
    if (M_PREFS->getForegroundVisible()) p->renderOptions.options |= RendererOptions::ForegroundVisible; else p->renderOptions.options &= ~RendererOptions::ForegroundVisible;
    ui->viewStyleForegroundAction->setChecked(M_PREFS->getForegroundVisible());
    invalidateView();
}

void MainWindow::on_viewStyleTouchupAction_triggered()
{
    M_PREFS->setTouchupVisible(!M_PREFS->getTouchupVisible());
    if (M_PREFS->getTouchupVisible()) p->renderOptions.options |= RendererOptions::TouchupVisible; else p->renderOptions.options &= ~RendererOptions::TouchupVisible;
    ui->viewStyleTouchupAction->setChecked(M_PREFS->getTouchupVisible());
    invalidateView();
}

void MainWindow::on_viewNamesAction_triggered()
{
    M_PREFS->setNamesVisible(!M_PREFS->getNamesVisible());
    if (M_PREFS->getNamesVisible()) p->renderOptions.options |= RendererOptions::NamesVisible; else p->renderOptions.options &= ~RendererOptions::NamesVisible;
    ui->viewNamesAction->setChecked(M_PREFS->getNamesVisible());
    invalidateView();
}

void MainWindow::on_viewVirtualNodesAction_triggered()
{
    M_PREFS->setVirtualNodesVisible(!M_PREFS->getVirtualNodesVisible());
    if (M_PREFS->getVirtualNodesVisible()) p->renderOptions.options |= RendererOptions::VirtualNodesVisible; else p->renderOptions.options &= ~RendererOptions::VirtualNodesVisible;
    ui->viewVirtualNodesAction->setChecked(M_PREFS->getVirtualNodesVisible());
    invalidateView();
}

void MainWindow::on_viewTrackPointsAction_triggered()
{
    M_PREFS->setTrackPointsVisible(!M_PREFS->getTrackPointsVisible());
    if (M_PREFS->getTrackPointsVisible()) p->renderOptions.options |= RendererOptions::NodesVisible; else p->renderOptions.options &= ~RendererOptions::NodesVisible;
    ui->viewTrackPointsAction->setChecked(M_PREFS->getTrackPointsVisible());
    invalidateView();
}

void MainWindow::on_viewTrackSegmentsAction_triggered()
{
    M_PREFS->setTrackSegmentsVisible(!M_PREFS->getTrackSegmentsVisible());
    if (M_PREFS->getTrackSegmentsVisible()) p->renderOptions.options |= RendererOptions::TrackSegmentVisible; else p->renderOptions.options &= ~RendererOptions::TrackSegmentVisible;
    ui->viewTrackSegmentsAction->setChecked(M_PREFS->getTrackSegmentsVisible());
    invalidateView();
}

void MainWindow::on_viewRelationsAction_triggered()
{
    M_PREFS->setRelationsVisible(!M_PREFS->getRelationsVisible());
    if (M_PREFS->getRelationsVisible()) p->renderOptions.options |= RendererOptions::RelationsVisible; else p->renderOptions.options &= ~RendererOptions::RelationsVisible;
    ui->viewRelationsAction->setChecked(M_PREFS->getRelationsVisible());
    invalidateView();
}

void MainWindow::on_viewGotoAction_triggered()
{
    GotoDialog* Dlg = new GotoDialog(theView, this);
    if (Dlg->exec() == QDialog::Accepted) {
        if (!Dlg->newViewport().isNull() && !Dlg->newViewport().isEmpty()) {
            theView->setViewport(Dlg->newViewport(), theView->rect());
            invalidateView();
        }
    }
    delete Dlg;
}

void MainWindow::on_viewArrowsNeverAction_triggered(bool checked)
{
    if (checked) {
        M_PREFS->setDirectionalArrowsVisible(RendererOptions::ArrowsNever);
        p->renderOptions.arrowOptions = RendererOptions::ArrowsNever;
        invalidateView();
    }
}

void MainWindow::on_viewArrowsOnewayAction_triggered(bool checked)
{
    if (checked) {
        M_PREFS->setDirectionalArrowsVisible(RendererOptions::ArrowsOneway);
        p->renderOptions.arrowOptions = RendererOptions::ArrowsOneway;
        invalidateView();
    }
}

void MainWindow::on_viewArrowsAlwaysAction_triggered(bool checked)
{
    if (checked) {
        M_PREFS->setDirectionalArrowsVisible(RendererOptions::ArrowsAlways);
        p->renderOptions.arrowOptions = RendererOptions::ArrowsAlways;
        invalidateView();
    }
}

void MainWindow::on_fileNewAction_triggered()
{
    theView->launch(0);
    p->theProperties->setSelection(0);

    if (theDocument)
        saveTemplateDocument(TEMPLATE_DOCUMENT);

    if (!theDocument || !hasUnsavedChanges() || mayDiscardUnsavedChanges(this)) {
        p->theFeats->invalidate();
        SAFE_DELETE(theDocument);
        theView->setDocument(NULL);
        p->latSaveDirtyLevel = 0;
        g_feat_rndId = 0;

        if (M_PREFS->getHasAutoLoadDocument())
            loadTemplateDocument(M_PREFS->getAutoLoadDocumentFilename());
        else if (!g_Merk_IgnoreStartupTemplate)
            loadTemplateDocument(TEMPLATE_DOCUMENT);

        if (!theDocument) {
            theDocument = new Document(theLayers);
            theDocument->addDefaultLayers();
            theView->projection().setProjectionType(M_PREFS->getProjectionType());
            theView->setViewport(WORLD_COORDBOX, theView->rect());
        }
        theView->setDocument(theDocument);
        theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
        connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
        connect (theDocument, SIGNAL(historyChanged()), this, SIGNAL(content_changed()));
        theDirty->updateList();

        fileName = "";
        setWindowTitle(QString("%1 - %2").arg(theDocument->title()).arg(p->title));

        updateProjectionMenu();

        emit content_changed();
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
    QList< QPair <QString, QString> > tags;
    int Sides = QInputDialog::getInteger(this, MainWindow::tr("Create Polygon"), MainWindow::tr("Specify the number of sides"), M_PREFS->getPolygonSides(), 3);
    M_PREFS->setPolygonSides(Sides);
    theView->launch(new CreatePolygonInteraction(this, theView, Sides, tags));
    theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createRectangleAction_triggered()
{
    QList< QPair <QString, QString> > tags;
    tags << qMakePair(QString("building"), QString("yes"));
    theView->launch(new CreatePolygonInteraction(this, theView, 4, tags));
    theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_createRoadAction_triggered()
{
    Node * firstPoint = NULL;

    if (p->theProperties->size() == 1)
    {
        Feature * feature = p->theProperties->selection(0);
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
    joinRoads(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        emit content_changed();
        invalidateView();
    }
}

void MainWindow::on_roadSplitAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Split Roads"), NULL);
    splitRoads(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        emit content_changed();
        invalidateView();
    }
}

void MainWindow::on_roadBreakAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Break Roads"), NULL);
    breakRoads(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        emit content_changed();
        invalidateView();
    }
}

void MainWindow::on_roadSimplifyAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Simplify Roads"), NULL);
    qreal threshold = 3.0; // in metres; TODO: allow user-specified threshold
    simplifyRoads(theDocument, theList, p->theProperties, threshold);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        emit content_changed();
        invalidateView();
    }
}

void MainWindow::on_featureSelectChildrenAction_triggered()
{
    QList<Feature*> theFeatures;
    foreach (Feature* F, p->theProperties->selection()) {
        theFeatures << F;
        for (int i=0; i<F->size(); ++i)
            theFeatures << F->get(i);
    }
    p->theProperties->setSelection(theFeatures);
    p->theProperties->checkMenuStatus();
    invalidateView();
}

void MainWindow::on_featureDeleteAction_triggered()
{
    Feature* F = p->theProperties->selection(0);
    if (!F)
        return;

    while (F->sizeParents()) {
        Feature* p = (Feature*)(F->getParent(0));
        if (p)
            p->remove(F);
    }
    F->layer()->deleteFeature(F);
    p->theProperties->setSelection(0);

    emit content_changed();
    invalidateView();
}

void MainWindow::on_featureCommitAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Force Feature upload"), NULL);
    commitFeatures(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_featureOsbClose_triggered()
{
    Feature* bugNd = p->theProperties->selection(0);

    QUrl osbUrl;
    osbUrl.setUrl(M_PREFS->getOpenStreetBugsUrl());
    osbUrl.setPath(osbUrl.path() + "closePOIexec");
    osbUrl.addQueryItem("id", Feature::stripToOSMId(bugNd->id()));
    qDebug() << osbUrl.toString();

    QString rply;
    bool ret = Utils::sendBlockingNetRequest(osbUrl, rply);

    if (!ret) {
        QMessageBox::warning(0, tr("Network timeout"), tr("Cannot contact OpenStreetBugs."), QMessageBox::Ok);
        return;
    }

    qDebug() << "openStreetBugs reply: " << rply;
    if (rply.contains("ok")) {
        bugNd->layer()->deleteFeature(bugNd);
        p->theProperties->setSelection(0);
        invalidateView();
    } else
        QMessageBox::warning(this, tr("Error closing bug"), tr("Cannot delete bug. Server message is:\n%1").arg(rply), QMessageBox::Ok);

    return;
}

void MainWindow::on_roadCreateJunctionAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Create Junction"), NULL);
    int n = createJunction(theDocument, theList, p->theProperties, false);
    if (n > 1) {
        MDiscardableMessage dlg(view(),
            MainWindow::tr("Multiple intersection."),
            MainWindow::tr("Those roads have multiple intersections.\nDo you still want to create a junction for each one (Unwanted junctions can still be deleted afterhand)?"));
        if (dlg.check() != QDialog::Accepted)
            return;
    }
    createJunction(theDocument, theList, p->theProperties, true);

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

    addStreetNumbers(theDocument, theList, p->theProperties);

    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_roadSubdivideAction_triggered()
{
#if QT_VERSION < 0x040500
    {
        int divisions = QInputDialog::getInteger(this, MainWindow::tr("Number of segments to divide into"), MainWindow::tr("Specify the number of segments"), 2, 99);
#else
    QInputDialog *Dlg = new QInputDialog(this);
    Dlg->setInputMode(QInputDialog::IntInput);
    Dlg->setIntRange(2, 99);
    Dlg->setLabelText(tr("Number of segments to divide into"));
    if (Dlg->exec() == QDialog::Accepted) {
        int divisions = Dlg->intValue();
#endif
        CommandList* theList = new CommandList(MainWindow::tr("Subdivide road into %1").arg(divisions), NULL);
        subdivideRoad(theDocument, theList, p->theProperties, divisions);
        if (theList->empty())
            delete theList;
        else {
            theDocument->addHistory(theList);
            invalidateView();
        }
    }
#if QT_VERSION > 0x040499
    delete Dlg;
#endif
}

void MainWindow::on_roadAxisAlignAction_triggered()
{
    const unsigned int max_axes = 16;
    bool ok;
    unsigned int axes;

    axes = axisAlignGuessAxes(p->theProperties, view()->projection(), max_axes);
    if (!axes)
        axes = 4;
    axes = QInputDialog::getInteger(this, tr("Axis Align"),
                                    tr("Specify the number of regular axes to align edges on (e.g. 4 for rectangular)"),
                                    axes, 3, max_axes, 1, &ok);
    if (!ok)
        return;

    // Create a command description
    const QString special_names[] = {
        tr("triangular"),
        tr("rectangular"),
        tr("pentagonal"),
        tr("hexagonal"),
        tr("heptagonal"),
        tr("octagonal"),
    };
    QString command_name;
    if (axes < 3 + (sizeof(special_names)/sizeof(special_names[0])))
        command_name = tr("Align onto %1 axes").arg(special_names[axes-3]);
    else
        command_name = tr("Align onto %1 regular axes").arg(axes);

    // Do the manipulation
    CommandList* theList = new CommandList(command_name, NULL);
    AxisAlignResult result = axisAlignRoads(theDocument, theList, p->theProperties, view()->projection(), axes);
    if (result != AxisAlignSuccess || theList->empty()) {
        if (result == AxisAlignSharpAngles)
            QMessageBox::critical(this, tr("Unable to align to axes"),
                                  tr("Align to axes operation failed. Please adjust any sharp corners and try again."));
        else if (result == AxisAlignFail)
            QMessageBox::critical(this, tr("Unable to align to axes"),
                                  tr("Align to axes operation failed and did not converge on a solution."));
        delete theList;
    } else {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_roadExtrudeAction_triggered()
{
    theView->launch(new ExtrudeInteraction(theView));
    theInfo->setHtml(theView->interaction()->toHtml());
}

void MainWindow::on_roadBingExtractAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Bing Extract"), NULL);
    bingExtract(theDocument, theList, p->theProperties, theView->viewport());
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
    //	theView->properties()->setSelection(F);
        invalidateView();
    }
}

void MainWindow::on_nodeAlignAction_triggered()
{
    //MapFeature* F = theView->properties()->selection(0);
    CommandList* theList = new CommandList(MainWindow::tr("Align Nodes"), NULL);
    alignNodes(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
    //	theView->properties()->setSelection(F);
        invalidateView();
    }
}

void MainWindow::on_nodeSpreadAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Spread Nodes"), NULL);
    spreadNodes(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_nodeMergeAction_triggered()
{
    Feature* F = p->theProperties->selection(0);
    CommandList* theList = new CommandList(MainWindow::tr("Merge Nodes into %1").arg(F->id().numId), F);
    mergeNodes(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        p->theProperties->setSelection(F);
        invalidateView();
    }
}

void MainWindow::on_nodeDetachAction_triggered()
{
    Feature* F = p->theProperties->selection(0);
    CommandList* theList = new CommandList(MainWindow::tr("Detach Node %1").arg(F->id().numId), F);
    detachNode(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else
    {
        theDocument->addHistory(theList);
        p->theProperties->setSelection(F);
        invalidateView();
    }
}

void MainWindow::on_relationAddMemberAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Add member to relation"), NULL);
    addRelationMember(theDocument, theList, p->theProperties);
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
    removeRelationMember(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_relationAddToMultipolygonAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Add to Multipolygon"), NULL);
    addToMultipolygon(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_areaJoinAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Join areas"), NULL);
    joinAreas(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_areaSplitAction_triggered()
{
    CommandList* theList = new CommandList(MainWindow::tr("Split area"), NULL);
    splitArea(theDocument, theList, p->theProperties);
    if (theList->empty())
        delete theList;
    else {
        theDocument->addHistory(theList);
        invalidateView();
    }
}

void MainWindow::on_areaTerraceAction_triggered()
{
    TerraceDialog* Dlg = new TerraceDialog(this);
    if (Dlg->exec() == QDialog::Accepted) {
        int divisions = Dlg->numHouses();
        CommandList* theList = new CommandList(MainWindow::tr("Terrace area into %1").arg(divisions), NULL);
        terraceArea(theDocument, theList, p->theProperties, divisions);
        // Add the house numbers to the houses in the selection
        if (Dlg->hasHouseNumbers()) {
            QStringList numbers = Dlg->houseNumbers();
            QList<Feature*> areas = p->theProperties->selection();
            int i = 0;
            foreach (Feature* area, areas) {
                if (i >= numbers.size())
                    break;
                if (!numbers[i].isEmpty())
                    theList->add(new SetTagCommand(area, "addr:housenumber", numbers[i]));
                ++i;
            }
        }
        if (theList->empty())
            delete theList;
        else {
            theDocument->addHistory(theList);
            invalidateView();
        }
    }
    delete Dlg;
}

void MainWindow::on_createRelationAction_triggered()
{
    Relation* R = g_backend.allocRelation(document()->getDirtyOrOriginLayer());
    CommandList* theList = new CommandList(MainWindow::tr("Create Relation %1").arg(R->description()), R);
    theList->add(
        new AddFeatureCommand(document()->getDirtyOrOriginLayer(), R, true));
    for (int i = 0; i < p->theProperties->size(); ++i)
        theList->add(new RelationAddFeatureCommand(R, "", p->theProperties->selection(i)));
    theDocument->addHistory(theList);
    p->theProperties->setSelection(R);
    invalidateView();
}

void MainWindow::on_editMapStyleAction_triggered()
{
    PaintStyleEditor* dlg = new PaintStyleEditor(this, M_STYLE->getGlobalPainter(), M_STYLE->getPainters());
    connect(dlg, SIGNAL(stylesApplied(GlobalPainter*, QList<Painter>* )), this, SLOT(applyPainters(GlobalPainter*, QList<Painter>* )));
    GlobalPainter saveGlobalPainter = M_STYLE->getGlobalPainter();
    QList<Painter> savePainters = M_STYLE->getPainters();
    if (dlg->exec() == QDialog::Accepted) {
        M_STYLE->setGlobalPainter(dlg->theGlobalPainter);
        M_STYLE->setPainters(dlg->thePainters);
    } else {
        M_STYLE->setGlobalPainter(saveGlobalPainter);
        M_STYLE->setPainters(savePainters);
    }

    theDocument->setPainters(dlg->thePainters);
    for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
        i.get()->invalidatePainter();
    invalidateView();
    delete dlg;
}

void MainWindow::applyStyles(QString NewStyle)
{
    if (NewStyle != M_PREFS->getDefaultStyle())
    {
        if (M_STYLE->isDirty() && !mayDiscardStyleChanges(this)) {
            on_mapStyleSaveAction_triggered();
        }
        M_PREFS->setDefaultStyle(NewStyle);
        M_STYLE->loadPainters(M_PREFS->getDefaultStyle());
        document()->setPainters(M_STYLE->getPainters());
        for (FeatureIterator it(document()); !it.isEnd(); ++it)
        {
            it.get()->invalidatePainter();
        }
        invalidateView(false);
    }
}

void MainWindow::applyPainters(GlobalPainter* theGlobalPainter, QList<Painter>* thePainters)
{
    M_STYLE->setGlobalPainter(*theGlobalPainter);
    M_STYLE->setPainters(*thePainters);

    theDocument->setPainters(*thePainters);
    for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
        i.get()->invalidatePainter();
    invalidateView(false);
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
    QString f = M_STYLE->getFilename();
    if (f.isEmpty() || f.startsWith(":") || f.startsWith("qrc:")) {
        on_mapStyleSaveAsAction_triggered();
        return;
    }
    M_STYLE->savePainters(f);
}

void MainWindow::on_mapStyleSaveAsAction_triggered()
{
    QString f;
    QFileDialog dlg(this, tr("Save map style"), M_PREFS->getCustomStyle(), tr("Merkaartor map style (*.mas)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("mas");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            f = dlg.selectedFiles()[0];
    }
//    f = QFileDialog::getSaveFileName(this, tr("Save map style"), M_PREFS->getCustomStyle(), tr("Merkaartor map style (*.mas)"));
    if (!f.isNull()) {
        M_STYLE->savePainters(f);
    }
    updateStyleMenu();
}

void MainWindow::on_mapStyleLoadAction_triggered()
{
    if (M_STYLE->isDirty() && !mayDiscardStyleChanges(this)) {
        on_mapStyleSaveAction_triggered();
    }

    QString f = QFileDialog::getOpenFileName(this, tr("Load map style"), QString(),
                                             tr("Supported formats")+" (*.mas *.css)\n" \
                                             + tr("Merkaartor map style (*.mas)\n")
                                             + tr("MapCSS stylesheet (*.css)"));
    if (!f.isNull()) {
        if (f.endsWith("css"))
            MapCSSPaintstyle::instance()->loadPainters(f);
        else {
            M_STYLE->loadPainters(f);
            document()->setPainters(M_STYLE->getPainters());
            for (VisibleFeatureIterator i(theDocument); !i.isEnd(); ++i)
                i.get()->invalidatePainter();
            invalidateView();
        }
    }
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
    ProjPreferencesDialog* prefDlg = new ProjPreferencesDialog();
    if (prefDlg->exec() == QDialog::Accepted) {
        updateProjectionMenu();
    }
}

void MainWindow::on_toolsFiltersAction_triggered()
{
    FilterPreferencesDialog* prefDlg = new FilterPreferencesDialog();
    prefDlg->exec();
//    if (prefDlg->exec() == QDialog::Accepted) {
//        updateFilterMenu();
//    }
}

void MainWindow::on_toolsResetDiscardableAction_triggered()
{
    QSettings* Sets = M_PREFS->getQSettings();
    Sets->remove("DiscardableDialogs");
}

void MainWindow::on_toolsRebuildHistoryAction_triggered()
{
    QMessageBox::StandardButton ret = QMessageBox::warning(this, tr("Rebuild History"), tr("An attempt will be mode to rebuild the history.\nNo guarantee, though, and no Undo.\nAre you sure you want to try this? ")
                                                         , QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
    if (ret == QMessageBox::Yes) {
        theDocument->rebuildHistory();
        theDirty->updateList();
    }
}

void MainWindow::on_toolsShortcutsAction_triggered()
{
    QList<QAction*> theActions;

    foreach(QAction* a, ui->menuFile->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuEdit->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuView->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menu_Show->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuShow_directional_Arrows->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuGps->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuLayers->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuCreate->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menu_Feature->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuOpenStreetBugs->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menu_Node->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuRoad->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuRelation->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuTools->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuWindow->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }
    foreach(QAction* a, ui->menuHelp->actions()) {
        if (!a->isSeparator() && !a->menu())
            theActions << a;
    }

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
    connect (Pref, SIGNAL(preferencesChanged(PreferencesDialog*)), this, SLOT(preferencesChanged(PreferencesDialog*)));
    Pref->exec();
    p->lastPrefTabIndex = Pref->tabPref->currentIndex();
}

void MainWindow::preferencesChanged(PreferencesDialog* prefs)
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
            QApplication::setStyle(QStyleFactory::create(M_PREFS->getMerkaartorStyleString()));
        }
    }
    ui->mnuProjections->menuAction()->setEnabled(true);
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
    if (M_PREFS->getLocalServer()) {
        if (!p->theListeningServer) {
            p->theListeningServer = new QTcpServer(this);
            connect(p->theListeningServer, SIGNAL(newConnection()), this, SLOT(incomingLocalConnection()));
            if (!p->theListeningServer->listen(QHostAddress::LocalHost, 8111))
                qDebug() << "Remote control: Unable to listen on 8111";
        }
    } else {
        if (p->theListeningServer) {
            delete p->theListeningServer;
            p->theListeningServer = NULL;
        }
    }

    applyStyles(prefs->cbStyles->itemData(prefs->cbStyles->currentIndex()).toString());
    updateStyleMenu();

    updateMenu();
    theView->launch(new EditInteraction(theView));
    invalidateView(false);
}

void MainWindow::on_fileSaveAsAction_triggered()
{
    QFileDialog dlg(this, tr("Save Merkaartor document"), QString("%1/%2.mdc").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("mdc");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            fileName = dlg.selectedFiles()[0];
    }

//    fileName = QFileDialog::getSaveFileName(this,
//        tr("Save Merkaartor document"), QString("%1/%2.mdc").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)"));

    if (!fileName.isEmpty()) {
        saveDocument(fileName);
        M_PREFS->addRecentOpen(fileName);
        updateRecentOpenMenu();
    }
}

void MainWindow::on_fileSaveAsTemplateAction_triggered()
{
    QString tfileName;

    QFileDialog dlg(this, tr("Save Merkaartor template document"), QString("%1/%2.mdc").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("mdc");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            tfileName = dlg.selectedFiles()[0];
    }
//    QString tfileName = QFileDialog::getSaveFileName(this,
//        tr("Save Merkaartor template document"), QString("%1/%2.mdc").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor documents Files (*.mdc)"));

    if (!tfileName.isEmpty()) {
        saveTemplateDocument(tfileName);
    }
}

void MainWindow::on_fileSaveAction_triggered()
{
    if (fileName != "") {
        saveDocument(fileName);
    } else {
        on_fileSaveAsAction_triggered();
    }
}

void MainWindow::doSaveDocument(QFile* file, bool asTemplate)
{

#ifndef Q_OS_SYMBIAN
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif


    QXmlStreamWriter stream(file);
    stream.setAutoFormatting(true);
    stream.setAutoFormattingIndent(2);
    stream.writeStartDocument();
    stream.writeStartElement("MerkaartorDocument");
    stream.writeAttribute("version", "1.2");
    stream.writeAttribute("creator", QString("%1").arg(p->title));

    QProgressDialog progress("Saving document...", "Cancel", 0, 0);
    progress.setWindowModality(Qt::WindowModal);

    theDocument->toXML(stream, asTemplate, &progress);
    theView->toXML(stream);

    stream.writeEndDocument();

    progress.setValue(progress.maximum());

    theDocument->setTitle(QFileInfo(fileName).fileName());
    setWindowTitle(QString("%1 - %2").arg(theDocument->title()).arg(p->title));

#ifndef Q_OS_SYMBIAN
    QApplication::restoreOverrideCursor();
#endif
}

void MainWindow::saveDocument(const QString& fn)
{
    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Unable to open save file"), tr("%1 could not be opened for writing.").arg(fn));
        on_fileSaveAsAction_triggered();
        return;
    }

    doSaveDocument(&file);
    file.close();

    p->latSaveDirtyLevel = theDocument->getDirtySize();
}

void MainWindow::saveTemplateDocument(const QString& fn)
{
    QFile file(fn);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Unable to open save template document"), tr("%1 could not be opened for writing.").arg(fn));
        return;
    }

    doSaveDocument(&file, true);
    file.close();
}

Document* MainWindow::doLoadDocument(QFile* file)
{
    QProgressDialog progress("Loading document...", "Cancel", 0, 0, this);
    progress.setWindowModality(Qt::WindowModal);

    QXmlStreamReader stream(file);
    while (stream.readNext() && stream.tokenType() != QXmlStreamReader::Invalid && stream.tokenType() != QXmlStreamReader::StartElement);
    if (stream.tokenType() != QXmlStreamReader::StartElement || stream.name() != "MerkaartorDocument") {
        QMessageBox::critical(this, tr("Invalid file"), tr("%1 is not a valid Merkaartor document.").arg(file->fileName()));
        return NULL;
    }
    double version = stream.attributes().value("version").toString().toDouble();

    progress.setMaximum(file->size());

    Document* newDoc = NULL;

    if (version < 2.) {
        stream.readNext();
        while(!stream.atEnd() && !stream.isEndElement()) {
            if (stream.name() == "MapDocument") {
                newDoc = Document::fromXML(QFileInfo(*file).fileName(), stream, version, theLayers, &progress);

                if (progress.wasCanceled())
                    break;
            } else if (stream.name() == "MapView") {
                view()->fromXML(stream);
            } else if (!stream.isWhitespace()) {
                qDebug() << "Main: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                stream.skipCurrentElement();
            }

            if (progress.wasCanceled())
                break;

            stream.readNext();
        }
    }
    progress.reset();

    updateProjectionMenu();

#ifdef GEOIMAGE
    if (theGeoImage)
        theGeoImage->clear();
#endif
    return newDoc;
}

void MainWindow::loadDocument(QString fn)
{
    QFile file(fn);
    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(this, tr("Invalid file"), tr("%1 could not be opened.").arg(fn));
        return;
    }

    Document* newDoc = doLoadDocument(&file);
    file.close();

    if (newDoc) {
        p->theProperties->setSelection(0);
        p->theFeats->invalidate();
        delete theDocument;
        theDocument = newDoc;
        theView->setDocument(theDocument);
        on_editPropertiesAction_triggered();
        theDocument->history().setActions(ui->editUndoAction, ui->editRedoAction, ui->fileUploadAction);
        connect (theDocument, SIGNAL(historyChanged()), theDirty, SLOT(updateList()));
        connect (theDocument, SIGNAL(historyChanged()), this, SIGNAL(content_changed()));
        theDirty->updateList();
        fileName = fn;
        setWindowTitle(QString("%1 - %2").arg(theDocument->title()).arg(p->title));
        p->latSaveDirtyLevel = theDocument->getDirtySize();
    }

    M_PREFS->addRecentOpen(fn);
    updateRecentOpenMenu();

    emit content_changed();
}

void MainWindow::loadTemplateDocument(QString fn)
{
    Document* newDoc = NULL;
    QFile file(fn);
    if (file.open(QIODevice::ReadOnly)) {
        newDoc = doLoadDocument(&file);
        file.close();
    }

    if (newDoc) {
        theDocument = newDoc;
        theDocument->setTitle(tr("untitled"));
    }
}

void MainWindow::on_exportOSMAction_triggered()
{
    QList<Feature*> theFeatures;

    createProgressDialog();
    if (!selectExportedFeatures(theFeatures))
        return;

    QString fileName;
    QFileDialog dlg(this, tr("Export OSM"), QString("%1/%2.osm").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("OSM Files (*.osm)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("osm");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            fileName = dlg.selectedFiles()[0];
    }
//    QString fileName = QFileDialog::getSaveFileName(this,
//        tr("Export OSM"), M_PREFS->getworkingdir() + "/untitled.osm", tr("OSM Files (*.osm)"));

    if (fileName != "") {

        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;

        theDocument->exportOSM(this, &file, theFeatures);
        file.close();

    }
    deleteProgressDialog();
}

void MainWindow::on_exportOSCAction_triggered()
{
#ifndef FRISIUS_BUILD
    QString fileName;
    QFileDialog dlg(this, tr("Export osmChange"), QString("%1/%2.osc").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("osmChange Files (*.osc)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("osc");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            fileName = dlg.selectedFiles()[0];
    }
//    QString fileName = QFileDialog::getSaveFileName(this,
//        tr("Export osmChange"), M_PREFS->getworkingdir() + "/untitled.osc", tr("osmChange Files (*.osc)"));

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
#endif
}


void MainWindow::on_exportGPXAction_triggered()
{
    QList<Feature*> theFeatures;

    createProgressDialog();
    if (!selectExportedFeatures(theFeatures))
        return;

    QString fileName;
    QFileDialog dlg(this, tr("Export GPX"), QString("%1/%2.gpx").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("GPX Files (*.gpx)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("gpx");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            fileName = dlg.selectedFiles()[0];
    }
//    QString fileName = QFileDialog::getSaveFileName(this,
//        tr("Export GPX"), M_PREFS->getworkingdir() + "/untitled.gpx", tr("GPX Files (*.gpx)"));

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
    deleteProgressDialog();
}

void MainWindow::on_exportGDALAction_triggered()
{
    QList<Feature*> theFeatures;

    createProgressDialog();
    if (!selectExportedFeatures(theFeatures))
        return;
#ifndef Q_OS_SYMBIAN
    QApplication::setOverrideCursor(Qt::BusyCursor);
#endif

    ImportExportGdal gdal(document());
    gdal.export_(theFeatures);

#ifndef Q_OS_SYMBIAN
    QApplication::restoreOverrideCursor();
#endif

    deleteProgressDialog();
}

void MainWindow::on_exportKMLAction_triggered()
{
    QList<Feature*> theFeatures;

    createProgressDialog();
    if (!selectExportedFeatures(theFeatures))
        return;

    QString fileName;
    QFileDialog dlg(this, tr("Export KML"), QString("%1/%2.kml").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("KML Files (*.kml)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("kml");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            fileName = dlg.selectedFiles()[0];
    }
//    QString fileName = QFileDialog::getSaveFileName(this,
//        tr("Export KML"), M_PREFS->getworkingdir() + "/untitled.kml", tr("KML Files (*.kml)"));

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
    deleteProgressDialog();
}

bool MainWindow::selectExportedFeatures(QList<Feature*>& theFeatures)
{
    QDialog dlg(this);
    Ui::ExportDialog dlgExport;
    dlgExport.setupUi(&dlg);
    switch(M_PREFS->getExportType()) {
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
            for (VisibleFeatureIterator i(document()); !i.isEnd(); ++i) {
                if (i.get()->notEverythingDownloaded())
                    continue;

                theFeatures.append(i.get());
            }
            M_PREFS->setExportType(Export_All);
            return true;
        }
        else if (dlgExport.rbViewport->isChecked()) {
            CoordBox aCoordBox = view()->viewport();

            theFeatures.clear();
            for (VisibleFeatureIterator i(document()); !i.isEnd(); ++i) {
                if (i.get()->notEverythingDownloaded())
                    continue;

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
            M_PREFS->setExportType(Export_Viewport);
        }
        else if (dlgExport.rbSelected->isChecked()) {
            theFeatures = p->theProperties->selection();
            M_PREFS->setExportType(Export_Selected);
        }

        QProgressDialog* dlg = getProgressDialog();
        if (dlg)
            dlg->setWindowTitle(tr("Feature extraction"));

        QProgressBar* Bar = getProgressBar();
        if (Bar) {
            Bar->setTextVisible(false);
            Bar->setMaximum(theFeatures.size());
        }

        QLabel* Lbl = getProgressLabel();
        if (Lbl)
            Lbl->setText(tr("Extracting features..."));

        if (dlg)
            dlg->show();

        theFeatures = document()->exportCoreOSM(theFeatures, false, dlg);
        return true;
    }
    return false;
}

void MainWindow::on_editSelectAction_triggered()
{
    SelectionDialog* Sel = new SelectionDialog(this);

    if (Sel->exec() == QDialog::Accepted) {
        QString out;
        int idx = 0;
        QString in = Sel->edTagQuery->text();
        QList<TagSelector*> terms;
        while (idx < in.length()) {
            TagSelector* t = TagSelector::parse(in, idx);
            if (!t) break;
            terms.append(t);
        }

        if (terms.count()) {
            out += terms[terms.count()-1]->asExpression(true);
            for (int i=terms.count()-2; i>=0; --i) {
                out += " and parent(";
                out += terms[i]->asExpression(true);
                out += ") ";
            }
        } else
            return;

        qDebug() << out;
        TagSelector* tsel = TagSelector::parse(out);
        if (!tsel)
            return;
        qDebug() << tsel->asExpression(false);

        int selMaxResult = Sel->sbMaxResult->value();

        QList <Feature *> selection;
        int added = 0;
        for (VisibleFeatureIterator i(theDocument); !i.isEnd() && (!selMaxResult || added < selMaxResult); ++i) {
            Feature* F = i.get();
            if (tsel->matches(F, theView->pixelPerM())) {
                selection.push_back(F);
                ++added;
            }
        }
        p->theProperties->setMultiSelection(selection);
        view()->properties()->checkMenuStatus();
    }
}

void MainWindow::closeEvent(QCloseEvent * event)
{
    if (hasUnsavedChanges() && !mayDiscardUnsavedChanges(this)) {
        event->ignore();
        return;
    }

    if (M_STYLE->isDirty() && !mayDiscardStyleChanges(this)) {
        on_mapStyleSaveAction_triggered();
    }

    M_PREFS->saveMainWindowState( this );
//    M_PREFS->setInitialPosition(theView);
    M_PREFS->setworkingdir(QDir::currentPath());

    saveTemplateDocument(TEMPLATE_DOCUMENT);
    M_PREFS->save();
    QMainWindow::closeEvent(event);
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
    bool projFound = false;
    foreach (ProjectionItem it, *M_PREFS->getProjectionsList()->getProjections()) {
        if (it.deleted)
            continue;
        QAction* a = new QAction(it.name, p->projActgrp);
        a->setCheckable (true);
        if (it.name.contains(theView->projection().getProjectionType(), Qt::CaseInsensitive)) {
            a->setChecked(true);
            projFound = true;
        }
        ui->mnuProjections->addAction(a);
    }
    if (!projFound) {
        QAction* a = new QAction(theView->projection().getProjectionType(), p->projActgrp);
        a->setCheckable (true);
        a->setChecked(true);
        ui->mnuProjections->addAction(a);
        M_PREFS->getProjectionsList()->addProjection(ProjectionItem(theView->projection().getProjectionType(), theView->projection().getProjectionProj4()));
    }
    connect (ui->mnuProjections, SIGNAL(triggered(QAction *)), this, SLOT(projectionTriggered(QAction *)));
#endif
    ui->mnuProjections->menuAction()->setEnabled(true);
    if (M_PREFS->getZoomBoris() && theDocument) {
        ImageMapLayer* l = NULL;
        for (LayerIterator<ImageMapLayer*> ImgIt(theDocument); !ImgIt.isEnd(); ++ImgIt) {
            l = ImgIt.get();
            break;
        }
        if (l && l->isTiled())
            ui->mnuProjections->menuAction()->setEnabled(false);
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
    ui->windowPropertiesAction->setChecked(p->theProperties->isVisible());
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
    if (hasUnsavedChanges() && !mayDiscardUnsavedChanges(this))
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
void MainWindow::projectionSet(const QString& prj)
{
    if(false == theView->projection().setProjectionType(prj))
        QMessageBox::critical(this, tr("Invalid projection"), tr("Unable to set projection \"%1\".").arg(prj));
    updateProjectionMenu();
    theView->setViewport(theView->viewport(), theView->rect());
    invalidateView();
}

void MainWindow::projectionTriggered(QAction* anAction)
{
    if(false == theView->projection().setProjectionType(anAction->text()))
        QMessageBox::critical(this, tr("Invalid projection"), tr("Unable to set projection \"%1\".").arg(anAction->text()));
    else
        M_PREFS->setProjectionType(anAction->text());
    theView->setViewport(theView->viewport(), theView->rect());
    invalidateView();
}
#endif

void MainWindow::styleTriggered(QAction* anAction)
{
    if (!anAction->isCheckable())
        return;

    QString NewStyle = anAction->data().toString();
    p->theStyle->setCurrent(anAction);
    applyStyles(NewStyle);
}

void MainWindow::on_windowPropertiesAction_triggered()
{
    p->theProperties->setVisible(!p->theProperties->isVisible());
    ui->windowPropertiesAction->setChecked(p->theProperties->isVisible());
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
    foreach (QObject* child, children()) {
        if (QToolBar* tb = qobject_cast<QToolBar*>(child))
            tb->setVisible(!tb->isVisible());
    }
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
    p->theProperties->setVisible(false);
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
        connect(aGps, SIGNAL(updatePosition(qreal, qreal, QDateTime, qreal, qreal, qreal)),
            this, SLOT(updateGpsPosition(qreal, qreal, QDateTime, qreal, qreal, qreal)));

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

void MainWindow::updateGpsPosition(qreal latitude, qreal longitude, QDateTime time, qreal altitude, qreal speed, qreal heading)
{
    Q_UNUSED(heading)
    if (theGPS->getGpsDevice()) {
        Coord gpsCoord(longitude,latitude);
        if (M_PREFS->getGpsMapCenter()) {
            CoordBox vp = theView->viewport();
            qreal lonDiff = vp.lonDiff();
            qreal latDiff = vp.latDiff();
            QRectF vpr = vp.adjusted(lonDiff / 4, -latDiff / 4, -lonDiff / 4, latDiff / 4);
            if (!vpr.contains(gpsCoord)) {
                theView->setCenter(gpsCoord, theView->rect());
                theView->invalidate(true, true);
            }
        }

        if (ui->gpsRecordAction->isChecked() && !ui->gpsPauseAction->isChecked()) {
            TrackNode* pt = g_backend.allocTrackNode(gpsRecLayer, gpsCoord);
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

            curGpsTrackSegment = g_backend.allocSegment(gpsRecLayer);
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
            curGpsTrackSegment = g_backend.allocSegment(gpsRecLayer);
            gpsRecLayer->add(curGpsTrackSegment);
        }
    }
}

void MainWindow::on_toolTemplatesSaveAction_triggered()
{
    QString f;
    QFileDialog dlg(this, tr("Save Tag Templates"), QString("%1/%2.mat").arg(M_PREFS->getworkingdir()).arg(tr("untitled")), tr("Merkaartor tag templates (*.mat)") + "\n" + tr("All Files (*)"));
    dlg.setFileMode(QFileDialog::AnyFile);
    dlg.setDefaultSuffix("mat");
    dlg.setAcceptMode(QFileDialog::AcceptSave);

    if (dlg.exec()) {
        if (dlg.selectedFiles().size())
            f = dlg.selectedFiles()[0];
    }
//    QString f = QFileDialog::getSaveFileName(this, tr("Save Tag Templates"), "", tr("Merkaartor tag templates (*.mat)"));
    if (!f.isNull()) {
        p->theProperties->saveTemplates(f);
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

    p->theProperties->mergeTemplates(fileName);
    p->theProperties->resetValues();
}

void MainWindow::on_toolTemplatesLoadAction_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(
                    this,
                    tr("Open Tag Templates"),
                    "", "Merkaartor tag templates (*.mat)" );

    if (fileName.isEmpty())
        return;

    p->theProperties->loadTemplates(fileName);
    p->theProperties->resetValues();
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
#if defined(Q_OS_MAC)
        QDir resources = QDir(QCoreApplication::applicationDirPath());
        resources.cdUp();
        resources.cd("Resources");
#endif
        qtTranslator = new QTranslator;
        bool retQt;
        if (g_Merk_Portable) {
            retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), QCoreApplication::applicationDirPath() + "/translations");
        } else {
#ifdef TRANSDIR_SYSTEM
            if (!QDir::isAbsolutePath(STRINGIFY(TRANSDIR_SYSTEM)))
                retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), QCoreApplication::applicationDirPath() + "/" + STRINGIFY(TRANSDIR_SYSTEM));
            else
                retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), STRINGIFY(TRANSDIR_SYSTEM));
#else
#if defined(Q_OS_MAC)
            retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), resources.absolutePath());
#else
            retQt = qtTranslator->load("qt_" + DefaultLanguage.left(2), QLibraryInfo::location(QLibraryInfo::TranslationsPath));
#endif // Q_OS_MAC
#endif // TRANSDIR_SYSTEM
        }

        if (retQt)
            QCoreApplication::installTranslator(qtTranslator);

        merkaartorTranslator = new QTranslator;
        // Do not prevent Merkaartor translations to be loaded, even if there is no Qt translation for the language.

        bool retM;
        if (g_Merk_Portable) {
            retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, QCoreApplication::applicationDirPath() + "/translations");
        } else {
#if defined(Q_OS_MAC)
            retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, resources.absolutePath());
#else
            retM = merkaartorTranslator->load("merkaartor_" + DefaultLanguage, QCoreApplication::applicationDirPath());
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
        }

        if (retM)
            QCoreApplication::installTranslator(merkaartorTranslator);
        else
            statusBar()->showMessage(tr("Warning! Could not load the Merkaartor translations for the \"%1\" language. Switching to default English.").arg(DefaultLanguage), 15000);
    }
    ui->retranslateUi(this);
}

void MainWindow::mapView_interactionChanged(Interaction* anInteraction)
{
    ui->editPropertiesAction->setChecked(dynamic_cast<EditInteraction*>(anInteraction) != NULL);
    ui->editMoveAction->setChecked(dynamic_cast<MoveNodeInteraction*>(anInteraction) != NULL);
    ui->editRotateAction->setChecked(dynamic_cast<RotateInteraction*>(anInteraction) != NULL);
    ui->editScaleAction->setChecked(dynamic_cast<ScaleInteraction*>(anInteraction) != NULL);
    ui->createNodeAction->setChecked(dynamic_cast<CreateNodeInteraction*>(anInteraction) != NULL);
    ui->createRoadAction->setChecked(dynamic_cast<CreateSingleWayInteraction*>(anInteraction) != NULL);
    ui->createAreaAction->setChecked(dynamic_cast<CreateAreaInteraction*>(anInteraction) != NULL);
    ui->roadExtrudeAction->setChecked(dynamic_cast<ExtrudeInteraction*>(anInteraction) != NULL);
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

void MainWindow::on_layersNewDrawingAction_triggered()
{
    if (theDocument)
        theDocument->addDrawingLayer();
}

void MainWindow::on_layersNewFilterAction_triggered()
{
    if (theDocument)
        theDocument->addFilterLayer();
}

bool MainWindow::hasUnsavedChanges()
{
    if (!theDocument)
        return false;

    if (theDocument->getDirtySize() == p->latSaveDirtyLevel)
        return false;

    return true;
}

void MainWindow::syncOSM(const QString& aWeb, const QString& aUser, const QString& aPwd)
{
#ifndef FRISIUS_BUILD
    if (checkForConflicts(theDocument))
    {
        QMessageBox::warning(this,tr("Unresolved conflicts"), tr("Please resolve existing conflicts first"));
        return;
    }

    bool ok;
    DirtyListBuild Future;
    theDocument->history().buildDirtyList(Future);
    DirtyListDescriber Describer(theDocument,Future);
    if (Describer.showChanges(this) && Describer.tasks())
    {
        Future.resetUpdates();
        DirtyListExecutorOSC Exec(theDocument,Future,aWeb,aUser,aPwd,Describer.tasks());
        ok = Exec.executeChanges(this);
        if (ok) {
            if (M_PREFS->getAutoHistoryCleanup() && !theDocument->getDirtyOrOriginLayer()->getDirtySize())
                theDocument->history().cleanup();

            p->latSaveDirtyLevel = theDocument->getDirtySize();

            if (!fileName.isEmpty()) {
                if (M_PREFS->getAutoSaveDoc()) {
                    saveDocument(fileName);
                } else {
                    if (QMessageBox::warning(this,tr("Unsaved changes"),
                                             tr("It is strongly recommended to save the changes to your document after an upload.\nDo you want to do this now?"),
                                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes) {
                        saveDocument(fileName);

                    }
                }
            }
        }
    }
#endif
}
