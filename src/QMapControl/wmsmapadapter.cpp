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
#include "Global.h"

WMSMapAdapter::WMSMapAdapter(WmsServer aServer)
 : theServer(aServer)
{
    loc = QLocale(QLocale::English);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
}

QString	WMSMapAdapter::getName() const
{
    return theServer.WmsName;
}

QString	WMSMapAdapter::getHost() const
{
    return theServer.WmsAdress;
}

IImageManager* WMSMapAdapter::getImageManager()
{
    return theImageManager;
}

void WMSMapAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;
}

QString WMSMapAdapter::projection() const
{
    if (theServer.WmsProjections == "OSGEO:41001")
        return "EPSG:3857";
    return theServer.WmsProjections;
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
    if (theServer.WmsImgFormat.startsWith("image"))
        return IMapAdapter::NetworkBackground;
    else
        return IMapAdapter::NetworkDataBackground;
}

QString WMSMapAdapter::getQuery(const QRectF& /*wgs84Bbox*/, const QRectF& projBbox, const QRect& size) const
{
    QUrl theUrl(theServer.WmsPath);
#ifdef QT5
    QUrlQuery theQuery(theUrl);
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
    theQuery.addQueryItem("WIDTH", QString::number(size.width()));
    theQuery.addQueryItem("HEIGHT", QString::number(size.height()));
    theQuery.addQueryItem("BBOX", loc.toString(projBbox.bottomLeft().x(),'f',6).append(",")
            .append(loc.toString(projBbox.bottomLeft().y(),'f',6)).append(",")
            .append(loc.toString(projBbox.topRight().x(),'f',6)).append(",")
            .append(loc.toString(projBbox.topRight().y(),'f',6))
            );
#ifdef QT5
    theUrl.setQuery(theQuery);
#endif
#undef theQuery

    return theUrl.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
}

QString WMSMapAdapter::getSourceTag() const
{
    return theServer.WmsSourceTag;
}

QString WMSMapAdapter::getLicenseUrl() const
{
    return theServer.WmsLicenseUrl;
}
