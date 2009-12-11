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

#include "yahootiledmapadapter.h"
#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

// {10F45DE0-E529-11DE-8B99-9E7D56D89593}
static const QUuid theUid ( 0x10F45DE0, 0xE529, 0x11DE, 0x8B, 0x99, 0x9E, 0x7D, 0x56, 0xD8, 0x95, 0x93);

YahooTiledMapAdapter::YahooTiledMapAdapter()
	: theImageManager(0)

{
	host = "";
	serverPath = "qrc:/Html/ymap.html?";
	tilesize = 512;
	max_zoom = 17;
	min_zoom = 0;
	current_zoom = 0;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);

	loc = QLocale(QLocale::English);
	loc.setNumberOptions(QLocale::OmitGroupSeparator);
}

YahooTiledMapAdapter::~YahooTiledMapAdapter()
{
}

QString	YahooTiledMapAdapter::getHost() const
{
	return "";
}

QUuid YahooTiledMapAdapter::getId() const
{
	return QUuid(theUid);
}

IMapAdapter::Type YahooTiledMapAdapter::getType() const
{
	return IMapAdapter::BrowserBackground;
}

QString	YahooTiledMapAdapter::getName() const
{
	return "Yahoo! (Tiled)";
}

QString YahooTiledMapAdapter::projection() const
{
	return ("EPSG:3785");
}

int YahooTiledMapAdapter::tilesonzoomlevel(int zoomlevel) const
{
	return int(pow(2, zoomlevel+1.0));
}

QString YahooTiledMapAdapter::getQuery(int i, int j, int /* z */) const
{
	QPointF ul = displayToCoordinate(QPoint(i*tilesize, j*tilesize));
	QPointF br = displayToCoordinate(QPoint((i+1)*tilesize, (j+1)*tilesize));
	return getQ(ul, br);
}
QString YahooTiledMapAdapter::getQ(QPointF ul, QPointF br) const
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

int YahooTiledMapAdapter::getyoffset(int y) const
{
	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;

	int tiles = int(pow(2, zoom+0.0));
	y = y*(-1)+tiles-1;
	return int(y);
}

//*********** mapadapter

int YahooTiledMapAdapter::getTileSize() const
{
	return tilesize;
}

int YahooTiledMapAdapter::getMinZoom() const
{
	return min_zoom;
}

int YahooTiledMapAdapter::getMaxZoom() const
{
	return max_zoom;
}

int YahooTiledMapAdapter::getAdaptedMinZoom() const
{
	return 0;
}

int YahooTiledMapAdapter::getAdaptedMaxZoom() const
{
	return max_zoom > min_zoom ? max_zoom - min_zoom : min_zoom - max_zoom;
}

int YahooTiledMapAdapter::getZoom() const
{
	return current_zoom;
}

int YahooTiledMapAdapter::getAdaptedZoom() const
{
	return max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom - min_zoom;
}

IImageManager* YahooTiledMapAdapter::getImageManager()
{
	return theImageManager;
}

void YahooTiledMapAdapter::setImageManager(IImageManager* anImageManager)
{
	theImageManager = anImageManager;
}


//********* tiledmapadapter

void YahooTiledMapAdapter::zoom_in()
{
	if (min_zoom > max_zoom)
		current_zoom = current_zoom > max_zoom ? current_zoom-1 : max_zoom;
	else if (min_zoom < max_zoom)
		current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);//pow(2, zoom);

}
void YahooTiledMapAdapter::zoom_out()
{
	if (min_zoom > max_zoom)
		current_zoom = current_zoom < min_zoom ? current_zoom+1 : min_zoom;
	else if (min_zoom < max_zoom)
		current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;

	int zoom = max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom;
	numberOfTiles = tilesonzoomlevel(zoom);//pow(2, zoom);

}

static double deg_rad(double x)
{
	return x * (M_PI/180.);
}
static double rad_deg(double x)
{
	return x * (180./M_PI);
}


QPoint YahooTiledMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
// 	qDebug() << "numberOfTiles: " << numberOfTiles;

	double x = (coordinate.x()+180) * (numberOfTiles*tilesize)/360.;		// coord to pixel!
	double y = (1-(log(tan(M_PI/4+deg_rad(coordinate.y())/2)) /M_PI)) /2  * (numberOfTiles*tilesize);

	return QPoint(int(x), int(y));

}

QPointF YahooTiledMapAdapter::displayToCoordinate(const QPoint& pt) const
{
	QPointF point(pt);
	double longitude = (point.x()*(360.0/(numberOfTiles*(double)tilesize)))-180.0;
	double latitude = rad_deg(atan(sinh((1.0-point.y()*(2.0/(numberOfTiles*(double)tilesize)))*M_PI)));

	return QPointF(longitude, latitude);

}

bool YahooTiledMapAdapter::isValid(int x, int y, int z) const
{
	return true;

	if (max_zoom < min_zoom)
	{
		z= min_zoom - z;
	}

	if ((x<0) || (x>pow(2,z+0.0)-1) ||
			(y<0) || (y>pow(2,z+0.0)-1))
	{
		return false;
	}
	return true;

}


Q_EXPORT_PLUGIN2(MYahooTiledBackgroundPlugin, YahooTiledMapAdapter)
