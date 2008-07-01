#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QTranslator>
#include <QLocale>

#include "MainWindow.h" 
#include "Preferences/MerkaartorPreferences.h"

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

	QCoreApplication::setOrganizationName("BartVanhauwaert");
	QCoreApplication::setOrganizationDomain("www.irule.be");
	QCoreApplication::setApplicationName("Merkaartor");

#if defined(Q_OS_MAC)
	QDir dir(QApplication::applicationDirPath());
	dir.cdUp();
	dir.cd("plugins");
	QApplication::setLibraryPaths(QStringList(dir.absolutePath()));
#endif
	QTranslator qtTranslator;
	qtTranslator.load("qt_" + QLocale::system().name()
#ifdef TRANSDIR_SYSTEM
        , TRANSDIR_SYSTEM
#endif
        );
	app.installTranslator(&qtTranslator);

	QTranslator merkaartorTranslator;
	merkaartorTranslator.load("merkaartor_" + QLocale::system().name()
#ifdef TRANSDIR_MERKAARTOR
        , TRANSDIR_MERKAARTOR
#endif
        );
	app.installTranslator(&merkaartorTranslator);

	MainWindow Main;

	QStringList fileNames = QCoreApplication::arguments();
	fileNames.removeFirst();
	Main.loadFiles(fileNames);

	if (fileNames.isEmpty())
		QDir::setCurrent(MerkaartorPreferences::instance()->getWorkingDir());

	Main.show();

	return app.exec();
}


