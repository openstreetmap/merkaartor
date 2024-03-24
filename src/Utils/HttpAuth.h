#ifndef __HTTPAUTH_H__
#define __HTTPAUTH_H__

#include <QtCore>
#include <QtNetwork>

#include <QOAuth2AuthorizationCodeFlow>

class HttpAuth : public QObject
{
    Q_OBJECT

public:
    HttpAuth(QObject *parent = nullptr);
    void Login();

    void setBaseUrl(const QUrl url);

    QString token() const { return oauth2.token(); }

public slots:
    void grant() { oauth2.grant(); };

signals:
    void authenticated();
    void failed(int error);

private:
    QOAuth2AuthorizationCodeFlow oauth2;
    bool permanent = false;
    QString codeVerifier;
    QString codeChallenge;
};


#endif
