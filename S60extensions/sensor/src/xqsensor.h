#ifndef XQSENSOR_H
#define XQSENSOR_H

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQSensorPrivate;

// CLASS DECLARATION
class XQSensor: public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        NotFoundError,
        UnknownError = -1
    };

public:
    virtual ~XQSensor();

public Q_SLOTS:
    void open();
    void close();
    void startReceiving();
    void stopReceiving();

public:
    XQSensor::Error error() const;

protected:
    XQSensor(XQSensorPrivate &dd, QObject *parent = 0);

protected:
    XQSensorPrivate* d;

};

#endif /*XQSENSOR_H*/

// End of file
