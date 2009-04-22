#include "xqresourceaccess.h"
#include "xqresourceaccess_p.h"

/*!
    \class XQResourceAccess

    \brief The XQResourceAccess class provides methods to load strings of the specific resource file by resource id.

    Example:
    \code
    XQResourceAccess *resourceAccess = new XQResourceAccess(this);
    resourceAccess->openResourceFile("Z:\\resource\\avkon.r01");
    QString monday = resourceAccess->loadStringFromResourceFile(R_QTN_WEEK_LONG_MONDAY);
    \endcode
*/

/*!
    Constructs a XQResourceAccess object with the given parent.
    \sa loadStringFromResourceFile(), openResourceFile()
*/
XQResourceAccess::XQResourceAccess(QObject *parent):
    QObject(parent), d(new XQResourceAccessPrivate(this))
{
}

/*!
    Destroys the XQResourceAccess object.
*/
XQResourceAccess::~XQResourceAccess()
{
    delete d;
}

/*!
    \enum XQResourceAccess::Error

    This enum defines the possible errors for a XQResourceAccess object.
*/
/*! \var XQResourceAccess::Error XQResourceAccess::NoError
    No error occured.
*/
/*! \var XQResourceAccess::Error XQResourceAccess::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQResourceAccess::Error XQResourceAccess::FileNotFoundError
    File can't be found.
*/
/*! \var XQResourceAccess::Error XQResourceAccess::ResourceNotFoundError
    Resource can't be found.
*/
/*! \var XQResourceAccess::Error XQResourceAccess::ResourceFileOpenError
    Resource file is already opened
*/
/*! \var XQResourceAccess::Error XQResourceAccess::UnknownError
    Unknown error.
*/

/*!
    Reads a resource string specified by resourceId.
    If null string is returned, an error has possibly occurred. Call error() to get a value of 
    XQResourceAccess::Error that indicates which error occurred if any.

    \return  The resource string is returned. If a resourceId is invalid a null QString is returned.
    \param resourceId  The numeric ID of the resource string to be read.
    \sa openResourceFile(), error()
*/
QString XQResourceAccess::loadStringFromResourceFile(int resourceId) const
{
    return d->loadStringFromResourceFile(resourceId);
}

/*!
    This function is used to handle resource file initialization.
    Note that you can open only one file per instance. If you try to open multiple resources
    you will be faced with ResourceFileOpenError.

    \param fileName Name of the resource file. Name can be an absolute path to the file or the file name when it is searched from the location c:\\resource\\apps
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQResourceAccess::Error that indicates which error occurred.
    \sa loadStringFromResourceFile(), error()
*/
bool XQResourceAccess::openResourceFile(QString fileName)
{
    return d->openResourceFile(fileName);
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError.
    \return Error code
*/
XQResourceAccess::Error XQResourceAccess::error() const
{
    return d->error();
}
