#include "OsmLink.h"

#include <QApplication>
#include <QMessageBox>
#include <QStringList>
#include "Global.h"

OsmLink::OsmLink(QString url)
    : m_IsValid(false)
{
    QString s = parseUrl(QUrl(url, QUrl::TolerantMode));
    if (!m_IsValid)
        qDebug() << "OsmLink:" << s;
}

OsmLink::OsmLink(QUrl url)
    : m_IsValid(false)
{
    QString s = parseUrl(url);
    if (!m_IsValid)
        qDebug() << "OsmLink:" << s;
}

#define ARG_VALID(param) if (!parseOk) return QString("Unparsed " #param "=\"%1\"").arg(theQuery.queryItemValue(#param))
#define PARSE_ERROR(val)  if (!parseOk) return QString("Unparsed " #val "=\"%1\"").arg(val)

QString OsmLink::parseUrl(QUrl theUrl)
{
    // On parse failure, we bail early, leaving m_isValid unset
    bool parseOk;
#ifdef QT5
    QUrlQuery theQuery;
#define theQuery theQuery
#else
#define theQuery theUrl
#endif

    if (!theUrl.isValid()) return QString("Invalid URL: %1").arg(theUrl.toString());;

    if (theUrl.toString().contains("openstreetmap.org/") && theUrl.toString().contains("#map=")) {
        // first is 'map', zoom, lat and lon follows
        QStringList list = theUrl.fragment().split(QRegExp("[/=]"));
        qreal zoom = list[1].toInt(&parseOk);   ARG_VALID(zoom);
        qreal lat = list[2].toDouble(&parseOk); ARG_VALID(lat);
        qreal lon = list[3].toDouble(&parseOk); ARG_VALID(lon);

        setLatLonZoom(lat, lon, zoom);
    }
    else if (theUrl.toString().contains("osm.org/go"))
    {
        parseShortUrl(theUrl.path().section('/', -1));
    }
    else if (theQuery.hasQueryItem("lat") && theQuery.hasQueryItem("lon") && theQuery.hasQueryItem("zoom"))
    {
        qreal lat = theQuery.queryItemValue("lat").toDouble(&parseOk);   ARG_VALID(lat);
        qreal lon = theQuery.queryItemValue("lon").toDouble(&parseOk);   ARG_VALID(lon);
        qreal zoom = theQuery.queryItemValue("zoom").toInt(&parseOk);    ARG_VALID(zoom);

        setLatLonZoom(lat, lon, zoom);
    }
    else if (theQuery.hasQueryItem("mlat") && theQuery.hasQueryItem("mlon") && theQuery.hasQueryItem("zoom"))
    {
        qreal lat = theQuery.queryItemValue("mlat").toDouble(&parseOk);   ARG_VALID(lat);
        qreal lon = theQuery.queryItemValue("mlon").toDouble(&parseOk);   ARG_VALID(lon);
        qreal zoom = theQuery.queryItemValue("zoom").toInt(&parseOk);     ARG_VALID(zoom);

        setLatLonZoom(lat, lon, zoom);
    }
    else if (theQuery.hasQueryItem("minlon") && theQuery.hasQueryItem("maxlon") &&
         theQuery.hasQueryItem("minlat") && theQuery.hasQueryItem("maxlat"))
    {
        qreal bottom = theQuery.queryItemValue("minlat").toDouble(&parseOk);   ARG_VALID(minlat);
        qreal left = theQuery.queryItemValue("minlon").toDouble(&parseOk);     ARG_VALID(minlon);
        qreal top = theQuery.queryItemValue("maxlat").toDouble(&parseOk);      ARG_VALID(maxlat);
        qreal right = theQuery.queryItemValue("maxlon").toDouble(&parseOk);    ARG_VALID(maxlon);

        setMinMax(bottom, left, top, right);
    }
    else if (theQuery.hasQueryItem("left") && theQuery.hasQueryItem("right") &&
         theQuery.hasQueryItem("bottom") && theQuery.hasQueryItem("top"))
    {
        qreal bottom = theQuery.queryItemValue("bottom").toDouble(&parseOk);  ARG_VALID(minlat);
        qreal left = theQuery.queryItemValue("left").toDouble(&parseOk);      ARG_VALID(minlon);
        qreal top = theQuery.queryItemValue("top").toDouble(&parseOk);        ARG_VALID(maxlat);
        qreal right = theQuery.queryItemValue("right").toDouble(&parseOk);    ARG_VALID(maxlon);

        setMinMax(bottom, left, top, right);
    }
    else if (theQuery.hasQueryItem("left") && theQuery.hasQueryItem("right") &&
         theQuery.hasQueryItem("bottom") && theQuery.hasQueryItem("top"))
    {
        qreal bottom = theQuery.queryItemValue("bottom").toDouble(&parseOk);  ARG_VALID(minlat);
        qreal left = theQuery.queryItemValue("left").toDouble(&parseOk);      ARG_VALID(minlon);
        qreal top = theQuery.queryItemValue("top").toDouble(&parseOk);        ARG_VALID(maxlat);
        qreal right = theQuery.queryItemValue("right").toDouble(&parseOk);    ARG_VALID(maxlon);

        setMinMax(bottom, left, top, right);
    }
    else if ((theUrl.host().contains("maps.google.com") || theUrl.host().contains("maps.google.co.uk")) &&
         theQuery.hasQueryItem("ll") && theQuery.hasQueryItem("spn"))
    {
        QStringList ll = theQuery.queryItemValue("ll").split(",");     if (ll.count() != 2) return QString("Unsplit=\"%2\" (%1 elements)").arg(ll.count()).arg(theQuery.queryItemValue("ll"));
        qreal lat = ll[0].toDouble(&parseOk);                       PARSE_ERROR(ll[0]);
        qreal lon = ll[1].toDouble(&parseOk);                       PARSE_ERROR(ll[1]);
        QStringList spn = theQuery.queryItemValue("spn").split(",");   if (spn.count() != 2) return QString("Unsplit=\"%2\" (%1 elements)").arg(spn.count()).arg(theQuery.queryItemValue("spn"));
        qreal spanLat = spn[0].toDouble(&parseOk);                  PARSE_ERROR(spn[0]);
        qreal spanLon = spn[1].toDouble(&parseOk);                  PARSE_ERROR(spn[1]);

        setMinMax(lat-spanLat, lon-spanLon/2, lat+spanLat, lon+spanLon/2);
    } else if (theUrl.host().contains("mapy.cz")) {

        qreal zoom = theQuery.queryItemValue("z").toInt(&parseOk);   ARG_VALID(zoom);
        qreal lat = theQuery.queryItemValue("y").toDouble(&parseOk); ARG_VALID(lat);
        qreal lon = theQuery.queryItemValue("x").toDouble(&parseOk); ARG_VALID(lon);

        setLatLonZoom(lat, lon, zoom);
    } else if (theUrl.toString().contains("/#map=")) {
        // http://www.openstreetmap.org/#map=<zoom>/<lat>/<lon>
        QRegExp rx("/#map=([0-9]+)/([0-9.]+)/([0-9.]+)$");
        if (rx.indexIn(theUrl.toString()) >= 0) {
            qreal zoom = rx.cap(1).toUInt(&parseOk); PARSE_ERROR(rx.cap(1));
            qreal lat = rx.cap(2).toDouble(&parseOk); PARSE_ERROR(rx.cap(2));
            qreal lon = rx.cap(3).toDouble(&parseOk); PARSE_ERROR(rx.cap(3));

            setLatLonZoom(lat, lon, zoom);
      }
    } else if (theUrl.toString().contains("/maps/@")) {
        // https://www.google.de/maps/@<lat>,<lon>,<zoom>z
        QRegExp rx("/maps/@([0-9.]+),([0-9.]+),([0-9.]+)z$");
        if (rx.indexIn(theUrl.toString()) >= 0) {
            qreal lat = rx.cap(1).toDouble(&parseOk); PARSE_ERROR(rx.cap(1));
            qreal lon = rx.cap(2).toDouble(&parseOk); PARSE_ERROR(rx.cap(2));
            qreal zoom = rx.cap(3).toDouble(&parseOk); PARSE_ERROR(rx.cap(3));

            setLatLonZoom(lat, lon, zoom);
        }
    } else if (theUrl.toString().startsWith("geo:")) {
        // geo:<lat>,<lon>?z=<zoom>
        QRegExp rx("^geo:([0-9.]+),([0-9.]+)");
        if (rx.indexIn(theUrl.toString()) >= 0) {
            qreal lat = rx.cap(1).toDouble(&parseOk); PARSE_ERROR(rx.cap(1));
            qreal lon = rx.cap(2).toDouble(&parseOk); PARSE_ERROR(rx.cap(2));
            qreal zoom = 16; // default value

            rx.setPattern("\\?z=([0-9]+)$");
            if (rx.indexIn(theUrl.toString()) >= 0)
                zoom = rx.cap(1).toUInt(&parseOk); PARSE_ERROR(rx.cap(1));

            setLatLonZoom(lat, lon, zoom);
        }
    }
#undef theQuery
    return QString("Unrecognised URL: %1").arg(theUrl.toString());
}

