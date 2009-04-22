#ifndef XQDEVICEORIENTATIONPRIVATE_H
#define XQDEVICEORIENTATIONPRIVATE_H

#include "XQAccSensor.h"
#include "xqdeviceorientation.h"

class XQDeviceOrientationPrivate : public XQAbstractAccelerationSensorFilter
{
public:
    XQDeviceOrientationPrivate(XQDeviceOrientation& qq);
    virtual ~XQDeviceOrientationPrivate();

    void setResolution(TInt resolution);
    TInt resolution() const;

    int xRotation() const;
    int yRotation() const;
    int zRotation() const;
    XQDeviceOrientation::DisplayOrientation orientation() const;

protected:  //from XQAbstractAccelerationSensorFilter
    bool filter(int &xAcceleration, int &yAcceleration, int &zAcceleration);

private:
    TInt RoundAngle(TReal aAngle) const;

private:
    XQDeviceOrientation &q;
    XQAccelerationSensor iAccelerationSensor;
    XQAccelerationDataPostFilter iAccelerationPostFilter;
    TInt iResolution;
    TInt iXRotation;
    TInt iYRotation;
    TInt iZRotation;
    XQDeviceOrientation::DisplayOrientation iOrientation;
};

#endif /*XQDEVICEORIENTATIONPRIVATE_H*/

// End of file
