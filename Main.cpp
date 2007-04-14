#include <QtGui/QApplication>
#include <QtGui/QMessageBox> 

#include "MainWindow.h" 

int main(int argc, char** argv)
{
	QApplication app(argc,argv);

	QCoreApplication::setOrganizationName("BartVanhauwaert");
	QCoreApplication::setOrganizationDomain("www.irule.be");
	QCoreApplication::setApplicationName("Merkaartor");

	MainWindow Main;
	Main.show();
	return app.exec();
}