void OsmLink::parseShortUrl(QString code)
{
    // TWO_32 defined in terms of 1<<16 because 1L<<32 may overflow if long is only 32 bits
    static const qreal TWO_32 = (double)(1<<16) * (double)(1<<16);
    static const QByteArray possibleChar("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_@");
        int x = 0;
        int y = 0;
        int z = 0;
        int z_offset = 0;

        QByteArray ar = code.toLatin1();
        for (int i=0; i<ar.size(); ++i) {
        qint8 t = possibleChar.indexOf(ar.at(i));
        if (t == -1)
            z_offset += 1;
        else {
            for (int j=0; j<3; ++j) {
                x <<= 1;
                if ((t & 32) != 0)
                    x |= 1;
                t <<= 1;

                y <<= 1;
                if ((t & 32) != 0)
                    y |= 1;
                t <<= 1;
            }
            z += 3;
        }
        }

        x <<= (32 - z);
        y <<= (32 - z);

        qreal lon = (x * 360. / TWO_32) - 180.;
        qreal lat = (y * 180. / TWO_32) - 90.;
        if (z_offset)
        z_offset = (3 - (z_offset % 3));
        int zoom = z - 8  - z_offset;

        if (lon < -180.) lon += 360.;
        if (lon > 180.) lon -= 360;
        if (lat < -90.) lat += 180;
        if (lat > 90.) lat -= 180;

    setLatLonZoom(lat, lon, zoom);
}

