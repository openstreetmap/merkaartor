//
// C++ Interface: WMSServersList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef WMSSERVERS_LIST_H
#define WMSSERVERS_LIST_H

#include <QString>
#include <QMap>
#include <QtXml>

class WmsServer
{
	public:
		WmsServer();
		WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat, bool Deleted=false);

		void toXml(QDomElement parent);
		static WmsServer fromXml(QDomElement parent);

	public:
		QString WmsName;
		QString WmsAdress;
		QString WmsPath;
		QString WmsLayers;
		QString WmsProjections;
		QString WmsStyles;
		QString WmsImgFormat;
		bool deleted;
};
typedef QMap<QString, WmsServer> WmsServerList;
typedef QMapIterator<QString, WmsServer> WmsServerListIterator;

class WmsServersList
{
	public:
		void add(WmsServersList aWmsServersList);
		void addServer(WmsServer aServer);
		bool contains(QString name) const;
		WmsServerList* getServers();
		WmsServer getServer(QString name) const;
		void toXml(QDomElement parent);
		static WmsServersList fromXml(QDomElement parent);

	private:
		WmsServerList theServers;
};

#endif // WMSSERVERS_LIST_H
