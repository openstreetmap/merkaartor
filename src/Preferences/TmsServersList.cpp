//
// C++ Implementation: TmsServersList
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

#include "TmsServersList.h"

TmsServer::TmsServer()
{
	TmsServer(QApplication::translate("MerkaartorPreferences","New Server"), "", "", 256, 0, 17);
}

TmsServer::TmsServer(QString Name, QString Adress, QString Path, int tileSize, int minZoom, int maxZoom, bool Deleted)
	: TmsName(Name), TmsAdress(Adress), TmsPath(Path), TmsTileSize(tileSize), TmsMinZoom(minZoom), TmsMaxZoom(maxZoom), deleted(Deleted)
{
	if (Name == "") {
		TmsName = QApplication::translate("MerkaartorPreferences","New Server");
	}
}

void TmsServer::toXml(QDomElement parent)
{
	QDomElement p = parent.ownerDocument().createElement("TmsServer");
	parent.appendChild(p);
	p.setAttribute("name", TmsName);
	p.setAttribute("address", TmsAdress);
	p.setAttribute("path", TmsPath);
	p.setAttribute("tilesize", QString::number(TmsTileSize));
	p.setAttribute("minzoom", QString::number(TmsMinZoom));
	p.setAttribute("maxzoom", QString::number(TmsMaxZoom));
	if (deleted)
		p.setAttribute("deleted", "true");
}

TmsServer TmsServer::fromXml(QDomElement parent)
{
	TmsServer theServer;

	if (parent.tagName() == "TmsServer") {
		theServer.TmsName = parent.attribute("name");
		theServer.TmsAdress = parent.attribute("address");
		theServer.TmsPath = parent.attribute("path");
		theServer.TmsTileSize = parent.attribute("tilesize").toInt();
		theServer.TmsMinZoom = parent.attribute("minzoom").toInt();
		theServer.TmsMaxZoom = parent.attribute("maxzoom").toInt();
		theServer.deleted = (parent.attribute("deleted") == "true" ? true : false);
	}

	return theServer;
}

/** **/

void TmsServersList::add(TmsServersList aTmsServersList)
{
	QMapIterator <QString, TmsServer> it(*(aTmsServersList.getServers()));
	while (it.hasNext()) {
		it.next();

		TmsServer anItem = it.value();
		theServers.insert(anItem.TmsName, anItem);
	}
}

TmsServerList* TmsServersList::getServers()
{
	return &theServers;
}

void TmsServersList::addServer(TmsServer aServer)
{
	theServers.insert(aServer.TmsName, aServer);
}

bool TmsServersList::contains(QString name) const
{
	if (theServers.contains(name))
		return true;
	else {
		TmsServerListIterator it(theServers);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return true;
		}
	}
	return false;
}

TmsServer TmsServersList::getServer(QString name) const
{
	if (theServers.contains(name))
		return theServers.value(name);
	else {
		TmsServerListIterator it(theServers);
		while (it.hasNext()) {
			it.next();

			if (it.key().contains(name, Qt::CaseInsensitive))
				return it.value();
		}
	}
	return TmsServer();
}

void TmsServersList::toXml(QDomElement parent)
{
	QDomElement rt = parent.ownerDocument().createElement("TmsServers");
	parent.appendChild(rt);
	rt.setAttribute("creator", QString("Merkaartor %1").arg(VERSION));

	TmsServerListIterator it(theServers);
	while (it.hasNext()) {
		it.next();

		TmsServer i = it.value();
		i.toXml(rt);
	}
}

TmsServersList TmsServersList::fromXml(QDomElement parent)
{
	TmsServersList theServersList;

	if (parent.nodeName() == "TmsServers") {
		QDomElement c = parent.firstChildElement();
		while(!c.isNull()) {
			if (c.tagName() == "TmsServer") {
				theServersList.addServer(TmsServer::fromXml(c));
			} 

			c = c.nextSiblingElement();
		}
	}

	return theServersList;
}
