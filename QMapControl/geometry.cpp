/***************************************************************************
 *   Copyright (C) 2007 by Kai Winter   *
 *   kaiwinter@gmx.de   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "geometry.h"

Geometry::Geometry(QString name)
	: GeometryType("Geometry"), parentGeometry(0), pen(0), visible(true), name(name)
{
}


Geometry::~Geometry()
{
}

QString		Geometry::getName() const
{
	return name;
}
Geometry*	Geometry::getParentGeometry() const
{
	return parentGeometry;
}
void	Geometry::setParentGeometry(Geometry* geom)
{
	parentGeometry = geom;
}
bool			Geometry::hasPoints() const
{
	return false;
}
bool			Geometry::hasClickedPoints() const
{
	return false;
}
QList<Geometry*> Geometry::getClickedPoints()
{
	QList<Geometry*> tmp;
	return tmp;
}

bool Geometry::isVisible() const
{
	return visible;
}
void Geometry::setVisible(bool visible)
{
	this->visible = visible;
	emit(updateRequest(getBoundingBox()));
}

void Geometry::setName(QString name)
{
	this->name = name;
}

void Geometry::setPen(QPen* pen)
{
	this->pen = pen;
}
QPen* Geometry::getPen() const
{
	return pen;
}
