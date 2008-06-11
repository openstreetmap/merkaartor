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

#define MAX_REQ 8

MapNetwork::MapNetwork(IImageManager* parent)
		: parent(parent), http(new QHttp(this)), loaded(0)
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
	qDebug() << "requesting: " << QString(host).append(url);
// 	http->setHost(host);
// 	int getId = http->get(url);

	loadingRequests.enqueue(new LoadingRequest(hash, host, url));

	if (loadingMap.size() < MAX_REQ)
		launchRequest();
	else
		qDebug() << "queue full";
}

void MapNetwork::launchRequest()
{
	if (loadingRequests.isEmpty())
		return;
	LoadingRequest* R = loadingRequests.dequeue();
	qDebug() << "getting: " << QString(R->host).append(R->url);
	http->setHost(R->host);
	QHttpRequestHeader header("GET", R->url);
	header.setValue("User-Agent", "Mozilla");
	header.setValue("Host", R->host);
	int getId = http->request(header);

	if (vectorMutex.tryLock()) {
		loadingMap[getId] = R->hash;
		vectorMutex.unlock();
	}

	delete R;
}

void MapNetwork::requestFinished(int id, bool error)
{
// 	sleep(1);
// 	qDebug() << "MapNetwork::requestFinished" << http->state() << ", id: " << id;
	if (error) {
		qDebug() << "network error: " << http->errorString();
		//restart query

	} else
		if (vectorMutex.tryLock()) {
			// check if id is in map?
			if (loadingMap.contains(id)) {

				QString hash = loadingMap[id];
				loadingMap.remove(id);
				vectorMutex.unlock();
// 		qDebug() << "request finished for id: " << id;
				QByteArray ax;

				if (http->bytesAvailable() > 0) {
					QPixmap pm;
					ax = http->readAll();
//			qDebug() << ax.size();

					if (pm.loadFromData(ax)) {
						loaded += pm.size().width() * pm.size().height() * pm.depth() / 8 / 1024;
// 				qDebug() << "Network loaded: " << (loaded);
					} else {
						qDebug() << "NETWORK_PIXMAP_ERROR: " << ax;
					}
					parent->receivedImage(pm, hash);
				}

			} else
				vectorMutex.unlock();

		}

	launchRequest();
	if (loadingMap.isEmpty() && loadingRequests.isEmpty()) {
// 		qDebug () << "all loaded";
		parent->loadingQueueEmpty();
	}
}

void MapNetwork::abortLoading()
{
	http->clearPendingRequests();
// 	http->abort();

	if (vectorMutex.tryLock()) {
		loadingMap.clear();
		loadingRequests.clear();
		vectorMutex.unlock();
	}
}

bool MapNetwork::imageIsLoading(QString hash)
{
	QListIterator<LoadingRequest*> i(loadingRequests);
	while (i.hasNext())
		if (i.next()->hash == hash)
			return true;
	return loadingMap.values().contains(hash);
}

void MapNetwork::setProxy(QString host, int port)
{
#ifndef Q_WS_QWS
	http->setProxy(host, port);
#endif
}
