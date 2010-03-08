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

#include "yahoolegalmapadapter.h"

// {67CC0481-8C6A-4735-8666-BBA6A1B04E19}
static const QUuid theUid ( 0x67cc0481, 0x8c6a, 0x4735, 0x86, 0x66, 0xbb, 0xa6, 0xa1, 0xb0, 0x4e, 0x19);

YahooLegalMapAdapter::YahooLegalMapAdapter()
		: theImageManager(0)
{
	loc = QLocale(QLocale::English);
	loc.setNumberOptions(QLocale::OmitGroupSeparator);
}


YahooLegalMapAdapter::~YahooLegalMapAdapter()
{
}

QString	YahooLegalMapAdapter::getHost() const
{
	return "";
}

QUuid YahooLegalMapAdapter::getId() const
{
	return QUuid(theUid);
}

IMapAdapter::Type YahooLegalMapAdapter::getType() const
{
	return IMapAdapter::BrowserBackground;
}

QString	YahooLegalMapAdapter::getName() const
{
	return "Yahoo! (WMS)";
}

QRectF YahooLegalMapAdapter::getBoundingbox() const
{
	return QRectF();
}

QString YahooLegalMapAdapter::projection() const
{
	return ("EPSG:3785");
}

QString YahooLegalMapAdapter::getQuery(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& size) const
{
	if (size.width() < 150 || size.height() < 150)
		return "";

	return QString()
						.append("qrc:/Html/ymap.html?")
						.append("WIDTH=").append(QString::number(size.width()+100))
						.append("&HEIGHT=").append(QString::number(size.height()+100))
						.append("&BBOX=")
						.append(loc.toString(wgs84Bbox.bottomLeft().x(),'f',8)).append(",")
						.append(loc.toString(wgs84Bbox.bottomLeft().y(),'f',8)).append(",")
						.append(loc.toString(wgs84Bbox.topRight().x(),'f',8)).append(",")
						.append(loc.toString(wgs84Bbox.topRight().y(),'f',8))
						;
}

IImageManager* YahooLegalMapAdapter::getImageManager()
{
	return theImageManager;
}

void YahooLegalMapAdapter::setImageManager(IImageManager* anImageManager)
{
	theImageManager = anImageManager;
}

Q_EXPORT_PLUGIN2(MYahooBackgroundPlugin, YahooLegalMapAdapter)
