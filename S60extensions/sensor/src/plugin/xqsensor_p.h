#ifndef XQSENSORPRIVATE_H
#define XQSENSORPRIVATE_H

// INCLUDES
#include "xqsensor.h"
#include "xqsensor_p.h"
#include "private/qobject_p.h"
#include <rrsensorapi.h>

// FORWARD DECLARATIONS
class CSensrvChannelFinder;

// CLASS DECLARATION
class XQSensorPrivate: public QObject, public CBase,
    public MRRSensorDataListener
{
protected:
    enum TState
    {
        EStateClosed,
        EStateNotReceiving,
        EStateReceiving
    };

protected:
    XQSensorPrivate(XQSensor& qq);
    ~XQSensorPrivate();
    void open();
    void close();
    void startReceiving();
    void stopReceiving();
    XQSensor::Error error() const;
    virtual TInt SensorId() const = 0;

protected:
    friend class XQSensor;
    XQSensor& q;

protected:
    CRRSensorApi* iRRSensorApi;

    TState iState;
    TInt iError;
};

#endif /*XQSENSORPRIVATE_H*/

// End of file
