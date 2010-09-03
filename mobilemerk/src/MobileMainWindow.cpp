#include "MobileMainWindow.h"
#include "ui_MobileMainWindow.h"

#include "MapView.h"

//#define START_COORDBOX CoordBox(Coord(COORD_MAX/4, -COORD_MAX/4), Coord(-COORD_MAX/4, COORD_MAX/4))
#define START_COORDBOX CoordBox(Coord(50.8607371, 4.3314877), Coord(50.8296372, 4.3802123)) // BXL

class MainWindowPrivate
{
public:
    MainWindowPrivate()
        : theView(0)
    {}
public:
    MapView* theView;
};

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    p = new MainWindowPrivate;
    ui->setupUi(this);
    p->theView = new MapView(this);
    setCentralWidget(p->theView);

    QTimer::singleShot(0, this, SLOT(initialize()));
}

void MainWindow::initialize()
{
    p->theView->setViewport(START_COORDBOX, p->theView->rect());
}

MainWindow::~MainWindow()
{
    delete p->theView;
    delete ui;
}

void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
