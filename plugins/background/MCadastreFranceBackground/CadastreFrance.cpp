/***************************************************************************
 *   Copyright (C) 2010 by Chris Browet                                    *
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

#include "CadastreFrance.h"

// {67CC0481-8C6A-4735-8666-BBA6A1B04E19}
static const QUuid theUid ( 0x14a9ff26, 0x634e, 0x4406, 0x94, 0xa5, 0x4c, 0x6d, 0x9c, 0xf0, 0xb1, 0x1d);

CadastreFranceAdapter::CadastreFranceAdapter()
        : theImageManager(0)
{
    loc = QLocale(QLocale::English);
    loc.setNumberOptions(QLocale::OmitGroupSeparator);
}


CadastreFranceAdapter::~CadastreFranceAdapter()
{
}

QString	CadastreFranceAdapter::getHost() const
{
    return "";
}

QUuid CadastreFranceAdapter::getId() const
{
    return QUuid(theUid);
}

IMapAdapter::Type CadastreFranceAdapter::getType() const
{
    return IMapAdapter::NetworkBackground;
}

QString	CadastreFranceAdapter::getName() const
{
    return "Cadastre (France)";
}

QString CadastreFranceAdapter::projection() const
{
    return ("EPSG:3857");
}

QString CadastreFranceAdapter::getQuery(const QRectF& wgs84Bbox, const QRectF& /*projBbox*/, const QRect& size) const
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

IImageManager* CadastreFranceAdapter::getImageManager()
{
    return theImageManager;
}

void CadastreFranceAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;
}

Q_EXPORT_PLUGIN2(MYahooBackgroundPlugin, CadastreFranceAdapter)
