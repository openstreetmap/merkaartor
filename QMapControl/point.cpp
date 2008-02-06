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
#include "point.h"

Point::Point()
{}
Point::Point(const Point& point)
	:Geometry(point.getName()), X(point.getLongitude()), Y(point.getLatitude())
{
	visible = point.isVisible();
	mywidget = 0;
	mypixmap = 0;
	this->pen = point.pen;
	homelevel = -1;
	minsize = QSize(-1,-1);
	maxsize = QSize(-1,-1);
}
//protected: 
Point::Point(double x, double y, QString name, enum Alignment alignment)
	: Geometry(name), X(x), Y(y), alignment(alignment)
{
	GeometryType = "Point";
	mywidget = 0;
	mypixmap = 0;
	visible = true;
	homelevel = -1;
	minsize = QSize(-1,-1);
	maxsize = QSize(-1,-1);
}

Point::Point(double x, double y, QWidget* widget, QString name, enum Alignment alignment)
	: Geometry(name), X(x), Y(y), mywidget(widget), alignment(alignment)
{
// 	Point(x, y, name, alignment);
	GeometryType = "Point";
	mypixmap = 0;
	visible = true;
	size = widget->size();
	homelevel = -1;
	minsize = QSize(-1,-1);
	maxsize = QSize(-1,-1);
	mywidget->show();
}
Point::Point(double x, double y, QPixmap* pixmap, QString name, enum Alignment alignment)
	: Geometry(name), X(x), Y(y), mypixmap(pixmap), alignment(alignment)
{
	GeometryType = "Point";
	mywidget = 0;
	visible = true;
	size = pixmap->size();
	homelevel = -1;
	minsize = QSize(-1,-1);
	maxsize = QSize(-1,-1);
}
/*
Point& Point::operator=(const Point& rhs)
{
	if (this == &rhs)
		return *this;
	else
	{
		X = rhs.X;
		Y = rhs.Y;
		size = rhs.size;
		
		mywidget = rhs.mywidget;
		mypixmap = rhs.mypixmap;
		alignment = rhs.alignment;
		homelevel = rhs.homelevel;
		minsize = rhs.minsize;
		maxsize = rhs.maxsize;
	}
}
*/
Point::~Point()
{
	delete mywidget;
	delete mypixmap;
}

void Point::setVisible(bool visible)
{
	this->visible = visible;
	if (mywidget !=0)
	{
		mywidget->setVisible(visible);
	}
}

QRectF Point::getBoundingBox()
{
	//TODO: have to be calculated in relation to alignment...
	return QRectF(QPointF(X, Y), displaysize);
}

double Point::getLongitude() const
{
	return X;
}
double Point::getLatitude() const
{
	return Y;
}
QPointF Point::getCoordinate() const
{
	return QPointF(X, Y);
}

void Point::draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &viewport, const QPoint offset)
{
	if (!visible)
		return;
	
	if (homelevel > 0)
	{

		int currentzoom = mapadapter->getMaxZoom() < mapadapter->getMinZoom() ? mapadapter->getMinZoom() - mapadapter->getZoom() : mapadapter->getZoom();
				
// 		int currentzoom = mapadapter->getZoom();
		int diffzoom = homelevel-currentzoom;
		int viewheight = size.height();
		int viewwidth = size.width();
		viewheight = int(viewheight / pow(2, diffzoom));
		viewwidth = int(viewwidth / pow(2, diffzoom));
		
		if (minsize.height()!= -1 && viewheight < minsize.height())
			viewheight = minsize.height();
		else if (maxsize.height() != -1 && viewheight > maxsize.height())
			viewheight = maxsize.height();
		
		
		if (minsize.width()!= -1 && viewwidth < minsize.width())
			viewwidth = minsize.width();
		else if (maxsize.width() != -1 && viewwidth > maxsize.width())
			viewwidth = maxsize.width();
		
		
		displaysize = QSize(viewwidth, viewheight);
	}
	else
	{
		displaysize = size;
	}
	
	
	if (mypixmap !=0)
	{
		const QPointF c = QPointF(X, Y);
		QPoint point = mapadapter->coordinateToDisplay(c);

		if (viewport.contains(point))
		{
			QPoint alignedtopleft = getAlignedPoint(point);
			painter->drawPixmap(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height(), *mypixmap);
		}
		
	}
	else if (mywidget!=0)
	{
		drawWidget(mapadapter, offset);
	}
	
}

