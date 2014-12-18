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

#include <QtWidgets/QApplication>
#include <QDesktopServices>
#include <QDebug>
#include <QDir>
#include "cadastrebrowser.h"
#include "searchdialog.h"

int main(int argc, char *argv[])
{
    QApplication::setGraphicsSystem("raster");
    QApplication a(argc, argv);
    a.setApplicationName("Qadastre");
    a.setApplicationVersion("0.1");
    if (!QDir(QDesktopServices::storageLocation (QDesktopServices::DataLocation)).exists()) {
        QDir().mkpath(QDesktopServices::storageLocation (QDesktopServices::DataLocation));
    }
    CadastreBrowser w;
    w.show();
    return a.exec();
}
