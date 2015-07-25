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

#include "MerkaartorPreferences.h"

#include <QNetworkRequest>
#include <QNetworkReply>

#define MAX_REQ 8

MapNetwork::MapNetwork(IImageManager* parent)
        : parent(parent)
{
    m_networkManager = parent->getNetworkManager();
    m_networkManager->setProxy(M_PREFS->getProxy(QUrl("http://merkaartor.be")));
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(requestFinished(QNetworkReply*)));
}

MapNetwork::~MapNetwork()
{
}


void MapNetwork::load(const QString& hash, const QString& host, const QString& url)
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

    QUrl theUrl;
    if (R->host.contains("://")) {
        theUrl.setUrl(QString(R->host).append(R->url));
    } else {
        theUrl.setUrl("http://" + QString(R->host).append(R->url));
    }

    qDebug() << "getting: " << theUrl.toString();

    launchRequest(theUrl, R);
}

void MapNetwork::launchRequest(QUrl url, LoadingRequest* R)
{
    QNetworkRequest req(url);

    req.setRawHeader("Host", url.host().toLatin1());
    req.setRawHeader("Accept", "image/*");
    req.setRawHeader("User-Agent", USER_AGENT.toLatin1());

    QNetworkReply* reply = m_networkManager->get(req);
    loadingMap[reply] = R;

    QTimer* timeoutTimer = new QTimer();
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(timeout()));
    timeoutTimer->setInterval(M_PREFS->getNetworkTimeout());
    timeoutTimer->setSingleShot(true);

    timeoutMap[timeoutTimer] = reply;
    timeoutTimer->start();
}

void MapNetwork::requestFinished(QNetworkReply* reply)
{
    if (!loadingMap.contains(reply)){
        // Don't react on setProxy and setHost requestFinished...
        return;
    }
    QTimer* t = timeoutMap.key(reply);
    Q_ASSERT(t);
    t->stop();
    timeoutMap.remove(t);
    delete t;

    LoadingRequest* R = loadingMap[reply];
    loadingMap.remove(reply);

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            qDebug() << "network error: " << statusCode << " " << reply->errorString();
    } else
        switch (statusCode) {
            case 301:
            case 302:
            case 307:
                qDebug() << "redirected: " << R->host << R->url;
                launchRequest(reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl(), new LoadingRequest(R->hash, R->host, R->url));
                return;
            case 404:
                qDebug() << "404 error: " << R->host << R->url;
                break;
            case 500:
                qDebug() << "500 error: " << R->host << R->url;
                break;
                // Redirected
            default:
                if (statusCode != 200)
                    qDebug() << "Other http code (" << statusCode << "): "  << R->host << R->url;
                QString hash = R->hash;
                // 		qDebug() << "request finished for id: " << id;
                QByteArray ax;
                QHash<QString, QString> headers;

                if (reply->bytesAvailable() > 0) {
                    ax = reply->readAll();
                    foreach (QByteArray k, reply->rawHeaderList()) {
                        headers[QString(k)] = QString(reply->rawHeader(k));
                    }

                    parent->receivedData(ax, headers, hash);
                }
                break;
        }
    delete R;

    launchRequest();
    if (loadingMap.isEmpty() && loadingRequests.isEmpty()) {
// 		qDebug () << "all loaded";
        parent->loadingQueueEmpty();
    }
}

void MapNetwork::abortLoading()
{
    foreach (QNetworkReply* rply, loadingMap.keys())
        rply->abort();
    loadingMap.clear();
    while (!loadingRequests.isEmpty())
        delete loadingRequests.dequeue();
    //loadingRequests.clear();
}

bool MapNetwork::isLoading(QString hash)
{
    QListIterator<LoadingRequest*> i(loadingRequests);
    while (i.hasNext())
        if (i.next()->hash == hash)
            return true;

    QMapIterator<QNetworkReply*, LoadingRequest*> j(loadingMap);
    while (j.hasNext())
        if (j.next().value()->hash == hash)
            return true;

    return false;
}

void MapNetwork::timeout()
{
    QTimer* t = qobject_cast<QTimer*>(sender());
    Q_ASSERT(t);

    QNetworkReply* rply = timeoutMap[t];
    if (!loadingMap.contains(rply))
        return;

    LoadingRequest* R = loadingMap[rply];
    qDebug() << "MapNetwork::timeout: " << R->host << R->url;
    loadingMap.remove(rply);
    loadingRequests.enqueue(R);

    rply->abort();
    timeoutMap.remove(t);
    delete t;

    if (loadingMap.size() < MAX_REQ)
        launchRequest();
}
