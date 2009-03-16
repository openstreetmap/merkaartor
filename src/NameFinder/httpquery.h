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
#ifndef NAMEFINDERHTTPQUERY_H
#define NAMEFINDERHTTPQUERY_H

#include <QObject>
#include <QIODevice>
#include <QtNetwork/QHttp>
#include <QUrl>
#include <QtNetwork/QNetworkProxy>

namespace NameFinder {

    /**
    	@author Łukasz Jernaś <deejay1@srem.org>
    */
class HttpQuery : public QObject {
        Q_OBJECT
public:
        HttpQuery(QObject *parent, QIODevice *device);
        HttpQuery(QObject *parent, QUrl service, QIODevice *device);
        ~HttpQuery();

        bool   startSearch(QString question);
        //! Sets up a network proxy
        void setProxy(QString host, int port);

signals:
	void done();
	void doneWithError(QHttp::Error error);

private:
        QHttp connection;
        QUrl url;
//! What we are looking for...
        QString myQuestion;
        QUrl myService;
//! Our input device - so we can use QFile, QBuffer, etc...
        QIODevice *myDevice;
        //! Do we have an http proxy
        QNetworkProxy myProxy;
        bool proxyEnabled;
		int reqId;

private slots:
		void on_requestFinished ( int id, bool error );
		void on_responseHeaderReceived(const QHttpResponseHeader & hdr);
};

}

#endif
