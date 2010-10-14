//
// C++ Interface: TMSServersList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//

#ifndef TMSSERVERS_LIST_H
#define TMSSERVERS_LIST_H

#include <QString>
#include <QMap>
#include <QtXml>

class TmsServer
{
    public:
        TmsServer();
        TmsServer(QString Name, QString Adress, QString Path, QString Projection, int tileSize, int minZoom, int maxZoom, QString SourceTag, QString LicenseUrl, QString TmsBaseUrl = "", bool Origin=false, bool Deleted=false);

        void toXml(QDomElement parent);
        static TmsServer fromXml(QDomElement parent);

    public:
        QString TmsName;
        QString TmsAdress;
        QString TmsPath;
        QString TmsProjection;
        int TmsTileSize;
        int TmsMinZoom;
        int TmsMaxZoom;
        QString TmsSourceTag;
        QString TmsLicenseUrl;
        QString TmsBaseUrl;
        bool TmsBlOrigin;
        bool deleted;
};
typedef QMap<QString, TmsServer> TmsServerList;
typedef QMapIterator<QString, TmsServer> TmsServerListIterator;

class TmsServersList
{
    public:
        void add(TmsServersList aTmsServersList);
        void addServer(TmsServer aServer);
        bool contains(QString name) const;
        TmsServerList* getServers();
        TmsServer getServer(QString name) const;
        void toXml(QDomElement parent);
        static TmsServersList fromXml(QDomElement parent);

    private:
        TmsServerList theServers;
};

#endif // TMSSERVERS_LIST_H
