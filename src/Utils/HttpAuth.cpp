
#include "HttpAuth.h"
//#include <QNetworkAuth>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QMultiMap>
#include <QByteArray>


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

#include <random>

QString generateCodeVerifier() {
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

HttpAuth::HttpAuth(QObject *parent) : QObject(parent) {
    auto replyHandler = new QOAuthHttpServerReplyHandler(1337, this);
    oauth2.setReplyHandler(replyHandler);
    oauth2.setAuthorizationUrl(QUrl("https://www.openstreetmap.org/oauth2/authorize"));
    oauth2.setAccessTokenUrl(QUrl("https://www.openstreetmap.org/oauth2/token"));
    oauth2.setScope("read_prefs write_prefs write_api read_gpx write_gpx write_notes");
    oauth2.setClientIdentifier("wv3ui28EyHjH0c4C1Wuz6_I-o47ithPAOt7Qt1ov9Ps");

    codeVerifier = generateCodeVerifier();
    codeChallenge = QString(QCryptographicHash::hash(codeVerifier.toUtf8(), QCryptographicHash::Sha256).toBase64(QByteArray::Base64UrlEncoding | QByteArray::OmitTrailingEquals));

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::statusChanged, [=](
            QAbstractOAuth::Status status) {
        qDebug() << "OAuth status changed to " << int(status);
        if (status == QAbstractOAuth::Status::Granted) {
            qDebug() << "Granted, token: " << oauth2.token();
            emit authenticated();
            auto reply = oauth2.get(QUrl("https://api.openstreetmap.org/api/0.6/user/details"));
            connect(reply, &QNetworkReply::finished, [=]() {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError) {
                    qCritical() << "Error:" << reply->errorString();
                    return;
                }

                qDebug() << reply->readAll();
            });
        } else {
            qWarning() << "Status is not granted: " << int(status);
            emit failed(int(status));
        }
    });
    oauth2.setModifyParametersFunction([&](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant>*params){
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

    connect(&oauth2, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            &QDesktopServices::openUrl);
}
