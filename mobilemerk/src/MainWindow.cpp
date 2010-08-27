#include "MainWindow.h"
#include "ui_MainWindow.h"

#include "MapView.h"

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
