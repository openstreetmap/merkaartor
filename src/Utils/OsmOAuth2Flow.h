#include <QOAuth2AuthorizationCodeFlow>
#include <QString>

class OsmOAuth2Flow : public QOAuth2AuthorizationCodeFlow
{
    Q_OBJECT

    public:
        OsmOAuth2Flow(QObject* parent = nullptr);
        void requestAccessToken(const QString& code);
};

