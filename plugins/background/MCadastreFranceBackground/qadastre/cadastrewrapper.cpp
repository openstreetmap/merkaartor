/*
   This file is part of Qadastre.
   Copyright (C)  2010 Pierre Ducroquet <pinaraf@pinaraf.info>

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

#include "cadastrewrapper.h"
#include <QUrl>
#include <QDebug>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QWebPage>
#include <QWebFrame>
#include <QWebElement>
#include <QDesktopServices>
#include <QDir>
#include <QFile>
#include <QApplication>
#include <QList>
#include <QSettings>
#include <QImage>
#include <QColor>

CadastreWrapper *CadastreWrapper::m_instance = 0;

CadastreWrapper *CadastreWrapper::instance()
{
    if (!CadastreWrapper::m_instance)
        CadastreWrapper::m_instance = new CadastreWrapper;
    return CadastreWrapper::m_instance;
}

CadastreWrapper::CadastreWrapper(QObject *parent) :
    QObject(parent), m_gotCookie(false)
{
    setRootCacheDir(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
}

void CadastreWrapper::setNetworkManager(QNetworkAccessManager *aManager)
{
    m_networkManager = aManager;
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkFinished(QNetworkReply*)));
    m_networkManager->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/accueil.do")));
}

void CadastreWrapper::searchVille(const QString &city, const QString &department)
{
    // {"numerovoie": "", "indiceRepetition": "", "nomvoie": "", "lieuDit": "", "ville": city.upper(), "codePostal": "", "codeDepartement": dept, "nbResultatParPage": 20, "x": 0, "y" : 0}
    QString data = QString("numerovoie=&indiceRepetition=&nomvoie=&lieuDit=&ville=%1&codePostal=&codeDepartement=%2&nbResultatParPage=20&x=0&y=0")
                   .arg(QString::fromAscii(QUrl::toPercentEncoding(city.toUpper())))
                   .arg(department);
    qDebug() << data;
    qDebug() << data.toAscii();
    qDebug() << m_networkManager;
    m_networkManager->post(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/rechercherPlan.do")), data.toAscii());
}

void CadastreWrapper::searchCode(const QString &code, const QString &department)
{
    // {"numerovoie": "", "indiceRepetition": "", "nomvoie": "", "lieuDit": "", "ville": city.upper(), "codePostal": "", "codeDepartement": dept, "nbResultatParPage": 20, "x": 0, "y" : 0}
    QString data = QString("numerovoie=&indiceRepetition=&nomvoie=&lieuDit=&codeCommune=%1&codeDepartement=%2&nbResultatParPage=20&x=0&y=0")
                   .arg(QString::fromAscii(QUrl::toPercentEncoding(code.toUpper())))
                   .arg(department);
    qDebug() << data;
    qDebug() << data.toAscii();
    qDebug() << m_networkManager;
    m_networkManager->post(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/rechercherPlan.do")), data.toAscii());
}

City CadastreWrapper::requestCity(const QString &code)
{
    qDebug() << "Request city: " << code;
    QDir cache = m_cacheDir;
    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/afficherCarteCommune.do?c=" + code)));
    while (!reply->isFinished())
        qApp->processEvents();
    cache.cd(code);
    QSettings raw_city(cache.absoluteFilePath("cache.ini"), QSettings::IniFormat);
    City result(code);
    result.setName(raw_city.value("name").toString());
    result.setDepartement(raw_city.value("department").toString());
    result.setGeometry(raw_city.value("geometry").toRect());
    result.setProjection(raw_city.value("projection").toString());
    return result;
}

QString CadastreWrapper::tileFile(const QString &code, int row, int column)
{
    QDir cache = m_cacheDir;
    cache.cd(code);
    QString fileName = QString("%1-%2.png").arg(row).arg(column);
    return cache.absoluteFilePath(fileName);
}

bool CadastreWrapper::downloadTiles(City city)
{
    m_progress = new QProgressDialog();
    m_progress->setWindowTitle(QApplication::tr("Downloading tiles..."));
    m_progress->setMaximum(city.tileRows() * city.tileColumns());
    m_progress->setMinimum(0);
    m_progress->setValue(0);
    m_progress->show();

    QDir cache = m_cacheDir;
    cache.cd(city.code());

    for (int r = 0 ; r < city.tileRows() ; ++r) {
        for (int c = 0 ; c < city.tileColumns() ; ++c) {
            QString fileName = QString("%1-%2.png").arg(r).arg(c);
            qDebug() << fileName;
            if (cache.exists(fileName) && QFileInfo(cache, fileName).size()) {
                // the file already exists, cool !
            } else {
                QRect rect = city.tileGeometry(r, c);
                QString bbox = QString("%1.0,%2.0,%3.0,%4.0").arg(rect.left()).arg(rect.bottom()).arg(rect.right()).arg(rect.top());
                fileName = cache.absoluteFilePath(fileName);
                m_waitingTiles[fileName] = city.tileGeometry(r, c);
            }
        }
    }
    m_progress->setMaximum(m_waitingTiles.count());
    m_startTime = QDateTime::currentDateTime();
    while (m_waitingTiles.count() > 0 && !m_progress->wasCanceled()) {
        QString fileName = m_waitingTiles.begin().key();
        QRect rect = m_waitingTiles.begin().value();
        m_waitingTiles.take(fileName);
        QString bbox = QString("%1.0,%2.0,%3.0,%4.0").arg(rect.left()).arg(rect.top()).arg(rect.right()).arg(rect.bottom());
        QString url = QString("http://www.cadastre.gouv.fr/scpc/wms?version=1.1&request=GetMap&layers=CDIF:LS3,CDIF:LS2,CDIF:LS1,CDIF:PARCELLE,CDIF:NUMERO,CDIF:PT3,CDIF:PT2,CDIF:PT1,CDIF:LIEUDIT,CDIF:COMMUNE&format=image/png&bbox=%1&width=600&height=600&exception=application/vnd.ogc.se_inimage&styles=LS3_90,LS2_90,LS1_90,PARCELLE_90,NUMERO_90,PT3_90,PT2_90,PT1_90,LIEUDIT_90,COMMUNE_90").arg(bbox);
        qDebug() << url;
        m_pendingTiles[m_networkManager->get(QNetworkRequest(QUrl(url)))] = fileName;
        while (m_pendingTiles.count() > 0 && !m_progress->wasCanceled()) {
            qApp->processEvents();
        }
    }

    bool ret = true;
    if (m_progress->wasCanceled())
        ret = false;
    else {
        QSettings settings(cache.absoluteFilePath("cache.ini"), QSettings::IniFormat);
        settings.setValue("complete", true);
        settings.sync();
    }

    m_progress->hide();
    m_progress->deleteLater();
    m_progress = NULL;

    return ret;
}

void CadastreWrapper::networkFinished(QNetworkReply *reply)
{
    if (m_pendingTiles.contains(reply)) {
        QFile target(m_pendingTiles[reply]);
        QByteArray ba = reply->readAll();

        // white -> transparent
        QImage img;
        img.loadFromData(ba);
        QImage img2 = img.convertToFormat(QImage::Format_ARGB32);
        Q_ASSERT(img2.hasAlphaChannel());
        int w=0;
        for (int y=0; y<img2.height(); ++y) {
            for (int x=0; x<img2.width(); ++x) {
                QColor col = QColor(img2.pixel(x, y));
                if (col == QColor(255, 255, 255)) {
                    col.setAlpha(0);
                    img2.setPixel(x, y, col.rgba());
                    ++w;
               }
            }
        }
        //Full transparent
        if (w == img2.height()*img2.width()) {
            img2 = QImage(1, 1, QImage::Format_ARGB32);
            img2.setPixel(0, 0, QColor(0, 0, 0, 0).rgba());
        }

        target.open(QIODevice::WriteOnly);
//        target.write(reply->readAll());
        img2.save(&target, "PNG");
        target.close();
        m_pendingTiles.remove(reply);
        if (m_progress) {
            m_progress->setValue(m_progress->value()+1);
            if (m_progress->value() > 10) {
                double ms = m_startTime.secsTo(QDateTime::currentDateTime());
                double us = ms/m_progress->value();
                int tot = us*(m_progress->maximum() - m_progress->value());

                if (tot<3600)
                    m_progress->setLabelText(tr("Downloaded: %1/%2\nRemaining time: %3:%4").arg(m_progress->value()).arg(m_progress->maximum()).arg(int(tot/60)).arg(int(tot%60), 2, 10, QChar('0')));
                else
                    m_progress->setLabelText(tr("Downloaded: %1/%2\nRemaining time: %3:%4:%5").arg(m_progress->value()).arg(m_progress->maximum()).arg(int(tot/3600)).arg(int((tot%3600)/60), 2, 10, QChar('0')).arg(int(tot%60), 2, 10, QChar('0')));

            } else
                m_progress->setLabelText(tr("Downloaded: %1/%2").arg(m_progress->value()).arg(m_progress->maximum()));
        }
    } else if (reply->url() == QUrl("http://www.cadastre.gouv.fr/scpc/accueil.do")) {
        qDebug() << "Ok, I've got a cookie... I LOVE COOKIES.";
        reply->readAll();
        m_gotCookie = true;
    } else if (reply->url() == QUrl("http://www.cadastre.gouv.fr/scpc/rechercherPlan.do")) {
        QString pageData = reply->readAll();
        QWebPage parsedPage(this);
        QWebFrame *frame = parsedPage.mainFrame();
        frame->setHtml(pageData);
        QWebElement codeCommune = frame->findFirstElement("#codeCommune");
        QMap<QString, QString> results;
        if (!codeCommune.isNull()) {
            // If there is a codeCommune object in the DOM, it means that the search was not successfull.
            if (codeCommune.tagName().toLower() != "select") {
                qDebug() << "Invalid page ???";
                return;
            }
            QWebElementCollection options = codeCommune.findAll("option");
            foreach (QWebElement option, options) {
                if (!option.attribute("value").isEmpty())
                    results[option.attribute("value")] = option.toPlainText();
            }
        } else {
            // We may have been successfull, who knows ?
            QString name = frame->findFirstElement("#ville").attribute("value");
            QWebElementCollection links = frame->findAllElements(".resultat > .parcelles .view a");
            QRegExp linkRE("c=(\\w+)");
            foreach (QWebElement link, links) {
                QString js = link.attribute("onclick");
                int pos = linkRE.indexIn(js);
                if (pos > -1) {
                    results[linkRE.cap(1)] = name;
                }
            }
        }
        qDebug() << results;
        emit(resultsAvailable(results));
    } else if (reply->url().toString().startsWith("http://www.cadastre.gouv.fr/scpc/afficherCarteCommune.do?c=")) {
        qDebug() << "Got a result !";
        QString pageData = reply->readAll();
        if (pageData.isEmpty())
            return;
//        qDebug() << pageData;
        QString name, code, projection;
        code = reply->url().queryItemValue("c");
        qDebug() << code;
        bool inGeoBox = false;
        QList<int> raw_geometry;
        foreach (QString line, pageData.split('\n')) {
            line = line.trimmed();
            if (name.isEmpty()) {
                if (line.contains("<title>"))
                    if (line.split(" : ").count() > 1)
                        name = line.split(" : ")[1].split(" - ")[0];
            }
            if (projection.isEmpty()) {
                if (line.contains("projectionName")) {
                    QRegExp reg("<span id=\"projectionName\">(.+)</span>");
                    reg.setMinimal(true);
                    if (reg.indexIn(line) > -1) {
                        projection = reg.cap(1);
                        qDebug() << projection;
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
                    }
                }
            }
            if (inGeoBox) {
                raw_geometry.append(line.split(".")[0].split(")")[0].toInt());
                if (line.contains(')'))
                    inGeoBox = false;
            }
            if (line == "new GeoBox(")
                inGeoBox = true;
        }
        if (!raw_geometry.size())
            return;
        qDebug() << raw_geometry;
        QRect geometry(raw_geometry[0], raw_geometry[1], raw_geometry[2]-raw_geometry[0], raw_geometry[3]-raw_geometry[1]);
        qDebug() << geometry;
        QDir cache = m_cacheDir;
        if (!cache.exists(code))
            cache.mkdir(code);
        cache.cd(code);
        qDebug() << cache.absoluteFilePath("cache.ini");
        QSettings settings(cache.absoluteFilePath("cache.ini"), QSettings::IniFormat);
        settings.setValue("name", name);
        settings.setValue("geometry", geometry);
        settings.setValue("department", name.mid(name.lastIndexOf('(')+1, name.lastIndexOf(')')-name.lastIndexOf('(')-1));
        settings.setValue("projection", projection);
        settings.sync();
    }
    reply->close();
}

void CadastreWrapper::setRootCacheDir(QDir dir)
{
    m_cacheDir = dir;
    if (!m_cacheDir.cd("qadastre")) {
        m_cacheDir.mkdir("qadastre");
        m_cacheDir.cd("qadastre");
    }
}

QDir CadastreWrapper::getCacheDir()
{
    return m_cacheDir;
}
