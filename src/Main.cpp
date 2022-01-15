#include "Global.h"

#include <QApplication>
#include <QMessageBox>

#include <QLibraryInfo>
#include <QSplashScreen>

#include <qtsingleapplication.h>
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "proj.h"
#include "gdal_version.h"
#include "Global.h"
#include "build-metadata.hpp"

#include "IMapAdapterFactory.h"

QLoggingCategory lc_Main("merk.Main");

FILE* pLogFile = NULL;

void showVersion()
{
    QString o;
    o = QString("%1 %2\n").arg(BuildMetadata::PRODUCT).arg(BuildMetadata::REVISION);
    fprintf(stdout, "%s", o.toLatin1().data());
    o = QString("using Qt version %1 (built with %2)\n").arg(qVersion()).arg(QT_VERSION_STR);
    fprintf(stdout, "%s", o.toLatin1().data());
    fprintf(stdout, "Copyright Bart Vanhauwaert, Chris Browet and others, 2006-2010\n");
    fprintf(stdout, "This program is licensed under the Version 2 of the GNU General Public License or any later version\n");
}

void showHelp()
{
    showVersion();
    fprintf(stdout, "\n");
    fprintf(stdout, "Usage: merkaartor [-h|--help] [-v|--version] [-l|--log logfilename] [filenames...]\n");
    fprintf(stdout, "\n");
    fprintf(stdout, "  -h, --help\t\tShow help information\n");
    fprintf(stdout, "  -l, --log logfilename\t\tSave debugging information to file \"logfilename\"\n");
    fprintf(stdout, "  -v, --version\t\tShow version information\n");
    fprintf(stdout, "  -n, --noreuse\t\tDo not reuse an existing instance\n");
    fprintf(stdout, "  -p, --portable\t\tExecute as a portable application (all files saved in the application directory)\n");
    fprintf(stdout, "  --enable_special_layers\t\tEnable old style \"Dirty\" and \"Uploaded\" layers\n");
    fprintf(stdout, "  --importag-tags-as-is\t\tDo not add underscores to imported tags (allow for immediate upload)\n");
    fprintf(stdout, "  --ignore-preferences\t\tIgnore saved preferences\n");
    fprintf(stdout, "  --reset-preferences\t\tReset saved preferences to default\n");
    fprintf(stdout, "  --ignore-startup-template\t\tIgnore the saved startup template document and start with a new document\n");
    fprintf(stdout, "  [filenames]\t\tOpen designated files \n");
}

void loadPluginsFromDir( QDir & pluginsDir ) {
    qInfo(lc_Main) << "Loading plugins from directory " << pluginsDir.absolutePath();
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        qDebug(lc_Main) << "  Loading" << fileName << "as plugin.";
        if (plugin) {
            IMapAdapterFactory *fac = qobject_cast<IMapAdapterFactory *>(plugin);
            if (fac) {
                M_PREFS->addBackgroundPlugin(fac);
                qInfo(lc_Main) << "  Plugin loaded: " << fileName << ".";
            } else {
                qWarning(lc_Main) << "  Failed to load plugin: " << fileName << ".";
            }
        } else {
            qDebug(lc_Main) << "  Not a plugin: " << fileName << ".";
        }
    }
}

void debugMessageHandler(QtMsgType, const QMessageLogContext&, const QString &msg)
{
    if (pLogFile) {
        const auto bytes = msg.toUtf8();
        fwrite(bytes.constData(), 1, bytes.size(), pLogFile);
#if defined(Q_OS_UNIX)
        constexpr const char * newline = " \n";
#else
        constexpr const char * newline = "\r\n";
#endif
        fwrite(newline, 1, 2, pLogFile);
        fflush(pLogFile);
    }
}

