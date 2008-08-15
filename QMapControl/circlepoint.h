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
#ifndef CIRCLEPOINT_H
#define CIRCLEPOINT_H

#include "point.h"

//! Draws a circle into the map
/*! This is a conveniece class for Point.
 * It configures the pixmap of a Point to draw a circle.
 * A QPen could be used to change the color or line-width of the circle
 * 
 * @author Kai Winter <kaiwinter@gmx.de>
*/
class CirclePoint : public Point
{
	public:
		//!
		/*!
		 * 
		 * @param x longitude
		 * @param y latitude
		 * @param name name of the circle point
		 * @param alignment alignment (Middle or TopLeft)
		 * @param pen QPen for drawing
		 */
		CirclePoint(double x, double y, QString name = QString(), Alignment alignment = Middle, QPen* pen=0);
		
		//!
		/*!
		 * 
		 * @param x longitude
		 * @param y latitude
		 * @param radius the radius of the circle
		 * @param name name of the circle point
		 * @param alignment alignment (Middle or TopLeft)
		 * @param pen QPen for drawing
		 */
		CirclePoint(double x, double y, int radius = 10, QString name = QString(), Alignment alignment = Middle, QPen* pen=0);
		virtual ~CirclePoint();
		
		//! sets the QPen which is used for drawing the circle
		/*!
		 * A QPen can be used to modify the look of the drawn circle
		 * @param pen the QPen which should be used for drawing
		 * @see http://doc.trolltech.com/4.3/qpen.html
		 */
		virtual void setPen(QPen* pen);

};

#endif
