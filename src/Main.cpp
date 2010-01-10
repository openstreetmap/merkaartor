#include <QtGui/QApplication>
#include <QtGui/QMessageBox>

#include <QLibraryInfo>
#include <QSplashScreen>

#include "MainWindow.h"
#include "Preferences/MerkaartorPreferences.h"
#include "revision.h"

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
	fprintf(stdout, "Copyright Bart Vanhauwaert, Chris Browet and others, 2006-2009\n");
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
	fprintf(stdout, "  [filenames]\t\tOpen designated files \n");
}

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

	QString logFilename;
#ifndef NDEBUG
#if defined(Q_OS_UNIX)
	logFilename = QString(QDir::homePath() + "/merkaartor.log");
#else
	logFilename = QString(qApp->applicationDirPath() + "/merkaartor.log");
#endif
#endif
	QStringList fileNames;
	QStringList args = QCoreApplication::arguments();
	args.removeFirst();
	for (int i=0; i < args.size(); ++i) {
		if (args[i] == "-l" || args[i] == "--log") {
			++i;
			logFilename = args[i];
		} else
		if (args[i] == "-v" || args[i] == "--version") {
			showVersion();
			exit(0);
		} else
		if (args[i] == "-h" || args[i] == "--help") {
			showHelp();
			exit(0);
		} else
			fileNames.append(args[i]);
	}

	if (!logFilename.isNull())
		pLogFile = fopen(logFilename.toLatin1(), "a");
	qInstallMsgHandler(myMessageOutput);

	qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Starting " << QString("Merkaartor %1%2(%3)").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
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

	QCoreApplication::setOrganizationName("BartVanhauwaert");
	QCoreApplication::setOrganizationDomain("www.irule.be");
	QCoreApplication::setApplicationName("Merkaartor");

#ifdef _MOBILE
	QFont appFont = QApplication::font();
	appFont.setPointSize(6);
	QApplication::setFont(appFont);
#endif

	QPixmap pixmap(":/Splash/Mercator_splash.png");
	QSplashScreen splash(pixmap);
	splash.show();
	app.processEvents();

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2(%3)\nLoading plugins...")).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

	if (!QDir::home().exists(".merkaartor"))
		QDir::home().mkdir(".merkaartor");
#if defined(Q_OS_WIN32)
	QDir pluginsDir = QDir(qApp->applicationDirPath() + "/" + STRINGIFY(PLUGINS_DIR));

	// Fixes MacOSX plugin dir (fixes #2253)
#elif defined(Q_OS_MAC)
	QDir pluginsDir = QDir(qApp->applicationDirPath());
	pluginsDir.cdUp();
	pluginsDir.cd("plugins");
#else
	QDir pluginsDir = QDir(STRINGIFY(PLUGINS_DIR));
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

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2(%3)\nInitializing...")).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

	MainWindow Main;


#ifdef _MOBILE
	app.setActiveWindow(&Main);
	Main.showMaximized();
#else
	Main.show();
#endif
	Main.loadFiles(fileNames);

	if (fileNames.isEmpty())
		QDir::setCurrent(M_PREFS->getWorkingDir());

	Main.show();
	splash.finish(&Main);

	int x = app.exec();

	qDebug() << "**** " << QDateTime::currentDateTime().toString(Qt::ISODate) << " -- Ending " << QString("Merkaartor %1%2(%3)").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)).arg(STRINGIFY(SVNREV));
	if(pLogFile)
		fclose(pLogFile);

	return x;
}

