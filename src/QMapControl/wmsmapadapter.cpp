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
	return IMapAdapter::NetworkBackground;
}

QString WMSMapAdapter::getQuery(const QRectF& /*wgs84Bbox*/, const QRectF& projBbox, const QRect& size) const
{
	return QString()
						.append(theServer.WmsPath)
						.append("SERVICE=WMS")
						.append("&VERSION=1.1.1")
						.append("&REQUEST=GetMap")
						.append("&TRANSPARENT=TRUE")
						.append("&LAYERS=").append(theServer.WmsLayers)
						.append("&SRS=").append(theServer.WmsProjections)
						.append("&STYLES=").append(theServer.WmsStyles)
						.append("&FORMAT=").append(theServer.WmsImgFormat)
						.append("&WIDTH=").append(QString::number(size.width()))
						.append("&HEIGHT=").append(QString::number(size.height()))
						.append("&BBOX=")
						.append(loc.toString(projBbox.bottomLeft().x(),'f',6)).append(",")
						 .append(loc.toString(projBbox.bottomLeft().y(),'f',6)).append(",")
						 .append(loc.toString(projBbox.topRight().x(),'f',6)).append(",")
						 .append(loc.toString(projBbox.topRight().y(),'f',6))
						 ;
}
