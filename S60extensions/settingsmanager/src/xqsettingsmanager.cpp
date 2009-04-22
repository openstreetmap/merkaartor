#include "xqsettingsmanager.h"
#include "xqsettingsmanager_p.h"

/*!
    \class XQSettingsManager

    \brief The XQSettingsManager class provides methods to handle settings items

    Example:
    \code
    XQRepositoryItem profileItem(KCRUidProfileEngine.iUid,KProEngActiveProfile);
    XQSettingsManager *settingsManager = new XQSettingsManager(profileItem);
    if (settingsManager->item(profileItem).value() == QVariant::Int) {
        int currentProfile = settingsManager->item(profileItem).value().toInt();
    }
    settingsManager->stopMonitoring(profileItem);
    \endcode
*/

/*!
    Constructs a XQSettingsManager object with the given parent and starts monitoring about the item defined as paramater.
*/
XQSettingsManager::XQSettingsManager(XQSettingsItem item, QObject *parent)
 : QObject(parent), d(new XQSettingsManagerPrivate(item, this))
{    
}

/*!
    Constructs a XQSettingsManager object with the given parent.
*/
XQSettingsManager::XQSettingsManager(QObject *parent) :
        QObject(parent), d(new XQSettingsManagerPrivate(this))
{
}

/*!
    Destroys the XQSettingsManager object.
*/
XQSettingsManager::~XQSettingsManager()
{
    delete d;
}

/*!
    \enum XQSettingsManager::Error

    This enum defines the possible errors for a XQSettingsManager object.
*/
/*! \var XQSettingsManager::Error XQSettingsManager::NoError
    No error occured.
*/
/*! \var XQSettingsManager::Error XQSettingsManager::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQSettingsManager::Error XQSettingsManager::NotFoundError
    Item not found error.
*/
/*! \var XQSettingsManager::Error XQSettingsManager::AlreadyExistsError
    Item already exists error.
*/
/*! \var XQSettingsManager::Error XQSettingsManager::UnknownError
    Unknown error.
*/

/*!
    Creates a new property item
    
    \return True if success, othewise false
    \param item Item to be created  
    \sa deleteItem()
*/
bool XQSettingsManager::createItem(const XQPropertyItem& item)
{
    return d->createItem(item);
}

/*!
    Deletes an item specified as parameter
    
    \return True if success, othewise false
    \param item Item to be deleted  
    \sa createItem()
*/
bool XQSettingsManager::deleteItem(const XQPropertyItem& item)
{
    return d->deleteItem(item);
}

/*!
    Sets a new value for the specified item
    
    \return  True if success, othewise false
    \param item Item to be set 
    \sa deleteItem(), createItem()
*/
bool XQSettingsManager::setItemValue(const XQPropertyItem& item)
{
    return d->setItemValue(item);  
}

/*!
    Gets an item specified as parameter
    
    \return Item which contains also value of the key
    \param item Item which value is wanted to know 
    \sa createItem(), setItemValue(), deleteItem()
*/
XQSettingsItem XQSettingsManager::item(const XQSettingsItem& item) const
{
    return d->item(item);
}

/*!
    Starts the monitoring changes in the item specified as parameter
    \param item  Item to be monitored  
    \sa stopMonitoring()
    \return Error code
*/
bool XQSettingsManager::startMonitoring(const XQSettingsItem& item)
{
    return d->startMonitoring(item);
}

/*!
    Stopts the monitoring changes in the item specified as parameter
    \param item  Item to be monitored  
    \sa startMonitoring()
    \return  True if success, othewise false
*/
bool XQSettingsManager::stopMonitoring(const XQSettingsItem& item)
{
    return d->stopMonitoring(item);
}

/*!
    Returns current error.
    \return  True if success, othewise false
*/
XQSettingsManager::Error XQSettingsManager::error() const
{
    return d->error();
}

/*!
    \fn  void XQSettingsManager::valueChanged(const XQSettingsItem& item)

    This signal is emitted when changes happens in item which is being monitored
    
    \param item Item which has changed
    \sa startMonitoring()
*/

/*!
    \fn void XQSettingsManager::itemDeleted(const XQSettingsItem& item)

    This signal is emitted when item is deleted

    \param item Item which has deleted
    \sa startMonitoring(), deleteItem()
*/
