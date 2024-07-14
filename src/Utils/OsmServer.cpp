#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QAuthenticator>
#include <QDesktopServices>
#include <QTimer>

#include "OsmServer.h"

#include <random>
//
//FIXME: The client ID and secret are not really a secret, since there is no
//way to hide them in a desktop application. In case of abuse, the client ID
//can be revoked by osm.org. It might be a good idea to make it configurable,
//so that the user can enter their own client ID and secret in this case.
//
//Client ID 	    wv3ui28EyHjH0c4C1Wuz6_I-o47ithPAOt7Qt1ov9Ps
//Client Secret 	evCjgZOGTRL70ezsXs3VbxG0ugjJ5hq7pFQMB6toBcM // FIXME: The secret is not needed. Why?
//Make sure to save this secret - it will not be accessible again
//Permissions
//
//    Read user preferences (read_prefs)
//    Modify user preferences (write_prefs)
//    Create diary entries, comments and make friends (write_diary)
//    Modify the map (write_api)
//    Read private GPS traces (read_gpx)
//    Upload GPS traces (write_gpx)
//    Modify notes (write_notes)
//
//Redirect URIs
//
//    http://127.0.0.1:1337/

class OsmServerImplBasic : public IOsmServerImpl {
    public:
        OsmServerImplBasic(OsmServerInfo& info, QNetworkAccessManager& manager)
            : m_info(info), m_manager(manager) {
        }

        QUrl baseUrl() const {
            return m_info.Url;
        }

        OsmServerInfo const getServerInfo() const { return m_info; }

        virtual QNetworkReply* get(const QUrl &url) {
            return m_manager.get(QNetworkRequest(baseUrl().resolved(url)));
        }

        virtual QNetworkReply* put(const QUrl &url, const QByteArray &data) {
            return m_manager.put(QNetworkRequest(baseUrl().resolved(url)), data);
        }

        virtual QNetworkReply* deleteResource(const QUrl &url) {
            return m_manager.deleteResource(QNetworkRequest(baseUrl().resolved(url)));
        }

        virtual QNetworkReply* sendRequest(QNetworkRequest &request, const QByteArray &verb, const QByteArray &data) {
            return m_manager.sendCustomRequest(request, verb, data);
        }

        virtual void authenticate() {
            QUrl base(m_info.Url);
            auto reply = m_manager.get(QNetworkRequest(base.resolved(QUrl("api/0.6/user/details"))));
            connect(reply, &QNetworkReply::finished, [=]() {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError) {
                    qCritical() << "Error:" << reply->errorString();
                    emit failed(Error::Unauthorized, reply->errorString());
                } else {
                    qDebug() << reply->readAll();
                    emit authenticated();
                }
            });
        }

    private slots:
        void on_authenticationRequired( QNetworkReply *reply, QAuthenticator *auth );
        void on_sslErrors(QNetworkReply *reply, const QList<QSslError>& errors);

    private:
        OsmServerInfo m_info;
        QNetworkAccessManager& m_manager;
};

class OsmServerImplOAuth2 : public IOsmServerImpl {
    public:
        OsmServerImplOAuth2(OsmServerInfo& info, QNetworkAccessManager& manager);

        QUrl baseUrl() const {
            return m_info.Url;
        }

        OsmServerInfo const getServerInfo() const { return m_info; }

        virtual QNetworkReply* get(const QUrl &url) {
            qDebug() << "get: " << baseUrl().resolved(url);
            return m_oauth2.get(baseUrl().resolved(url));
        }

        virtual QNetworkReply* put(const QUrl &url, const QByteArray &data) {
            qDebug() << "put: " << baseUrl().resolved(url);
            return m_oauth2.put(baseUrl().resolved(url), data);
        }

        virtual QNetworkReply* deleteResource(const QUrl &url) {
            qDebug() << "delete: " << baseUrl().resolved(url);
            return m_oauth2.deleteResource(baseUrl().resolved(url));
        }
        
