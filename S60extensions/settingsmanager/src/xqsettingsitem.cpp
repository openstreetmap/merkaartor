#include "xqsettingsitem.h"

/****************************************************
 *
 * XQSettingsItem
 *
 ****************************************************/

/*!
    \class XQSettingsItem

    \brief The XQSettingsItem is a base class for XQRepositoryItem and XQPropertyItem
*/

/*!
    Constructs a XQSettingsItem object.
*/
XQSettingsItem::XQSettingsItem(long int category, unsigned long int key,
        XQSettingsItem::Type type, const QVariant& value) :
        iCategory(category), iKey(key), iType(type), iValue(value)
{   
}

/*!
    \enum XQSettingsItem::Type

    This enum defines the possible item types for a XQSettingsItem object.
*/
/*! \var XQSettingsItem::TypeRepository XQSettingsItem::TypeRepository
    The corresponding item type to symbian's central repository
*/
/*! \var XQSettingsItem::TypeProperty XQSettingsItem::TypeProperty
    The corresponding item type to symbian's publish and subscribe
*/

/*!
    Constructs a XQSettingsItem object.
*/
XQSettingsItem::XQSettingsItem() : iCategory(0), iKey(0), iValue(0)
{   
}

/*!
    Destroys the item.
*/
XQSettingsItem::~XQSettingsItem()
{
}
/*!
    Gets a category id of the item

    \return Category id
*/
long int XQSettingsItem::category() const
{
    return iCategory;
}

/*!
    Gets a key of the item

    \return Key
*/
unsigned long int XQSettingsItem::key() const
{
    return iKey;
}

/*!
    Gets a value of the key

    \return value as QVariant
*/
QVariant XQSettingsItem::value() const
{
    return iValue;
}

/*!
    Gets a type of the item.

    \return type
*/
XQSettingsItem::Type XQSettingsItem::type() const
{
    return iType;
}

/*!
    Returns true if the item is empty; otherwise false.

    \return true if the item is empty; otherwise false.
*/
bool XQSettingsItem::isNull() const
{
    return iValue.isNull();
}

/*!
    Sets the category of the item.

    \param category the category of the item
    \sa category()
*/
void XQSettingsItem::setCategory(long int category)
{
    iCategory = category;
}

/*!
    Sets the key of the item.

    \param key the key of the item
    \sa category()
*/
void XQSettingsItem::setKey(unsigned long int key)
{
    iKey = key;
}

/*!
    Sets the value of the item.

    \param value the value of the item
    \sa value()
*/
void XQSettingsItem::setValue(const QVariant& value)
{
    iValue = value;
}

/*!
    Sets the type of the item.

    \param type the type of the item
    \sa value()
*/
void XQSettingsItem::setType(XQSettingsItem::Type type)
{
    iType = type;
}

/****************************************************
 *
 * XQRepositoryItem
 *
 ****************************************************/

/*!
    \class XQRepositoryItem

    \brief The XQRepositoryItem is used as a data class to hold information about the item.
*/

/*!
    Constructs a XQRepositoryItem object.
*/
XQRepositoryItem::XQRepositoryItem(long int category, long int key,
        const QVariant& value) : XQSettingsItem(category, key,
        XQSettingsItem::TypeRepository, value)
{   
}

/****************************************************
 *
 * XQPropertyItem
 *
 ****************************************************/

/*!
    \class XQPropertyItem

    \brief The XQPropertyItem is a base class for XQRepositoryItem and XQPropertyItem
*/

/*!
     \brief The XQRepositoryItem is used  as a data class to hold information about the item.
*/
XQPropertyItem::XQPropertyItem(long int category, long int key,
        const QVariant& value) : XQSettingsItem(category, key,
        XQSettingsItem::TypeProperty, value) 
{   
}
// End of file


