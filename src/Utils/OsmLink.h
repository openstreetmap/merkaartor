#ifndef OSMLINK_H
#define OSMLINK_H

#include <QUrl>

#include "Maps/Coord.h"

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
    void setLatLonZoom(double lat, double lon, int zoom);
    void setMinMax(double bottom, double left, double top, double right);
};

#endif // OSMLINK_H