        virtual QNetworkReply* sendRequest(QNetworkRequest &request, const QByteArray &verb, const QByteArray &data) {
            qDebug() << "custom request" << request.url() << " with verb" << verb << "and data" << data;
            m_oauth2.prepareRequest(&request, data);
            return m_manager.sendCustomRequest(request, verb, data);
        }

        virtual void authenticate();

    private:
        OsmServerInfo m_info;
        QNetworkAccessManager& m_manager;
        QOAuth2AuthorizationCodeFlow m_oauth2;

        QString generateCodeVerifier();
};

std::shared_ptr<IOsmServerImpl> makeOsmServer(OsmServerInfo& info, QNetworkAccessManager& manager) {
    if (info.Type == OsmServerInfo::AuthType::Basic) {
        return std::make_shared<OsmServerImplBasic>(info, manager);
    } else if (info.Type == OsmServerInfo::AuthType::OAuth2) {
        return std::make_shared<OsmServerImplOAuth2>(info, manager);
    } else {
        qWarning() << "Error creating OsmServer: Unknown AuthType" << static_cast<int>(info.Type);
        return nullptr;
    }
}

/***********************************************************************
 * OsmServerBasic
 **********************************************************************/

void OsmServerImplBasic::on_authenticationRequired( QNetworkReply *reply, QAuthenticator *auth ) {
    static QNetworkReply *lastReply = NULL;

    /* Only provide authentication the first time we see this reply, to avoid
     * infinite loop providing the same credentials. */
    if (lastReply != reply) {
        lastReply = reply;
        qDebug(/*lc_MerkaartorPreferences */) << "Authentication required and provided.";
        auth->setUser(m_info.User);
        auth->setPassword(m_info.Password);
    }
}

void OsmServerImplBasic::on_sslErrors(QNetworkReply *reply, const QList<QSslError>& errors) {
    Q_UNUSED(reply);
    qDebug(/*lc_MerkaartorPreferences*/) << "We stumbled upon some SSL errors: ";
    foreach ( QSslError error, errors ) {
        qDebug(/*lc_MerkaartorPreferences*/) << "1:";
        qDebug(/*lc_MerkaartorPreferences*/) << error.errorString();
    }
}

/***********************************************************************
 * OsmServerOAuth2
 **********************************************************************/

QString OsmServerImplOAuth2::generateCodeVerifier() {
    std::random_device rd;
    std::mt19937 gen(rd());
    int bytes = 32;
    std::uniform_int_distribution<int> dis(0, 255);
    QString result;
    for (int i = 0; i < bytes; i++) {
        result.append(QString::number(dis(gen), 16));
    }
    return result;
}

OsmServerImplOAuth2::OsmServerImplOAuth2(OsmServerInfo& info, QNetworkAccessManager& manager)
    : m_info(info), m_manager(manager),m_oauth2(&manager)
{
    m_oauth2.setScope("read_prefs write_prefs write_api read_gpx write_gpx write_notes");
    //m_oauth2.setScope("read_prefs%20write_prefs%20write_api%20read_gpx%20write_gpx%20write_notes");
    QString host = QUrl(m_info.Url).host();
    if (host == "master.apis.dev.openstreetmap.org") {
        m_oauth2.setClientIdentifier("Ydpjx_nAy3fnGN5X1LiFwqToueYvwV0Nl1_IUUi7H3I");
        m_oauth2.setClientIdentifierSharedKey("SuWJYZr9hjCmZH4PaMrvB48aVR_vgIw4D2VUEKPlR4c");
    } else {
        if (host != "openstreetmap.org") {
            qWarning() << "Unknown OSM API host " << host << ", assuming alias to openstreetmap.org";
        }
        m_oauth2.setClientIdentifier("wv3ui28EyHjH0c4C1Wuz6_I-o47ithPAOt7Qt1ov9Ps");
        m_oauth2.setClientIdentifierSharedKey("evCjgZOGTRL70ezsXs3VbxG0ugjJ5hq7pFQMB6toBcM");
    }
    m_oauth2.setToken(m_info.Password);

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, &QDesktopServices::openUrl);
}