void OsmLink::setLatLonZoom(qreal lat, qreal lon, int zoom)
{
    // Constrain zoom
    if (zoom > 20)
        zoom = 20;
    if (zoom < 1) // probably a parse error; at least try
        zoom = 15;

    /* term to calculate the angle from the zoom-value */
    qreal zoomLat = 360.0 / (double)(1 << zoom);
    qreal zoomLon = zoomLat / fabs(cos(angToRad(lat)));
    /* the following line is equal to the line above. (just for explanation) */
    //qreal zoomLon = zoomLat / aParent->view()->projection().latAnglePerM() * aParent->view()->projection().lonAnglePerM(angToRad(lat));

    /* the OSM link contains the coordinates from the middle of the visible map so we have to add and sub zoomLon/zoomLat */
    setMinMax(lat-zoomLat, lon-zoomLon, lat+zoomLat, lon+zoomLon);
}

void OsmLink::setMinMax(qreal bottom, qreal left, qreal top, qreal right)
{
    const qreal minwidth = 0.0001;
    if (right-left < minwidth) {
        left = (left + right - minwidth) / 2.0;
        right = left + minwidth;
    }
    if (top-bottom < minwidth) {
        top = (top + bottom + minwidth) / 2.0;
        bottom = top - minwidth;
    }
    m_Box = CoordBox(Coord(left, bottom), Coord(right, top));
    m_IsValid = true;
}
