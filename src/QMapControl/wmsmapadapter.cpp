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
#include "Maps/Projection.h"

WMSMapAdapter::WMSMapAdapter(QString host, QString serverPath, QString wlayers, QString wSrs, QString wStyles, QString wImgFormat, int tilesize)
 : TileMapAdapter(host, serverPath, tilesize, 0, 99)
{
	name = "WMS-"+ host;

	wms_version = "1.1.1";
	wms_request = "GetMap";
	wms_format = wImgFormat; //"image/png";
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
	//coord_per_x_tile = 360. / numberOfTiles;
	//coord_per_y_tile = 180. / numberOfTiles;
}

QString WMSMapAdapter::projection() const
{
	return wms_srs;
}

WMSMapAdapter::~WMSMapAdapter()
{
}

QUuid WMSMapAdapter::getId() const
{
	return QUuid("{E238750A-AC27-429e-995C-A60C17B9A1E0}");
}

IMapAdapter::Type WMSMapAdapter::getType() const
{
	return IMapAdapter::DirectBackground;
}

QPoint WMSMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
#ifndef _MOBILE
	double spanX = 360.;
	double spanY = 180.;

	QPointF p = Projection::projProject(Coord(angToInt(coordinate.y()), angToInt(coordinate.x())));

	double x = (p.x()) * (numberOfTiles*tilesize)/spanX;		// coord to pixel!
	double y = -1*(p.y()) * (numberOfTiles/2*tilesize)/spanY;	// coord to pixel!

	return QPoint(int(x), int(y));
#else
	double x = (coordinate.x()+180) * (numberOfTiles*tilesize)/360.;		// coord to pixel!
	double y = -1*(coordinate.y()-90) * (numberOfTiles*tilesize)/180.;	// coord to pixel!
	return QPoint(int(x), int(y));
#endif
}

QPointF WMSMapAdapter::displayToCoordinate(const QPoint& pt) const
{
#ifndef _MOBILE
	double spanX = 360.;
	double spanY = 180.;

	double Lon = (pt.x()*(spanX/(numberOfTiles*tilesize)));
	double Lat = -(pt.y()*(spanY/(numberOfTiles/2*tilesize)));

	Coord c = Projection::projInverse(QPointF(Lon, Lat));
	return QPointF(intToAng(c.lon()), intToAng(c.lat()));
#else
	double lon = (pt.x()*(360./(numberOfTiles*tilesize)))-180;
	double lat = -(pt.y()*(180./(numberOfTiles*tilesize)))+90;
	return QPointF(lon, lat);
#endif
} 

bool WMSMapAdapter::isValid(int /* x */, int /* y */, int /* z */) const
{
// 	if (x>0 && y>0 && z>0)
	{
		return true;
	}
// 	return false;
}

int WMSMapAdapter::tilesonzoomlevel(int zoomlevel) const
{
	return int(pow(2, zoomlevel+1.0));
}


QString WMSMapAdapter::getQuery(int i, int j, int /* z */) const
{
	QPointF ul = displayToCoordinate(QPoint(i*tilesize, j*tilesize));
	QPointF br = displayToCoordinate(QPoint((i+1)*tilesize, (j+1)*tilesize));
	return getQ(ul, br);

}
QString WMSMapAdapter::getQ(QPointF ul, QPointF br) const
{
	QPointF ulp;
	QPointF brp;

#ifndef _MOBILE
	ulp = Projection::projProject(Coord(angToInt(ul.y()), angToInt(ul.x())));
	brp = Projection::projProject(Coord(angToInt(br.y()), angToInt(br.x())));
	if (Projection::projIsLatLong() ) {
		ulp = QPointF(rad_deg(ulp.x()), rad_deg(ulp.y()));
		brp = QPointF(rad_deg(brp.x()), rad_deg(brp.y()));
	}
#else
	ulp = ul;
	brp = br;
#endif

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
						 .append(loc.toString(ulp.x(),'f',6)).append(",")
						 .append(loc.toString(brp.y(),'f',6)).append(",")
						 .append(loc.toString(brp.x(),'f',6)).append(",")
						 .append(loc.toString(ulp.y(),'f',6));
}
