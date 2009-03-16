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

#ifndef QGPSSATELLITETRACKER_H
#define QGPSSATELLITETRACKER_H

#include <QWidget>

#include "SatelliteStrengthView.h"

class QGPSSatelliteTracker : public QWidget
{
    Q_OBJECT
    
    public:
    
        QGPSSatelliteTracker(QWidget *parent = 0);
    
        void paintEvent(QPaintEvent *);

	void setSatellites(const std::vector<Satellite>& aList);
	void setHeading(int x);
        
    private:
        void getCoordsFromPos(int rad, int elevation, int azimuth, int &x, int &y);
        
	std::vector<Satellite> List;
	int Heading;
};

#endif
