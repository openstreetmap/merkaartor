#include "XQAccSensor.h"
#include "XQAccSensor_p.h"

/*!
    \class XQAccelerationSensor

    \brief The XQAccelerationSensor class is used to determine current
    acceleration of the device.

    Note that the orientation of the device affects on the acceleration sensors
    due to acceleration caused by Earth's gravity. Therefore it is wrong to
    assume that the axis values are zero when the device stands still.
    
    XQAccelerationSensor axis data shouldn't be directly used to determine the
    device orientation, though. It may sound good to use XQAccelerationSensor
    in a game that is controlled by leaning the device but the data ranges and
    even axis orientation may differ highly in different devices. Use
    XQDeviceOrientation class for that purpose instead.
    
    Example:
    \code
    #include "XQAccSensor.h"
    class MyAccelerationFilter : public QObject,
        public XQAbstractAccelerationSensorFilter
    {
        Q_OBJECT
    protected:  //from XQAbstractAccelerationSensorFilter
        bool filter(int &xAcceleration, int &yAcceleration, int &zAcceleration);
    private:
        int m_xAcceleration;
    };
    
    bool MyAccelerationFilter::filter(int &xAcceleration, 
        int &yAcceleration, int &zAcceleration)
    {
        zAcceleration = 0; //Suppress z axis acceleration
        m_xAcceleration = xAcceleration;    //Save for further use
        return false;   //Allow next filter to be called
    }
    
    XQAccelerationSensor *sensor = new XQAccelerationSensor(this);
    sensor->open();

    MyAccelerationFilter *filter = new MyAccelerationFilter(this);
    sensor->addFilter(*filter);

    sensor->startReceiving();
    \endcode
*/

/*!
    Constructs a XQAccelerationSensor object with parent \a parent.
    \sa open(), startReceiving()
*/
XQAccelerationSensor::XQAccelerationSensor(QObject *parent):
    XQSensor(*new XQAccelerationSensorPrivate(*this), parent)
{
}

/*!
    Destroys the XQAccelerationSensor, deleting all its children.
*/
XQAccelerationSensor::~XQAccelerationSensor()
{
}

/*!
    Adds a filter on top of filter stack.
*/
void XQAccelerationSensor::addFilter(XQAbstractAccelerationSensorFilter &filter)
{
    static_cast<XQAccelerationSensorPrivate*>(d)->addFilter(filter);
}


/*!
    Returns a filter stack
    \return List of filters
*/
QList<XQAbstractAccelerationSensorFilter *>& XQAccelerationSensor::filters()
{
    return static_cast<XQAccelerationSensorPrivate*>(d)->filters();
}

/*!
    Returns the x axis acceleration
    \return Raw axis data
*/
int XQAccelerationSensor::xAcceleration() const
{
    return static_cast<XQAccelerationSensorPrivate*>(d)->xAcceleration();
}

/*!
    Returns the y axis acceleration
    \return Raw axis data
*/
int XQAccelerationSensor::yAcceleration() const
{
    return static_cast<XQAccelerationSensorPrivate*>(d)->yAcceleration();
}

/*!
    Returns the z axis acceleration
    \return Raw axis data
*/
int XQAccelerationSensor::zAcceleration() const
{
    return static_cast<XQAccelerationSensorPrivate*>(d)->zAcceleration();
}

/*!
    \class XQAbstractAccelerationSensorFilter

    \brief The XQAbstractAccelerationSensorFilter is an interface that can be
    used to filter and/or manipulate data received from acceleration sensor.
    
    The client which is interested in sensor events implements this interface.

    There may potentially be several filters in stack. They are called in
    reversed order (i.e. the last filter in stack is always called first.) and
    each of them may manipulate data and also allow or deny the next filter in
    stack to be called.
*/

/*!
    \fn virtual bool XQAbstractAccelerationSensorFilter::filter(int &xAcceleration, int &yAcceleration, int &zAcceleration) = 0;
    
    This function is called when the acceleration of the device has changed.

    \param xAcceleration Reference to x axis acceleration
    \param yAcceleration Reference to y axis acceleration
    \param zAcceleration Reference to z axis acceleration
    \return True if this filter has "filtered out" the event, i.e. the event
    is not to be sent to next filter in stack. Returning false allows next
    filter in stack to be called.
    \sa startReceiving()
*/

/*!
    \class XQAccelerationDataPostFilter

    \brief The XQAccelerationDataPostFilter is a special acceleration sensor
    filter which modifies acceleration sensor data so that the orientation of each
    acceleration axis match with different native APIs.
*/
