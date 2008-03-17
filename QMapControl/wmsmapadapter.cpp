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
#include "wmsmapadapter.h"

WMSMapAdapter::WMSMapAdapter(QString host, QString serverPath, QString wlayers, QString wSrs, QString wStyles, int tilesize)
 : MapAdapter(host, serverPath, tilesize, 0, 17)
{
	name = "WMS-"+ host;

	wms_version = "1.1.1";
	wms_request = "GetMap";
	wms_format = "image/png";
	wms_transparent = "TRUE";
	wms_width = loc.toString(tilesize);
	wms_height = loc.toString(tilesize);

	wms_layers = wlayers;
	wms_styles = wStyles;
	wms_srs = wSrs;

	wms_bgcolor = "";
	wms_exceptions = "";
	wms_time = "";
	wms_elevation = "";

	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}


WMSMapAdapter::~WMSMapAdapter()
{
}

QPoint WMSMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
// 	double x = ((coordinate.x()+180)*(tilesize*numberOfTiles/360));
// 	double y = (((coordinate.y()*-1)+90)*(tilesize*numberOfTiles/180));

	double x = (coordinate.x()+180) * (numberOfTiles*tilesize)/360.;		// coord to pixel!
	double y = -1*(coordinate.y()-90) * (numberOfTiles*tilesize)/180.;	// coord to pixel!
	return QPoint(int(x), int(y));
}
QPointF WMSMapAdapter::displayToCoordinate(const QPoint& point) const
{
// 	double lon = ((point.x()/tilesize*numberOfTiles)*360)-180;
// 	double lat = (((point.y()/tilesize*numberOfTiles)*180)-90)*-1;

	double lon = (point.x()*(360./(numberOfTiles*tilesize)))-180;
	double lat = -(point.y()*(180./(numberOfTiles*tilesize)))+90;
	return QPointF(lon, lat);
}
void WMSMapAdapter::zoom_in()
{
	current_zoom+=1;
	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}
void WMSMapAdapter::zoom_out()
{
	current_zoom-=1;
	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}

bool WMSMapAdapter::isValid(int /* x */, int /* y */, int /* z */) const
{
// 	if (x>0 && y>0 && z>0)
	{
		return true;
	}
// 	return false;
}
QString WMSMapAdapter::getQuery(int i, int j, int /* z */) const
{
	return getQ(-180+i*coord_per_x_tile,
 					90-(j+1)*coord_per_y_tile,
 					-180+i*coord_per_x_tile+coord_per_x_tile,
 					90-(j+1)*coord_per_y_tile+coord_per_y_tile);
}
QString WMSMapAdapter::getQ(double ux, double uy, double ox, double oy) const
{
	return QString().append(serverPath)
						.append("SERVICE=WMS")
						.append("&VERSION=").append(wms_version)
						.append("&REQUEST=").append(wms_request)
						.append("&LAYERS=").append(wms_layers)
						.append("&SRS=").append(wms_srs)
						.append("&STYLES=").append(wms_styles)
						.append("&FORMAT=").append(wms_format)
						.append("&TRANSPARENT=").append(wms_transparent)
						.append("&WIDTH=").append(wms_width)
						.append("&HEIGHT=").append(wms_height)
						.append("&BBOX=")
						 .append(loc.toString(ux)).append(",")
						 .append(loc.toString(uy)).append(",")
						 .append(loc.toString(ox)).append(",")
						 .append(loc.toString(oy));
}
