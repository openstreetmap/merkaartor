#include "Downloader.h"

#include "BrowserView.h"
#include "MyPreferences.h"

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkProxyQuery>
#include <QNetworkCookieJar>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QDesktopServices>

Downloader::Downloader(BrowserView *parent) :
    QObject(parent)
{
}

Downloader::Downloader(BrowserView* parent, const QNetworkRequest& r, QNetworkCookieJar* j)
    : QObject(parent)
    , m_parent(parent)
    , m_request(r)
    , m_reply(0)
    , m_cookieJar(j)
    , m_out(0)
    , m_launch(false)
{
    m_am = new QNetworkAccessManager(this);
}

Downloader::Downloader(BrowserView* parent, QNetworkReply* r)
    : QObject(parent)
    , m_parent(parent)
    , m_am(0)
    , m_reply(r)
    , m_out(0)
    , m_launch(true)
{
    m_reply->setParent(this);
    //    qDebug() << "Filename: " << m_reply->url();
    //    foreach(QByteArray k, m_reply->rawHeaderList()) {
    //        qDebug() << k << m_reply->rawHeader(k);
    //    }

    QString m_filename;
    if (m_reply->hasRawHeader("Content-Disposition")) {
        QStringList s1 = QString(m_reply->rawHeader("Content-Disposition")).split(";", QString::SkipEmptyParts);
        if (s1.count() && s1[0] == "attachment") {
            QStringList s2 = s1[1].split("=");
            if (s2.count() == 2) {
                m_filename = s2[1].remove('"');
            }
        }
    }
    if (m_filename.isEmpty())
        m_filename = m_reply->url().path().section('/', -1);

    if (!m_filename.isEmpty()) {
        m_out = new QFile(QDesktopServices::storageLocation(QDesktopServices::TempLocation) + "/" + m_filename, this);
        m_out->open(QIODevice::WriteOnly);
    } else {
        m_out = new QTemporaryFile(this);
        ((QTemporaryFile*)m_out)->open();
    }

    if (m_reply->isFinished())
        slotFinished();
    else {
        connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(slotError(QNetworkReply::NetworkError)));
#ifndef QT_NO_OPENSSL
        connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
            this, SLOT(slotSslErrors(QList<QSslError>)));
#endif
        connect(m_reply, SIGNAL(finished()), SLOT(slotFinished()));
    }
}

void Downloader::go()
{
    QNetworkProxyQuery npq(m_request.url());
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (listOfProxies.size())
        m_am->setProxy(listOfProxies[0]);

    m_am->setCookieJar(m_cookieJar);
    m_cookieJar->setParent(0);

    m_reply = m_am->get(m_request);
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(slotReadyRead()));
#ifndef QT_NO_OPENSSL
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
        this, SLOT(slotError(QNetworkReply::NetworkError)));
#endif
    connect(m_reply, SIGNAL(sslErrors(QList<QSslError>)),
        this, SLOT(slotSslErrors(QList<QSslError>)));
    connect(m_reply, SIGNAL(finished()), SLOT(slotFinished()));

    m_out = new QTemporaryFile(this);
    ((QTemporaryFile*)m_out)->open();
}

void Downloader::setRequest(const QNetworkRequest &r)
{
    m_request = r;
}

#ifndef QT_NO_OPENSSL
void Downloader::slotSslErrors(QList<QSslError>)
{
    m_reply->ignoreSslErrors();
}
#endif

void Downloader::slotError(QNetworkReply::NetworkError err)
{
}

void Downloader::slotReadyRead()
{
    m_out->write(m_reply->readAll());
}

void Downloader::slotFinished()
{
    m_out->write(m_reply->readAll());

    m_reply->close();
    m_out->close();
    if (!m_reply->error()) {
        if (m_launch)
            QDesktopServices::openUrl(QUrl::fromLocalFile(m_out->fileName()));
        else {
            QString m_filename;
            QString mimeType = m_reply->header(QNetworkRequest::ContentTypeHeader).toString().section(QLatin1Char(';'), 0, 0);
            if (mimeType.startsWith("image")) {
                m_filename = M_PREFS->getGeneralLastPicDownloadDir();
                if (m_filename.isEmpty())
                    m_filename = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation);
            } else {
                m_filename = M_PREFS->getGeneralLastDataDownloadDir();
                if (m_filename.isEmpty())
                    m_filename = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
            }
            m_filename = QFileDialog::getSaveFileName(m_parent, tr("Save File"), m_filename + "/" + m_reply->url().path().section('/', -1));
            if (m_filename.isEmpty())
                m_reply->abort();
            else {
                if (mimeType.startsWith("image")) {
                    M_PREFS->setGeneralLastPicDownloadDir(QFileInfo(m_filename).absolutePath());
                } else {
                    M_PREFS->setGeneralLastDataDownloadDir(QFileInfo(m_filename).absolutePath());
                }
                m_out->copy(m_filename);
            }
        }
    }
}

