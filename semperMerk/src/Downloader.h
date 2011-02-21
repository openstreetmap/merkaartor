#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QObject>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTemporaryFile>
#include <QSslError>

class QNetworkAccessManager;
class QNetworkReply;
class QNetworkCookieJar;
class BrowserView;

class Downloader : public QObject
{
Q_OBJECT
public:
    explicit Downloader(BrowserView *parent = 0);
    Downloader(BrowserView* parent, const QNetworkRequest& r, QNetworkCookieJar* j);
    Downloader(BrowserView* parent, QNetworkReply* r);

public:
    void setRequest(const QNetworkRequest& r);
    void go();

signals:

public slots:
    void slotReadyRead();
    void slotFinished();
#ifndef QT_NO_OPENSSL
    void slotSslErrors( QList<QSslError>);
#endif
    void slotError(QNetworkReply::NetworkError err);

protected:
    BrowserView* m_parent;
    QNetworkAccessManager* m_am;
    QNetworkRequest m_request;
    QNetworkReply* m_reply;
    QNetworkCookieJar* m_cookieJar;

    QFile *m_out;

    bool m_launch;
};

#endif // DOWNLOADER_H
