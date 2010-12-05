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
#include "MerkaartorPreferences.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>

const QString Utils::encodeAttributes(const QString & text)
{
    QString s = text;
    s.replace( "&", "&amp;" );
    s.replace( ">", "&gt;" );
    s.replace( "<", "&lt;" );
    s.replace( "\"", "&quot;" );
    s.replace( "\'", "&apos;" );
    return s;
}

bool Utils::QRectInterstects(const QRectF& r, const QLineF& lF, QPointF& a, QPointF& b)
{
    QPointF pF;
    bool hasP1 = false;
    bool hasP2 = false;

    if (QLineF(r.topLeft(), r.bottomLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
        a = pF;
        hasP1 = true;
    }
    if (QLineF(r.bottomLeft(), r.bottomRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
        if (hasP1) {
            b = pF;
            hasP2 = true;
        } else {
            a = pF;
            hasP1 = true;
        }
    }
    if (QLineF(r.bottomRight(), r.topRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
        if (hasP1) {
            b = pF;
            hasP2 = true;
        } else {
            a = pF;
            hasP1 = true;
        }
    }
    if (QLineF(r.topRight(), r.topLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
        if (hasP1) {
            b = pF;
            hasP2 = true;
        } else {
            a = pF;
            hasP1 = true;
        }
    }

    if (hasP1 && hasP2) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
        double la1 = QLineF(a,b).angleTo(lF);
#else
        double la1 = QLineF(a,b).angle(lF);
#endif
        if (la1 > 15.0 && la1 < 345.0) {
            QPointF t = b;
            b = a;
            a = t;
        }
    }
    if (hasP1)
        return true;
    else
        return false;
}

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

    tT.start(M_PREFS->getNetworkTimeout());
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