void OsmServerImplOAuth2::authenticate() {
    m_oauth2.setAuthorizationUrl(baseUrl().resolved(QUrl("oauth2/authorize")));
    m_oauth2.setAccessTokenUrl(baseUrl().resolved(QUrl("oauth2/token")));
    qDebug() << "Base URL: " << baseUrl();
    qDebug() << "Authorization URL: " << m_oauth2.authorizationUrl();
    qDebug() << "Access Token URL: " << m_oauth2.accessTokenUrl();

    auto replyHandler = new QOAuthHttpServerReplyHandler(QHostAddress("127.0.0.1"), 1337, this);
    QTimer::singleShot(60000, replyHandler, [=]() {
        qWarning() << "OAuth2 reply handler timed out.";
        replyHandler->deleteLater();
        emit failed(Error::Timeout, tr("OAuth2 reply handler timed out."));
    });

    m_oauth2.setReplyHandler(replyHandler);
    QString codeVerifier = generateCodeVerifier();
    QString codeChallenge = QString(QCryptographicHash::hash(codeVerifier.toUtf8(), QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        qDebug() << "OAuth status changed to " << int(status);
        if (status == QAbstractOAuth::Status::Granted) {
            qDebug() << "Granted, token: " << m_oauth2.token();
            m_info.Password = m_oauth2.token();
            emit authenticated();
            replyHandler->deleteLater();
            auto reply = get(QUrl("/api/0.6/user/details"));
            connect(reply, &QNetworkReply::finished, [=]() {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError) {
                    qCritical() << "Error:" << reply->error() << reply->errorString();
                    return;
                }

                qDebug() << reply->readAll();
            });
        } else if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
            qDebug() << "Temporary credentials.";
        } else {
            qWarning() << "Status is not granted: " << int(status);
        }
    });

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::error, replyHandler, [this, replyHandler]() {
        qDebug() << "QOAuth2AuthorizationCodeFlow::error raised.";
        replyHandler->deleteLater();
        emit failed(Error::ReplyError, tr("QOAuth2AuthorizationCodeFlow::error raised."));
    });

    connect(replyHandler, &QOAuthHttpServerReplyHandler::tokenRequestErrorOccurred, this, [this, replyHandler](QAbstractOAuth::Error error, const QString& errorString) {
        qDebug() << "Token request error occurred: " << int(error) << errorString;
        replyHandler->deleteLater();
        emit failed(Error::TokenRequestError, tr("Token request failed.") + "\n" + errorString);
    });

    m_oauth2.setModifyParametersFunction([codeVerifier,codeChallenge](QAbstractOAuth::Stage stage, auto*params){
        /* Note: params is QMap for Qt5 and QMultiMap for Qt6 */
        if (params) {
            qDebug() << "Stage: " << int(stage) << "with params" << *params;
            switch (stage) {
                case QAbstractOAuth::Stage::RequestingAuthorization:
                    qDebug() << "Requesting Authorization.";
                    params->insert("code_challenge", codeChallenge);
                    params->insert("code_challenge_method", "S256");
                    // FIXME: Can be replaced by ->replace when Qt 6.x is minimum supported version
                    params->remove("redirect_uri");
                    params->insert("redirect_uri", "http://127.0.0.1:1337/");
                    break;
                case QAbstractOAuth::Stage::RequestingAccessToken:
                    qDebug() << "Requesting Access Token.";
                    params->insert("code_verifier", codeVerifier);
                    //Note: technically, we no longer redirect, but OSM server rejects the token request unless we provide a matching redirect_uri
                    params->remove("redirect_uri");
                    params->insert("redirect_uri", "http://127.0.0.1:1337/");
                    break;
                default:
                    qDebug() << "default stage.";
            }
            qDebug() << "Stage: " << int(stage) << "with params" << *params;
        }
    });

    m_oauth2.grant();
}