int main(int argc, char** argv)
{
    QtSingleApplication instance(argc,argv);

    // Set logging message pattern early.
    qSetMessagePattern("%{time process} (%{if-debug}DD%{endif}%{if-info}II%{endif}%{if-warning}WW%{endif}%{if-critical}CC%{endif}%{if-fatal}FF%{endif}) [ %{category} ] %{message}%{if-critical}%{backtrace}%{endif}");
    QLoggingCategory::setFilterRules("merk.*.debug=false");

    bool reuse = true;
    bool testImport = false;
    QStringList argsIn = QCoreApplication::arguments();
    QStringList argsOut;
    argsIn.removeFirst();
    for (int i=0; i < argsIn.size(); ++i) {
        if (argsIn[i] == "-v" || argsIn[i] == "--version") {
            showVersion();
            exit(0);
        } else if (argsIn[i] == "-h" || argsIn[i] == "--help") {
            showHelp();
            exit(0);
        } else if (argsIn[i] == "-n" || argsIn[i] == "--noreuse") {
            reuse = false;
        } else if (argsIn[i] == "-p" || argsIn[i] == "--portable") {
            g_Merk_Portable = true;
        } else if (argsIn[i] == "--enable_special_layers") {
            g_Merk_Frisius = false;
        } else if (argsIn[i] == "--importag-tags-as-is") {
            g_Merk_NoGuardedTagsImport = true;
        } else if (argsIn[i] == "--ignore-preferences") {
            g_Merk_Ignore_Preferences = true;
        } else if (argsIn[i] == "--reset-preferences") {
            g_Merk_Reset_Preferences = true;
        } else if (argsIn[i] == "--ignore-startup-template") {
            g_Merk_IgnoreStartupTemplate = true;
        } else if (argsIn[i] == "--test-import") {
            testImport = true;
        } else if (argsIn[i] == "--selfclip") {
            g_Merk_SelfClip = true;
        } else
            argsOut << argsIn[i];
    }

    QCoreApplication::setOrganizationName("Merkaartor");
    QCoreApplication::setOrganizationDomain("merkaartor.org");
#ifdef FRISIUS_BUILD
    QCoreApplication::setApplicationName("Frisius");
#else
    QCoreApplication::setApplicationName("Merkaartor");
#endif
    QString message = argsOut.join("$");
    if (reuse)
        if (instance.sendMessage(message))
            return 0;

    QString logFilename;
    QStringList fileNames;
    for (int i=0; i < argsOut.size(); ++i) {
        if (argsOut[i] == "-l" || argsOut[i] == "--log") {
            ++i;
            logFilename = argsOut[i];
        } else
            fileNames.append(argsOut[i]);
    }

    if (!logFilename.isNull()) {
        pLogFile = fopen(logFilename.toLatin1(), "a");
        qInstallMessageHandler(debugMessageHandler);
    }

    qInfo(lc_Main) << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Starting " << QString("%1 %2").arg(qApp->applicationName()).arg(BuildMetadata::VERSION);
    qInfo(lc_Main) <<	"-------" << QString("using Qt version %1 (built with %2)").arg(qVersion()).arg(QT_VERSION_STR);
    PJ_INFO projVer = proj_info();
    qInfo(lc_Main) <<	"-------" << QString("using PROJ version %1.%2.%3").arg(projVer.major).arg(projVer.minor).arg(projVer.patch);
    qInfo(lc_Main) <<	"-------" << QString("using GDAL version %1").arg(GDAL_RELEASE_NAME);
    qInfo(lc_Main) << "-------" << "with arguments: " << QCoreApplication::arguments();

#ifdef _MOBILE
    QFont appFont = QApplication::font();
    appFont.setPointSize(6);
    QApplication::setFont(appFont);
#endif

    qApp->setStyleSheet(
            " LayerWidget { border: 1px solid black; min-height: 20px}"
            " LayerWidget QLineEdit { color: black; }"

            " LayerWidget QCheckBox::indicator:checked { image: url(:Icons/eye.xpm); }"
            " LayerWidget QCheckBox::indicator:unchecked { image: url(:Icons/empty.xpm); }"

            " DrawingLayerWidget { background-color: #a5d1ff; }"
            " ImageLayerWidget { background-color: #ffffcc; }"
            " TrackLayerWidget { background-color: #7acca6; }"
            " DirtyLayerWidget { background-color: #c8c8c8; }"
            " UploadedLayerWidget { background-color: #c8c8c8; }"
            " OsbLayerWidget { background-color: #a2d1c0; }"
            " FilterLayerWidget { background-color: #c8c8c8; }"

            " LayerWidget:checked { background-color: lightsteelblue; }"
            );

    QPixmap pixmap(QString(":/Splash/%1_splash.png").arg(qApp->applicationName()));
    QSplashScreen splash(pixmap);
    splash.show();
    instance.processEvents();

    splash.showMessage(QString(instance.translate("Main", "%1 v%2\nLoading plugins...")).arg(qApp->applicationName()).arg(BuildMetadata::REVISION), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
    instance.processEvents();

    /* Create configuration directory for non-portable build. */
    if (!g_Merk_Portable) {
#ifdef Q_OS_MAC
        if (!QDir::home().exists(HOMEDIR))
            QDir::home().mkpath(HOMEDIR);
#else
        if (!QDir::home().exists("." + qApp->applicationName().toLower()))
            QDir::home().mkdir("." + qApp->applicationName().toLower());
#endif
    }


    /* Load plugins if executed from build directory and in debug build. */
    QDir buildPluginsDir = QDir(qApp->applicationDirPath());
    if (buildPluginsDir.exists("CMakeCache.txt")) {
        qWarning() << "Build directory detected. Looking for plugins in application directory first.";
        QCoreApplication::addLibraryPath(buildPluginsDir.path());
        loadPluginsFromDir(buildPluginsDir);
    }

    /* Load plugins; this handles different OS habits. */
#if defined(Q_OS_WIN32)
    QDir pluginsDir = QDir(qApp->applicationDirPath() + "/");
#elif defined(Q_OS_MAC)
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cdUp();
    pluginsDir.cd("plugins");
#elif defined(Q_OS_SYMBIAN)
    QDir pluginsDir(QLibraryInfo::location(QLibraryInfo::PluginsPath));
#else

    /* Try directory with the application first. */
    QDir pluginsDir = qApp->applicationDirPath() + "/plugins";
    if (!pluginsDir.exists()) {
        pluginsDir = QDir(BuildMetadata::GetLibDir() + "/plugins");
    }
#endif

    QCoreApplication::addLibraryPath(pluginsDir.path());
    loadPluginsFromDir(pluginsDir);

    pluginsDir.cd("background");
    loadPluginsFromDir(pluginsDir);

    splash.showMessage(QString(instance.translate("Main", "%1 v%2\nInitializing...")).arg(qApp->applicationName()).arg(BuildMetadata::REVISION), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
    instance.processEvents();

//    QFatFsHandler* fatHandler = new QFatFsHandler(50000, 8192);
    //ZipEngineHandler* zipHandler = new ZipEngineHandler();


    MainWindow Main;
    g_Merk_MainWindow = &Main;
    instance.setActivationWindow(&Main, false);
    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
             &instance, SLOT(activateWindow()));
    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)),
             &Main, SLOT(handleMessage(const QString&)));
