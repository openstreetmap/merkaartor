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
        if (projection.compare("RGF93CC42", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 1 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=42.000000000 +lon_0=3.000000000 +lat_1=41.250000000 +lat_2=42.750000000 +x_0=1700000.000 +y_0=1200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC43", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 2 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=43.000000000 +lon_0=3.000000000 +lat_1=42.250000000 +lat_2=43.750000000 +x_0=1700000.000 +y_0=2200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC44", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 3 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=44.000000000 +lon_0=3.000000000 +lat_1=43.250000000 +lat_2=44.750000000 +x_0=1700000.000 +y_0=3200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC45", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 4 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=45.000000000 +lon_0=3.000000000 +lat_1=44.250000000 +lat_2=45.750000000 +x_0=1700000.000 +y_0=4200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC46", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 5 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=46.000000000 +lon_0=3.000000000 +lat_1=45.250000000 +lat_2=46.750000000 +x_0=1700000.000 +y_0=5200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC47", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 6 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=47.000000000 +lon_0=3.000000000 +lat_1=46.250000000 +lat_2=47.750000000 +x_0=1700000.000 +y_0=6200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC48", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 7 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=48.000000000 +lon_0=3.000000000 +lat_1=47.250000000 +lat_2=48.750000000 +x_0=1700000.000 +y_0=7200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC49", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 8 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=49.000000000 +lon_0=3.000000000 +lat_1=48.250000000 +lat_2=49.750000000 +x_0=1700000.000 +y_0=8200000.000 +units=m +no_defs";
        else if (projection.compare("RGF93CC50", Qt::CaseInsensitive) == 0)
            projection = "+title=Projection conique conforme Zone 9 +proj=lcc +towgs84=0.0000,0.0000,0.0000 +a=6378137.0000 +rf=298.2572221010000 +lat_0=50.000000000 +lon_0=3.000000000 +lat_1=49.250000000 +lat_2=50.750000000 +x_0=1700000.000 +y_0=9200000.000 +units=m +no_defs";
        else if (projection.compare("LAMB1", Qt::CaseInsensitive) == 0)
            projection = "+title=Lambert I +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=49.500000000 +lon_0=0.000000000 +k_0=0.99987734 +lat_1=49.500000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
        else if (projection.compare("LAMB2", Qt::CaseInsensitive) == 0)
            projection = "+title=Lambert II +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=46.800000000 +lon_0=0.000000000 +k_0=0.99987742 +lat_1=46.800000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
        else if (projection.compare("LAMB3", Qt::CaseInsensitive) == 0)
            projection = "+title=Lambert III +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=44.100000000 +lon_0=0.000000000 +k_0=0.99987750 +lat_1=44.100000000 +x_0=600000.000 +y_0=200000.000 +units=m +no_defs";
        else if (projection.compare("LAMB4", Qt::CaseInsensitive) == 0)
            projection = "+title=Lambert IV +proj=lcc +nadgrids=ntf_r93.gsb,null +towgs84=-168.0000,-60.0000,320.0000 +a=6378249.2000 +rf=293.4660210000000 +pm=2.337229167 +lat_0=42.165000000 +lon_0=0.000000000 +k_0=0.99994471 +lat_1=42.165000000 +x_0=234.358 +y_0=185861.369 +units=m +no_defs";
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
