/***************************************************************************
 *   Copyright (C) 2008 by Łukasz Jernaś   *
 *   deejay1@srem.org   *
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
#include "httpquery.h"
#include <QtDebug>
#include "Global.h"
#include <QUrlQuery>

#include "MerkaartorPreferences.h"

namespace NameFinder
{

    HttpQuery::HttpQuery ( QObject *parent ) : QObject ( parent )
    {
        myService = M_PREFS->getNominatimUrl();
    }
    HttpQuery::HttpQuery ( QObject *parent, QUrl service ) : QObject ( parent )
    {
        myService = service;
    }


    HttpQuery::~HttpQuery()
    {
    }

    bool HttpQuery::startSearch ( QString question )
    {
        connect(&connection,SIGNAL(finished(QNetworkReply*)),this,SLOT(on_requestFinished(QNetworkReply*)));

#ifdef QT5
        QUrlQuery theQuery(myService);
#define theQuery theQuery
#else
#define theQuery myService
#endif
        theQuery.addQueryItem ( "q",question );
        theQuery.addQueryItem ( "format","xml" );
#ifdef QT5
        myService.setQuery(theQuery);
#endif
#undef theQuery

        QUrl url("http://"+myService.host());
        url.setPath(myService.path());
        url.setQuery(myService.query());

        qDebug() << "HttpQuery: getting " << url;
        QNetworkRequest req(url);

        req.setRawHeader(QByteArray("User-Agent"), USER_AGENT.toLatin1());

        connection.setProxy(M_PREFS->getProxy(myService));

        myReply = connection.get( req );
        return true;
    }

    void HttpQuery::on_requestFinished ( QNetworkReply *reply )
    {
        if (reply == myReply) {
            if (!reply->error()) {
                qDebug() << "HttpQuery: request completed without error.";
                emit done(reply);
            } else {
                qDebug() << "HttpQuery: request returned with error " << reply->error() << ": " << reply->errorString();
                emit doneWithError(reply->error());
            }
        } else {
            qDebug() << "HttpQuery: reply received to unknown request";
        }
    }
}
