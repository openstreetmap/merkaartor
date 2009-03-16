/***************************************************************************
 *   Copyright (C) 2008 by Chris Browet                                    *
 *   cbro@semperpax.com                                                    *
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
#include <QtPlugin>

#include "yahoolegalmapadapter.h"

// {67CC0481-8C6A-4735-8666-BBA6A1B04E19}
static const QUuid theUid ( 0x67cc0481, 0x8c6a, 0x4735, 0x86, 0x66, 0xbb, 0xa6, 0xa1, 0xb0, 0x4e, 0x19);

YahooLegalMapAdapter::YahooLegalMapAdapter()
: TileMapAdapter("", "qrc:/Html/ymap.html?", 512, 17, 0)
{
	name = "Yahoo!";

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = pow(2, zoom+1.0);
}


YahooLegalMapAdapter::~YahooLegalMapAdapter()
{
}

QUuid YahooLegalMapAdapter::getId() const
{
	return QUuid(theUid);
}

IMapAdapter::Type YahooLegalMapAdapter::getType() const
{
	return IMapAdapter::BrowserBackground;
}


bool YahooLegalMapAdapter::isValid(int /* x */, int /* y */, int /* z */) const
{
	return true;
}

//bool YahooLegalMapAdapter::isValid(int x, int y, int z) const
//{
// 	if (x<0 || y<0 || z<0)
//		return false;
//
//	if ( (((x+1)*coord_per_x_tile) > 360) || (((y+1)*coord_per_y_tile) > 180) )
//		return false;
//
// 	return true;
//}

int YahooLegalMapAdapter::tilesonzoomlevel(int zoomlevel) const
{
	return int(pow(2, zoomlevel+1.0));
}

QString YahooLegalMapAdapter::getQuery(int i, int j, int /* z */) const
{
	QPointF ul = displayToCoordinate(QPoint(i*tilesize, j*tilesize));
	QPointF br = displayToCoordinate(QPoint((i+1)*tilesize, (j+1)*tilesize));
	return getQ(ul, br);
}

QString YahooLegalMapAdapter::getQ(QPointF ul, QPointF br) const
{
	return QString().append(serverPath)
						.append("WIDTH=").append(QString().setNum(tilesize))
						.append("&HEIGHT=").append(QString().setNum(tilesize))
						.append("&BBOX=")
						 .append(loc.toString(ul.x(),'f',6)).append(",")
						 .append(loc.toString(ul.y(),'f',6)).append(",")
						 .append(loc.toString(br.x(),'f',6)).append(",")
						 .append(loc.toString(br.y(),'f',6));
}

int YahooLegalMapAdapter::getyoffset(int y) const
{
	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;

	int tiles = int(pow(2, zoom+0.0));
	y = y*(-1)+tiles-1;
	return int(y);
}

Q_EXPORT_PLUGIN2(MYahooBackgroundPlugin, YahooLegalMapAdapter)
