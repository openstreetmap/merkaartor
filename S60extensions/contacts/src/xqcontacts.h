#ifndef XQCONTACTS_H
#define XQCONTACTS_H

// INCLUDES
#include "xqcontact.h"
#include <QObject>
#include <QString>
#include <QList>

// FORWARD DECLARATIONS
class XQContactsPrivate;

// CLASS DECLARATION
class XQContacts : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        InternalError,
        UnknownError = -1
    };    
    
    XQContacts(QObject *parent = 0);
    ~XQContacts();
    
    XQContacts::Error error() const;
    int count() const;
    QList<long int> contactIds() const;
    QList<long int> findContactIds(QString value, XQContactField::ContactFieldType type) const;
    QList<XQContact> findContacts(QString value, XQContactField::ContactFieldType type) const;
    XQContact contactById(long int contactId) const;
    QString contactNameById(long int contactId) const;
    long int addContact(const XQContact& contact);
    bool updateContact(const XQContact& contact);
    bool updateContactFields(long int contactId, const QList<XQContactField>& fields);
    bool removeContact(long int contactId);
    int importvCard(const QString& fileName);
    bool exportAsvCard(const QString& fileName, long int contactId) const;
    
private: // Data
    XQContactsPrivate *d;	
};

#endif // XQCONTACTS_H

// End of file

