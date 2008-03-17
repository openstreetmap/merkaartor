/***************************************************************************
 *   Copyright (C) 2007 by Kai Winter   *
 *   kaiwinter@gmx.de   *
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
#include "mapnetwork.h"
#include <QWaitCondition>
MapNetwork::MapNetwork(ImageManager* parent)
	:parent(parent), http(new QHttp(this)), loaded(0)
{
	connect(http, SIGNAL(requestFinished(int, bool)),
			  this, SLOT(requestFinished(int, bool)));
// 	http->setProxy("www-cache.mi.fh-wiesbaden.de", 8080);
}

MapNetwork::~MapNetwork()
{
	http->clearPendingRequests();
	delete http;
}


void MapNetwork::loadImage(const QString& hash, const QString& host, const QString& url)
{
	qDebug() << "getting: " << QString(host).append(url);
// 	http->setHost(host);
// 	int getId = http->get(url);

	http->setHost(host);
	QHttpRequestHeader header("GET", url);
	header.setValue("User-Agent", "Mozilla");
	header.setValue("Host", host);
	int getId = http->request(header);

	if (vectorMutex.tryLock())
	{
		loadingMap[getId] = hash;
		vectorMutex.unlock();
	}
}

void MapNetwork::requestFinished(int id, bool error)
{
// 	sleep(1);
// 	qDebug() << "MapNetwork::requestFinished" << http->state() << ", id: " << id;
	if (error)
	{
		qDebug() << "network error: " << http->errorString();
		//restart query

	}
	else if (vectorMutex.tryLock())
	{
	// check if id is in map?
	if (loadingMap.contains(id))
	{

		QString hash = loadingMap[id];
		loadingMap.remove(id);
		vectorMutex.unlock();
// 		qDebug() << "request finished for id: " << id;
		QByteArray ax;

		if (http->bytesAvailable()>0)
		{
			QPixmap pm;
			ax = http->readAll();
//			qDebug() << ax.size();

			if (pm.loadFromData(ax))
			{
				loaded += pm.size().width()*pm.size().height()*pm.depth()/8/1024;
// 				qDebug() << "Network loaded: " << (loaded);
				parent->receivedImage(pm, hash);
			}
			else
			{
				qDebug() << "NETWORK_PIXMAP_ERROR: " << ax;
			}
		}

	}
	else
		vectorMutex.unlock();

	}
	if (loadingMap.size() == 0)
	{
// 		qDebug () << "all loaded";
		parent->loadingQueueEmpty();
	}
}

void MapNetwork::abortLoading()
{
	http->clearPendingRequests();
// 	http->abort();
	if (vectorMutex.tryLock())
	{
		loadingMap.clear();
		vectorMutex.unlock();
	}
}

bool MapNetwork::imageIsLoading(QString hash)
{
	return loadingMap.values().contains(hash);
}

void MapNetwork::setProxy(QString host, int port)
{
#ifndef Q_WS_QWS
	http->setProxy(host, port);
#endif
}
