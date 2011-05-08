#ifndef BROWSERWINDOW_H
#define BROWSERWINDOW_H

#include <QWidget>
#include <QDateTime>
#include <QFile>

#include "IMerkMainWindow.h"
#include "IProgressWindow.h"

class QTimeLine;
class QUrl;

class MapView;
class Document;
class HomeView;
class QGPS;

class ControlStrip;
class ControlButton;
class MouseMachine;

class MainWindow : public QWidget, public IMerkMainWindow, public IProgressWindow
{
    Q_OBJECT

public:
    MainWindow();
    ~MainWindow();

public:
    ControlStrip* controlStrip() { return m_controlStrip; }
    MapView* view() { return m_mapView; }
    Document* document() { return m_document; }

    QGPS* gps() { return m_gpsview; }
    PropertiesDock* properties() { return NULL; }
    InfoDock* info() { return NULL; }
    FeaturesDock* features()  { return NULL; }

    QProgressDialog* getProgressDialog() { return NULL; }
    QProgressBar* getProgressBar() { return NULL; }
    QLabel*		  getProgressLabel() { return NULL; }

    void updateLanguage();

private:
    Document * doLoadDocument(QFile *file);
    void loadTemplateDocument(QString fn);

public slots:
    void invalidateView(bool UpdateDock = true);
    void updateGpsPosition(qreal latitude, qreal longitude, QDateTime time, qreal altitude, qreal speed, qreal heading);

private slots:
    void initialize();

public slots:
    void showMenu();
    void showMapView();
    void showGpsView();
    void showHomeView();
    void showPreferencesView();
    void sideSlide(int);
    void downSlide(int);

protected:
    void keyReleaseEvent(QKeyEvent *event);
    void resizeEvent(QResizeEvent *event);

private:
    HomeView *m_homeView;
    MapView *m_mapView;
    Document *m_document;
    ControlStrip *m_controlStrip;
    QTimeLine *m_timeLine;
    QGPS* m_gpsview;

    QWidget* viewToShow;
};

#endif // BROWSERWINDOW_H
