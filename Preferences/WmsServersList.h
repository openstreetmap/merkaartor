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
		WmsServer(QString Name, QString Adress, QString Path, QString Layers, QString Projections, QString Styles, QString ImgFormat);

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
};
typedef QMap<QString, WmsServer> WmsServerList;
typedef QMapIterator<QString, WmsServer> WmsServerListIterator;

class WmsServersList
{
	public:
		void addServer(QString name, WmsServer aServer);
		WmsServer getServer(QString name);
		void toXml(QDomElement parent);
		static WmsServersList fromXml(QDomElement parent);

	private:
		QMap <QString, WmsServer> theServers;
};

#endif // WMSSERVERS_LIST_H
