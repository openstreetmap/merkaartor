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
#include "circlepoint.h"

CirclePoint::CirclePoint(double x, double y, int radius, QString name, Alignment alignment, QPen* pen)
 : Point(x, y, name, alignment)
{
	size = QSize(radius, radius);
	mypixmap = new QPixmap(radius+1, radius+1);
	mypixmap->fill(Qt::transparent);
	QPainter painter(mypixmap);
	if (pen != 0)
	{
		painter.setPen(*pen);
	}
	painter.drawEllipse(0,0,radius, radius);
}

CirclePoint::CirclePoint(double x, double y, QString name, Alignment alignment, QPen* pen)
	: Point(x, y, name, alignment)
{
	int radius = 10;
	size = QSize(radius, radius);
	mypixmap = new QPixmap(radius+1, radius+1);
	mypixmap->fill(Qt::transparent);
	QPainter painter(mypixmap);
	if (pen != 0)
	{
		painter.setPen(*pen);
	}
	painter.drawEllipse(0,0,radius, radius);
}

CirclePoint::~CirclePoint()
{
	delete mypixmap;
}

void CirclePoint::setPen(QPen* pen)
{
	this->pen = pen;
	mypixmap = new QPixmap(size.width()+1, size.height()+1);
	mypixmap->fill(Qt::transparent);
	QPainter painter(mypixmap);
	painter.setPen(*pen);
	painter.drawEllipse(0,0, size.width(), size.height());
}
