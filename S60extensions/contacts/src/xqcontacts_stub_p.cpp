// INCLUDE FILES
#include "xqcontacts.h"
#include "xqcontacts_s60_p.h"

XQContactsPrivate::XQContactsPrivate()
{
}

XQContactsPrivate::~XQContactsPrivate()
{
}

void XQContactsPrivate::count() const
{
}

XQContact XQContactsPrivate::contactById(long int contactId) const
{
}

QList<long int> XQContactsPrivate::findContactIds(QString value, XQContactField::ContactFieldType type) const
{
}

QList<XQContact> XQContactsPrivate::findContacts(QString value, XQContactField::ContactFieldType type) const
{
}

QString XQContactsPrivate::contactNameById(long int contactId) const
{
}

long int XQContactsPrivate::addContact(const XQContact & contact)
{
}

bool XQContactsPrivate::updateContact(const XQContact & contact)
{
}

bool XQContactsPrivate::removeContact(long int contactId)
{
}

int importvCard(const QString& fileName)
{
}

bool exportAsvCard(const QString& fileName, long int contactId) const
{
}

// End of file

