// INCLUDE FILES
#include "xqaccesspointmanager.h"
#include "xqwlan_p.h"
#ifdef Q_OS_SYMBIAN
#include "xqaccesspointmanager_s60_p.h"
#else
#include "xqaccesspointmanager_stub_p.h"
#endif

/****************************************************
 *
 * XQAccessPointManager
 *
 ****************************************************/
/*!
    \class XQAccessPointManager

    \brief The XQAccessPointManager class can be used for listing available
           Internet Access Points and to set Access Point to use.
    
    Example:
    \code
    XQAccessPointManager *apmanager = new XQAccessPointManager(this);
    QList<XQAccessPoint> aps = apmanager->accessPoints();
    apmanager->setDefaultAccessPoint(aps[0]);
    unsigned long int iap = apmanager->defaultAccessPoint().id();
    \endcode
*/

/*!
    Constructs a XQAccessPointManager object with the given parent.
    \sa listIAPs(), defaultIAP(), setDefaultIAP()
*/
XQAccessPointManager::XQAccessPointManager(QObject *parent)
    : QObject(parent),
      d(new XQAccessPointManagerPrivate)
{
}

/*!
    Destroys the XQAccessPointManager object.
*/
XQAccessPointManager::~XQAccessPointManager()
{
    delete d;
}

/*!
    Returns the currently used Access Point (default Access Point)<br>
    Note: Access Point can be returned only if Access Point
          has been set using setDefaultAccessPoint() function

    \return currently used Access Point on success; otherwise returns null Access Point
    \sa setDefaultAccessPoint()
*/
const XQAccessPoint& XQAccessPointManager::defaultAccessPoint() const
{
    return d->defaultAccessPoint();
}

/*!
    Returns true if SetDefaultAccessPoint is supported; otherwise returns
    false.<br>
    Note: SetDefaultAccessPoint is supported by coming Open C release.
          SetDefaultAccessPoint is not supported by currently released
          Open C version.

    \return true if SetDefaultAccessPoint is supported; otherwise returns false.

    \sa defaultAccessPoint(), setDefaultAccessPoint()
*/
bool XQAccessPointManager::isSetDefaultAccessPointSupported() const
{
    return d->isSetDefaultAccessPointSupported();
}

/*!
    Returns the list of Access Points.<br>
    Note: All access points are listed, including access
          points which can not be currently used to connect
          internet.

    \return the list of Access Points.
    \sa defaultAccessPoint(), setDefaultAccessPoint()
*/
QList<XQAccessPoint> XQAccessPointManager::accessPoints() const
{
    return d->accessPoints();
}

/*!
    Returns the list of available Access Points.<br>
    Note: All access points are not listed. Returned list
          contains only access points which can be currently
          used to connect internet.

    \return the list of available Access Points.
    \sa defaultAccessPoint(), setDefaultAccessPoint()
*/
QList<XQAccessPoint> XQAccessPointManager::availableAccessPoints() const
{
    return d->availableAccessPoints();
}

/*!
    Returns the list of active Access Points.

    \return the list of active Access Points.
    \sa defaultAccessPoint(), setDefaultAccessPoint()
*/
QList<XQAccessPoint> XQAccessPointManager::activeAccessPoints() const
{
    return d->activeAccessPoints();
}

/*!
    Sets default Access Point.
    After setting default Access Point all Qt network connections will use
    newly set Access Point.

    \return true if the Access Point was successfully set; otherwise returns false.
    \sa defaultAccessPoint()
*/
bool XQAccessPointManager::setDefaultAccessPoint(const XQAccessPoint& accessPoint)
{
    return d->setDefaultAccessPoint(accessPoint);
}

/*!
    Returns Access Point using system default behaviour.
    If system is set to show Connection Selection Dialog, dialog will be shown
    to user. If system is set to use default connection, Connection Selection
    Dialog won't be shown to user.<br>
    If connection can be succesfully initialized, Access Point will be returned
    to caller.<br>
    Note: setDefaultAccessPoint() must be called to get Access Point into use.

    \return Access Point on success; otherwise returns null Access Point.
    \sa setDefaultAccessPoint()
*/
XQAccessPoint XQAccessPointManager::systemAccessPoint() const
{
    return d->systemAccessPoint();
}

