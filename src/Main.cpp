#include "Global.h"

#include <QApplication>
#include <QMessageBox>

#include <QLibraryInfo>
#include <QSplashScreen>

#include <qtsingleapplication.h>
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "revision.h"
#include "proj_api.h"
#include "gdal_version.h"
//#include "QFatFs.h"
//#include "ZipEngine.h"
#include "Global.h"

#include "IMapAdapterFactory.h"

FILE* pLogFile = NULL;

#ifdef QT5
void myMessageOutput(QtMsgType msgType, const QMessageLogContext &, const QString & str) {
    const char * buf = str.toStdString().c_str();
#else
void myMessageOutput(QtMsgType msgType, const char *buf) {
#endif
// From corelib/global/qglobal.cpp : qt_message_output

#if defined(Q_CC_MWERKS) && !defined(Q_OS_SYMBIAN)
    mac_default_handler(buf);
#elif defined(Q_OS_WINCE)
    QString fstr = QString::fromLatin1(buf);
    fstr += QLatin1String("\n");
    OutputDebugString(reinterpret_cast<const wchar_t *> (fstr.utf16()));
#else
#ifndef NDEBUG
    fprintf(stderr, "%s\n", buf);
    fflush(stderr);
#endif
    if (pLogFile && msgType == QtDebugMsg) {
        fprintf(pLogFile, "%s\n", buf);
        fflush(pLogFile);
    }
#endif

    if (msgType == QtFatalMsg
        || (msgType == QtWarningMsg
        && (!qgetenv("QT_FATAL_WARNINGS").isNull())) ) {

#if defined(Q_CC_MSVC) && defined(QT_DEBUG) && defined(_DEBUG) && defined(_CRT_ERROR)
            // get the current report mode
            int reportMode = _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_WNDW);
            _CrtSetReportMode(_CRT_ERROR, reportMode);
#if !defined(Q_OS_WINCE)
            int ret = _CrtDbgReport(_CRT_ERROR, __FILE__, __LINE__, QT_VERSION_STR, buf);
#else
            int ret = _CrtDbgReportW(_CRT_ERROR, _CRT_WIDE(__FILE__),
                __LINE__, _CRT_WIDE(QT_VERSION_STR), reinterpret_cast<const wchar_t *> (QString::fromLatin1(buf).utf16()));
#endif
            if (ret == 0  && reportMode & _CRTDBG_MODE_WNDW)
                return; // ignore
            else if (ret == 1)
                _CrtDbgBreak();
#endif

#if (defined(Q_OS_UNIX) || defined(Q_CC_MINGW))
            abort(); // trap; generates core dump
#else
            exit(1); // goodbye cruel world
#endif
    }
}

void showVersion()
{
    QString o;
    o = QString("%1 %2%3(%4)\n").arg(STRINGIFY(PRODUCT)).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
    fprintf(stdout, "%s", o.toLatin1().data());
    o = QString("using Qt version %1 (built with %2)\n").arg(qVersion()).arg(QT_VERSION_STR);
    fprintf(stdout, "%s", o.toLatin1().data());
    fprintf(stdout, "Copyright Bart Vanhauwaert, Chris Browet and others, 2006-2010\n");
    fprintf(stdout, "This program is licensed under the Version 2 of the GNU Public License or any later version\n");
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
    qDebug() << "Loading plugins from directory " << pluginsDir.dirName();
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        qDebug() << "  Loading" << fileName << "as plugin.";
        if (plugin) {
            IMapAdapterFactory *fac = qobject_cast<IMapAdapterFactory *>(plugin);
            if (fac) {
                M_PREFS->addBackgroundPlugin(fac);
                qDebug() << "  Plugin loaded: " << fileName << ".";
            } else {
                qWarning() << "  Failed to load plugin: " << fileName << ".";
            }
        } else {
            qWarning() << "  Not a plugin: " << fileName << ".";
        }
    }
}

int main(int argc, char** argv)
{
    QtSingleApplication instance(argc,argv);

    bool reuse = true;
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
#ifndef NDEBUG
#if defined(Q_OS_UNIX)
    logFilename = QString(QDir::homePath() + "/" + qApp->applicationName().toLower() + ".log");
#else
    logFilename = QString(qApp->applicationDirPath() + "/" + qApp->applicationName().toLower() + ".log");
#endif
#endif
    QStringList fileNames;
    for (int i=0; i < argsOut.size(); ++i) {
        if (argsOut[i] == "-l" || argsOut[i] == "--log") {
            ++i;
            logFilename = argsOut[i];
        } else
            fileNames.append(argsOut[i]);
    }

    if (!logFilename.isNull())
        pLogFile = fopen(logFilename.toLatin1(), "a");
#ifdef QT5
    qInstallMessageHandler(myMessageOutput);
#else
    qInstallMsgHandler(myMessageOutput);
#endif

    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Starting " << USER_AGENT;
    qDebug() <<	"-------" << QString("using Qt version %1 (built with %2)").arg(qVersion()).arg(QT_VERSION_STR);
    QString projVer = QString(STRINGIFY(PJ_VERSION));
    qDebug() <<	"-------" << QString("using PROJ4 version %1.%2.%3").arg(projVer.left(1)).arg(projVer.mid(1, 1)).arg(projVer.right(1));
    qDebug() <<	"-------" << QString("using GDAL version %1").arg(GDAL_RELEASE_NAME);
    qDebug() << "-------" << "with arguments: " << QCoreApplication::arguments();

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

    splash.showMessage(QString(instance.translate("Main", "%1 v%2%3(%4)\nLoading plugins...")).arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
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

    /* Load plugins; this handles different OS habits. */
#if defined(Q_OS_WIN32)
    QDir pluginsDir = QDir(qApp->applicationDirPath() + "/" + STRINGIFY(PLUGINS_DIR));
#elif defined(Q_OS_MAC)
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cdUp();
    pluginsDir.cd("plugins");
#elif defined(Q_OS_SYMBIAN)
    QDir pluginsDir(QLibraryInfo::location(QLibraryInfo::PluginsPath));
#else

    /* Try directory with the application first. */
    QDir pluginsDir = qApp->applicationDirPath() + "/plugins";
    if (!pluginsDir.exists())
        pluginsDir = QDir(STRINGIFY(PLUGINS_DIR));
#endif

    qDebug() << "PluginsDir:" << pluginsDir.path();
    QCoreApplication::addLibraryPath(pluginsDir.path());
    loadPluginsFromDir(pluginsDir);

    pluginsDir.cd("background");
    loadPluginsFromDir(pluginsDir);

    splash.showMessage(QString(instance.translate("Main", "%1 v%2%3(%4)\nInitializing...")).arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
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
    Main.handleMessage(message);

    splash.finish(&Main);

    int x;
    try {
        x = instance.exec();
    } catch (const std::bad_alloc &) {
        qDebug() << "Out of memory";
        x = 254;
    } catch (...) {
        qDebug() << "Exception";
        x = 255;
    }

    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Ending " << QString("%1 %2%3(%4)").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
    if(pLogFile) {
        fclose(pLogFile);
        pLogFile = NULL;
    }

//    delete fatHandler;
    //delete zipHandler;

    return x;
}

