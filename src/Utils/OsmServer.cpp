#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>

#include "OsmServer.h"
#include "HttpAuth.h"

class OsmServerImplBasic : public IOsmServerImpl {
    public:
        OsmServerImplBasic(OsmServerInfo& info, QNetworkAccessManager& manager)
            : m_info(info), m_manager(manager) {
        }

        QUrl baseUrl() const {
            return m_info.Url;
        }

        virtual QNetworkReply* get(const QUrl &url) {
            return m_manager.get(QNetworkRequest(baseUrl().resolved(url)));
        }

        virtual QNetworkReply* put(const QUrl &url, const QByteArray &data) {
            return m_manager.put(QNetworkRequest(baseUrl().resolved(url)), data);
        }

        virtual QNetworkReply* deleteResource(const QUrl &url) {
            return m_manager.deleteResource(QNetworkRequest(baseUrl().resolved(url)));
        }

        virtual void authenticate() {
            QUrl base(m_info.Url);
            auto reply = m_manager.get(QNetworkRequest(base.resolved(QUrl("api/0.6/user/details"))));
            connect(reply, &QNetworkReply::finished, [=]() {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError) {
                    qCritical() << "Error:" << reply->errorString();
                    emit failed(reply->error());
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
        OsmServerInfo& m_info;
        QNetworkAccessManager& m_manager;
};

class OsmServerImplOAuth2 : public IOsmServerImpl {
    public:
        OsmServerImplOAuth2(OsmServerInfo& info, QNetworkAccessManager& manager)
            : m_info(info), m_manager(manager),m_oauth2(&manager)
        {
            m_oauth2.setScope("read_prefs write_prefs write_api read_gpx write_gpx write_notes");
            m_oauth2.setClientIdentifier("wv3ui28EyHjH0c4C1Wuz6_I-o47ithPAOt7Qt1ov9Ps");
        }

        QUrl baseUrl() const {
            return m_info.Url;
        }

        virtual QNetworkReply* get(const QUrl &url) {
            return m_oauth2.get(baseUrl().resolved(url));
        }

        virtual QNetworkReply* put(const QUrl &url, const QByteArray &data) {
            return m_oauth2.put(baseUrl().resolved(url), data);
        }

        virtual QNetworkReply* deleteResource(const QUrl &url) {
            return m_oauth2.deleteResource(baseUrl().resolved(url));
        }

        virtual void authenticate();

    private:
        OsmServerInfo& m_info;
        QNetworkAccessManager& m_manager;
        QOAuth2AuthorizationCodeFlow m_oauth2;

        QString generateCodeVerifier();
};

std::shared_ptr<IOsmServer> makeOsmServer(OsmServerInfo& info, QNetworkAccessManager& manager) {
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

void OsmServerImplOAuth2::authenticate() {
    m_oauth2.setAuthorizationUrl(baseUrl().resolved(QUrl("oauth2/authorize")));
    m_oauth2.setAccessTokenUrl(baseUrl().resolved(QUrl("oauth2/token")));
    qDebug() << "Base URL: " << baseUrl();
    qDebug() << "Authorization URL: " << m_oauth2.authorizationUrl();
    qDebug() << "Access Token URL: " << m_oauth2.accessTokenUrl();

    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    qDebug() << "connect.";
    m_oauth2.setReplyHandler(replyHandler);
    qDebug() << "connect2.";
    QString codeVerifier = generateCodeVerifier();
    QString codeChallenge = QString(QCryptographicHash::hash(codeVerifier.toUtf8(), QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        qDebug() << "OAuth status changed to " << int(status);
        if (status == QAbstractOAuth::Status::Granted) {
            qDebug() << "Granted, token: " << m_oauth2.token();
            m_info.Password = m_oauth2.token();
            emit authenticated();
            auto reply = get(QUrl("user/details"));
            connect(reply, &QNetworkReply::finished, [=]() {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError) {
                    qCritical() << "Error:" << reply->errorString();
                    return;
                }

                qDebug() << reply->readAll();
            });
        } else if (status == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
            qDebug() << "Temporary credentials.";
        } else {
            qWarning() << "Status is not granted: " << int(status);
            emit failed(int(status));
        }
    });
    m_oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>*params){
        qDebug() << "Stage: " << int(stage) << "with params" << *params;
        if (params) {
            switch (stage) {
                case QAbstractOAuth::Stage::RequestingAuthorization:
                    qDebug() << "Requesting Authorization.";
                    params->insert("code_challenge", codeChallenge);
                    params->insert("code_challenge_method", "S256");
                    break;
                case QAbstractOAuth::Stage::RequestingAccessToken:
                    qDebug() << "Requesting Access Token.";
                    params->insert("code_verifier", codeVerifier);
                    break;
                default:
                    qDebug() << "default stage.";
            }
        }
        qDebug() << "Stage: " << int(stage) << "with params" << *params;
    });

    connect(&m_oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
}
