#ifndef OSMLINK_H
#define OSMLINK_H

#include <QUrl>

#include "Coord.h"

class OsmLink
{
public:
    OsmLink(QUrl url);
    OsmLink(QString url);

    bool isValid() const { return m_IsValid; }
    CoordBox getCoordBox() const { return m_Box; }

private:
    bool m_IsValid;
    CoordBox m_Box;

    QString parseUrl(QUrl url);
    void parseShortUrl(QString code);
    void setLatLonZoom(qreal lat, qreal lon, int zoom);
    void setMinMax(qreal bottom, qreal left, qreal top, qreal right);
};

#endif // OSMLINK_H
