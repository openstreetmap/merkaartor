
#include "MainWindow.h"

#include <QtCore>
#include <QtWidgets>

#include "MapView.h"
#include "Document.h"
#include "ImageMapLayer.h"
#include "EditInteraction.h"

#include "HomeView.h"
#include "MyPreferences.h"

#include "AddressBar.h"
#include "ControlStrip.h"
#include "ControlButton.h"
#include "ViewMenu.h"
#include "MouseMachine/MouseMachine.h"

#include "qgps.h"
#include "qgpsdevice.h"

MainWindow::MainWindow()
    : QWidget()
    , m_homeView(0)
    , m_mapView(0)
    , m_gpsview(0)
    , m_controlStrip(0)
    , viewToShow(0)
{
    m_timeLine = new QTimeLine(800, this);
    m_timeLine->setCurveShape(QTimeLine::EaseInOutCurve);
    QTimer::singleShot(0, this, SLOT(initialize()));
}

MainWindow::~MainWindow()
{
    M_PREFS->setInitialPosition(m_mapView);
    M_PREFS->getQSettings()->sync();
}

void MainWindow::initialize()
{
    MyPreferences::m_prefInstance = new MyPreferences(this);
    SEMPERMERK_PREFS->hide();
    SEMPERMERK_PREFS->resize(size());
    SEMPERMERK_PREFS->move(0, 0);

    m_controlStrip = new ControlStrip(this);
    m_controlStrip->resizeBar(size(), true);
    m_controlStrip->show();

    m_homeView = new HomeView(this);

    m_mapView = new MapView(this);
    m_mapView->show();
    m_mapView->resize(QSize(width(), height()));

    loadTemplateDocument(":/doc/initial.mdc");
    m_mapView->setDocument(m_document);

//    ImageMapLayer* ilayer = m_document->addImageLayer();
//    ilayer->setMapAdapter(TMS_ADAPTER_UUID, "OSM Mapnik");
//    ilayer->setVisible(true);

    M_PREFS->initialPosition(m_mapView);
    m_mapView->launch(new EditInteraction(m_mapView));
    m_mapView->invalidate(true, true);

    m_homeView->resize(QSize(width(), height()));
    m_homeView->move(width(), m_homeView->y());
    m_homeView->show();

    m_gpsview = new QGPS(this);
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    QGPSMobileDevice* aGps = new QGPSMobileDevice();
    if (aGps->openDevice()) {
        connect(aGps, SIGNAL(updatePosition(qreal, qreal, QDateTime, qreal, qreal, qreal)),
            this, SLOT(updateGpsPosition(qreal, qreal, QDateTime, qreal, qreal, qreal)));

//        ui->gpsConnectAction->setEnabled(false);
//        ui->gpsReplayAction->setEnabled(false);
//        ui->gpsDisconnectAction->setEnabled(true);
//        ui->gpsRecordAction->setEnabled(true);
//        ui->gpsPauseAction->setEnabled(true);
        m_gpsview->setGpsDevice(aGps);
        m_gpsview->resetGpsStatus();
        m_gpsview->startGps();
    }
#elif defined(Q_WS_SIMULATOR)
    QGPSFileDevice* aGps = new QGPSFileDevice(":/test/test.nma");
    if (aGps->openDevice()) {
        connect(aGps, SIGNAL(updatePosition(qreal, qreal, QDateTime, qreal, qreal, qreal)),
            this, SLOT(updateGpsPosition(qreal, qreal, QDateTime, qreal, qreal, qreal)));

//        ui->gpsConnectAction->setEnabled(false);
//        ui->gpsReplayAction->setEnabled(false);
//        ui->gpsDisconnectAction->setEnabled(true);
//        ui->gpsRecordAction->setEnabled(true);
//        ui->gpsPauseAction->setEnabled(true);
        m_gpsview->setGpsDevice(aGps);
        m_gpsview->resetGpsStatus();
        m_gpsview->startGps();
    }
#endif
    m_gpsview->resize(QSize(width(), height()));
    m_gpsview->move(width(), m_gpsview->y());
    m_gpsview->show();

    SEMPERMERK_PREFS->show();

    m_controlStrip->raise();

    connect(m_homeView, SIGNAL(addressEntered(QString)), SLOT(gotoAddress(QString)));

//    connect(m_mapView, SIGNAL(menuButtonClicked()), SLOT(showHomeView()));

//    connect(m_controlStrip, SIGNAL(backClicked()), m_mapView, SLOT(backView()));
//    connect(m_controlStrip, SIGNAL(forwardClicked()), m_mapView, SLOT(fwdView()));
//    connect(m_controlStrip, SIGNAL(stopClicked()), m_mapView, SLOT(stop()));
//    connect(m_controlStrip, SIGNAL(zoomBestClicked()), m_mapView, SLOT(zoomBest()));
//    connect(m_controlStrip, SIGNAL(refreshClicked()), m_mapView, SLOT(reload()));
//    connect(m_controlStrip, SIGNAL(bookmarkClicked()), m_mapView, SIGNAL(menuButtonClicked()));
    connect(m_controlStrip, SIGNAL(zoomInClicked()), m_mapView, SLOT(zoomIn()));
    connect(m_controlStrip, SIGNAL(zoomOutClicked()), m_mapView, SLOT(zoomOut()));
    connect(m_controlStrip, SIGNAL(prefClicked()), SLOT(showPreferencesView()));
    connect(m_controlStrip, SIGNAL(menuClicked()), SLOT(showMenu()));

//    m_controlStrip->bookmarkBt->show();
//    m_controlStrip->searchBt->show();
//    m_controlStrip->editBt->hide();

    downSlide(0);

//    QTimer::singleShot(2000, this, SLOT(showHomeView()));
}

