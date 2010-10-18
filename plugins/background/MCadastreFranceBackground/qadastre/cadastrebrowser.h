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

#ifndef CADASTREBROWSER_H
#define CADASTREBROWSER_H

#include <QMainWindow>
#include <QGraphicsScene>

namespace Ui {
    class CadastreBrowser;
}

class CadastreBrowser : public QMainWindow {
    Q_OBJECT
public:
    CadastreBrowser(QWidget *parent = 0);
    ~CadastreBrowser();

protected:
    void changeEvent(QEvent *e);

private:
    Ui::CadastreBrowser *ui;
    QGraphicsScene m_scene;
    qreal m_currentZoom;

private slots:
    void on_zoomSlider_valueChanged(int value);
    void on_actionLoad_triggered();
};

#endif // CADASTREBROWSER_H