/*!
    Returns preferred Access Point using following rules/order:<br>
    1. If network connection is already active, returns Access Point for
       active connection<br>
    2. If WLAN Access Point is available, returns WLAN Access Point<br>
    3. If GPRS/3G Access Point is available, returns GPRS/3G Access Point<br>
    <br>
    If preferred Access Point can be found, Access Point will be returned
    to caller.<br>
    Note: setDefaultAccessPoint() must be called to get Access Point into use.

    \return preferred Access Point on success; otherwise returns null Access Point.
    \sa setDefaultAccessPoint()
*/
XQAccessPoint XQAccessPointManager::preferredAccessPoint() const
{
    return d->preferredAccessPoint();
}

/*!
    Returns Access Point by id.

    \return Access Point by id.
*/
XQAccessPoint XQAccessPointManager::accessPointById(unsigned long int id) const
{
    return d->accessPointById(id);
}

/*!
    Returns the list of available WLANs.

    \return the list of available WLANs.
*/
QList<XQWLAN> XQAccessPointManager::availableWLANs() const
{
    return d->availableWLANs2();
}

/****************************************************
 *
 * XQAccessPoint
 *
 ****************************************************/
class XQAccessPointPrivate
{
public:
    XQAccessPointPrivate(): ref(1), id(0), modemBearer(XQAccessPoint::ModemBearerUnknown)  {}
    XQAccessPointPrivate(const XQAccessPointPrivate &copy)
        : ref(1),id(copy.id), name(copy.name), gprsName(copy.gprsName) {}
    ~XQAccessPointPrivate() {}
    
    QAtomicInt        ref;
    unsigned long int id;
    QString           name;
    QString           gprsName;
    XQAccessPoint::ModemBearer modemBearer;
};

/*!
    \class XQAccessPoint

    \brief The XQAccessPoint class can be used for storing and handling internet
           Access Points.
*/

/*!
    Constructs a null XQAccessPoint object
*/
XQAccessPoint::XQAccessPoint()
    :  d(0)
{
}

/*!
    Constructs a new XQAccessPoint object using given \a name and \a id.

    \param name the name of the Access Point
    \param id the id of the Access Point
*/
XQAccessPoint::XQAccessPoint(const QString & name, unsigned long int id)
    :  d(new XQAccessPointPrivate)
{
    d->id = id;
    d->name = name;
}

/*!
    Constructs a copy of \a other Access Point.

    \param other the access point to be copied
*/
XQAccessPoint::XQAccessPoint(const XQAccessPoint &other)
    : d(other.d)
{
    if (d) {
        d->ref.ref();
    }
}

/*!
    Destroys the XQAccessPoint object.
*/
XQAccessPoint::~XQAccessPoint()
{
}

/*!
    Sets Access Point id.

    \sa id()
*/
void XQAccessPoint::setId(unsigned long int id)
{
    detach();
    d->id = id;
}

/*!
    Returns the Access Point id.

    \return Access Point id
    \sa setId()
*/
unsigned long int XQAccessPoint::id() const
{
    return d ? d->id : 0;
}

/*!
    Sets Access Point name.

    \sa name()
*/
void XQAccessPoint::setName(const QString& name)
{
    detach();
    d->name = name;
}

/*!
    Returns the Access Point name.

    \return Access Point name
    \sa setName()
*/
QString XQAccessPoint::name() const
{
    return d ? d->name : QString();
}

/*!
    Sets GPRS name.

    \sa gprsName()
*/
void XQAccessPoint::setGprsName(const QString& name)
{
    detach();
    d->gprsName = name;
}

/*!
    Returns the GPRS name.

    \return connection name
    \sa setGPRSName()
*/
QString XQAccessPoint::gprsName() const
{
    return d ? d->gprsName : QString();
}

/*!
    Sets Access Point modem bearer.

    \sa modemBearer()
*/
void XQAccessPoint::setModemBearer(XQAccessPoint::ModemBearer bearer)
{
    detach();
    d->modemBearer = bearer;
}