void Point::drawWidget(const MapAdapter* mapadapter, const QPoint offset)
{
	const QPointF c = QPointF(X, Y);
	QPoint point = mapadapter->coordinateToDisplay(c);
 	point -= offset;

	QPoint alignedtopleft = getAlignedPoint(point);
	mywidget->setGeometry(alignedtopleft.x(), alignedtopleft.y(), displaysize.width(), displaysize.height());
}

QPoint Point::getAlignedPoint(const QPoint point) const
{
	QPoint alignedtopleft;
	if (alignment == Middle)
	{
		alignedtopleft.setX(point.x()-displaysize.width()/2);
		alignedtopleft.setY(point.y()-displaysize.height()/2);
	}
	else if (alignment == TopLeft)
	{
		alignedtopleft.setX(point.x());
		alignedtopleft.setY(point.y());
	}
	else if (alignment == TopRight)
	{
		alignedtopleft.setX(point.x()-displaysize.width());
		alignedtopleft.setY(point.y());
	}
	else if (alignment == BottomLeft)
	{
		alignedtopleft.setX(point.x());
		alignedtopleft.setY(point.y()-displaysize.height());
	}
	else if (alignment == BottomRight)
	{
		alignedtopleft.setX(point.x()-displaysize.width());
		alignedtopleft.setY(point.y()-displaysize.height());
	}
	return alignedtopleft;
}


bool Point::Touches(Point* p, const MapAdapter* mapadapter)
{
	if (this->isVisible() == false)
		return false;
	if (mypixmap == 0)
		return false;
	
	QPointF c = p->getCoordinate();
				// coordinate nach pixel umrechnen
	QPoint pxOfPoint = mapadapter->coordinateToDisplay(c);
		// size/2 Pixel toleranz aufaddieren
	QPoint p1;
	QPoint p2;

	switch (alignment)
	{
		case Middle:
			p1 = pxOfPoint - QPoint(displaysize.width()/2,displaysize.height()/2);
			p2 = pxOfPoint + QPoint(displaysize.width()/2,displaysize.height()/2);
			break;
		case TopLeft:
			p1 = pxOfPoint - QPoint(displaysize.width(),displaysize.height());
			p2 = pxOfPoint;
			break;
		case TopRight:
			p1 = pxOfPoint - QPoint(0, displaysize.height());
			p2 = pxOfPoint + QPoint(displaysize.width(),0);
			break;
		case BottomLeft:
			p1 = pxOfPoint - QPoint(displaysize.width(), 0);
			p2 = pxOfPoint + QPoint(0, displaysize.height());
			break;			
		case BottomRight:
			p1 = pxOfPoint;
			p2 = pxOfPoint + QPoint(displaysize.width(), displaysize.height());
			break;
	}

		// "Bounding Box" in koordinate umrechnen
	QPointF c1 = mapadapter->displayToCoordinate(p1);
	QPointF c2 = mapadapter->displayToCoordinate(p2);
		

	if(this->getLongitude()>=c1.x() && this->getLongitude()<=c2.x())
	{
		if (this->getLatitude()<=c1.y() && this->getLatitude()>=c2.y())
		{
			emit(geometryClickEvent(this, QPoint(0,0)));
			return true;
		}
	}
	return false;
}

void Point::setCoordinate(QPointF point)
{
// 	emit(updateRequest(this));
// 	emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
	X = point.x();
	Y = point.y();
// 	emit(updateRequest(this));
	emit(updateRequest(QRectF(X, Y, size.width(), size.height())));
	
	emit(positionChanged(this));
}
QList<Point*> Point::getPoints()
{
	//TODO: assigning temp?!
	QList<Point*> points;
	points.append(this);
	return points;
}

QWidget* Point::getWidget()
{
	return mywidget;
}

QPixmap* Point::getPixmap()
{
	return mypixmap;
}

void Point::setBaselevel(int zoomlevel)
{
	homelevel = zoomlevel;
}
void Point::setMinsize(QSize minsize)
{
	this->minsize = minsize;
}
void Point::setMaxsize(QSize maxsize)
{
	this->maxsize = maxsize;
}
Point::Alignment Point::getAlignment() const
{
	return alignment;
}
