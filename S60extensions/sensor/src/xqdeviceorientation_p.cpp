#include "xqdeviceorientation_p.h"
#include <e32math.h>

const TInt KDefaultResolution = 15;

XQDeviceOrientationPrivate::XQDeviceOrientationPrivate(
        XQDeviceOrientation& qq) : q(qq), iResolution(KDefaultResolution)
{
    iAccelerationSensor.addFilter(*this);
    iAccelerationSensor.addFilter(iAccelerationPostFilter);

    iAccelerationSensor.open();
    iAccelerationSensor.startReceiving();
}

XQDeviceOrientationPrivate::~XQDeviceOrientationPrivate()
{
    iAccelerationSensor.stopReceiving();
    iAccelerationSensor.close();
    iAccelerationSensor.filters().removeOne(this);
}

void XQDeviceOrientationPrivate::setResolution(TInt resolution)
{
    iResolution = resolution;
}

TInt XQDeviceOrientationPrivate::resolution() const
{
    return iResolution;
}

int XQDeviceOrientationPrivate::xRotation() const
{
    return iXRotation;
}

int XQDeviceOrientationPrivate::yRotation() const
{
    return iYRotation;
}

int XQDeviceOrientationPrivate::zRotation() const
{
    return iZRotation;
}

XQDeviceOrientation::DisplayOrientation
        XQDeviceOrientationPrivate::orientation() const
{
    return iOrientation;
}

bool XQDeviceOrientationPrivate::filter(int &xAcceleration, 
    int &yAcceleration, int &zAcceleration)
{
    TReal xAcc = xAcceleration;
    TReal yAcc = yAcceleration;
    TReal zAcc = zAcceleration;

    TReal angle = 0;

    //axis X rotation
    if (yAcc != 0) {
        Math::ATan(angle, Abs(zAcc / yAcc));
        angle *= 180 / KPi;  //rad to degrees
        if (zAcc > 0 && yAcc > 0) {
            //do nothing
        } else if (zAcc > 0 && yAcc < 0) {
            angle = 90 + (90 - angle);
        } else if (zAcc < 0 && yAcc < 0) {
            angle += 180;
        } else {
            angle = 270 + (90 - angle);
        }
    } else {
        if (zAcceleration > 0) {
            angle = 90;
        } else {
            angle = 270;
        }
    }
    TInt xRotation = RoundAngle(angle);

    //axis Y rotation
    if (xAcc != 0) {
        Math::ATan(angle, Abs(zAcc / xAcc));
        angle *= 180 / KPi;  //rad to degrees
        if (zAcc > 0 && xAcc > 0) {
            angle += 90;
        } else if (zAcc > 0 && xAcc < 0) {
            angle = 270 - angle;
        } else if (zAcc < 0 && xAcc < 0) {
            angle += 270;
        } else {
            angle = 90 - angle;
        }
    } else {
        if (zAcceleration > 0) {
            angle = 180;
        } else {
            angle = 0;
        }
    }
    TInt yRotation = RoundAngle(angle);

    //axis Z rotation
    if (yAcc != 0) {
        Math::ATan(angle, Abs(xAcc / yAcc));
        angle *= 180 / KPi;  //rad to degrees
        if (xAcc > 0 && yAcc > 0) {
            angle = 360 - angle;
        } else if (xAcc > 0 && yAcc < 0) {
            angle += 180;
        } else if (xAcc <= 0 && yAcc <= 0) {
            angle = 180 - angle;
        } else {
            //Do nothing
        }
    } else {
        if (xAcceleration > 0) {
            angle = 270;
        } else {
            angle = 90;
        }
    }
    TInt zRotation = RoundAngle(angle);

    iXRotation = xRotation;
    iYRotation = yRotation;
    iZRotation = zRotation;

    XQDeviceOrientation::DisplayOrientation orientation =
            XQDeviceOrientation::OrientationUndefined;
    if (((315 < xRotation && xRotation < 360) ||
            (0 <= xRotation && xRotation <= 45)) &&
         ((315 < zRotation && zRotation < 360) ||
            (0 <= zRotation && zRotation <= 45))) {
        orientation = XQDeviceOrientation::OrientationDisplayUp;
    } else if (135 < xRotation && xRotation <= 225 && 135 < zRotation &&
            zRotation <= 225) {
        orientation = XQDeviceOrientation::OrientationDisplayDown;
    } else if (225 < yRotation && yRotation <= 315 && 45 < zRotation &&
            zRotation <= 135) {
        orientation = XQDeviceOrientation::OrientationDisplayLeftUp;
    } else if (45 < yRotation && yRotation <= 135 && 225 < zRotation &&
            zRotation <= 315) {
        orientation = XQDeviceOrientation::OrientationDisplayRightUp;
    } else if (45 < xRotation && xRotation <= 135 && 135 < yRotation &&
            yRotation <= 225) {
        orientation = XQDeviceOrientation::OrientationDisplayUpwards;
    } else if (225 < xRotation && xRotation <= 315 &&
        ((315 < yRotation && yRotation < 360) || (0 <= yRotation &&
            yRotation <= 45))) {
        orientation = XQDeviceOrientation::OrientationDisplayDownwards;
    }

    emit q.rotationChanged(xRotation, yRotation, zRotation);

    if (orientation != iOrientation) {
        if (orientation != XQDeviceOrientation::OrientationUndefined) {
            emit q.orientationChanged(orientation);
        }
        iOrientation = orientation;
    }

    return false;
}

TInt XQDeviceOrientationPrivate::RoundAngle(TReal aAngle) const
{
    return ((TInt(aAngle) + iResolution / 2) / iResolution * iResolution) % 360;
}

// End of file
