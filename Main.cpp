#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include <QTranslator>
#include <QLocale>

#include "MainWindow.h" 

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

	QCoreApplication::setOrganizationName("BartVanhauwaert");
	QCoreApplication::setOrganizationDomain("www.irule.be");
	QCoreApplication::setApplicationName("Merkaartor");

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

	const QStringList & params = QCoreApplication::arguments();
	if (params.size() == 2)
		Main.loadFile(params.at(1));
	
	Main.show();

	return app.exec();
}


