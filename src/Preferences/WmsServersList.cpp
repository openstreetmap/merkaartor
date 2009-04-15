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

WmsServer::WmsServer(const WmsServer& other)
	: WmsName(other.WmsName), WmsAdress(other.WmsAdress), WmsPath(other.WmsPath), WmsLayers(other.WmsLayers), 
		WmsProjections(other.WmsProjections), WmsStyles(other.WmsStyles), WmsImgFormat(other.WmsImgFormat), deleted(other.deleted)
{
}

WmsServer::WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat, bool Deleted)
	: WmsName(Name), WmsAdress(Adress), WmsPath(Path), WmsLayers(Layers), WmsProjections(Projections), WmsStyles(Styles), WmsImgFormat(ImgFormat), deleted(Deleted)
{
	if (Name == "") {
		WmsName = QApplication::translate("MerkaartorPreferences","New Server");
	}
}

void WmsServer::toXml(QDomElement parent)
{
	QDomElement p = parent.ownerDocument().createElement("WmsServer");
	parent.appendChild(p);
	p.setAttribute("name", WmsName);
	p.setAttribute("address", WmsAdress);
	p.setAttribute("path", WmsPath);
	p.setAttribute("layers", WmsLayers);
	p.setAttribute("projections", WmsProjections);
	p.setAttribute("styles", WmsStyles);
	p.setAttribute("format", WmsImgFormat);
	if (deleted)
		p.setAttribute("deleted", "true");
}

WmsServer WmsServer::fromXml(QDomElement parent)
{
	WmsServer theServer;

	if (parent.tagName() == "WmsServer") {
		theServer.WmsName = parent.attribute("name");
		theServer.WmsAdress = parent.attribute("address");
		theServer.WmsPath = parent.attribute("path");
		theServer.WmsLayers = parent.attribute("layers");
		theServer.WmsProjections = parent.attribute("projections");
		theServer.WmsStyles = parent.attribute("styles");
		theServer.WmsImgFormat = parent.attribute("format");
		theServer.deleted = (parent.attribute("deleted") == "true" ? true : false);
	}

	return theServer;
}

/** **/

void WmsServersList::add(WmsServersList aWmsServersList)
{
	QMapIterator <QString, WmsServer> it(*(aWmsServersList.getServers()));
	while (it.hasNext()) {
		it.next();

		WmsServer anItem = it.value();
		theServers.insert(anItem.WmsName, anItem);
	}
}

WmsServerList* WmsServersList::getServers()
{
	return &theServers;
}

void WmsServersList::addServer(WmsServer aServer)
{
	theServers.insert(aServer.WmsName, aServer);
}

bool WmsServersList::contains(QString name) const
{
	if (theServers.contains(name))
		return true;
	else {
		WmsServerListIterator it(theServers);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return true;
		}
	}
	return false;
}

WmsServer WmsServersList::getServer(QString name) const
{
	if (theServers.contains(name))
		return theServers.value(name);
	else {
		WmsServerListIterator it(theServers);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return it.value();
		}
	}
	return WmsServer();
}

void WmsServersList::toXml(QDomElement parent)
{
	QDomElement rt = parent.ownerDocument().createElement("WmsServers");
	parent.appendChild(rt);
	rt.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

	WmsServerListIterator it(theServers);
	while (it.hasNext()) {
		it.next();

		WmsServer i = it.value();
		i.toXml(rt);
	}
}

WmsServersList WmsServersList::fromXml(QDomElement parent)
{
	WmsServersList theServersList;

	if (parent.nodeName() == "WmsServers") {
		QDomElement c = parent.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "WmsServer") {
				theServersList.addServer(WmsServer::fromXml(c));
			} 

			c = c.nextSiblingElement();
		}
	}

	return theServersList;
}
