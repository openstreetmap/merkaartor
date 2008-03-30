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
	qtTranslator.load("qt_" + QLocale::system().name());
	app.installTranslator(&qtTranslator);

	QTranslator merkaartorTranslator;
	merkaartorTranslator.load("merkaartor_" + QLocale::system().name());
	app.installTranslator(&merkaartorTranslator);

	MainWindow Main;

	const QStringList & params = QCoreApplication::arguments();
	if (params.size() == 2)
		Main.loadFile(params.at(1));
	
	Main.show();

	return app.exec();
}


