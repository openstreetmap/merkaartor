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

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
#define EQUATORIALMETERCIRCUMFERENCE  40075016.68
#define EQUATORIALMETERHALFCIRCUMFERENCE  20037508.34
#define EQUATORIALMETERPERDEGREE    222638.981555556

TileMapAdapter::TileMapAdapter(const QString& host, const QString& path, const QString& projection, int theTilesize, int minZoom, int maxZoom, bool blOrigin)
    :MapAdapter(host, path, projection, minZoom, maxZoom)
    , tilesize(theTilesize)
    , BlOrigin(blOrigin)
{
    name = "tiles";

    serverPath.replace("%1", "%z");
    serverPath.replace("%2", "%x");
    serverPath.replace("%3", "%y");

    int paramz = serverPath.indexOf("%z");
    int paramx = serverPath.indexOf("%x");
    int paramy = serverPath.indexOf("%y");

    if (paramx == -1 && paramy == -1 && paramz == -1) {
        // Check for potlach-style url
        paramz = serverPath.indexOf('!');
        if (paramz)
            serverPath.replace(paramz, 1, "%z");
        paramx = serverPath.indexOf('!');
        if (paramx)
            serverPath.replace(paramx, 1, "%x");
        paramy = serverPath.indexOf('!');
        if (paramy)
            serverPath.replace(paramy, 1, "%y");

    }

    isProj4326 = (Projection.contains(":4326"));
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
}

TileMapAdapter::~TileMapAdapter()
{
}

QUuid TileMapAdapter::getId() const
{
    return QUuid("{CA8A07EC-A466-462b-929F-3805BC9DEC95}");
}

IMapAdapter::Type TileMapAdapter::getType() const
{
    return IMapAdapter::NetworkBackground;
}

QRectF TileMapAdapter::getBoundingbox() const
{
    if (isProj4326)
        return QRectF(QPointF(-180.00, -90.00), QPointF(180.00, 90.00));
    else
        return QRectF(QPointF(-EQUATORIALMETERHALFCIRCUMFERENCE, -EQUATORIALMETERHALFCIRCUMFERENCE), QPointF(EQUATORIALMETERHALFCIRCUMFERENCE, EQUATORIALMETERHALFCIRCUMFERENCE));
}

int TileMapAdapter::getTileSize() const
{
    return tilesize;
}

//TODO: rausziehen? ->MapAdapter?
void TileMapAdapter::zoom_in()
{
    if (min_zoom > max_zoom)
        current_zoom = current_zoom > max_zoom ? current_zoom-1 : max_zoom;
    else if (min_zoom < max_zoom)
        current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;
}
void TileMapAdapter::zoom_out()
{
    if (min_zoom > max_zoom)
        current_zoom = current_zoom < min_zoom ? current_zoom+1 : min_zoom;
    else if (min_zoom < max_zoom)
        current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
}

QString TileMapAdapter::getQuery		(int x, int y, int z) const
{
    if (BlOrigin)
        y = getTilesNS(current_zoom)-1 - y;
    QString str = serverPath;
    str.replace("%z", QString::number(z));
    str.replace("%y", QString::number(y));
    str.replace("%x", QString::number(x));
    return str;
}

bool TileMapAdapter::isValid(int x, int y, int z) const
{
    if ((x<0) || (x>getTilesWE(z)) ||
            (y<0) || (y>getTilesNS(z)))
    {
        return false;
    }
    return true;
}

int TileMapAdapter::getTilesWE(int zoomlevel) const
{
    int t = int(pow(2, zoomlevel+0.0));
    if (isProj4326)
        t *= 2;

    return t;
}

int TileMapAdapter::getTilesNS(int zoomlevel) const
{
    return int(pow(2, zoomlevel+0.0));
}
