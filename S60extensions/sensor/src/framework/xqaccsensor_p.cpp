#include "XQAccSensor_p.h"
#include "XQAccSensor.h"
#include <sensrvaccelerometersensor.h>
#include <sensrvchannel.h>

XQAccelerationSensorPrivate::XQAccelerationSensorPrivate(XQSensor& qq) :
    XQSensorPrivate(qq)
{
}

XQAccelerationSensorPrivate::~XQAccelerationSensorPrivate()
{
    close();
    delete iXyzAxisChannel;
}

void XQAccelerationSensorPrivate::open()
{
    TRAP(iError,
        if(!iXyzAxisChannel) {
            iXyzAxisChannel = CreateChannelL(KSensrvChannelTypeIdAccelerometerXYZAxisData);
        }
        iXyzAxisChannel->OpenChannelL();
    )
}

void XQAccelerationSensorPrivate::close()
{
    if (iXyzAxisChannel) {
        iXyzAxisChannel->CloseChannel();
    }
}

void XQAccelerationSensorPrivate::startReceiving()
{
    if (iXyzAxisChannel) {
        TRAP(iError,
            iXyzAxisChannel->StartDataListeningL(this, 1, 16, 0);
        )
    } else {
        iError = KErrNotReady;
    }
}

void XQAccelerationSensorPrivate::stopReceiving()
{
    if (iXyzAxisChannel) {
        iXyzAxisChannel->StopDataListening();
    }
}

void XQAccelerationSensorPrivate::DataReceived(CSensrvChannel& aChannel,
    TInt aCount, TInt aDataLost)
{
    TSensrvChannelTypeId typeId = aChannel.GetChannelInfo().iChannelType;
    if (typeId == KSensrvChannelTypeIdAccelerometerXYZAxisData) {
        HandleXyzAxisData(aChannel, aCount, aDataLost);
    }
}

void XQAccelerationSensorPrivate::HandleXyzAxisData(CSensrvChannel& aChannel, TInt aCount, TInt /*aDataLost*/)
{
    for(TInt i = 0; i < aCount; i++) {
        TPckgBuf<TSensrvAccelerometerAxisData> dataBuf;
        if (aChannel.GetData(dataBuf) == KErrNone) {
            TSensrvAccelerometerAxisData data = dataBuf();
            for (int i = iFilters.size() - 1; i >= 0; --i) {
                XQAbstractAccelerationSensorFilter *filter = iFilters.at(i);
                if (filter->filter(data.iAxisX, data.iAxisY, data.iAxisZ))
                    break;
            }
            iXAcceleration = data.iAxisX;
            iYAcceleration = data.iAxisY;
            iZAcceleration = data.iAxisZ;
        }
    }
}

XQSensor::Error XQAccelerationSensorPrivate::error() const
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
    const int maxAcceleration = 127;    //for framework
    xAcceleration = xAcceleration * 100 / maxAcceleration;
    yAcceleration = yAcceleration * 100 / maxAcceleration;
    zAcceleration = zAcceleration * 100 / maxAcceleration;
    return false;
}

// End of file
