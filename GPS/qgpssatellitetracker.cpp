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

#include <QPainter>
#include <QPen>
#include <QBrush>

#include "qgpssatellitetracker.h"

QGPSSatelliteTracker::QGPSSatelliteTracker(QWidget *parent) : QWidget(parent)
{
	resetSatInfo();
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
    
    painter.setPen(QPen(palette().mid(), 1, Qt::SolidLine));
    painter.setBrush(QBrush(Qt::black, Qt::NoBrush));
    
    // first paint the two reference circles, one at 0 degrees, and
    // the other at 45 degrees
    painter.drawEllipse(rect().x(), rect().y(), rect().width()-1, rect().height()-1);
    painter.drawEllipse(
        rect().x() + ((rect().width()-1) / 4), 
        rect().y() + ((rect().height()-1) / 4), 
        (rect().width()-1) / 2, 
        (rect().height()-1) / 2);
    
    // now the reference lines, one vertical and the other horizontal
    painter.drawLine(rect().x(), (rect().height()-1) / 2, rect().width()-1, (rect().height()-1) / 2);
    painter.drawLine((rect().width()-1) / 2, rect().y(), (rect().width()-1) / 2, rect().height()-1);
    
    // now plot the satellites
    painter.setPen(QPen(palette().foreground(), 2, Qt::SolidLine));
    painter.setBrush(QBrush(palette().foreground()));
    
    for(int i = 0; i < 50; i ++)
    {
        int x, y;
        
        if(satArray[i][2] != 0)
        {
            getCoordsFromPos(satArray[i][0], satArray[i][1], x, y);
            
            painter.drawEllipse(x - 2, y - 2, 4, 4);
            painter.drawText(x + 5, y - 1, QString("%1").arg(i));
        }
    }
}

void QGPSSatelliteTracker::resetSatInfo()
{
    for(int i = 0; i < 50; i ++)
    {
        satArray[i][0] = satArray[i][1] = satArray[i][2] = 0;
    }
}

/**
 * private void QGPSSatelliteTracker::setSatInfo()
 *
 * Stores satellite position in the satArray
 */
 
void QGPSSatelliteTracker::setSatInfo(int index, int elevation, int azimuth, int snr)
{
    satArray[index][0] = elevation;
    satArray[index][1] = azimuth;
    satArray[index][2] = snr;
    
    update();
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
 
void QGPSSatelliteTracker::getCoordsFromPos(int elevation, int azimuth, int &x, int &y)
{
    int theta;

    // translate the azimuth angle to an angle relative to the x-axis
    // i.e., "snap" it to the x-axis
    if(azimuth > 270)
        theta = azimuth - 270;
    else if(azimuth > 180)
        theta = 270 - azimuth;
    else if(azimuth > 90)
        theta = azimuth - 90;
    else
        theta = 90 - azimuth;
    
    // since the "origin" of the sky is 90 (directly overhead), reverse
    // elevation to make the origin 0 (an elevation of 90 becomes 0, etc)
    elevation = 90 - elevation;
    
    // you should know this (slept too much in trig)
    x = (cos(theta*M_PI/180)) * elevation;
    y = (sin(theta*M_PI/180)) * elevation;
    
    // here we need to make sure that coordinates in quadrants II, III, and IV
    // have the proper signs
    if(azimuth > 270) {
        x = x - (2 * x);
    }
    else if(azimuth > 180) {
        x = x - (2 * x);
        y = y - (2 * y);
    }
    else if(azimuth > 90) {
        y = y - (2 * y);
    }
    
    // scale our x and y to the size of the widget
    x = ((double)rect().width() / 180.0) * x;
    y = ((double)rect().height() / 180.0) * y;
    
    // translate our coordinates by shifting the plane right and up
    // to match the coordinate plane of the widget (to prevent drawing
    // only one quarter of the plane onto the widget
    x = (rect().width() / 2) + x;
    y = (rect().height() / 2) + y;
    
    // mirror y since QPainter's positive-y goes down and the trigonometric
    // positive-y goes up
    y = rect().height() - y;
}
