#ifndef XQACCELERATIONSENSORPRIVATE_H
#define XQACCELERATIONSENSORPRIVATE_H

// INCLUDES
#include "xqsensor_p.h"
#include "xqaccsensor.h"
#include "xqaccsensor_p.h"

// FORWARD DECLARATIONS
class CSensrvChannel;

// CLASS DECLARATION
class XQAccelerationSensorPrivate: public XQSensorPrivate
{
public:
    XQAccelerationSensorPrivate(XQSensor& qq);
    virtual ~XQAccelerationSensorPrivate();

    void open();
    void close();
    void startReceiving();
    void stopReceiving();
    XQSensor::Error error() const;

    void addFilter(XQAbstractAccelerationSensorFilter &filter);
    QList<XQAbstractAccelerationSensorFilter *>& filters();

    int xAcceleration() const;
    int yAcceleration() const;
    int zAcceleration() const;

protected:
    void DataReceived(CSensrvChannel& aChannel, TInt aCount, TInt aDataLost);

private:
    void HandleXyzAxisData(CSensrvChannel& aChannel, TInt aCount, TInt aDataLost);

private:
    CSensrvChannel* iXyzAxisChannel;
    TInt iXAcceleration;
    TInt iYAcceleration;
    TInt iZAcceleration;    
    TInt iError;
    
    QList<XQAbstractAccelerationSensorFilter *> iFilters;
};

#endif /*XQACCELERATIONSENSORPRIVATE_H*/

// End of file
