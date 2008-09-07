/***************************************************************************
 *   Copyright (C) 2005 by Robin Gingras                                   *
 *   neozenkai@cox.net                                                     *
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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

#include <QPainter>
#include <QPen>
#include <QBrush>

#include "qgpssatellitetracker.h"

QGPSSatelliteTracker::QGPSSatelliteTracker(QWidget *parent) 
: QWidget(parent), Heading(0)
{
}

void QGPSSatelliteTracker::setSatellites(const std::vector<Satellite>& aList)
{
	List = aList;
	update();
}

void QGPSSatelliteTracker::setHeading(int x)
{
	Heading = x;
	update();
}

/**
 * private void QGPSSatelliteTracker::paintEvent()
 *
 * Does the actual drawing of the widget. Reads the satArray array to
 * get satellite positions and PRNs. Reads the satActive array to get
 * which satellites are being used.
 *
 * @param QPaintEvent * Ignored by this function
 */

void QGPSSatelliteTracker::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(width()/2,height()/2);

    int rad = width();
    if (height() < rad)
	    rad = height();
    rad /= 2;
    rad -= 2;

    painter.setPen(QPen(palette().mid(), 1, Qt::SolidLine));
    painter.setBrush(QBrush(Qt::black, Qt::NoBrush));

    // first paint the two reference circles, one at 0 degrees, and
    // the other at 45 degrees
    painter.drawEllipse(-rad,-rad, rad*2,rad*2);
    painter.drawEllipse(-rad/2,-rad/2,rad,rad);

    // now the reference lines, one vertical and the other horizontal
    painter.drawLine(-rad,0,rad,0);
    painter.drawLine(0,-rad,0,rad);

   // plot heading
   if (List.size())
    {
    	painter.setPen(QPen(QColor(240,32,32),3,Qt::SolidLine,Qt::RoundCap));
	float Alfa = Heading-90;
	Alfa = Alfa*3.141592/180;
	float fx = cos(Alfa)*rad*3/4;
	float fy = sin(Alfa)*rad*3/4;
	painter.drawLine(0,0,fx,fy);
	painter.drawLine(fx,fy,
		fx+cos(Alfa+3.1415*5/6)*8,fy+sin(Alfa+3.1415*5/6)*8);
	painter.drawLine(fx,fy,
		fx+cos(Alfa-3.1415*5/6)*8,fy+sin(Alfa-3.1415*5/6)*8);
    }


    // now plot the satellites
    painter.setPen(QPen(palette().foreground(), 2, Qt::SolidLine));
    painter.setBrush(QBrush(palette().foreground()));

    int x,y;
    for (unsigned int i=0; i<List.size(); ++i)
    {
        if(List[i].SignalStrength > 0)
        {
            getCoordsFromPos(rad,List[i].Elevation,List[i].Azimuth, x, y);

            painter.drawEllipse(x - 2, y - 2, 4, 4);
            painter.drawText(x + 5, y - 1, QString("%1").arg(List[i].Id));
        }
    }
}

/**
 * private void QGPSSatelliteTracker::getCoordsFromPos()
 *
 * Inputs and elevation and azimuth, and translates them to coordinates
 * that QPainter can use. This one took some thought, but I think it's
 * correct. Satellites jump around, maybe a problem?
 *
 * @param int elevation Elevation of the satellite in degrees, 0 - 90
 * @param int azimuth   Azimuth of the satellite in degrees, 0 - 360
 * @param int &x        Pointer to x-coordinate
 * @param int &y        Pointer to y-coordinate
 */

void QGPSSatelliteTracker::getCoordsFromPos(int rad, int elevation, int azimuth, int &x, int &y)
{
    int theta = azimuth-90;

    // since the "origin" of the sky is 90 (directly overhead), reverse
    // elevation to make the origin 0 (an elevation of 90 becomes 0, etc)
    elevation = 90 - elevation;

    // you should know this (slept too much in trig)
    x = cos(theta*M_PI/180) * elevation * rad / 90;
    y = sin(theta*M_PI/180) * elevation * rad / 90;
}
