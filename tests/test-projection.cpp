#include <QtTest/QtTest>

#include "common/Projection.h"

class TestProjection: public QObject
{
  Q_OBJECT
  private slots:

  void projectionInit() {
    ProjectionBackend proj("XX", [](QString x) {return x;});
    QVERIFY(true);
  }
};

QTEST_MAIN(TestProjection)
#include "test-projection.moc"
