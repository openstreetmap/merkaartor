#include "xqsensor_p.h"
#include "xqsensor.h"

#include <rrsensorapi.h>

XQSensorPrivate::XQSensorPrivate(XQSensor& qq) : q(qq)
{
}

XQSensorPrivate::~XQSensorPrivate()
{
    close();
}

void XQSensorPrivate::open()
{
    if (!iRRSensorApi) {
        TRAP(iError,
            RArray<TRRSensorInfo> sensorList;
            CRRSensorApi::FindSensorsL(sensorList);
            CleanupClosePushL(sensorList);

            TInt sensorCount = sensorList.Count();
            for (TInt i = 0; i != sensorCount; ++i) {
                if (sensorList[i].iSensorId == SensorId()) {
                    iRRSensorApi = CRRSensorApi::NewL(sensorList[i]);
                    iState = XQSensorPrivate::EStateNotReceiving;
                    break;
                }
            }
            if (iState != XQSensorPrivate::EStateNotReceiving) {
                User::Leave(KErrNotFound);
            }
            
            CleanupStack::PopAndDestroy(&sensorList);
        )
    }
}

void XQSensorPrivate::startReceiving()
{
    if (iRRSensorApi) {
        iRRSensorApi->AddDataListener(this);
        iState = XQSensorPrivate::EStateReceiving;
    }
}

void XQSensorPrivate::stopReceiving()
{
    if (iRRSensorApi) {
        iRRSensorApi->RemoveDataListener();
        iState = XQSensorPrivate::EStateNotReceiving;
    }    
}

void XQSensorPrivate::close()
{
    if (iRRSensorApi
        && iState == XQSensorPrivate::EStateReceiving) {
        stopReceiving();
    }
    delete iRRSensorApi;
    iRRSensorApi = NULL;
    iState = XQSensorPrivate::EStateClosed;
}

XQSensor::Error XQSensorPrivate::error() const
{
    switch (iError) {
        case KErrNone:
            return XQSensor::NoError;
        case KErrNoMemory:
            return XQSensor::OutOfMemoryError;
        case KErrNotFound:
            return XQSensor::NotFoundError;
        default:
            return XQSensor::UnknownError;
    }
}

// End of file