Document* MainWindow::doLoadDocument(QFile* file)
{
    QXmlStreamReader stream(file);
    while (stream.readNext() && stream.tokenType() != QXmlStreamReader::Invalid && stream.tokenType() != QXmlStreamReader::StartElement);
    if (stream.tokenType() != QXmlStreamReader::StartElement || stream.name() != "MerkaartorDocument") {
        return NULL;
    }
    double version = stream.attributes().value("version").toString().toDouble();

    Document* newDoc = NULL;

    if (version < 2.) {
        stream.readNext();
        while(!stream.atEnd() && !stream.isEndElement()) {
            if (stream.name() == "MapDocument") {
                newDoc = Document::fromXML(QFileInfo(*file).fileName(), stream, version, NULL, NULL);
            } else if (stream.name() == "MapView") {
                m_mapView->fromXML(stream);
            } else if (!stream.isWhitespace()) {
                qDebug() << "Main: logic error: " << stream.name() << " : " << stream.tokenType() << " (" << stream.lineNumber() << ")";
                stream.skipCurrentElement();
            }

            stream.readNext();
        }
    }
    return newDoc;
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
        m_document = newDoc;
        m_document->setTitle(tr("untitled"));
    }
}


void MainWindow::sideSlide(int pos)
{
    m_mapView->move(pos, m_mapView->y());
    if (viewToShow)
        viewToShow->move(width() + pos, viewToShow->y());
}

void MainWindow::downSlide(int pos)
{
    m_mapView->move(m_mapView->x(), pos);
    if (viewToShow)
        viewToShow->move(viewToShow->x(), pos);
    SEMPERMERK_PREFS->move(0, pos + height());
}

void MainWindow::showHomeView()
{
    if (m_timeLine->state() != QTimeLine::NotRunning)
        return;

    m_timeLine->setFrameRange(0, -width());
    disconnect(m_timeLine, SIGNAL(frameChanged(int)), 0, 0);
    connect(m_timeLine, SIGNAL(frameChanged(int)), SLOT(sideSlide(int)));

//    m_controlStrip->backAct->setEnabled(true);
//    m_controlStrip->forwardBt->hide();
//    m_controlStrip->bookmarkBt->hide();
//    m_controlStrip->stopBt->hide();
//    m_controlStrip->searchBt->hide();
//    m_controlStrip->refreshBt->hide();
//    m_controlStrip->editBt->show();

//    disconnect(m_controlStrip, 0, 0, 0);
//    connect(m_controlStrip, SIGNAL(backClicked()), m_homeView, SIGNAL(back()));
//    connect(m_controlStrip, SIGNAL(forwardClicked()), m_homeView->addressBar(), SLOT(processAddress()));
//    connect(m_controlStrip, SIGNAL(editClicked()), m_homeView, SLOT(toggleEdit()));
//    connect(m_controlStrip, SIGNAL(prefClicked()), SLOT(showPreferencesView()));

    m_homeView->blockSignals(false);
    m_mapView->blockSignals(true);
    SEMPERMERK_PREFS->blockSignals(true);

    m_controlStrip->Expand();

    m_timeLine->start();
//    m_homeView->setCurrentAdress(m_mapView->getCurrentName(), m_mapView->getCurrentAdress());
    m_homeView->stackUnder(m_controlStrip);
//    m_homeView->setFocus();
}

void MainWindow::showMapView()
{
    if (m_timeLine->state() != QTimeLine::NotRunning)
        return;

    m_timeLine->setFrameRange(-width(), 0);
    disconnect(m_timeLine, SIGNAL(frameChanged(int)), 0, 0);
    connect(m_timeLine, SIGNAL(frameChanged(int)), SLOT(sideSlide(int)));

//    m_controlStrip->forwardBt->show();
//    m_controlStrip->bookmarkBt->show();
////    m_controlStrip->stopBt->show();
//    m_controlStrip->searchBt->show();
////    m_controlStrip->refreshBt->show();
//    m_controlStrip->editBt->hide();

//    disconnect(m_controlStrip, 0, 0, 0);
//    connect(m_controlStrip, SIGNAL(backClicked()), m_mapView, SLOT(backView()));
//    connect(m_controlStrip, SIGNAL(forwardClicked()), m_mapView, SLOT(fwdView()));
//    connect(m_controlStrip, SIGNAL(stopClicked()), m_mapView, SLOT(stop()));
//    connect(m_controlStrip, SIGNAL(zoomBestClicked()), m_mapView, SLOT(zoomBest()));
//    connect(m_controlStrip, SIGNAL(refreshClicked()), m_mapView, SLOT(reload()));
//    connect(m_controlStrip, SIGNAL(bookmarkClicked()), m_mapView, SIGNAL(menuButtonClicked()));
//    connect(m_controlStrip, SIGNAL(prefClicked()), SLOT(showPreferencesView()));

    m_homeView->blockSignals(true);
    m_mapView->blockSignals(false);
    SEMPERMERK_PREFS->blockSignals(true);

//    m_mapView->updateButtons();
    m_controlStrip->show();

    m_timeLine->start();
    m_mapView->stackUnder(m_controlStrip);
//    m_browserView->setFocus();

}

