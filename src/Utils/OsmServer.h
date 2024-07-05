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
        OAuth2 //password field is the token if this type is used
    };

    static AuthType typeFromString(const QString& type) {
        if (type == "basic")  return AuthType::Basic;
        if (type == "oauth2") return AuthType::OAuth2;
        qWarning() << "Error parsing AuthType: Unknown AuthType" << type;
        return AuthType::Basic;
    }

    static QString typeToString(AuthType type) {
        switch (type) {
            case AuthType::OAuth2: return "oauth2";
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
};

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
    signals:
        void authenticated();
        void failed(int error);
};

Q_DECLARE_INTERFACE(IOsmServerImpl, "InterfaceIOsmServerImpl")

using OsmServer = std::shared_ptr<IOsmServerImpl>;

OsmServer makeOsmServer(OsmServerInfo& info, QNetworkAccessManager& manager);

#endif
