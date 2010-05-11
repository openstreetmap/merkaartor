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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
#include <QUrl>

class Utils: public QObject
{
    Q_OBJECT

public:

    static bool sendBlockingNetRequest(const QUrl& theUrl, QString& reply);
};

#endif // UTILS_H
