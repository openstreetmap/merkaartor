/*
    This file is part of Qadastre.

    Qadastre is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Qadastre is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Qadastre. If not, see <http://www.gnu.org/licenses/>.
*/

#include "cadastrebrowser.h"
#include "ui_cadastrebrowser.h"

#include "cadastrewrapper.h"
#include "searchdialog.h"
#include <QDebug>
#include <QProgressDialog>
#include <QGraphicsPixmapItem>
#include "tile.h"

CadastreBrowser::CadastreBrowser(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::CadastreBrowser),
    m_scene(this), m_currentZoom(1.0)
{
    ui->setupUi(this);
    ui->graphicsView->setScene(&m_scene);
}

CadastreBrowser::~CadastreBrowser()
{
    delete ui;
}

void CadastreBrowser::changeEvent(QEvent *e)
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

void CadastreBrowser::on_actionLoad_triggered()
{
    SearchDialog *dial = new SearchDialog(this);
    dial->setModal(true);
    if (dial->exec()) {
        QProgressDialog *pDial = new QProgressDialog(this);
        pDial->setMaximum(0);
        pDial->setMinimum(0);
        pDial->setValue(0);
        pDial->show();
        QString code = dial->cityCode();
        qDebug() << code;
        City my_city = CadastreWrapper::instance()->requestCity(code);
        qDebug() << my_city.code();
        qDebug() << my_city.name();
        qDebug() << my_city.geometry();
        CadastreWrapper::instance()->downloadTiles(my_city);
        for (int r = 0 ; r < my_city.tileRows() ; ++r) {
            for (int c = 0 ; c < my_city.tileColumns() ; ++c) {
                QString tileFile = CadastreWrapper::instance()->tileFile(my_city.code(), r, c);
                qDebug() << tileFile;
                Tile *tile = new Tile(tileFile);
                tile->setX(600 * c);
                tile->setY(600 * r);
                m_scene.addItem(tile);
            }
        }
        pDial->hide();
        pDial->deleteLater();
    }
}

void CadastreBrowser::on_zoomSlider_valueChanged(int value)
{
    qreal requestedScale = value/100.;
    qDebug() << requestedScale << m_currentZoom << requestedScale / m_currentZoom;

    ui->graphicsView->scale(requestedScale / m_currentZoom, requestedScale / m_currentZoom);

    m_currentZoom = requestedScale;
}
