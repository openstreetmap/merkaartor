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
#include <QtNetwork/QNetworkProxy>

namespace NameFinder
{

	HttpQuery::HttpQuery ( QObject *parent, QIODevice *device ) : QObject ( parent )
	{
		myService = "http://gazetteer.openstreetmap.org/namefinder/search.xml";
		myDevice = device;
		proxyEnabled = false;
	}
	HttpQuery::HttpQuery ( QObject *parent, QUrl service, QIODevice *device ) : QObject ( parent )
	{
		myService = service;
		myDevice = device;
		proxyEnabled = false;
	}


	HttpQuery::~HttpQuery()
	{
	}
// TODO: error reporting - QMessageBox??
	bool HttpQuery::startSearch ( QString question )
	{
		QObject::connect ( &connection, SIGNAL ( done ( bool ) ), this, SLOT ( httpDone ( bool ) ) );
		//if (!myService.isValid() || myService.scheme() != "http" || myService.path().isEmpty())
		//  return false;

		myDevice->open ( QIODevice::ReadWrite );
		myService.addQueryItem ( "find",question );
		if ( proxyEnabled )
			connection.setProxy ( myProxy );
		connection.setHost ( myService.host(), myService.port ( 80 ) );
		connection.get ( myService.path() + "?" + myService.encodedQuery(), myDevice );
		connection.close();
		return true;
	}

	void HttpQuery::httpDone ( bool error )
	{
		myDevice->close();
		emit done();
	}

	void HttpQuery::setProxy ( QNetworkProxy proxy )
	{
		myProxy = proxy;
		proxyEnabled = true;
	}

}
