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
#include "tilemapadapter.h"

TileMapAdapter::TileMapAdapter(const QString& host, const QString& serverPath, int tilesize, int minZoom, int maxZoom)
	:MapAdapter(host, serverPath, tilesize, minZoom, maxZoom)
{
	PI = acos(-1.0);

// 	qDebug() << "creating adapter: min, max, current: " << minZoom << ", " << maxZoom << ", " << current_zoom << ", " << (min_zoom < max_zoom);

	/*
		Initialize the "substring replace engine". First the string replacement
		in getQuery was made by QString().arg() but this was very slow. So this
		splits the servers path into substrings and when calling getQuery the
		substrings get merged with the parameters of the URL.
		Pretty complicated, but fast.
	*/
	param1 = serverPath.indexOf("%1");
	param2 = serverPath.indexOf("%2");
	param3 = serverPath.indexOf("%3");

	int min = param1 < param2 ? param1 : param2;
	min = param3 < min ? param3 : min;

	int max = param1 > param2 ? param1 : param2;
	max = param3 > max ? param3 : max;

	int middle = param1+param2+param3-min-max;

	order[0][0] = min;
	if (min == param1)
		order[0][1] = 0;
	else if (min == param2)
		order[0][1] = 1;
	else
		order[0][1] = 2;

	order[1][0] = middle;
	if (middle == param1)
		order[1][1] = 0;
	else if (middle == param2)
		order[1][1] = 1;
	else
		order[1][1] = 2;

	order[2][0] = max;
	if (max == param1)
		order[2][1] = 0;
	else if(max == param2)
		order[2][1] = 1;
	else
		order[2][1] = 2;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);
	loc.setNumberOptions(QLocale::OmitGroupSeparator);
}

TileMapAdapter::~TileMapAdapter()
{
}
//TODO: rausziehen? ->MapAdapter?
void TileMapAdapter::zoom_in()
{
	if (min_zoom > max_zoom)
		current_zoom = current_zoom > max_zoom ? current_zoom-1 : max_zoom;
	else if (min_zoom < max_zoom)
		current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);//pow(2, zoom);

}
void TileMapAdapter::zoom_out()
{
	if (min_zoom > max_zoom)
		current_zoom = current_zoom < min_zoom ? current_zoom+1 : min_zoom;
	else if (min_zoom < max_zoom)
		current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);//pow(2, zoom);

}

double TileMapAdapter::deg_rad(double x) const
{
	return x * (PI/180.0);
}
double TileMapAdapter::rad_deg(double x) const
{
	return x * (180/PI);
}

QString TileMapAdapter::getQuery		(int x, int y, int z) const
{
	x = getxoffset(x);
	y = getyoffset(y);

	int a[3] = {z, x, y};
	return QString(serverPath).replace(order[2][0],2, loc.toString(a[order[2][1]]))
									  .replace(order[1][0],2, loc.toString(a[order[1][1]]))
									  .replace(order[0][0],2, loc.toString(a[order[0][1]]));

}

QPoint TileMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
// 	qDebug() << "numberOfTiles: " << numberOfTiles;

	double x = (coordinate.x()+180) * (numberOfTiles*tilesize)/360.;		// coord to pixel!
	double y = (1-(log(tan(PI/4+deg_rad(coordinate.y())/2)) /PI)) /2  * (numberOfTiles*tilesize);

	return QPoint(int(x), int(y));

}

QPointF TileMapAdapter::displayToCoordinate(const QPoint& point) const
{
	double longitude = (point.x()*(360/(numberOfTiles*tilesize)))-180;
	double latitude = rad_deg(atan(sinh((1-point.y()*(2/(numberOfTiles*tilesize)))*PI)));

	return QPointF(longitude, latitude);

}

bool TileMapAdapter::isValid(int x, int y, int z) const
{
	if (max_zoom < min_zoom)
	{
		z= min_zoom - z;
	}

	if (x<0 || x>pow(2,z+0.0)-1 ||
			y<0 || y>pow(2,z+0.0)-1)
	{
		return false;
	}
	return true;

}
int TileMapAdapter::tilesonzoomlevel(int zoomlevel) const
{
	return int(pow(2, zoomlevel+0.0));
}
int TileMapAdapter::getxoffset(int x) const
{
	return x;
}
int TileMapAdapter::getyoffset(int y) const
{
	return y;
}
