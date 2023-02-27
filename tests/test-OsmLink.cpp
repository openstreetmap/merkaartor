
#include <QtTest/QtTest>

#include "Utils/OsmLink.h"
#include "common/Coord.h"

static CoordBox box(qreal lat, qreal lon, int zoom) {
    qreal zoomLat = 360.0 / (double)(1 << zoom);
    qreal zoomLon = zoomLat / fabs(cos(angToRad(lat)));
    return CoordBox(Coord(lon-zoomLon, lat-zoomLat), Coord(lon+zoomLon, lat+zoomLat));
}

static QMarginsF margin = QMarginsF(0.0001, 0.0001, 0.0001, 0.0001);

static QRectF testbox(qreal lat, qreal lon, int zoom) {
    return ((QRectF)box(lat, lon, zoom)).normalized() + margin;
}

class TestOsmLink : public QObject
{
    Q_OBJECT
        private slots:

        void urlInvalid() {
            OsmLink link("invalid url");
            QVERIFY(!link.isValid());
        }

    void urlOsmOrg() {
        {
            OsmLink link("https://www.openstreetmap.org/#map=15/56.1715/92.9781");
            QVERIFY(link.isValid());
            QCOMPARE(link.getCoordBox(), box(56.1715, 92.9781, 15));
        }
        {
            OsmLink link("https://www.openstreetmap.org/#map=15/56.1715/92.9781&layers=Y");
            QVERIFY(link.isValid());
            QCOMPARE(link.getCoordBox(), box(56.1715, 92.9781, 15));
        }
    }

    void urlOsmCz() {
        {
            OsmLink link("https://www.openstreetmap.cz/#map=15/56.1715/92.9781");
            QVERIFY(link.isValid());
            QCOMPARE(link.getCoordBox(), box(56.1715, 92.9781, 15));
        }
        {
            OsmLink link("https://www.openstreetmap.cz/#map=15/56.1715/92.9781&layers=d");
            QVERIFY(link.isValid());
            QCOMPARE(link.getCoordBox(), box(56.1715, 92.9781, 15));
        }
    }

    void urlShort() {
        // https://wiki.openstreetmap.org/wiki/Shortlink
        OsmLink link("https://osm.org/go/0EEQjE==");
        QVERIFY(link.isValid());
        QVERIFY(testbox(51.5108, 0.0549, 9).contains(link.getCoordBox()));
    }

    void urlQuery() {
        {
            OsmLink link("https://example.com/?lat=48.8552&lon=02.4335&zoom=10");
            QVERIFY(link.isValid());
            QVERIFY(testbox(48.8552, 02.4335, 10).contains(link.getCoordBox()));
        }
        {
            OsmLink link("https://example.com/?mlat=48.8552&mlon=02.4335&zoom=10");
            QVERIFY(link.isValid());
            QVERIFY(testbox(48.8552, 02.4335, 10).contains(link.getCoordBox()));
        }
    }

    void urlMinMax() {
        {
            OsmLink link("https://example.com?minlon=48.0001&maxlon=48.9999&minlat=02.0001&maxlat=02.9999");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY((CoordBox(Coord(48.0, 02.0), Coord(49.0, 3.0))+margin).contains(link.getCoordBox()));
        }
        {
            OsmLink link("https://example.com?left=48.0001&right=48.9999&bottom=02.0001&top=02.9999");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY((CoordBox(Coord(48.0, 02.0), Coord(49.0, 3.0))+margin).contains(link.getCoordBox()));
        }
    }

    void urlGoogleMaps() {
        {
            OsmLink link("https://www.google.com/maps/@50.0596696,14.4656239,11z");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.0596, 14.4656, 11).contains(link.getCoordBox()));
        }
        {
            OsmLink link("https://www.google.de/maps/@50.1547008,14.3327232,14z");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.1547, 14.3327, 14).contains(link.getCoordBox()));
        }
        {
            OsmLink link("https://www.google.de/maps/@50.1547008,14.3327232,10.6z");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.1547, 14.3327, 10.6).contains(link.getCoordBox()));
        }
        {
            OsmLink link("https://www.google.de/maps/@50.1547008,14.3327232,14z/data=!5m2!1e1!1e4");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.1547, 14.3327, 14).contains(link.getCoordBox()));
        }

    }

    void urlMapyCz() {
        OsmLink link("https://en.mapy.cz/zakladni?x=15.2022594&y=49.8235194&z=7");
        QVERIFY(link.isValid());
        qDebug() << link.getCoordBox();
        QVERIFY(testbox(49.8235, 15.2022, 7).contains(link.getCoordBox()));
    }

    void urlGeo() {
        {
            OsmLink link("geo:50.0000,14.0000");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.0000, 14.0000, 16).contains(link.getCoordBox()));
        }
        {
            OsmLink link("geo:50.0000,14.0000?u=30&z=17");
            QVERIFY(link.isValid());
            qDebug() << link.getCoordBox();
            QVERIFY(testbox(50.0000, 14.0000, 17).contains(link.getCoordBox()));
        }
    }

};

QTEST_MAIN(TestOsmLink)
#include "test-OsmLink.moc"
