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
    qDebug() << "Building a cadastreWrapper.";
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkFinished(QNetworkReply*)));
    m_networkManager->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/accueil.do")));
}

void CadastreWrapper::search(const QString &city, const QString &department)
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

City CadastreWrapper::requestCity(const QString &code)
{
    QDir cache(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    QNetworkReply *reply = m_networkManager->get(QNetworkRequest(QUrl("http://www.cadastre.gouv.fr/scpc/afficherCarteCommune.do?c=" + code)));
    while (!reply->isFinished())
        qApp->processEvents();
    cache.cd(code);
    QSettings raw_city(cache.absoluteFilePath("cache.ini"), QSettings::IniFormat);
    City result(code);
    result.setName(raw_city.value("name").toString());
    result.setDepartement(raw_city.value("department").toString());
    result.setGeometry(raw_city.value("geometry").toRect());
    return result;
}

QString CadastreWrapper::tileFile(const QString &code, int row, int column)
{
    QDir cache(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    cache.cd(code);
    QString fileName = QString("%1-%2.png").arg(row).arg(column);
    return cache.absoluteFilePath(fileName);
}

void CadastreWrapper::downloadTiles(City city)
{
    m_progress = new QProgressDialog();
    m_progress->setWindowTitle(QApplication::tr("Downloading tiles..."));
    m_progress->setMaximum(city.tileRows() * city.tileColumns());
    m_progress->setMinimum(0);
    m_progress->setValue(0);
    m_progress->show();

    QDir cache(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
    cache.cd(city.code());

    for (int r = 0 ; r < city.tileRows() ; ++r) {
        for (int c = 0 ; c < city.tileColumns() ; ++c) {
            QString fileName = QString("%1-%2.png").arg(r).arg(c);
            qDebug() << fileName;
            if (cache.exists(fileName)) {
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

    m_progress->hide();
    m_progress->deleteLater();
    m_progress = NULL;
}

void CadastreWrapper::networkFinished(QNetworkReply *reply)
{
    if (m_pendingTiles.contains(reply)) {
        QFile target(m_pendingTiles[reply]);
        target.open(QIODevice::WriteOnly);
        target.write(reply->readAll());
        target.close();
        m_pendingTiles.remove(reply);
        if (m_progress) {
            m_progress->setValue(m_progress->value()+1);
            m_progress->setLabelText(tr("Downloaded: %2/%3").arg(m_progress->value()).arg(m_progress->maximum()));
        }
    } else if (reply->url() == QUrl("http://www.cadastre.gouv.fr/scpc/accueil.do")) {
        qDebug() << "Ok, I've got a cookie... I LOVE COOKIES.";
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
        qDebug() << pageData;
        QString name, code, projection;
        code = reply->url().queryItemValue("c");
        qDebug() << code;
        bool inGeoBox = false;
        QList<int> raw_geometry;
        foreach (QString line, pageData.split('\n')) {
            line = line.trimmed();
            if (name.isEmpty())
                if (line.contains("<title>"))
                    name = line.split(" : ")[1].split(" - ")[0];
            if (projection.isEmpty()) {
                if (line.contains("projectionName")) {
                    QRegExp reg("<span id=\"projectionName\">(.+)</span>");
                    reg.setMinimal(true);
                    if (reg.indexIn(line) > -1) {
                        projection = reg.cap(1);
                        qDebug() << projection;
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
        qDebug() << raw_geometry;
        QRect geometry(raw_geometry[0], raw_geometry[3], raw_geometry[2]-raw_geometry[0], raw_geometry[3]-raw_geometry[1]);
        qDebug() << geometry;
        QDir cache(QDesktopServices::storageLocation(QDesktopServices::DataLocation));
        if (!cache.exists(code))
            cache.mkdir(code);
        cache.cd(code);
        qDebug() << cache.absoluteFilePath("cache.ini");
        QSettings settings(cache.absoluteFilePath("cache.ini"), QSettings::IniFormat);
        settings.setValue("name", name);
        settings.setValue("geometry", geometry);
        settings.setValue("department", name.mid(name.lastIndexOf('(')+1, 2));
        settings.setValue("projection", projection);
        settings.sync();
    }
}
