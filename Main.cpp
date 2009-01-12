#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QLibraryInfo>

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

	int x = app.exec();
	return x;
}


