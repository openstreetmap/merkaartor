//
// C++ Implementation: WMSServersList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QApplication>

#include "WmsServersList.h"

WmsServer::WmsServer()
{
	WmsServer(QApplication::translate("MerkaartorPreferences","New Server"), "", "", "", "", "", "");
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat)
	: WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles), WmsImgFormat(ImgFormat)
{
	if (Name == "") {
		WmsName = QApplication::translate("MerkaartorPreferences","New Server");
	}
}

void WmsServer::toXml(QDomElement parent)
{
}

WmsServer WmsServer::fromXml(QDomElement parent)
{
	WmsServer theServer;

	return theServer;
}

/** **/

void WmsServersList::addServer(QString name, WmsServer aServer)
{
}

WmsServer WmsServersList::getServer(QString name)
{
	WmsServer theServer;

	return theServer;
}

void WmsServersList::toXml(QDomElement parent)
{
}

WmsServersList WmsServersList::fromXml(QDomElement parent)
{
	WmsServersList theServersList;

	return theServersList;
}
