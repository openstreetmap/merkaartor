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
#include "WmscMapAdapter.h"

WmscMapAdapter::WmscMapAdapter(WmsServer aServer)
    : MapAdapter("", "", 0, 0, 0), theServer(aServer)
{
    //	:MapAdapter(host, serverPath, tilesize, minZoom, maxZoom)
    name = theServer.WmsName;
    host = theServer.WmsAdress;
    serverPath = theServer.WmsPath;
    tilesize = theServer.WmsCLayer.TileWidth;
    max_zoom = theServer.WmsCLayer.Resolutions.size() - 1;
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
}

WmscMapAdapter::~WmscMapAdapter()
{
}

QUuid WmscMapAdapter::getId() const
{
    return QUuid("{E238750A-AC27-429e-995C-A60C17B9A1E0}");
}

IMapAdapter::Type WmscMapAdapter::getType() const
{
    return IMapAdapter::NetworkBackground;
}

QString WmscMapAdapter::projection() const
{
    return theServer.WmsProjections;
}

QRectF	WmscMapAdapter::getBoundingbox() const
{
    return theServer.WmsCLayer.BoundingBox;
}

void WmscMapAdapter::zoom_in()
{
    current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;

}
void WmscMapAdapter::zoom_out()
{
    current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
}

QString WmscMapAdapter::getQuery(int i, int j, int /* z */) const
{
    // WMS-C Y origin is lower left
//    j = getTilesNS(current_zoom)-1 - j;

    qreal tileWidth = getBoundingbox().width() / getTilesWE(current_zoom);
    qreal tileHeight = getBoundingbox().height() / getTilesNS(current_zoom);

    QPointF ul = QPointF(i*tileWidth+theServer.WmsCLayer.BoundingBox.topLeft().x(), -theServer.WmsCLayer.BoundingBox.topLeft().y()-j*tileHeight);
    QPointF br = QPointF((i+1)*tileWidth+theServer.WmsCLayer.BoundingBox.topLeft().x(), -theServer.WmsCLayer.BoundingBox.topLeft().y()- (j+1)*tileHeight);

    return QString()
                        .append(theServer.WmsPath)
                        .append("SERVICE=WMS")
                        .append("&VERSION=1.1.1")
                        .append("&REQUEST=GetMap")
                        .append("&TRANSPARENT=TRUE")
                        .append("&tiled=true")
                        .append("&LAYERS=").append(theServer.WmsLayers)
                        .append("&SRS=").append(theServer.WmsProjections)
                        .append("&STYLES=").append(theServer.WmsStyles)
                        .append("&FORMAT=").append(theServer.WmsImgFormat)
                        .append("&WIDTH=").append(QString::number(tilesize))
                        .append("&HEIGHT=").append(QString::number(tilesize))
                        .append("&BBOX=")
                        .append(loc.toString(ul.x(),'f',6)).append(",")
                        .append(loc.toString(br.y(),'f',6)).append(",")
                        .append(loc.toString(br.x(),'f',6)).append(",")
                        .append(loc.toString(ul.y(),'f',6));
                         ;
}

bool WmscMapAdapter::isValid(int x, int y, int z) const
{
    if ((x<0) || (x>getTilesWE(z)) ||
            (y<0) || (y>getTilesNS(z)))
    {
        return false;
    }
    return true;

}

int WmscMapAdapter::getTilesWE(int zoomlevel) const
{
    qreal unitPerTile = theServer.WmsCLayer.Resolutions[zoomlevel] * tilesize; // Size of 1 tile in projected units
    return qRound(theServer.WmsCLayer.BoundingBox.width() / unitPerTile);
}

int WmscMapAdapter::getTilesNS(int zoomlevel) const
{
    qreal unitPerTile = theServer.WmsCLayer.Resolutions[zoomlevel] * tilesize; // Size of 1 tile in projected units
    return qRound(theServer.WmsCLayer.BoundingBox.height() / unitPerTile);
}
