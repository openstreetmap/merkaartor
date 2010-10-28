/* This file is part of Qadastre
 * Copyright (C) 2010 Pierre Ducroquet <pinaraf@pinaraf.info>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef CADASTREWRAPPER_H
#define CADASTREWRAPPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSignalMapper>

class CadastreWrapper : public QObject
{
    Q_OBJECT
public:
    explicit CadastreWrapper(QObject *parent = 0);

    void requestDepartmentList();

    QMap<QString, QString> listDepartments();

    void requestCities(const QString &department);

    QMap<QString, QString> listCities(const QString &department);

    void requestPDF(const QString &cityCode, const QString &cityName);

signals:
    void departmentAvailable();
    void citiesAvailable(const QString &department);
    void cityDownloaded(const QString &code, const QString &name, const QRect &bbox, const QString& projection, QIODevice *data);
    void downloadFailed(const QString &name);

public slots:

private slots:
    void bboxAvailable(QObject* networkReply);
    void pdfReady(QObject* networkReply);

private:
    QMap<QString, QString> m_departments;
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_departmentsRequest;
    QMap<QString, QNetworkReply*> m_citiesRequest;
    QMap<QString, QMap<QString, QString> > m_cities;
    QSignalMapper m_citiesSignalMapper, m_bboxSignalMapper, m_pdfSignalMapper;
};

#endif // CADASTREWRAPPER_H
