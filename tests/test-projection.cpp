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
    ProjectionBackend proj("+init=epsg:5514", [](QString x) {return x;});
    qDebug() << proj.getProjectionType();
    qDebug() << proj.getProjectionProj4();
    QCOMPARE(proj.getProjectionType(), "+init=epsg:5514");
    QVERIFY(proj.getProjectionProj4().contains("+proj=krovak"));
  }


  /**
   * This test verifies proj4 library is able to project correctly from WGS84 to a non-standard projection.
   */
  void projectionWGS84toEPSG5514() {
    ProjectionBackend proj("+proj=krovak +lat_0=49.5 +lon_0=24.83333333333333 +alpha=30.28813972222222 +k=0.9999 +x_0=0 +y_0=0 +ellps=bessel +pm=greenwich +units=m +no_defs +towgs84=570.8,85.7,462.8,4.998,1.587,5.261,3.56", [](QString x) {return x;});

    QCOMPARE(proj.project(QPointF(14.4157,50.1038)), QPointF(-742955.5923625092255,-1041158.8852684112499));
    QCOMPARE(proj.project(QPointF(18.9816,50.1259)), QPointF(-418013.62494312302442,-1073425.9725897577591));
  }
};

QTEST_MAIN(TestProjection)
#include "test-projection.moc"
