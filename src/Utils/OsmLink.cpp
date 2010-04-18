#include "OsmLink.h"

#include <QApplication>
#include <QMessageBox>
#include <QStringList>

OsmLink::OsmLink(QString url)
    : m_IsValid(false)
    , m_IsShort(false)
    , m_hasSpan(false)
{
    theUrl = QUrl(url, QUrl::TolerantMode);
    checkUrl();
}

OsmLink::OsmLink(QUrl url)
    : m_IsValid(false)
    , m_IsShort(false)
    , m_hasSpan(false)
{
    theUrl = url;
    checkUrl();
}

void OsmLink::checkUrl()
{
    if (!theUrl.isValid()) {
        m_IsValid = false;
        return;
    }
    if (theUrl.toString().contains("osm.org/go")) {
        m_IsValid = true;
        m_IsShort = true;
    } else
    if (theUrl.hasQueryItem("lat") && theUrl.hasQueryItem("lon") && theUrl.hasQueryItem("zoom"))
    {
        m_Lat = theUrl.queryItemValue("lat").toDouble();
        m_Lon = theUrl.queryItemValue("lon").toDouble();
        m_Zoom = theUrl.queryItemValue("zoom").toInt();

        m_IsValid = true;
        m_IsShort = false;
    } else
    if (theUrl.hasQueryItem("mlat") && theUrl.hasQueryItem("mlon") && theUrl.hasQueryItem("zoom"))
    {
        m_Lat = theUrl.queryItemValue("mlat").toDouble();
        m_Lon = theUrl.queryItemValue("mlon").toDouble();
        m_Zoom = theUrl.queryItemValue("zoom").toInt();

        m_IsValid = true;
        m_IsShort = false;
    } else
    if (theUrl.host().contains("maps.google.com") && theUrl.hasQueryItem("ll") && theUrl.hasQueryItem("spn")) {
        QStringList ll = theUrl.queryItemValue("ll").split(",");
        m_Lat = ll[0].toDouble();
        m_Lon = ll[1].toDouble();
        QStringList spn = theUrl.queryItemValue("spn").split(",");
        m_spanLat = spn[0].toDouble();
        m_spanLon = spn[1].toDouble();
        m_IsValid = true;
        m_hasSpan = true;
    }
}

CoordBox OsmLink::getCoordBox()
{
    CoordBox cb;
    if (!m_IsValid)
        return cb;

    if (m_hasSpan) {
        cb = CoordBox(Coord(angToInt(m_Lat - m_spanLat/2), angToInt(m_Lon - m_spanLon/2)), Coord(angToInt(m_Lat + m_spanLat/2), angToInt(m_Lon + m_spanLon/2)));
        return cb;
    } else
    if (!m_IsShort) {
        if (m_Zoom <= 10) {
            QMessageBox::warning(0, QApplication::translate("Downloader", "Zoom factor too low"),
                QApplication::translate("Downloader", "Please use a higher zoom factor!"));
            return cb;
        }
    } else {
        QByteArray possibleChar("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_@");
        QString theCode = theUrl.path().section('/', -1);
        int x = 0;
        int y = 0;
        int z = 0;
        int z_offset = 0;

        QByteArray ar = theCode.toAscii();
        for (int i=0; i<ar.size(); ++i) {
            qint8 t = possibleChar.indexOf(ar.at(i));
            if (t == -1)
                z_offset += 1;
            else {
                for (int j=0; j<3; ++j) {
                    x <<= 1;
                    if ((t & 32) != 0)
                        x = x | 1;
                    t <<= 1;

                    y <<= 1;
                    if ((t & 32) != 0)
                        y = y | 1;
                    t <<= 1;
                }
                z += 3;
            }
        }

        x <<= (32 - z);
        y <<= (32 - z);

        m_Lon = (x * 360. / pow(2, 32)) - 180.;
        m_Lat = (y * 180. / pow(2, 32)) - 90.;
        if (z_offset)
            z_offset = (3 - (z_offset % 3));
        m_Zoom = z - 8  - z_offset;

        if (m_Lon < -180.) m_Lon += 360.;
        if (m_Lon > 180.) m_Lon -= 360;
        if (m_Lat < -90.) m_Lat += 180;
        if (m_Lat > 90.) m_Lat -= 180;
    }

    if (m_Zoom < 1 || m_Zoom > 18) // use default when not in bounds
        m_Zoom = 15;

    /* term to calculate the angle from the zoom-value */
    double zoomLat = 360.0 / (double)(1 << m_Zoom);
    double zoomLon = zoomLat / fabs(cos(angToRad(m_Lat)));
    /* the following line is equal to the line above. (just for explanation) */
    //double zoomLon = zoomLat / aParent->view()->projection().latAnglePerM() * aParent->view()->projection().lonAnglePerM(angToRad(lat));

    /* the OSM link contains the coordinates from the middle of the visible map so we have to add and sub zoomLon/zoomLat */
    cb = CoordBox(Coord(angToInt(m_Lat-zoomLat), angToInt(m_Lon-zoomLon)), Coord(angToInt(m_Lat+zoomLat), angToInt(m_Lon+zoomLon)));

    return cb;
}
