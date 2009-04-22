#include "xqdeviceorientation.h"
#include "xqdeviceorientation_p.h"

/*!
    \class XQDeviceOrientation

    \brief The XQDeviceOrientation class is used to determine the orientation
    of the device.

    Note that the orientation is calculated from the values received from
    acceleration sensor so the orientation is reliable only when the speed of
    the device is constant.
    
    Example:
    \code
    XQDeviceOrientation *orientation = new XQDeviceOrientation(this);
    orientation->setResolution(5);  //degrees
    QObject::connect(
        orientation, SIGNAL(orientationChanged(XQDeviceOrientation::DisplayOrientation)),
        this, SLOT(updateOrientation(XQDeviceOrientation::DisplayOrientation)));
    int xRotation = orientation->xRotation();
    \endcode
*/

/*!
    \enum XQDeviceOrientation::DisplayOrientation

    This enum defines the possible orientations of the device.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationUndefined
    The orientation can't be determined.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayUp
    The orientation of the device is up, i.e. standing.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayDown
    The device is upside down, i.e. bottom up.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayLeftUp
    The left side of the device is up.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayRightUp
    The right side of the device is up.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayUpwards
    The device is laying on its back.
*/

/*! \var XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::OrientationDisplayDownwards
    The device is laying on its face.
*/


/*!
    Constructs a XQDeviceOrientation object with parent \a parent.
    \sa open(), startReceiving()
*/
XQDeviceOrientation::XQDeviceOrientation(QObject *parent):
    QObject(parent),
    d(new XQDeviceOrientationPrivate(*this))
{
}

/*!
    Destroys the XQDeviceOrientation, deleting all its children.
*/
XQDeviceOrientation::~XQDeviceOrientation()
{
}

/*!
    Sets the resolution of rotation angles. If this function is not called the
    default resolution (15 degrees) is used.
    \param resolution Resolution in degrees.
*/
void XQDeviceOrientation::setResolution(int resolution)
{
    d->setResolution(resolution);
}

/*!
    Returns the current resolution of rotation angles.
    \return Resolution in degrees
*/
int XQDeviceOrientation::resolution() const
{
    return d->resolution();
}

/*!
    Returns the x axis rotation
    \return Rotation in degrees
*/
int XQDeviceOrientation::xRotation() const
{
    return d->xRotation();
}

/*!
    Returns the y axis rotation
    \return Rotation in degrees
*/
int XQDeviceOrientation::yRotation() const
{
    return d->yRotation();
}

/*!
    Returns the z axis rotation
    \return Rotation in degrees
*/
int XQDeviceOrientation::zRotation() const
{
    return d->zRotation();
}

/*!
    Returns current orientation
    \return Current orientation
*/
XQDeviceOrientation::DisplayOrientation XQDeviceOrientation::orientation() const
{
    return d->orientation();
}

/*!
    \fn void XQDeviceOrientation::rotationChanged(int xRotation, int yRotation, int zRotation);
    
    This signal is emitted when rotation of the device has changed.

    \param xRotation Rotation of x axis in degrees (0-359).
    \param yRotation Rotation of y axis in degrees (0-359).
    \param zRotation Rotation of z axis in degrees (0-359).
    \sa startReceiving()
*/

/*!
    \fn void XQDeviceOrientation::orientationChanged(XQDeviceOrientation::DisplayOrientation orientation);
    
    This signal is emitted when orientation of the device has changed.

    \param orientation Current orientation
    \sa startReceiving()
*/
