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
#ifndef LINESTRING_H
#define LINESTRING_H

#include "curve.h"

//! A collection of Point objects to describe a line
/*!
 * A LineString is a Curve with linear interpolation between Points. Each consecutive pair of Points defines a Line segment.
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class LineString : public Curve
{
	Q_OBJECT
	public:
		LineString();
		//! constructor
		/*!
		 * The constructor of a LineString takes a list of Points to form a line.
		 * @param points a list of points
		 * @param name the name of the LineString
		 * @param pen a QPen can be used to modify the look of the line. 
		 * @see http://doc.trolltech.com/4.3/qpen.html
		 */
		LineString(QList<Point*> const points, QString name = QString(), QPen* pen = 0);
		virtual ~LineString();

		//! returns the points of the LineString
		/*!
		 * @return  a list with the points of the LineString
		 */
		QList<Point*>	getPoints();
		
		//! adds a point at the end of the LineString
		/*!
		 * @param point the point which should be added to the LineString
		 */
		void addPoint(Point* point);
		
		//! sets the given list as points of the LineString
		/*!
		 * @param points the points which should be set for the LineString
		 */
		void setPoints(QList<Point*> points);
		
		//! returns the number of Points the LineString consists of
		/*!
		 * @return the number of the LineString´s Points
		 */
		int getNumberOfPoints() const;
		
// 		virtual Geometry	Clone();
		virtual QRectF		getBoundingBox();
// 		virtual Point 		EndPoint();
// 		virtual Point 		StartPoint();
// 		virtual Point 		Value();
		
		//! returns true if the LineString has Childs
		/*!
		 * This is equal to: getNumberOfPoints() > 0
		 * @return true it the LineString has Childs (=Points)
		 * @see getClickedPoints()
		 */
		virtual bool hasPoints() const;
		
		//! returns true if the LineString has clicked Points
		/*!
		 * @return true if childs of a LineString were clicked
		 * @see getClickedPoints()
		 */
		virtual bool hasClickedPoints() const;
		
		//! returns the clicked Points
		/*!
		 * If a LineString was clicked it could be neccessary to figure out which of its points where clicked.
		 * Do do so the methods hasPoints() and getClickedPoints() can be used.
		 * When a point is added to a LineString the Point becomes its child.
		 * It is possible (depending on the zoomfactor) to click more than one Point of a LineString, so this method returns a list.
		 * @return the clicked Points of the LineString
		 */
		virtual QList<Geometry*> getClickedPoints();
		
	protected:
		virtual bool Touches(Geometry* geom, const MapAdapter* mapadapter);
		virtual bool Touches(Point* geom, const MapAdapter* mapadapter);		
		virtual void draw(QPainter* painter, const MapAdapter* mapadapter, const QRect &screensize, const QPoint offset);
		
	private:
		QList<Point*>	vertices;
		QList<Geometry*> 	touchedPoints;

};

#endif
