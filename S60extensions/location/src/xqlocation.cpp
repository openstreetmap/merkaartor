// INCLUDE FILES
#include "xqlocation.h"
#ifdef Q_OS_SYMBIAN
#include "xqlocation_s60_p.h"
#else
#include "xqlocation_stub_p.h"
#endif

#include <QTimer>

/****************************************************
 *
 * XQLocation
 *
 ****************************************************/
/*!
    \class XQLocation
    \brief The XQLocation class can be used for providing regular updates about one's 
           global position, and other positioning module related information, at a
           particular point in time.
    
    Example:
    \code
    XQLocation *location = new XQLocation(this);

    QLineEdit *displayLatitude = new LineDisplay();
    connect(location, SIGNAL(latitudeChanged(double,float)), displayLatitude, SLOT(setNum(double)));

    QLineEdit *displayLongitude = new LineDisplay();
    connect(location, SIGNAL(longitudeChanged(double,float)), displayLongitude, SLOT(setNum(double)));

    QLineEdit *displayAltitude = new LineDisplay();
    connect(location, SIGNAL(altitudeChanged(double,float)), displayAltitude, SLOT(setNum(double)));

    QLineEdit *displaySpeed = new LineDisplay();
    connect(location, SIGNAL(speedChanged(float)), displaySpeed, SLOT(setNum(float)));
    \endcode
*/    

/*!
    Constructs a XQLocation object with the given parent.
    \sa open(), startUpdates(), requestUpdate()
*/
XQLocation::XQLocation(QObject *parent)
 : QObject(parent), d(new XQLocationPrivate(this))
{
}

/*!
    Destroys the XQLocation object. If there is an open connection to positioning module,
    it is closed.
*/
XQLocation::~XQLocation()
{
    delete d;
}

/*!
    \enum XQLocation::Error

    This enum defines the possible errors for a XQLocation object.
*/
/*! \var XQLocation::Error XQLocation::NoError
    No error occured.
*/
/*! \var XQLocation::Error XQLocation::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQLocation::Error XQLocation::OpenError
    Error opening connection to locationing services.
*/
/*! \var XQLocation::Error XQLocation::DefaultModuleNotFoundError
    Default positioning module not found.
*/
/*! \var XQLocation::Error XQLocation::ModuleNotFoundError
    Positioning module not found.
*/
/*! \var XQLocation::Error XQLocation::InvalidUpdateIntervalError
    Invalid update interval.
*/
/*! \var XQLocation::Error XQLocation::AccessDeniedError
    Access is denied to locationing services.
*/
/*! \var XQLocation::Error XQLocation::InternalError
    Internal error.
*/
/*! \var XQLocation::Error XQLocation::UnknownError
    Unknown error.
*/

/*!
    \enum XQLocation::DeviceStatus

    This enum defines the possible statuses for a XQLocation object.

    \sa status(), statusChanged()
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusError
    This state is used to indicate that there are problems when using the device.
    For example, there may be hardware errors. It should not be confused with
    complete loss of data quality (see TDataQualityStatus), which indicates that
    the device is functioning correctly, but is currently unable to obtain position
    information. The error state is reported if the device cannot be successfully
    brought on line. For example, the positioning module may have been unable to
    communicate with the device or it is not responding as expected.
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusDisabled
    Although the device may be working properly, it has been taken off line and is
    regarded as being unavailable to obtain position information. This is generally
    done by the user via the control panel. In this state, Mobile Location FW will
    not use the device.
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusInactive
    This state signifies that the device is not being used by Mobile Location FW.
    This typically occurs because there are no clients currently obtaining positions
    from it.
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusInitialising
    This is a transient state. The device is being brought out of the Inactive state
    but has not reached either the Ready or Standby modes. The initializing state
    occurs when the positioning module is first selected to provide a client
    application with location information.
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusStandBy
    This state indicates that the device has entered the Sleep or Power save mode.
    This signifies that the device is online, but it is not actively retrieving
    position information. A device generally enters this mode when the next position
    update is not required for some time and it is more efficient to partially power
    down.
    Note: Not all positioning modules support this state, particularly when there is
          external hardware. 
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusReady
    The positioning device is online and is ready to retrieve position information.
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusActive
    The positioning device is actively in the process of retrieving position
    information.
    Note: Not all positioning modules support this state, particularly when there is
          external hardware. 
*/
/*! \var XQLocation::DeviceStatus XQLocation::StatusUnknown
    This is not a valid state and must never be reported.
*/

/*!
    \enum XQLocation::DataQuality

    This enum defines the possible data quality values for a XQLocation object.

    \sa dataQuality(), dataQualityChanged()
*/
/*! \var XQLocation::DataQuality XQLocation::DataQualityLoss
    This state indicates that the accuracy and contents of the position information
    has been completely compromised. It is no longer possible to return any coherent
    data. This situation occurs if the device has lost track of all the transmitters
    (for example, satellites or base stations). It should be noted that although it
    is currently not possible to obtain position information, the device may still be
    functioning correctly. This state should not be confused with a device error
    (see DeviceStatus above).
*/
/*! \var XQLocation::DataQuality XQLocation::DataQualityPartial
    The state signifies that there has been a partial degradation in the available
    position information. In particular, it is not possible to provide the required
    (or expected) quality of information. This situation may occur if the device has
    lost track of one of the transmitters (for example, satellites or base stations).
*/
/*! \var XQLocation::DataQuality XQLocation::DataQualityNormal
    The positioning device is functioning as expected.
*/
/*! \var XQLocation::DataQuality XQLocation::DataQualityUnknown
    This is an unassigned valued. This state should only be reported during an event
    indicating that a positioning module has been removed.
*/

