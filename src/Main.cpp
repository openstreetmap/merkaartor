#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <QLibraryInfo>
#include <QSplashScreen>

#include <qtsingleapplication.h>
#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "revision.h"
#ifdef USE_PROJ
#include "proj_api.h"
#endif

#include "IMapAdapter.h"

#if defined(Q_OS_WIN)
extern Q_CORE_EXPORT void qWinMsgHandler(QtMsgType t, const char* str);
#endif

FILE* pLogFile = NULL;

void myMessageOutput(QtMsgType msgType, const char *buf)
{
// From corelib/global/qglobal.cpp : qt_message_output

#if defined(Q_OS_WIN) && !defined(NDEBUG)
    qWinMsgHandler(msgType, buf);
#endif
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
    o = QString("Merkaartor %1%2(%3)\n").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
    fprintf(stdout, "%s", o.toLatin1().data());
    o = QString("using QT version %1 (built with %2)\n").arg(qVersion()).arg(QT_VERSION_STR);
    fprintf(stdout, "%s", o.toLatin1().data());
    fprintf(stdout, "Copyright Bart Vanhauwaert, Chris Browet and others, 2006-2010\n");
    fprintf(stdout, "This program is licensed under the GNU Public License v2\n");
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
    fprintf(stdout, "  -n, --noreuse\t\tDo not reuse an existing Merkaartor instance\n");
    fprintf(stdout, "  -p, --portable\t\tExecute Merkaartor as a portable application (all files saved in the application directory)\n");
    fprintf(stdout, "  --enable_special_layers\t\tEnable old style \"Dirty\" and \"Uploaded\" layers\n");
    fprintf(stdout, "  --importag-tags-as-is\t\tDo not add underscores to imported tags (allow for immediate upload)\n");
    fprintf(stdout, "  --ignore-preferences\t\tIgnore saved preferences\n");
    fprintf(stdout, "  --reset-preferences\t\tReset saved preferences to default\n");
    fprintf(stdout, "  [filenames]\t\tOpen designated files \n");
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
    qInstallMsgHandler(myMessageOutput);

    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Starting " << QString("%1 %2%3(%4)").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
    qDebug() <<	"-------" << QString("using QT version %1 (built with %2)").arg(qVersion()).arg(QT_VERSION_STR);
#ifdef USE_PROJ
    qDebug() <<	"-------" << QString("using PROJ4 version %1").arg(STRINGIFY(PJ_VERSION));
#endif
#ifdef Q_WS_X11
    qDebug() << "-------" << "on X11";
#endif
#ifdef Q_WS_WIN
    qDebug() << "-------" << "on Windows";
#endif
#ifdef Q_WS_MACX
    qDebug() << "-------" << "on Mac OS/X";
#endif
    qDebug() << "-------" << "with arguments: " << QCoreApplication::arguments();

#ifdef _MOBILE
    QFont appFont = QApplication::font();
    appFont.setPointSize(6);
    QApplication::setFont(appFont);
#endif

    qApp->setStyleSheet(
            " LayerWidget { color: black; border: 1px solid black; min-height: 20px}"

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

    if (!QDir::home().exists("." + qApp->applicationName().toLower()))
        QDir::home().mkdir("." + qApp->applicationName().toLower());
#if defined(Q_OS_WIN32)
    QDir pluginsDir = QDir(qApp->applicationDirPath() + "/" + STRINGIFY(PLUGINS_DIR));

    // Fixes MacOSX plugin dir (fixes #2253)
#elif defined(Q_OS_MAC)
    QDir pluginsDir = QDir(qApp->applicationDirPath());
    pluginsDir.cdUp();
    pluginsDir.cd("plugins");
#else
    QDir pluginsDir = (g_Merk_Portable ? QDir(qApp->applicationDirPath() + "/plugins") : QDir(STRINGIFY(PLUGINS_DIR)));
#endif
    QCoreApplication::addLibraryPath(pluginsDir.path());

    pluginsDir.cd("background");
    foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
        QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
        QObject *plugin = loader.instance();
        if (plugin) {
            IMapAdapter *plg = qobject_cast<IMapAdapter *>(plugin);
            if (plg)
#ifndef USE_WEBKIT
                if (plg->getType() != IMapAdapter::BrowserBackground)
#endif
                    M_PREFS->addBackgroundPlugin(plg);
        }
    }

    splash.showMessage(QString(instance.translate("Main", "%1 v%2%3(%4)\nInitializing...")).arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
    instance.processEvents();

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

    int x = instance.exec();

    qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Ending " << QString("%1 %2%3(%4)").arg(qApp->applicationName()).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
    if(pLogFile) {
        fclose(pLogFile);
        pLogFile = NULL;
    }

    return x;
}

