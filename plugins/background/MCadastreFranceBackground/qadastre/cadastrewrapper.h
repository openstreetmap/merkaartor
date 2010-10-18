/*
    This file is part of Qadastre.

    Qadastre is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Qadastre is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Qadastre. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CADASTREWRAPPER_H
#define CADASTREWRAPPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QMap>
#include "city.h"

class CadastreWrapper : public QObject
{
Q_OBJECT
public:
    static CadastreWrapper *instance();
    void search (const QString &city, const QString &department);
    bool ready() { return m_gotCookie; }
    City requestCity (const QString &code);
    void downloadTiles(City city);
    QString tileFile (const QString &code, int row, int column);

signals:
    void resultsAvailable(QMap<QString, QString> results);

private slots:
    void networkFinished(QNetworkReply *reply);

private:

    explicit CadastreWrapper(QObject *parent = 0);

    static CadastreWrapper *m_instance;

    QNetworkAccessManager *m_networkManager;
    bool m_gotCookie;
    // fileName ==> rect
    QMap<QString, QRect> m_waitingTiles;
    // reply ==> filename
    QMap<QNetworkReply*, QString> m_pendingTiles;
};

#endif // CADASTREWRAPPER_H
