#include <QtTest/QtTest>

#include "common/Projection.h"

class TestProjection: public QObject
{
  Q_OBJECT
  private slots:


  void projectionInit() {
    ProjectionBackend proj("EPSG:3857", [](QString x) {return x;});
    qDebug() << proj.getProjectionType();
    qDebug() << proj.getProjectionProj4();
    QCOMPARE(proj.getProjectionType(), "EPSG:3857"); // Would not be set if init failed.
  }

  /**
   * This test verifies the Proj4 library is loaded and correctly identifies non-standard projection.
   */
  void projectionInitProj4() {
    ProjectionBackend proj("EPSG:5514", [](QString x) {return x;});
    qDebug() << proj.getProjectionType();
    QCOMPARE(proj.getProjectionType(), "EPSG:5514");
  }


  void projectionWGS84toEPSG3031() {
    ProjectionBackend proj("EPSG:3031", [](QString x) {return x;});
    qDebug() << proj.getProjectionProj4();
  }

  /**
   * This test verifies proj4 library is able to project correctly from WGS84 to a non-standard projection.
   */
  void projectionWGS84toEPSG5514() {
    ProjectionBackend proj("+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=30.28813972222222 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +pm=greenwich +units=m +no_defs +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56", [](QString x) {return x;});
    QList<QPair<QPointF,QPointF>> list = {
      {{14.4157,50.1038},{-742955.5923625092255,-1041158.8852684112499}},
      {{18.9816,50.1259},{-418013.62494312302442,-1073425.9725897577591}}
    };
    for(auto& pair : list) {
      QCOMPARE(proj.project(pair.first), pair.second);
      //QCOMPARE(proj.inverse2Point(pair.second), pair.first); // TODO: This projection does not seem to converge very well.
    }
    QBENCHMARK(proj.project(QPointF(14.4157,50.1038)));
  }

  /**
   * Use default "LatLong" projection, which is in fact identity.
   */
  void projectionWGS84toLatLong() {
    ProjectionBackend proj("EPSG:4326", [](QString x) {return x;});
    QPointF point(14.4157,50.1038);
    QCOMPARE(proj.project(point), point);
    QCOMPARE(proj.inverse2Point(point), point);
    QBENCHMARK(proj.project(point));
  }

  /**
   * This test uses proj4 to do the identity projection. In process, it converts to radians from decimal degrees.
   */
  void projectionWGS84toWGS84() {
    ProjectionBackend proj("+proj=longlat +datum=WGS84", [](QString x) {return x;});
    QPointF point(14.4157,50.1038);
    QCOMPARE(proj.project(point), point);
    QCOMPARE(proj.inverse2Point(point), point);
    QBENCHMARK(proj.project(point));
  }

  /**
   * Test the internal Mercator projection.
   */
  void projectionWGS84toMercator() {
    ProjectionBackend proj("EPSG:3857", [](QString x) {return x;});
    QPointF point(14.4157,50.1038);
    QPointF projected(1604748.38320521079,6464271.615268512629);
    QCOMPARE(proj.project(point),  projected);
    QCOMPARE(proj.inverse2Point(projected), point);
    QBENCHMARK(proj.project(point));
  }
};

QTEST_MAIN(TestProjection)
#include "test-projection.moc"
