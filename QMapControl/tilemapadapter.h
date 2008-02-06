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
#ifndef TILEMAPADAPTER_H
#define TILEMAPADAPTER_H

#include "mapadapter.h"
//! MapAdapter for servers with image tiles
/*!
 * Use this derived MapAdapter to display maps from OpenStreetMap
 *	@author Kai Winter <kaiwinter@gmx.de>
*/
class TileMapAdapter : public MapAdapter
{
Q_OBJECT
public:
	//! constructor
	/*!
	 * Sample of a correct initialization of a MapAdapter:<br/>
	 * TileMapAdapter* ta = new TileMapAdapter("192.168.8.1", "/img/img_cache.php/%1/%2/%3.png", 256, 0,17);<br/>
 	 * The placeholders %1, %2, %3 stands for x, y, z<br/>
 	 * The minZoom is 0 (means the whole world is visible). The maxZoom is 17 (means it is zoomed in to the max)
 	 * @param host The servers URL
 	 * @param serverPath The path to the tiles with placeholders
 	 * @param tilesize the size of the tiles
 	 * @param minZoom the minimum zoom level
 	 * @param maxZoom the maximum zoom level
	 */
	TileMapAdapter(const QString& host, const QString& serverPath, int tilesize, int minZoom = 0, int maxZoom = 17);
	
	virtual ~TileMapAdapter();
	
	virtual QPoint		coordinateToDisplay(const QPointF&) const;
	virtual QPointF	displayToCoordinate(const QPoint&) const;
	
	double PI;
	
protected:
	double rad_deg(double) const;
	double deg_rad(double) const;
	
	virtual bool isValid(int x, int y, int z) const;
	virtual void zoom_in();
	virtual void zoom_out();
	virtual QString getQuery(int x, int y, int z) const;
	virtual int tilesonzoomlevel(int zoomlevel) const;
	virtual int getxoffset(int x) const;
	virtual int getyoffset(int y) const;
};

#endif
