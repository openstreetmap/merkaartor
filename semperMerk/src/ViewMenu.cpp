#include "ViewMenu.h"
#include "ui_ViewMenu.h"

ViewMenu::ViewMenu(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ViewMenu)
{
    ui->setupUi(this);
}

ViewMenu::~ViewMenu()
{
    delete ui;
}

void ViewMenu::changeEvent(QEvent *e)
{
    QDialog::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

void ViewMenu::on_btMap_clicked()
{
    emit mapRequested();
    accept();
}

void ViewMenu::on_btGps_clicked()
{
    emit gpsRequested();
    accept();
}
