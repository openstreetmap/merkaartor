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

#ifndef CITY_H
#define CITY_H

#include <QString>
#include <QRect>

class City
{
public:
    City(const QString & code);

    void setName(const QString &name);
    QString name();

    void setDepartement(const QString &department);
    QString department();

    QString code();

    void setGeometry(const QRect &geom);
    QRect geometry();

    int tileRows();
    int tileColumns();
    QRect tileGeometry(int row, int column);

private:
    QString m_name;
    QString m_department;
    QString m_code;
    QRect m_geometry;
};

#endif // CITY_H
