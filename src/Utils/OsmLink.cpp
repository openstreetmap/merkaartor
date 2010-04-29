#include "OsmLink.h"

#include <QApplication>
#include <QMessageBox>
#include <QStringList>

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

#define ARG_VALID(param) if (!parseOk) return QString("Unparsed " #param "=\"%1\"").arg(theUrl.queryItemValue(#param))
#define PARSE_ERROR(val)  if (!parseOk) return QString("Unparsed " #val "=\"%1\"").arg(val)

QString OsmLink::parseUrl(QUrl theUrl)
{
	// On parse failure, we bail early, leaving m_isValid unset
	bool parseOk;

	if (!theUrl.isValid()) return QString("Invalid URL: %1").arg(theUrl.toString());;

	if (theUrl.toString().contains("osm.org/go"))
	{
		parseShortUrl(theUrl.path().section('/', -1));
	}
	else if (theUrl.hasQueryItem("lat") && theUrl.hasQueryItem("lon") && theUrl.hasQueryItem("zoom"))
	{
		double lat = theUrl.queryItemValue("lat").toDouble(&parseOk);   ARG_VALID(lat);
		double lon = theUrl.queryItemValue("lon").toDouble(&parseOk);   ARG_VALID(lon);
		double zoom = theUrl.queryItemValue("zoom").toInt(&parseOk);    ARG_VALID(zoom);

		setLatLonZoom(lat, lon, zoom);
	}
	else if (theUrl.hasQueryItem("mlat") && theUrl.hasQueryItem("mlon") && theUrl.hasQueryItem("zoom"))
	{
		double lat = theUrl.queryItemValue("mlat").toDouble(&parseOk);   ARG_VALID(lat);
		double lon = theUrl.queryItemValue("mlon").toDouble(&parseOk);   ARG_VALID(lon);
		double zoom = theUrl.queryItemValue("zoom").toInt(&parseOk);     ARG_VALID(zoom);

		setLatLonZoom(lat, lon, zoom);
	}
	else if (theUrl.hasQueryItem("minlon") && theUrl.hasQueryItem("maxlon") &&
		 theUrl.hasQueryItem("minlat") && theUrl.hasQueryItem("maxlat"))
	{
		double bottom = theUrl.queryItemValue("minlat").toDouble(&parseOk);   ARG_VALID(minlat);
		double left = theUrl.queryItemValue("minlon").toDouble(&parseOk);     ARG_VALID(minlon);
		double top = theUrl.queryItemValue("maxlat").toDouble(&parseOk);      ARG_VALID(maxlat);
		double right = theUrl.queryItemValue("maxlon").toDouble(&parseOk);    ARG_VALID(maxlon);

		setMinMax(bottom, left, top, right);
	}
	else if ((theUrl.host().contains("maps.google.com") || theUrl.host().contains("maps.google.co.uk")) &&
		 theUrl.hasQueryItem("ll") && theUrl.hasQueryItem("spn"))
	{
		QStringList ll = theUrl.queryItemValue("ll").split(",");     if (ll.count() != 2) return QString("Unsplit=\"%2\" (%1 elements)").arg(ll.count()).arg(theUrl.queryItemValue("ll"));
		double lat = ll[0].toDouble(&parseOk);                       PARSE_ERROR(ll[0]);
		double lon = ll[1].toDouble(&parseOk);                       PARSE_ERROR(ll[1]);
		QStringList spn = theUrl.queryItemValue("spn").split(",");   if (spn.count() != 2) return QString("Unsplit=\"%2\" (%1 elements)").arg(spn.count()).arg(theUrl.queryItemValue("spn"));
		double spanLat = spn[0].toDouble(&parseOk);                  PARSE_ERROR(spn[0]);
		double spanLon = spn[1].toDouble(&parseOk);                  PARSE_ERROR(spn[1]);

		setMinMax(lat-spanLat, lon-spanLon/2, lat+spanLat, lon+spanLon/2);
	}
	return QString("Unrecognised URL: %1").arg(theUrl.toString());
}

void OsmLink::parseShortUrl(QString code)
{
	// TWO_32 defined in terms of 1<<16 because 1L<<32 may overflow if long is only 32 bits
	static const double TWO_32 = (double)(1<<16) * (double)(1<<16);
	static const QByteArray possibleChar("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_@");
        int x = 0;
        int y = 0;
        int z = 0;
        int z_offset = 0;

        QByteArray ar = code.toAscii();
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

        double lon = (x * 360. / TWO_32) - 180.;
        double lat = (y * 180. / TWO_32) - 90.;
        if (z_offset)
		z_offset = (3 - (z_offset % 3));
        int zoom = z - 8  - z_offset;

        if (lon < -180.) lon += 360.;
        if (lon > 180.) lon -= 360;
        if (lat < -90.) lat += 180;
        if (lat > 90.) lat -= 180;

	setLatLonZoom(lon, lat, zoom);
}

void OsmLink::setLatLonZoom(double lat, double lon, int zoom)
{
	// Constrain zoom
	if (zoom > 20)
		zoom = 20;
	if (zoom < 1) // probably a parse error; at least try
		zoom = 15;

	/* term to calculate the angle from the zoom-value */
	double zoomLat = 360.0 / (double)(1 << zoom);
	double zoomLon = zoomLat / fabs(cos(angToRad(lat)));
	/* the following line is equal to the line above. (just for explanation) */
	//double zoomLon = zoomLat / aParent->view()->projection().latAnglePerM() * aParent->view()->projection().lonAnglePerM(angToRad(lat));

	/* the OSM link contains the coordinates from the middle of the visible map so we have to add and sub zoomLon/zoomLat */
	setMinMax(lat-zoomLat, lon-zoomLon, lat+zoomLat, lon+zoomLon);
}

void OsmLink::setMinMax(double bottom, double left, double top, double right)
{
	m_Box = CoordBox(Coord(angToInt(bottom), angToInt(left)), Coord(angToInt(top), angToInt(right)));
	m_IsValid = true;
}
