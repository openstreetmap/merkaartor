#ifndef __OSMOAUTH2FLOW_H__
#define __OSMOAUTH2FLOW_H__

#include <QOAuth2AuthorizationCodeFlow>
#include <QString>

class OsmOAuth2Flow : public QOAuth2AuthorizationCodeFlow
{
    Q_OBJECT

    public:
        OsmOAuth2Flow(QObject* parent = nullptr);
        void requestAccessToken(const QString& code);
};

#endif // __OSMOAUTH2FLOW_H__
