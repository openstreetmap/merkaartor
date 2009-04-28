#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QLibraryInfo>
#include <QSplashScreen>
#include <QNetworkProxy>

#include "MainWindow.h" 
#include "Preferences/MerkaartorPreferences.h"

#include "IMapAdapter.h"

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

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

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2\nLoading plugins...")).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

	if (!QDir::home().exists(".merkaartor"))
		QDir::home().mkdir(".merkaartor");
#if defined(Q_OS_UNIX)
	QDir pluginsDir = QDir(qApp->applicationDirPath());
	if (!pluginsDir.exists("plugins")) {
		pluginsDir = QDir(pluginsDir.absolutePath().remove("/bin").append("/lib/Merkaartor"));
	}
#else
	QDir pluginsDir = QDir(qApp->applicationDirPath());
#endif
	QCoreApplication::addLibraryPath(pluginsDir.path() + "/plugins");

	pluginsDir.cd("plugins");

	pluginsDir.cd("background");
	foreach (QString fileName, pluginsDir.entryList(QDir::Files)) {
		QPluginLoader loader(pluginsDir.absoluteFilePath(fileName));
		QObject *plugin = loader.instance();
		if (plugin) {
			IMapAdapter *plg = qobject_cast<IMapAdapter *>(plugin);
			if (plg)
				M_PREFS->addBackgroundPlugin(plg);
		}
	}

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2\nInitializing...")).arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

#if defined(Q_OS_MAC)
	QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("plugins");
	QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

	MainWindow Main;


#ifdef _MOBILE
	app.setActiveWindow(&Main);
	Main.showMaximized();
#else
	Main.show();
#endif
	QStringList fileNames = QCoreApplication::arguments();
	fileNames.removeFirst();

	Main.loadFiles(fileNames);

	if (fileNames.isEmpty())
		QDir::setCurrent(MerkaartorPreferences::instance()->getWorkingDir());

	if (M_PREFS->M_PREFS->getProxyUse()) {
		QNetworkProxy proxy;
		proxy.setType(QNetworkProxy::HttpCachingProxy);
		proxy.setHostName(M_PREFS->getProxyHost());
		proxy.setPort(M_PREFS->getProxyPort());
		proxy.setUser(M_PREFS->getProxyUser());
		proxy.setPassword(M_PREFS->getProxyPassword());
		QNetworkProxy::setApplicationProxy(proxy);
	}

	Main.show();
	splash.finish(&Main);

	int x = app.exec();
	return x;
}