/*!
    Returns the Access Point modem bearer.

    \return Access Point modem bearer
    \sa setModemBearer()
*/
XQAccessPoint::ModemBearer XQAccessPoint::modemBearer() const
{
    return d ? d->modemBearer : XQAccessPoint::ModemBearerUnknown;
}

/*!
    Returns true if this Access Point is null; otherwise returns false.

    \return true if the Access Point is null; otherwise returns false.
*/
bool XQAccessPoint::isNull() const
{
    return !d; 
}


/*!
    Assigns the \a other Access Point to this Access Point and returns a
    reference to this Access Point.
*/
XQAccessPoint& XQAccessPoint::operator=(const XQAccessPoint& other)
{
    if (d == other.d) {
        return *this;
    }
    other.d->ref.ref();
    if (d && !d->ref.deref()) {
        delete d;
    }
    d = other.d;
    return *this;
}

void XQAccessPoint::detach()
{
    if (!d) {
        d = new XQAccessPointPrivate;
    } else {
        qAtomicDetach(d);
    }
}


/****************************************************
 *
 * XQWLAN
 *
 ****************************************************/
/*!
    \class XQWLAN

    \brief The XQWLAN class can be used for storing and handling WLANs.
*/

/*!
    Constructs a null XQWLAN object
*/
XQWLAN::XQWLAN()
    : d(0)
{
}

/*!
    Constructs a copy of \a other wlan.

    \param other the wlan to be copied
*/
XQWLAN::XQWLAN(const XQWLAN &other)
    : d(other.d)
{
    if (d) {
        d->ref.ref();
    }
}

/*!
    Destroys the XQWLAN object.
*/
XQWLAN::~XQWLAN()
{
    if (d && !d->ref.deref()) {
        delete d;
    }
}
 
/*!
    Returns the name (SSID) of the WLAN.

    \return the name of the WLAN
*/
QString XQWLAN::name() const
{
    return d ? d->ssid : QString(); 
}

/*!
    Returns the MAC Address of the WLAN.

    \return the MAC Address of the WLAN
*/
QString XQWLAN::macAddress() const
{
    return d ? d->mac : QString(); 
}

/*!
    Returns the signal strength of the WLAN.

    \return the signal strength of the WLAN
*/
short int XQWLAN::signalStrength() const
{
    return d ? d->signalStrength : -1;
}

/*!
    Returns true if this WLAN uses preshared key; otherwise returns false.

    \return true if this WLAN uses preshared key; otherwise returns false
*/
bool XQWLAN::usesPreSharedKey() const
{
    return d ? d->usesPreSharedKey : false;
}

/*!
    Returns the network mode of the WLAN.

    \return the network mode of the WLAN
*/
XQWLAN::WlanNetworkMode XQWLAN::networkMode() const
{
    return d ? d->networkMode : XQWLAN::WlanNetworkModeUnknown;
}

/*!
    Returns the security mode of the WLAN.

    \return the security mode of the WLAN
*/
XQWLAN::WlanSecurityMode XQWLAN::securityMode() const
{
    return d ? d->securityMode : XQWLAN::WlanSecurityModeUnknown;
}

/*!
    Returns true if this WLAN is visible; otherwise returns false.

    \return true if this WLAN is visible; otherwise returns false
*/
bool XQWLAN::isVisible() const
{
    return d ? d->visibility : false;
}

/*!
 Returns true if this wlan is null; otherwise returns false.

 \return true if this wlan is null; otherwise returns false
*/
bool XQWLAN::isNull() const
{
    return !d;
}

/*!
    Assigns the \a other wlan to this wlan and returns a
    reference to this wlan.
*/
XQWLAN& XQWLAN::operator=(const XQWLAN& other)
{
    if (d == other.d) {
        return *this;
    }
    other.d->ref.ref();
    if (d && !d->ref.deref()) {
        delete d;
    }
    d = other.d;
    return *this;
}

void XQWLAN::detach()
{
    if (!d) {
        d = new XQWLANPrivate;
    } else {
        qAtomicDetach(d);
    }
}

// End of file