void MainWindow::showGpsView()
{
    if (m_timeLine->state() != QTimeLine::NotRunning)
        return;

    m_timeLine->setFrameRange(0, -width());
    disconnect(m_timeLine, SIGNAL(frameChanged(int)), 0, 0);
    connect(m_timeLine, SIGNAL(frameChanged(int)), SLOT(sideSlide(int)));

    viewToShow = m_gpsview;
    m_timeLine->start();
    m_gpsview->stackUnder(m_controlStrip);
}

void MainWindow::showPreferencesView()
{
    if (m_timeLine->state() != QTimeLine::NotRunning)
        return;

    m_timeLine->setFrameRange(0, -SEMPERMERK_PREFS->height());
    disconnect(m_timeLine, SIGNAL(frameChanged(int)), 0, 0);
    connect(m_timeLine, SIGNAL(frameChanged(int)), SLOT(downSlide(int)));
    m_timeLine->start();
    m_controlStrip->hide();

    m_homeView->blockSignals(true);
    m_mapView->blockSignals(true);
    SEMPERMERK_PREFS->blockSignals(false);

    SEMPERMERK_PREFS->execPreferences();

    m_homeView->blockSignals(false);
    m_mapView->blockSignals(false);
    SEMPERMERK_PREFS->blockSignals(true);

    m_controlStrip->show();
    m_timeLine->setFrameRange(-height(), 0);
    m_timeLine->start();
}

void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    QWidget::keyReleaseEvent(event);

    if (event->key() == Qt::Key_F3) {
        if (m_homeView->isVisible())
            showMapView();
        else
            showHomeView();
    }
}

void MainWindow::resizeEvent(QResizeEvent *event)
{
    if (m_homeView)
        m_homeView->resize(QSize(width(), height()));
    if (m_mapView)
        m_mapView->resize(QSize(width(), height()));
    if (m_gpsview)
        m_gpsview->resize(QSize(width(), height()));
    SEMPERMERK_PREFS->resize(size());
    if (m_controlStrip)
        m_controlStrip->resizeBar(size(), true);

    if (m_mapView) {
        if (m_mapView->x() < 0)
            sideSlide(-width());
        else
            sideSlide(0);
        if (m_mapView->y() < 0) {
            downSlide(-SEMPERMERK_PREFS->height());
        } else {
            downSlide(0);
        }
    }
}

void MainWindow::invalidateView(bool UpdateDock)
{
    m_mapView->invalidate(true, true);
//    //theLayers->updateContent();
//    if (UpdateDock)
//        p->theProperties->resetValues();
}

void MainWindow::updateLanguage()
{
}

void MainWindow::showMenu()
{
    ViewMenu* menu = new ViewMenu(this);
    menu->move((width()-menu->width())/2, (height()-menu->height())/2);
    connect(menu, SIGNAL(mapRequested()), SLOT(showMapView()));
    connect(menu, SIGNAL(gpsRequested()), SLOT(showGpsView()));
    menu->exec();
}

void MainWindow::updateGpsPosition(qreal latitude, qreal longitude, QDateTime time, qreal altitude, qreal speed, qreal heading)
{
    Q_UNUSED(heading)
    if (m_gpsview->getGpsDevice()) {
        Coord gpsCoord(longitude,latitude);
        CoordBox vp = m_mapView->viewport();
        qreal lonDiff = vp.lonDiff();
        qreal latDiff = vp.latDiff();
        QRectF vpr = vp.adjusted(lonDiff / 4, -latDiff / 4, -lonDiff / 4, latDiff / 4);
        if (!vpr.contains(gpsCoord)) {
            m_mapView->setCenter(gpsCoord, m_mapView->rect());
            m_mapView->invalidate(true, true);
        }

//        if (ui->gpsRecordAction->isChecked() && !ui->gpsPauseAction->isChecked()) {
//            Node* pt = g_backend.allocNode(gpsRecLayer, gpsCoord);
//            pt->setTime(time);
//            pt->setElevation(altitude);
//            pt->setSpeed(speed);
//            gpsRecLayer->add(pt);
//            curGpsTrackSegment->add(pt);
//            m_mapView->invalidate(true, false);
//        }
    }
    m_mapView->update();
}
