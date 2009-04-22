#include "xqaccsensor_p.h"
#include "xqaccsensor.h"

#include <e32base.h>

const TInt KAccelerometerSensorUID = 0x10273024;

XQAccelerationSensorPrivate::XQAccelerationSensorPrivate(XQSensor& qq) :
    XQSensorPrivate(qq)
{
}

XQAccelerationSensorPrivate::~XQAccelerationSensorPrivate()
{
}

TInt XQAccelerationSensorPrivate::SensorId() const
{
    return KAccelerometerSensorUID;
}

void XQAccelerationSensorPrivate::HandleDataEventL(TRRSensorInfo aSensor, TRRSensorEvent aEvent)
{
    if (aSensor.iSensorId == KAccelerometerSensorUID) {
        for (int i = iFilters.size() - 1; i >= 0; --i) {
            XQAbstractAccelerationSensorFilter *filter = iFilters.at(i);
            if (filter->filter(aEvent.iSensorData1, aEvent.iSensorData2, aEvent.iSensorData3))
                break;
        }
        iXAcceleration = aEvent.iSensorData1;
        iYAcceleration = aEvent.iSensorData2;
        iZAcceleration = aEvent.iSensorData3;
    }
}

void XQAccelerationSensorPrivate::addFilter(XQAbstractAccelerationSensorFilter &filter)
{
    iFilters.append(&filter);
}

QList<XQAbstractAccelerationSensorFilter *>& XQAccelerationSensorPrivate::filters()
{
    return iFilters;
}

int XQAccelerationSensorPrivate::xAcceleration() const
{
    return iXAcceleration;
}

int XQAccelerationSensorPrivate::yAcceleration() const
{
    return iYAcceleration;
}

int XQAccelerationSensorPrivate::zAcceleration() const
{
    return iZAcceleration;
}

bool XQAccelerationDataPostFilter::filter(int &xAcceleration, int &yAcceleration, int &zAcceleration)
{
    const int maxAcceleration = 680;    //for plugin
    int xAcc = yAcceleration;
    int yAcc = -xAcceleration;
    int zAcc = -zAcceleration;
    xAcceleration = xAcc * 100 / maxAcceleration;
    yAcceleration = yAcc * 100 / maxAcceleration;
    zAcceleration = zAcc * 100 / maxAcceleration;
    return false;
}

// End of file
