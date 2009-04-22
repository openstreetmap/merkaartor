#include "xqutils.h"
#include "xqutils_p.h"

/*!
    \class XQUtils
    \brief The XQUtils is an utility class. The class constains some
    convenience functions e.g. for keeping the background lights on.

    Example:
    \code
    XQUtils *utils = new XQUtils(this);

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), utils, SLOT(resetInactivityTime()));
    timer->start(1000);
    \endcode
*/

/*!
    Constructs a XQUtils object with the given parent.
*/
XQUtils::XQUtils(QObject *parent)
 : QObject(parent), d(new XQUtilsPrivate(this))
{
}

/*!
    Destroys the XQUtils object.
*/
XQUtils::~XQUtils()
{
    delete d;
}

/*!
    \enum XQUtils::Error

    This enum defines the possible errors for a XQUtils object.
*/
/*! \var XQUtils::Error XQUtils::NoError
    No error occured.
*/
/*! \var XQUtils::Error XQUtils::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQUtils::Error XQUtils::UserCancelledError
    User cancelled an operation.
*/
/*! \var XQUtils::Error XQUtils::UnknownError
    Unknown error.
*/

/*!
    Try to launch file in a sufficient application based on the file type. Note! This function will be moved under Launcer API.
    
    \param filename Path to the file
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQUtils::Error that indicates which error occurred
    \sa error()
*/
bool XQUtils::launchFile(const QString& filename)
{
    return d->launchFile(filename);
}

/*!
    Resets system inactivity timer. Calling this function regularly keeps e.g.
    background lights on.
*/
void XQUtils::resetInactivityTime()
{
    d->resetInactivityTime();
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError.
    
    \return Error code
*/
XQUtils::Error XQUtils::error() const
{
    return d->error();
}

// End of file
