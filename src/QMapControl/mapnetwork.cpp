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

#include "Preferences/MerkaartorPreferences.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#define MAX_REQ 8

MapNetwork::MapNetwork(IImageManager* parent)
        : parent(parent), loaded(0)
{
    m_networkManager = parent->getNetworkManager();
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished(QNetworkReply*)));
}

MapNetwork::~MapNetwork()
{
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

    QUrl U("http://" + QString(R->host).append(R->url));
    qDebug() << "getting: " << U.toString();

    launchRequest(U, R->hash);

    delete R;
}

void MapNetwork::launchRequest(QUrl url, QString hash)
{
    m_networkManager->setProxy(M_PREFS->getProxy(url));
    QNetworkRequest req(url);

    req.setRawHeader("Host", url.host().toLatin1());
    req.setRawHeader("Accept", "image/*");
    req.setRawHeader("User-Agent", USER_AGENT.toLatin1());

    QNetworkReply* rply = m_networkManager->get(req);

    if (vectorMutex.tryLock()) {
        loadingMap[rply] = hash;
        vectorMutex.unlock();
    }
}

void MapNetwork::requestFinished(QNetworkReply* reply)
{
    if (!loadingMap.contains(reply)){
        // Don't react on setProxy and setHost requestFinished...
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "network error: " << statusCode << " " << reply->errorString();
        loadingMap.remove(reply);
    } else
        switch (statusCode) {
            case 301:
            case 302:
            case 307:
                qDebug() << "redirected: " << loadingMap[reply];
                launchRequest(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl(), loadingMap[reply]);
                loadingMap.remove(reply);
                return;
            case 404:
                qDebug() << "404 error: " << loadingMap[reply];
                loadingMap.remove(reply);
                break;
            case 500:
                qDebug() << "500 error: " << loadingMap[reply];
                loadingMap.remove(reply);
                break;
                // Redirected
            default:
                if (statusCode != 200)
                    qDebug() << "Other http code (" << statusCode << "): "  << loadingMap[reply];
                if (vectorMutex.tryLock()) {
                    // check if id is in map?
                    if (loadingMap.contains(reply)) {

                        QString hash = loadingMap[reply];
                        loadingMap.remove(reply);
                        vectorMutex.unlock();
        // 		qDebug() << "request finished for id: " << id;
                        QByteArray ax;

                        if (reply->bytesAvailable() > 0) {
                            QPixmap pm;
                            ax = reply->readAll();
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
                break;
        }


    launchRequest();
    if (loadingMap.isEmpty() && loadingRequests.isEmpty()) {
// 		qDebug () << "all loaded";
        parent->loadingQueueEmpty();
    }
}

void MapNetwork::abortLoading()
{
    if (vectorMutex.tryLock()) {
        foreach (QNetworkReply* rply, loadingMap.keys())
            rply->abort();
        loadingMap.clear();
        while (!loadingRequests.isEmpty())
            delete loadingRequests.dequeue();
        //loadingRequests.clear();
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
