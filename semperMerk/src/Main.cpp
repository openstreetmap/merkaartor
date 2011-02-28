//
// C++ Implementation: Main
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtCore>
#include <QtGui>

#include "MainWindow.h"
#include "MyMessageHandler.h"
#include "MerkaartorPreferences.h"

#include "IMapAdapterFactory.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QCoreApplication::setApplicationName("semperMerk");
    QCoreApplication::setOrganizationName("SemperPax");
    QCoreApplication::setOrganizationDomain("semperpax.com");
    QCoreApplication::setApplicationVersion("1.0.0");

#ifdef LOG_TO_FILE
#if defined(Q_OS_UNIX)
    QString logFilename = QString("/tmp/%1.log").arg(QCoreApplication::applicationName());
#elif defined(Q_OS_WIN)
    QString logFilename = QString(qApp->applicationDirPath() + "/%1.log").arg(QCoreApplication::applicationName());
#elif defined(Q_OS_SYMBIAN)
    QString logFilename = QString(QDir::homePath() + "/%1.log").arg(QCoreApplication::applicationName());
#endif

    pLogFile = new QFile(logFilename);
    pLogFile->open(QIODevice::Append | QIODevice::Text);
    ts.setDevice(pLogFile);
    qInstallMsgHandler(myMessageOutput);

    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Starting " << QString("%1 %2")
                            .arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
    qDebug() <<	"-------" << QString("using QT version %1 (built with %2)").arg(qVersion()).arg(QT_VERSION_STR);
#ifdef Q_WS_X11
    qDebug() << "-------" << "on X11";
#endif
#ifdef Q_WS_WIN
    qDebug() << "-------" << "on Windows";
#endif
#ifdef Q_WS_MACX
    qDebug() << "-------" << "on Mac OS/X";
#endif
#ifdef Q_WS_S60
    qDebug() << "-------" << "on S60";
#endif
#endif // LOG_TO_FILE

    MainWindow window;
#if defined(Q_OS_SYMBIAN) || defined(Q_WS_SIMULATOR)
    window.showFullScreen();
#else
    window.resize(360, 640);
    window.show();
    app.setStyle("windows");
#endif

    QString theStyle(
//                "					ControlStrip {"
//                "						background-color: qlineargradient(spread:pad, x1:0, y1:0, x2:0, y2:1, stop:0 rgba(0, 0, 0, 160), stop:0.500 rgba(50, 50, 50, 160), stop:0.501 rgba(52, 52, 52, 160), stop:1 rgba(0, 0, 0, 160)) ;"
//                "					}"
                "					ControlStrip {"
                "						background-color: rgba(0, 0, 0, 128);"
                "					}"
                "					ViewMenu {"
                "						background-color: rgba(0, 0, 0, 128);"
                "					}"
                "ViewMenu QPushButton {"
                "  color: #333;"
                "  border: 1px solid #555;"
                "  padding: 5px;"
                "  background: qradialgradient(cx: 0.3, cy: -0.4,"
                "    fx: 0.3, fy: -0.4,"
                "    radius: 1.35, stop: 0 #fff, stop: 1 #bbb);"
                "  min-width: 80px;"
                "}"
                "					ZoomStrip {"
                "						background: rgba(128, 128, 128, 128) ;"
                "					}"
                "					ControlButton {"
                "						border: none;"
                "                       background-color: transparent;"
                "					}"
                "BookmarksView QTreeView {"
                "   background-color: #333;"
                "  font: bold 24px;"
                "}"
                ""
                "BookmarksView QTreeView::item {"
                "  color: #333;"
                "  border: 1px solid #555;"
                "  padding: 5px;"
                "  background: qradialgradient(cx: 0.3, cy: -0.4,"
                "    fx: 0.3, fy: -0.4,"
                "    radius: 1.35, stop: 0 #fff, stop: 1 #bbb);"
                "  min-width: 80px;"
                "}"
                ""
                "BookmarksView QTreeView::item:has-children {"
                "  color: #ddd;"
                "  border: 1px solid #555;"
                "  padding: 5px;"
                "  background: qradialgradient(cx: 0.3, cy: -0.4,"
                "    fx: 0.3, fy: -0.4,"
                "    radius: 1.35, stop: 0 #fff, stop: 1 #333);"
                "  min-width: 80px;"
                "}"

                );
    app.setStyleSheet(theStyle);

#ifdef QT_KEYPAD_NAVIGATION
    QApplication::setNavigationMode(Qt::NavigationModeCursorAuto);
#endif

    // Plugins
#if defined(Q_OS_WIN32)
    QDir pluginsDir = QDir(qApp->applicationDirPath() + "/plugins");

    // Fixes MacOSX plugin dir (fixes #2253)
#elif defined(Q_OS_MAC)
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cdUp();
    pluginsDir.cd("plugins");
#elif defined(Q_OS_SYMBIAN)
    QDir pluginsDir(QLibraryInfo::location(QLibraryInfo::PluginsPath));
#else
    QDir pluginsDir = (g_Merk_Portable ? QDir(qApp->applicationDirPath() + "/plugins") : QDir(STRINGIFY(PLUGINS_DIR)));
#endif
    QCoreApplication::addLibraryPath(pluginsDir.path());

    pluginsDir.cd("background");
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            IMapAdapterFactory *fac = qobject_cast<IMapAdapterFactory *>(plugin);
            if (fac)
                M_PREFS->addBackgroundPlugin(fac);
        }
    }

    int x =  app.exec();

#ifdef LOG_TO_FILE
    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate)
                << " -- Ending " << QString("%1 %2")
                    .arg(QCoreApplication::applicationName()).arg(QCoreApplication::applicationVersion());
    if(pLogFile) {
        pLogFile->close();
        delete pLogFile;
    }
#endif // LOG_TO_FILE

    return x;
}
