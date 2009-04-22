// INCLUDE FILES
#include "xqcontacts.h"
#include "xqcontact.h"
#ifdef Q_OS_SYMBIAN
#include "xqcontacts_s60_p.h"
#else
#include "xqcontacts_stub_p.h"
#endif

/****************************************************
 *
 * XQContacts
 *
 ****************************************************/

/*!
    \class XQContacts

    \brief The XQContacts class can be used for managing contacts.
    
    Example:
    \code
    XQContacts* contacts = new XQContacts(this);
    QList<long int> contactIds = contacts->contactIds();
    QListWidget *contactList = new QListWidget(this);

    // Lists all phone numbers found in the contacts
    for ( int i = 0; i < contactIds.count(); i++ ) {
        XQContact contact = contacts->contactById(contactIds[i]);
        XQContactField field = contact.fieldByType(XQContactField::ContactFieldTypePhoneNumber);
        contactList->addItem(field.value().toString());
    }
    \endcode
*/

/*!
    Constructs a XQContacts object with the given parent.
*/
XQContacts::XQContacts(QObject * parent)
    : QObject(parent),
      d(new XQContactsPrivate)
{
}

/*!
    Destroys the XQContacts object.
*/
XQContacts::~XQContacts()
{
    delete d;
}

/*!
    \enum XQContacts::Error

    This enum defines the possible errors for a XQContacts object.
*/
/*! \var XQContacts::Error XQContacts::NoError
    No error occured.
*/
/*! \var XQContacts::Error XQContacts::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQContacts::Error XQContacts::InternalError
    Internal error.
*/
/*! \var XQContacts::Error XQContacts::UnknownError
    Unknown error.
*/

/*!
    Returns error from latest operation.

    \return error
*/
XQContacts::Error XQContacts::error() const
{
   return d->error;
}

/*!
    Returns the number of contacts.

    \return the number of contacts
*/
int XQContacts::count() const
{
    return d->count();
}

/*!
    Returns list containing all contact ids.

    \return list containing all contact ids
*/
QList<long int> XQContacts::contactIds() const
{
    return d->iContactIds;
}

/*!
    Returns list of contact ids which match to given contact search criteria.

    \param value field value which should match
    \param type type of the field from which /a value will be searched
    \return list of contact ids which match to search criteria
    \sa count()
*/
QList<long int> XQContacts::findContactIds(QString value, XQContactField::ContactFieldType type) const
{
    return d->findContactIds(value, type);
}

/*!
    Returns list of contacts which match to given search criteria.

    \param value field value which should match
    \param type type of the field from which /a value will be searched
    \return list of contacts which match to search criteria
    \sa count()
*/
QList<XQContact> XQContacts::findContacts(QString value, XQContactField::ContactFieldType type) const
{
    return d->findContacts(value, type);
}

/*!
    Returns the contact whose id is equal to given \a contactId.

    \param contactId id of the contact to be returned
    \return the contact on success; null contact on failure
    \sa count()
*/
XQContact XQContacts::contactById(long int contactId) const
{
    return d->contactById(contactId);
}

/*!
    Returns the name of the contact whose id is equal to given
    \a contactId.

    \param contactId id of the contact to be returned
    \return the contact name on success; null QString on failure
    \sa count()
*/
QString XQContacts::contactNameById(long int contactId) const
{
    return d->contactNameById(contactId);
}

/*!
    Adds new contact,

    \param contact contact to be added
    \return id of the added contact on success; otherwise returns 0.
    \sa removeContact(), updateContact(), count()
*/
long int XQContacts::addContact(const XQContact & contact)
{
    return d->addContact(contact);
}

/*!
    Updates the contact at specified /a index position.

    \param contact contact to be updated
    \return true if new contact was successfully added; otherwise returns false.
    \sa removeContact(), updateContact(), count()
*/
bool XQContacts::updateContact(const XQContact & contact)
{
    return d->updateContact(contact);
}

/*!
    Updates only given fields to the contact.

    \param contactId the id of the contact to be updated
    \param fields contact fields to be updated
    \return true if contact fields were successfully updated; otherwise returns false.
    \sa removeContact(), updateContact(), count()
*/
bool XQContacts::updateContactFields(long int contactId, const QList<XQContactField>& fields)
{
    return d->updateContactFields(contactId, fields);
}

/*!
    Removes the specified contact.

    \param contactId id of the contact to be removed
    \return true if new contact was successfully removed; otherwise returns false.
    \sa removeContact(), updateContact(), count()
*/
bool XQContacts::removeContact(long int contactId)
{
    return d->removeContact(contactId);
}

/*!
    Tries to import contact from specified file.

    \param fileName name of the file from which vCards (contacts) are tried to import
    \return count of imported vCards (contacts)
    \sa exportAsvCard()
*/
int XQContacts::importvCard(const QString& fileName)
{
    return d->importvCard(fileName);
}

/*!
    Exports to specified contact to specified file.

    \param fileName name of the file to which vCard (contact) is exported
    \param contactId id of the contact to be exported
    \return true if contact was successfully exported; otherwise returns false.
    \sa importvCard()
*/
bool XQContacts::exportAsvCard(const QString& fileName, long int contactId) const
{
    return d->exportAsvCard(fileName,contactId);
}

// End of file

