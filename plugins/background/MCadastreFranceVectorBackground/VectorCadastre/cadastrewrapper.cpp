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


#include "cadastrewrapper.h"
#include <QUrl>
#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <QFile>
#include <QRect>

CadastreWrapper::CadastreWrapper(QObject *parent) :
    QObject(parent)
{
    m_nam = new QNetworkAccessManager(this);
    m_departmentsRequest = 0;
    // Get a cookie
    m_nam->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/rechercherPlan.do")));

    connect(&m_citiesSignalMapper, SIGNAL(mapped(QString)), this, SIGNAL(citiesAvailable(QString)));
    connect(&m_bboxSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(bboxAvailable(QObject*)));
    connect(&m_pdfSignalMapper, SIGNAL(mapped(QObject*)), this, SLOT(pdfReady(QObject*)));
}

void CadastreWrapper::requestDepartmentList()
{
    if ((m_departments.count() == 0) && (!m_departmentsRequest)) {
        m_departmentsRequest = m_nam->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/accueil.do")));
        connect(m_departmentsRequest, SIGNAL(finished()), this, SIGNAL(departmentAvailable()));
    } else if (m_departments.count() > 0)
        emit departmentAvailable();
}

QMap<QString, QString> CadastreWrapper::listDepartments()
{
    if (m_departmentsRequest && (m_departments.count() == 0)) {
        // Parse the answer
        QRegExp optParser("<option value=\"(\\d+)\">(.*)</option>");
        optParser.setMinimal(true);
        QString code = QString::fromUtf8(m_departmentsRequest->readAll());
        QString options = code.split("<select name=\"codeDepartement\"")[1].split("</select>")[0];
        int pos = 0;
        while ((pos = optParser.indexIn(options, pos)) != -1) {
            m_departments[optParser.cap(1)] = optParser.cap(2);
            pos += optParser.matchedLength();
        }
        m_departmentsRequest->deleteLater();
        m_departmentsRequest = 0;
    }
    return m_departments;
}

void CadastreWrapper::requestCities(const QString &department)
{
    QString url = QString("http://www.cadastre.gouv.fr/scpc/listerCommune.do?codeDepartement=%1&libelle=&keepVolatileSession=&offset=5000").arg(department);
    QNetworkReply *req = m_nam->get(QNetworkRequest(url));
    m_citiesSignalMapper.setMapping(req, department);
    connect(req, SIGNAL(finished()), &m_citiesSignalMapper, SLOT(map()));
    m_citiesRequest[department] = req;
}

QMap<QString, QString> CadastreWrapper::listCities(const QString &department)
{
    if ((m_citiesRequest.contains(department)) && (m_cities[department].count() == 0)) {
        QRegExp tableExtractor("<table.*class=\"resultat\".*>(.*)</table>");
        QRegExp titleExtractor("<strong>(.*) </strong>");
        QRegExp codeExtractor("afficherCarteCommune.do\\?c=(.*)'");
        tableExtractor.setMinimal(true);
        codeExtractor.setMinimal(true);
        QString code = QString::fromUtf8(m_citiesRequest[department]->readAll());
        QStringList tables;
        int pos = 0;
        while ((pos = tableExtractor.indexIn(code, pos)) != -1) {
            tables.append(tableExtractor.cap(1));
            pos += tableExtractor.matchedLength();
        }
        foreach(QString table, tables) {
            // Only vectorial communes are required
            if (!table.contains("VECT")) {
                continue;
            }
            if (titleExtractor.indexIn(table) != -1) {
                if (codeExtractor.indexIn(table) != -1) {
                    m_cities[department][codeExtractor.cap(1)] = titleExtractor.cap(1);
                }
            }
        }
        m_citiesRequest[department]->deleteLater();
        m_citiesRequest.remove(department);
    }
    return m_cities[department];
}

void CadastreWrapper::requestPDF(const QString &cityCode, const QString &cityName)
{
    QString url = QString("http://www.cadastre.gouv.fr/scpc/afficherCarteCommune.do?c=%1&dontSaveLastForward&keepVolatileSession=").arg(cityCode);
    QNetworkReply *req = m_nam->get(QNetworkRequest(url));
    req->setProperty("cityCode", cityCode);
    req->setProperty("cityName", cityName);
    m_bboxSignalMapper.setMapping(req, req);
    connect(req, SIGNAL(finished()), &m_bboxSignalMapper, SLOT(map()));
}

void CadastreWrapper::bboxAvailable(QObject *networkReply)
{
    QNetworkReply *rep = qobject_cast<QNetworkReply*>(networkReply);
    if (!rep)
        return;
    QString cityCode = rep->property("cityCode").toString();

    // Search the bounding box
    QRegExp projExtracton("<span id=\"projectionName\">(.*)</span>");
    projExtracton.setMinimal(true);
    QRegExp bbExtractor("new GeoBox\\(\n\t*(\\d*\\.\\d*),\n\t*(\\d*\\.\\d*),\n\t*(\\d*\\.\\d*),\n\t*(\\d*\\.\\d*)\\)");
    QString pageCode = rep->readAll();
    if ((projExtracton.indexIn(pageCode) != -1) && (bbExtractor.indexIn(pageCode) != -1)) {
        qDebug() << bbExtractor.capturedTexts();
        QString bbox = QString("%1,%2,%3,%4").arg(bbExtractor.cap(1)).arg(bbExtractor.cap(2)).arg(bbExtractor.cap(3)).arg(bbExtractor.cap(4));
        // Now we have everything needed to request the PDF !
        QString postData = QString("WIDTH=%1&HEIGHT=%2&MAPBBOX=%3&SLD_BODY=&RFV_REF=%4").arg(90000).arg(90000).arg(bbox).arg(cityCode);
        qDebug() << postData;
        QNetworkReply *pdfRep = m_nam->post(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/imprimerExtraitCadastralNonNormalise.do")), postData.toLocal8Bit());
        pdfRep->setProperty("cityCode", cityCode);
        pdfRep->setProperty("cityName", rep->property("cityName").toString());
        pdfRep->setProperty("boundingBox", QRect(QPoint(bbExtractor.cap(1).toInt(), bbExtractor.cap(2).toInt()), QPoint(bbExtractor.cap(3).toInt(), bbExtractor.cap(4).toInt())));

        QString projection = projExtracton.cap(1).trimmed();
        pdfRep->setProperty("projection", projection);
        m_pdfSignalMapper.setMapping(pdfRep, pdfRep);
        connect(pdfRep, SIGNAL(finished()), &m_pdfSignalMapper, SLOT(map()));
    }
}

void CadastreWrapper::pdfReady(QObject *networkReply)
{
    qDebug() << "PDF ready";
    QNetworkReply *rep = qobject_cast<QNetworkReply*>(networkReply);

    if (!rep)
        return;
    QString cityCode = rep->property("cityCode").toString();
    QString cityName = rep->property("cityName").toString();
    QRect bbox = rep->property("boundingBox").toRect();
    QString projection = rep->property("projection").toString();

    if (rep->header(QNetworkRequest::ContentTypeHeader).toString().contains("pdf")) {
        emit cityDownloaded(cityCode, cityName, bbox, projection, rep);
    } else {
        qDebug() << rep->readAll();
        emit downloadFailed(cityName);
    }
}
