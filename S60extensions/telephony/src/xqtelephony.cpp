#include "xqtelephony.h"
#include "xqtelephony_p.h"

/*!
    \class XQTelephony
    \brief The XQTelephony class can be used for establishing circuit switched calls 
    and monitoring the status of the line. Monitoring can be started with the 
    XQTelephony::startMonitoringLine() function call. 
    
    Example:
    \code
    XQTelephony *telephony = new XQTelephony(this);
    connect(telephony,SIGNAL(lineStatusChanged(XQTelephony::LineStatus, QString)), 
    this, SLOT(handleStatusChange(XQTelephony::LineStatus, QString)));
    telephony->call("+3581234567");
    \endcode
*/    

/*!
    Constructs a XQTelephony object with the given parent.
    Call error() to get a value of XQTelephony::Error that indicates if error occurred during construction.
    \sa call(), error()
*/
XQTelephony::XQTelephony(QObject *parent) : QObject(parent), d(new XQTelephonyPrivate(this))
{
}

/*!
    Destroys the XQTelephony object.
*/
XQTelephony::~XQTelephony()
{
    delete d;
}

/*!
    \enum XQTelephony::Error
    This enum defines the possible errors for a XQTelephony object.
*/
/*! \var XQTelephony::Error XQTelephony::NoError
    No error occured.
*/
/*! \var XQTelephony::Error XQTelephony::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQTelephony::Error XQTelephony::UnknownError
    Unknown error.
*/

/*!
    \enum XQTelephony::LineStatus
    This enum defines the possible statuses of the line
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusUnknown
    Status is unknown.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusIdle
    Status is idle. No active calls.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusDialling
    Call dialling status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusRinging
    Call ringing status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusAnswering
    Call answering status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusConnecting
    Call connecting status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusConnected
    Call connected status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusReconnectPending
    Call is undergoing temporary channel loss and it may or may not be  reconnected.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusDisconnecting
    Call disconnecting status.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusHold
    Call status hold.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusTransferring
    Call is transferring.
*/
/*! \var XQTelephony::LineStatus XQTelephony::StatusTransferAlerting
    Call in transfer is alerting the remote party.
*/

/*!
    Initiates new call.
    Note! If error happends during the call establishing the error() signal is emitted
    
    \param phoneNumber Telephone number to call
    \sa error()
*/
void XQTelephony::call(const QString& phoneNumber)
{
    d->call(phoneNumber);
}

/*!
    Starts monitoring changes in the line. 
    XQTelephony::lineStatusChanged(XQTelephony::LineStatus status, QString number) is emitted
    when changes on the line (i.e. incoming call etc.).

    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQTelephony::Error that indicates which error occurred
    \sa error()
*/
bool XQTelephony::startMonitoringLine()
{
    return d->startMonitoringLine();
}

/*!
    Stops monitoring changes in the line.
*/
void XQTelephony::stopMonitoringLine()
{
    d->stopMonitoringLine();
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError.
    \return Error code
*/
XQTelephony::Error XQTelephony::error() const
{
    return d->error();
}

/*!
    \fn void XQTelephony::lineStatusChanged(XQTelephony::LineStatus status, QString number)
    
    This signal is emitted when line status is changed.

    \param status Current status is retrived as enumeration
    \param number Number if available
    \sa call()
*/

/*!
    \fn void XQTelephony::error(XQTelephony::Error error)

    This signal is emitted when error happends during the asynchronous function.
    
    \param error Error
*/

// End of file


