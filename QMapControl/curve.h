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
#ifndef CURVE_H
#define CURVE_H

#include "geometry.h"
#include "point.h"

//! A Curve Geometry, implemented to fullfil OGC Spec
/*!
 * The Curve class is used by LineString as parent class.
 * This class could not be used directly.
 * 
 * From the OGC Candidate Implementation Specification:
 * "A Curve is a 1-dimensional geometric object usually stored as a sequence of Points, with the subtype of Curve
 * specifying the form of the interpolation between Points. This specification defines only one subclass of Curve,
 * LineString, which uses a linear interpolation between Points."
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class Curve : public Geometry
{
	Q_OBJECT
	public:
		virtual ~Curve();
	
		double Length;

// 		virtual	Geometry	Clone();
// 		virtual	QRectF	GetBoundingBox();
		
// 		virtual Point EndPoint() = 0;
// 		virtual Point StartPoint() = 0;
// 		virtual Point Value() = 0;
		
	protected:
		Curve(QString name = QString());
		virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &screensize, const QPoint offset) = 0;
};

#endif
