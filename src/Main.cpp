#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QLibraryInfo>
#include <QSplashScreen>

#include "MainWindow.h" 
#include "Preferences/MerkaartorPreferences.h"

#include "IMapAdapter.h"

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

	QCoreApplication::setOrganizationName("BartVanhauwaert");
	QCoreApplication::setOrganizationDomain("www.irule.be");
	QCoreApplication::setApplicationName("Merkaartor");

	QPixmap pixmap(":/Splash/Mercator_splash.png");
	QSplashScreen splash(pixmap);
	splash.show();
	app.processEvents();

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2\nLoading plugins...")).arg(VERSION).arg(REVISION), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

	if (!QDir::home().exists(".merkaartor"))
		QDir::home().mkdir(".merkaartor");
#if defined(Q_OS_UNIX)
	QDir pluginsDir = QDir(qApp->applicationDirPath());
	if (pluginsDir.absolutePath().contains("/bin")) {
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

	splash.showMessage(QString(app.translate("Main", "Merkaartor v%1%2\nInitializing...")).arg(VERSION).arg(REVISION), Qt::AlignBottom | Qt::AlignHCenter, Qt::black);
	app.processEvents();

#if defined(Q_OS_MAC)
	QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("plugins");
	QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif

	MainWindow Main;


#ifdef _MOBILE
//	Main.showMaximized();
	Main.showFullScreen();
#else
	Main.show();
#endif
	QStringList fileNames = QCoreApplication::arguments();
	fileNames.removeFirst();

	Main.loadFiles(fileNames);

	if (fileNames.isEmpty())
		QDir::setCurrent(MerkaartorPreferences::instance()->getWorkingDir());

	Main.show();
	splash.finish(&Main);

	int x = app.exec();
	return x;
}


