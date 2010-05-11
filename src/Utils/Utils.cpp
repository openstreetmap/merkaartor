//***************************************************************
// CLass: Utils
//
// Description: Various static functions
//
//
// Author: Chris Browet <cbro@semperpax.com> (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//******************************************************************

#include "Utils.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

bool Utils::sendBlockingNetRequest(const QUrl& theUrl, QString& reply)
{
    QNetworkAccessManager manager;
    QEventLoop q;
    QTimer tT;

    tT.setSingleShot(true);
    connect(&tT, SIGNAL(timeout()), &q, SLOT(quit()));
    connect(&manager, SIGNAL(finished(QNetworkReply*)),
            &q, SLOT(quit()));

    QNetworkReply *netReply = manager.get(QNetworkRequest(theUrl));

    tT.start(10000); // 10s timeout
    q.exec();
    if(tT.isActive()) {
        // download complete
        tT.stop();
    } else {
        return false;
    }

    reply = netReply->readAll();
    return true;
}
