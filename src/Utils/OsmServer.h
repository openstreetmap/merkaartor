#ifndef __OSMSERVER_H__
#define __OSMSERVER_H__

#include <QString>
#include <QDebug>
#include <QNetworkReply>

#include <memory>

class QNetworkAccessManager;

// A dataclass for OSM server information
struct OsmServerInfo
{
    /* Note: Types here correspond to the UI combo box. Keep those in sync. */
    enum class AuthType : int{
        Basic,
        OAuth2Redirect, //password field is the token if OAuth2* is used
        OAuth2OOB,
    };

    static AuthType typeFromString(const QString& type) {
        if (type == "basic")  return AuthType::Basic;
        if (type == "oauth2redirect") return AuthType::OAuth2Redirect;
        if (type == "oauth2oob") return AuthType::OAuth2OOB;
        qWarning() << "Error parsing AuthType: Unknown AuthType" << type;
        return AuthType::Basic;
    }

    static QString typeToString(AuthType type) {
        switch (type) {
            case AuthType::OAuth2Redirect: return "oauth2redirect";
            case AuthType::OAuth2OOB: return "oauth2oob";
            case AuthType::Basic:  return "basic";
            default:
                qWarning() << "Error encoding AuthType: Unknown AuthType" << static_cast<int>(type);
                return "basic";
        }
    }

    bool Selected;
    AuthType Type;
    QString Url;
    QString User;
    QString Password;
    int CfgVersion = 1;
};

OsmServerInfo const defaultOsmServerInfo = OsmServerInfo{true, OsmServerInfo::AuthType::OAuth2Redirect, "https://www.openstreetmap.org/", "", ""};

class IOsmServerImpl : public QObject {
    Q_OBJECT

    public:
        virtual QNetworkReply* get(const QUrl &url) = 0;
        virtual QNetworkReply* put(const QUrl &url, const QByteArray &data) = 0;
        virtual QNetworkReply* deleteResource(const QUrl &url) = 0;
        virtual QNetworkReply* sendRequest(QNetworkRequest &request, const QByteArray &verb, const QByteArray &data) = 0;
        virtual void authenticate() = 0;

        virtual OsmServerInfo const getServerInfo() const = 0;

        virtual QUrl baseUrl() const {
            return QUrl(getServerInfo().Url);
        }

        virtual QUrl apiUrl() const {
            return QUrl(getServerInfo().Url).resolved(QUrl("/api/0.6"));
        }


        enum class Error {
            NoError,
            Unauthorized,
            Timeout,
            ReplyError,
            TokenRequestError,
            Cancelled,
        };
    signals:
        void authenticated();
        void failed(Error error, QString errorString);
};

Q_DECLARE_INTERFACE(IOsmServerImpl, "InterfaceIOsmServerImpl")

using OsmServer = std::shared_ptr<IOsmServerImpl>;

OsmServer makeOsmServer(OsmServerInfo& info, QNetworkAccessManager& manager);

bool migrateOsmServerInfo(OsmServerInfo& info);

#endif
