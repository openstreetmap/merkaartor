#ifndef XQCONTACTS_STUB_P_H_
#define XQCONTACTS_STUB_P_H_

// INCLUDES
#include <private/qobject_p.h>
#include <QString>
#include "xqcontacts.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION
class XQContactsPrivate : public QObjectPrivate, CBase
{
    Q_DECLARE_PUBLIC(XQContacts)

public:
    XQContactsPrivate();
    ~XQContactsPrivate();
    
    int count() const;
    XQContact contactById(long int contactId) const;
    QList<long int> findContactIds(QString value, XQContactField::ContactFieldType type) const;
    QList<XQContact> findContacts(QString value, XQContactField::ContactFieldType type) const;
    QString contactNameById(long int contactId) const;
    long int addContact(const XQContact & contact);
    bool updateContact(const XQContact & contact);
    bool removeContact(long int contactId);
    int importvCard(const QString& fileName);
    bool exportAsvCard(const QString& fileName, long int contactId) const;

public: // Data
    XQContacts::Error error;	

private: // Data
};

#endif // XQCONTACTS_STUB_P_H_

// End of file

