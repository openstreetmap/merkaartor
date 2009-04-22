#include "xqinstaller.h"
#include "xqinstaller_p.h"

/*!
    \class XQInstaller
    \brief The XQInstaller class is used to install sis-packages silently. 
    This extension can be used for example to install back-end applications.

    Example:
    \code
    XQInstaller *installer = new XQInstaller(this);
    QMap<QString, uint> applications = installer->applications();
    QListWidget *applicationList = new QListWidget(this);
    QList<QString> appNames = applications.keys();
    for (int i = 0; i < appNames.count(); i++) {
        applicationList->addItem(appNames.at(i));
    }   
    \endcode
*/

/*!
    Constructs a XQInstaller object with the given parent.
    \sa install(), remove()
*/
XQInstaller::XQInstaller(QObject *parent)
    : QObject(parent), d(new XQInstallerPrivate(this))
{
}

/*!
    Destroys the XQInstaller object.
*/
XQInstaller::~XQInstaller()
{
    delete d;
}

/*!
    Installs a sis package silently given as parameter.

    \param file Sis package
    \param drive Drive letter where the sis is installed to. Default value is 'C'.
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQInstaller::Error that indicates which error occurred
    \sa error()
*/  
bool XQInstaller::install(const QString& file, XQInstaller::Drive drive)
{
    return d->install(file, drive);
}

/*!
    Get list of installed applications
    If an empty QMap is returned, an error has possibly occurred. Call error() to get a value of 
    XQInstaller::Error that indicates which error occurred if any
    
    \return List of installed applications
    \sa error()
*/
QMap<QString, uint> XQInstaller::applications() const
{
    return d->applications();
}

/*!
    Removes application specified by the uid
   
    \param uid of the application
    \return True if removing was successfully started, otherwise false
    \sa error()
*/
bool XQInstaller::remove(uint uid)
{
    return d->remove(uid);
}

/*!
    \enum XQInstaller::Error

    This enum defines the possible errors for a XQInstaller object.
*/
/*! \var XQInstaller::Error XQInstaller::NoError
    No error occured.
*/
/*! \var XQInstaller::Error XQInstaller::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQInstaller::Error XQInstaller::AlreadyInUseError
    Installer is already in used.
*/
/*! \var XQInstaller::Error XQInstaller::UserCancelError
    Installer cancelled by the user.
*/
/*! \var XQInstaller::Error XQInstaller::PackageNotSupportedError
    Package not supported
*/
/*! \var XQInstaller::Error XQInstaller::SecurityFailureError
    Security failure
*/
/*! \var XQInstaller::Error XQInstaller::MissingDependencyError
    Missing dependency
*/
/*! \var XQInstaller::Error XQInstaller::NoRightsError
    No rights
*/
/*! \var XQInstaller::Error XQInstaller::BusyError
    Installer is busy
*/
/*! \var XQInstaller::Error XQInstaller::AccessDeniedError
    Access denied
*/
/*! \var XQInstaller::Error XQInstaller::UpgradeError
    Error while upgrading
*/
/*! \var XQInstaller::Error XQInstaller::UnknownError
    Unknown error.
*/

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError
    \return Error code
*/
XQInstaller::Error XQInstaller::error() const
{
    return d->error();
}

/*!
    \fn void XQInstaller::applicationInstalled()

    This signal is emitted when the application has been installed.

    \sa install()
*/

/*!
    \fn void XQInstaller::error(XQInstaller::Error)

    This signal is emitted if error occured during the asynchronous operation

    \sa install()
*/

/*!
    \fn void XQInstaller::applicationRemoved()

    This signal is emitted when the application has been removed.

    \sa remove()
*/

// End of file
