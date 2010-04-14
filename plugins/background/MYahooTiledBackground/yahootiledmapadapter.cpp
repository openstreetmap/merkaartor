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

QRectF	YahooTiledMapAdapter::getBoundingbox() const
{
    return QRectF(QPointF(-20037508.34, -20037508.34), QPointF(20037508.34, 20037508.34));
}

QString YahooTiledMapAdapter::projection() const
{
    return ("EPSG:3785");
}

int YahooTiledMapAdapter::getTilesWE(int zoomlevel) const
{
    return int(pow(2, zoomlevel+0.0));
}

int YahooTiledMapAdapter::getTilesNS(int zoomlevel) const
{
    return int(pow(2, zoomlevel+0.0));
}

static QPointF mercatorInverse(const QPointF& point)
{
    qreal longitude = point.x()*180.0/20037508.34;
    qreal latitude = (atan(sinh(point.y()/20037508.34*M_PI))) *180/M_PI;

    return QPointF(longitude, latitude);
}

QString YahooTiledMapAdapter::getQuery(int i, int j, int /* z */) const
{
    qreal tileWidth = getBoundingbox().width() / getTilesWE(current_zoom);
    qreal tileHeight = getBoundingbox().height() / getTilesNS(current_zoom);

    QPointF ul = mercatorInverse(QPointF(i*tileWidth-20037508.34, 20037508.34-j*tileHeight));
    QPointF br = mercatorInverse(QPointF((i+1)*tileWidth-20037508.34, 20037508.34- (j+1)*tileHeight));

    return getQ(ul, br);
}

QString YahooTiledMapAdapter::getQ(QPointF ul, QPointF br) const
{
    return QString().append(serverPath)
                        .append("WIDTH=").append(QString().setNum(tilesize+100))
                        .append("&HEIGHT=").append(QString().setNum(tilesize+100))
                        .append("&BBOX=")
                         .append(loc.toString(ul.x(),'f',8)).append(",")
                         .append(loc.toString(ul.y(),'f',8)).append(",")
                         .append(loc.toString(br.x(),'f',8)).append(",")
                         .append(loc.toString(br.y(),'f',8));
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
    return max_zoom - min_zoom;
}

int YahooTiledMapAdapter::getZoom() const
{
    return current_zoom;
}

int YahooTiledMapAdapter::getAdaptedZoom() const
{
    return current_zoom - min_zoom;
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
    current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;
}
void YahooTiledMapAdapter::zoom_out()
{
    current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
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
