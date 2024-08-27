#include "OsmOAuth2Flow.h"

OsmOAuth2Flow::OsmOAuth2Flow(QObject* parent)
    : QOAuth2AuthorizationCodeFlow(parent)
{ }

void OsmOAuth2Flow::requestAccessToken(const QString& code) {
    return QOAuth2AuthorizationCodeFlow::requestAccessToken(code);
}