/*!
    Sets interval between each update.
    if startUpdates() is already called, startUpdates() must
    be called again to make interval change effective.

    \param interval an interval in milliseconds.
    \sa updateInterval(), startUpdates()
*/
void XQLocation::setUpdateInterval(int interval)
{
    if (interval < 0) {
        interval = 0;
    }
    d->updateInterval = interval;
}

/*! 
    Returns interval between each update.

    \return update interval
    \sa setUpdateInterval(), startUpdates()
*/
int XQLocation::updateInterval() const
{
    return d->updateInterval;
}

/*! 
    Returns the current status.

    \return current status
    \sa statusChanged(), startUpdates(), requestUpdate(), stopUpdates()
*/
XQLocation::DeviceStatus XQLocation::status() const
{
    return d->status;
}

/*!
    Returns the current data quality.

    \return data quality
    \sa dataQualityChanged(), startUpdates(), requestUpdate(), stopUpdates()
*/
XQLocation::DataQuality XQLocation::dataQuality() const
{
    return d->dataQuality;
}

/*!
    Opens the connection to positioning module.

    \return NoError if the connection to positioning module module was successfully opened;
            otherwise returns error code.
    \sa close(), startUpdates(), requestUpdate(), stopUpdates()
*/
XQLocation::Error XQLocation::open()
{
    return d->openConnectionToPositioner();
}

/*!
    Closes the connection to positioning module.

    \sa open(), startUpdates(), requestUpdate(), stopUpdates()
*/
void XQLocation::close()
{
    d->closeConnectionToPositioner();
}

/*!
    Requests signal to be emitted with the current positioning module values if possible.
    This slot can be called regardless of whether startUpdates() has already been called.
    This is useful if you need to retrieve the current positioning module values but you do not
    need the periodic updates offered by startUpdates().
    If positioning module is not ready, the object will first be initialized and will provide
    update once it has acquired position fix.

    \sa startUpdates()
*/
void XQLocation::requestUpdate()
{
    d->requestUpdate();
}

/*!
    Starts emitting positioning module value updates with the interval specified by updateInterval(),
    or less frequently if updates are not available at a particular time. If updateInterval()
    is 0, position updates are emitted as soon as a valid update becomes available.
    If startUpdates() has already been called, this restarts with updateInterval() as the
    new update interval.
    If positioning module is not ready, the object will first be initialized and will begin to provide
    updates once it has acquired position fix.

    \sa stopUpdates(), setUpdateInterval(), updateInterval(), requestUpdate()
*/
void XQLocation::startUpdates()
{
    d->startUpdates();
}

/*!
    This is an overloaded member function, provided for convenience.
    Starts providing updates with an update interval of msec milliseconds.

    \sa stopUpdates(), setUpdateInterval(), updateInterval(), requestUpdate()
*/
void XQLocation::startUpdates(int msec)
{
    d->startUpdates(msec);
}


/*!
    Stops emitting positioning module value updates at regular intervals.

    \sa startUpdates(), setUpdateInterval(), updateInterval()
*/
void XQLocation::stopUpdates()
{
    d->stopUpdates();
}

/*!
    \fn void XQLocation::locationChanged(double latitude, double longitude, double altitude, float speed)

    This signal is emitted when location and speed update is available.

    \param latitude a latitude in WGS84 format
    \param longitude a longitude in WGS84 format
    \param altitude an altitude in WGS84 format
    \param speed a speed, in metres per second.
    \sa startUpdates(), requestUpdate()
*/

/*!
    \fn void XQLocation::latitudeChanged(double latitude, float accuracy)
    
    This signal is emitted when latitude update is available.

    \param latitude a latitude in WGS84 format
    \param accuracy the horizontal accuracy, in metres.
    \sa startUpdates(), requestUpdate(), locationChanged()
*/


/*!
    \fn void XQLocation::longitudeChanged(double longitude, float accuracy)

    This signal is emitted when longitude update is available.

    \param longitude a longitude in WGS84 format
    \param accuracy the horizontal accuracy, in metres.
    \sa startUpdates(), requestUpdate(), locationChanged()
*/

/*!
    \fn void XQLocation::altitudeChanged(double altitude, float accuracy)

    This signal is emitted when altitude update is available.

    \param altitude an altitude in WGS84 format
    \param accuracy the vertical accuracy, in metres.
    \sa startUpdates(), requestUpdate(), locationChanged()
*/

/*!
    \fn void XQLocation::speedChanged(float speed)

    This signal is emitted when speed update is available.

    \param speed a speed, in metres per second.
    \sa startUpdates(), requestUpdate(), locationChanged()
*/

/*!
    \fn void XQLocation::statusChanged(XQLocation::DeviceStatus status)
    
    This signal is emitted when status update is available.

    \param status a status of positioning module.
    \sa startUpdates(), requestUpdate()
*/

/*!
    \fn void XQLocation::dataQualityChanged(XQLocation::DataQuality dataQuality)

    This signal is emitted when data quality update is available.

    \param dataQuality a data quality of positioning module.
    \sa startUpdates(), requestUpdate()
*/

/*!
    \fn void XQLocation::numberOfSatellitesInViewChanged(int numSatellites)

    This signal is emitted when 'satellites in view' update is available.

    \param numSatellites the number of satellites in view.
    \sa startUpdates(), requestUpdate()
*/

/*!
    \fn void XQLocation::numberOfSatellitesUsedChanged(int numSatellites)

    This signal is emitted when 'used satellites' update is available.

    \param numSatellites The number of satellites used in the position calculation.
    \sa startUpdates(), requestUpdate()
*/

/*!
    \fn void XQLocation::error(XQLocation::Error errorCode)

    This signal is emitted when error has occured.

    \param errorCode a error code
    \sa startUpdates(), requestUpdate()
*/

// End of file
