/*
   This file is part of Qadastre.
   Copyright (C)  2010 Pierre Ducroquet <pinaraf@pinaraf.info>

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

#include "city.h"
#include <QDebug>

City::City()
{
}

City::City(const QString &code) :
    m_code(code)
{
}

QString City::name() const
{
    return m_name;
}

QString City::department() const
{
    return m_department;
}

QString City::projection() const
{
    return m_projection;
}

QString City::code() const
{
    return m_code;
}

QRect City::geometry() const
{
    return m_geometry;
}

void City::setName(const QString &name)
{
    m_name = name;
}

void City::setDepartement(const QString &department)
{
    m_department = department;
}

void City::setProjection(const QString &projection)
{
    m_projection = projection;
}

void City::setGeometry(const QRect &geom)
{
    m_geometry = geom;
}

// TODO: don't hardcode 100 meters here.
int City::tileRows() const
{
    return m_geometry.height()/100 + 1;
}

int City::tileColumns() const
{
    return m_geometry.width()/100 + 1;
}

QRect City::tileGeometry(int row, int column) const
{
    int left = m_geometry.left() / 100;
    left = left * 100;
    int top = m_geometry.top() / 100;
    top = top * 100;
    //top = top + 100;
    return QRect(left + column * 100, top - row * 100, 101, 101);
}
