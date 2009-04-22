#ifndef QS60ACCELERATIONSENSOR_H
#define QS60ACCELERATIONSENSOR_H

// INCLUDES
#include "xqsensor_p.h"
#include "xqaccsensor.h"

// CLASS DECLARATION
class XQAccelerationSensorPrivate: public XQSensorPrivate
{
public:
    XQAccelerationSensorPrivate(XQSensor& qq);
    ~XQAccelerationSensorPrivate();

    void addFilter(XQAbstractAccelerationSensorFilter &filter);
    QList<XQAbstractAccelerationSensorFilter *>& filters();

    int xAcceleration() const;
    int yAcceleration() const;
    int zAcceleration() const;

protected:  //from XQSensorPrivate
    TInt SensorId() const;
    
protected: // from MRRSensorDataListener
    void HandleDataEventL(TRRSensorInfo aSensor, TRRSensorEvent aEvent);

protected:
    int iXAcceleration;
    int iYAcceleration;
    int iZAcceleration;    
    QList<XQAbstractAccelerationSensorFilter *> iFilters;
};

#endif /*QS60ACCELERATIONSENSOR_H*/

// End of file
