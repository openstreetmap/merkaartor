#include <QtTest/QtTest>

#include "Utils/HttpAuth.h"

class TestHttpAuth : public QObject
{
    Q_OBJECT
        private slots:


    void simpleLogin() {
        QUrl url("hello");
        qDebug() << url;

        HttpAuth auth(this);
        auth.Login();
        auth.grant();
        QTest::qWait(10000);
    }
};

QTEST_MAIN(TestHttpAuth)
#include "test-HttpAuth.moc"