//    QObject::connect(&Main, SIGNAL(needToShow()), &instance, SLOT(activateWindow()));

#ifdef _MOBILE
    instance.setActiveWindow(&Main);
    Main.showMaximized();
#else
    Main.show();
#endif
    instance.processEvents();
    if (!testImport) {
        Main.handleMessage(message);
    }

    splash.finish(&Main);

    if (testImport) {
        int testsFailed = 0;
        for (auto& file : fileNames) {
            bool ret = Main.testImport(file);
            qDebug() << "Testing import of file" << file << "result:" << (ret ? "PASSED" : "FAILED");
            if (!ret) {
                testsFailed++;
            }
        }
        qDebug() << "All tests done, " << testsFailed << " tests failed.";
        return testsFailed;
    }

    int x;
    try {
        x = instance.exec();
    } catch (const std::bad_alloc &) {
        qDebug(lc_Main) << "Out of memory";
        x = 254;
    } catch (...) {
        qDebug(lc_Main) << "Exception";
        x = 255;
    }

    qDebug(lc_Main) << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Ending " << QString("%1 %2").arg(qApp->applicationName()).arg(BuildMetadata::VERSION);
    if(pLogFile) {
        fclose(pLogFile);
        pLogFile = NULL;
    }

//    delete fatHandler;
    //delete zipHandler;

    return x;
}

