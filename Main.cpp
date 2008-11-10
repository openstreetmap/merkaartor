#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QTranslator>
#include <QLocale>

#include "MainWindow.h" 
#include "Preferences/MerkaartorPreferences.h"
#ifdef CUSTOM_STYLE
	#include "QtStyles/skulpture/skulpture.h"
#endif

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

#ifdef CUSTOM_STYLE
	if (M_PREFS->getMerkaartorStyle())
		QApplication::setStyle(new SkulptureStyle);
#endif

	QTranslator* qtTranslator = 0;
	QTranslator* merkaartorTranslator = 0;

	QString DefaultLanguage = getDefaultLanguage();
	if (DefaultLanguage != "-")
	{

		if (DefaultLanguage == "")
			DefaultLanguage = QLocale::system().name();

		qtTranslator = new QTranslator;
		qtTranslator->load("qt_" + DefaultLanguage
	#ifdef TRANSDIR_SYSTEM
			, TRANSDIR_SYSTEM
	#endif
			);
		app.installTranslator(qtTranslator);

		merkaartorTranslator = new QTranslator;
		merkaartorTranslator->load("merkaartor_" + DefaultLanguage
	#ifdef TRANSDIR_MERKAARTOR
			, TRANSDIR_MERKAARTOR
	#endif
			);
		app.installTranslator(merkaartorTranslator);
	}

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
	delete qtTranslator;
	delete merkaartorTranslator;
	return x;
}


