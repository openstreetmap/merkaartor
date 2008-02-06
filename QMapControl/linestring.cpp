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
#include "linestring.h"

LineString::LineString()
 : Curve()
{
	GeometryType = "LineString";
}

LineString::LineString(QList<Point*> const points, QString name, QPen* pen)
	:Curve(name)
{
	this->pen = pen;
	LineString();
	setPoints(points);
	
	//TODO: bremst stark
// 	pen.setStyle(Qt::DashDotDotLine);
}

LineString::~LineString()
{
}

// Geometry		LineString::Clone(){}

// Point LineString::EndPoint(){}
// Point LineString::StartPoint(){}
// Point LineString::Value(){}


void 				LineString::addPoint(Point* point)
{
	vertices.append(point);
}

QList<Point*>	LineString::getPoints()
{
	return vertices;
}

void				LineString::setPoints(QList<Point*> points)
{
	for (int i=0; i<points.size(); i++)
	{
		points.at(i)->setParentGeometry(this);
	}
	vertices = points;
}

void LineString::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &screensize, const QPoint offset)
{
	if (!visible)
		return;
	
	QPolygon p = QPolygon();
	
	QPointF c;
	for (int i=0; i<vertices.size(); i++)
	{
		c = vertices[i]->getCoordinate();
		p.append(mapadapter->coordinateToDisplay(c));
	}
	if (pen != 0)
	{
		painter->save();
		painter->setPen(*pen);
	}
	painter->drawPolyline(p);
	if (pen != 0)
	{
		painter->restore();
	}
	for (int i=0; i<vertices.size(); i++)
	{
		vertices[i]->draw(painter, mapadapter, screensize, offset);
	}
}

int LineString::getNumberOfPoints() const
{
	return vertices.count();
}
bool LineString::Touches(Point* geom, const MapAdapter* mapadapter)
{
// 	qDebug() << "LineString::Touches Point";
	touchedPoints.clear();
	bool touches = false;
	for (int i=0; i<vertices.count(); i++)
	{
			// use implementation from Point
		if (vertices.at(i)->Touches(geom, mapadapter))
		{
			touchedPoints.append(vertices.at(i));
			
			touches = true;
		}
	}
	if (touches)
	{
		emit(geometryClickEvent(this, QPoint(0,0)));
	}
	return touches;
}
bool LineString::Touches(Geometry* geom, const MapAdapter* mapadapter)
{
// 	qDebug() << "LineString::Touches Geom";
	touchedPoints.clear();
	
	return false;
}

QList<Geometry*> LineString::getClickedPoints()
{
	return touchedPoints;
}
bool LineString::hasPoints() const
{
	return vertices.size() > 0 ? true : false;
}
bool LineString::hasClickedPoints() const
{
	return touchedPoints.size() > 0 ? true : false;
}

QRectF	LineString::getBoundingBox()
{
	double minlon=180;
	double maxlon=-180;
	double minlat=90;
	double maxlat=-90;
	for (int i=0; i<vertices.size(); i++)
	{
		Point* tmp = vertices.at(i);
		if (tmp->getLongitude() < minlon) minlon = tmp->getLongitude();
		if (tmp->getLongitude() > maxlon) maxlon = tmp->getLongitude();
		if (tmp->getLatitude() < minlat) minlat = tmp->getLatitude();
		if (tmp->getLatitude() > maxlat) maxlat = tmp->getLatitude();
	}
	QPointF min = QPointF(minlon, minlat);
	QPointF max = QPointF(maxlon, maxlat);
	QPointF dist = max - min;
	QSizeF si = QSizeF(dist.x(), dist.y());
	
	return QRectF(min, si);
	
}
