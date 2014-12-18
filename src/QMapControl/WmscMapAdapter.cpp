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
#include "Global.h"

WmscMapAdapter::WmscMapAdapter(WmsServer aServer)
    : MapAdapter("", "", 0, 0), theServer(aServer)
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

int WmscMapAdapter::getTileSizeW() const
{
    return tilesize;
}

int WmscMapAdapter::getTileSizeH() const
{
    return tilesize;
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

    QPointF ul = QPointF(i*tileWidth+getBoundingbox().topLeft().x(), getBoundingbox().bottomLeft().y()-j*tileHeight);
    QPointF br = QPointF((i+1)*tileWidth+getBoundingbox().topLeft().x(), getBoundingbox().bottomLeft().y()- (j+1)*tileHeight);

    QUrl theUrl(theServer.WmsPath);
#ifdef QT5
    QUrlQuery theQuery;
#define theQuery theQuery
#else
#define theQuery theUrl
#endif
    if (!theQuery.hasQueryItem("VERSION"))
        theQuery.addQueryItem("VERSION", "1.1.1");
    if (!theQuery.hasQueryItem("SERVICE"))
        theQuery.addQueryItem("SERVICE", "WMS");
    theQuery.addQueryItem("REQUEST", "GetMap");

    if (!theQuery.hasQueryItem("TRANSPARENT"))
        theQuery.addQueryItem("TRANSPARENT", "TRUE");
    if (!theQuery.hasQueryItem("LAYERS"))
        theQuery.addQueryItem("LAYERS", theServer.WmsLayers);
    if (!theQuery.hasQueryItem("SRS"))
        theQuery.addQueryItem("SRS", theServer.WmsProjections);
    if (!theQuery.hasQueryItem("STYLES"))
        theQuery.addQueryItem("STYLES", theServer.WmsStyles);
    if (!theQuery.hasQueryItem("FORMAT"))
        theQuery.addQueryItem("FORMAT", theServer.WmsImgFormat);
    theQuery.addQueryItem("WIDTH", QString::number(getTileSizeW()));
    theQuery.addQueryItem("HEIGHT", QString::number(getTileSizeH()));
    theQuery.addQueryItem("BBOX", QString()
                        .append(loc.toString(ul.x(),'f',6)).append(",")
                        .append(loc.toString(br.y(),'f',6)).append(",")
                        .append(loc.toString(br.x(),'f',6)).append(",")
                        .append(loc.toString(ul.y(),'f',6))
            );
    if (theServer.WmsIsTiled == 1)
        theQuery.addQueryItem("tiled", "true");
#ifdef QT5
    theUrl.setQuery(theQuery);
#endif
#undef theQuery


    return theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
}

bool WmscMapAdapter::isValid(int x, int y, int z) const
{
    // Origin is bottom-left
    y = getTilesNS(current_zoom)-1 - y;

    if ((x<0) || (x>=getTilesWE(z)) ||
            (y<0) || (y>=getTilesNS(z)))
    {
        return false;
    }
    return true;

}

int WmscMapAdapter::getTilesWE(int zoomlevel) const
{
    qreal unitPerTile = theServer.WmsCLayer.Resolutions[zoomlevel] * getTileSizeW(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().width() / unitPerTile);
}

int WmscMapAdapter::getTilesNS(int zoomlevel) const
{
    qreal unitPerTile = theServer.WmsCLayer.Resolutions[zoomlevel] * getTileSizeH(); // Size of 1 tile in projected units
    return qRound(getBoundingbox().height() / unitPerTile);
}

QString WmscMapAdapter::getSourceTag() const
{
    return theServer.WmsSourceTag;
}

QString WmscMapAdapter::getLicenseUrl() const
{
    return theServer.WmsLicenseUrl;
}
