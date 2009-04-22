#ifndef XQCONTACTS_S60_P_H_
#define XQCONTACTS_S60_P_H_

// INCLUDES
#include "xqcontacts.h"
#include "xqcontact_p.h"
#include <private/qobject_p.h>
#include <QString>
#include "cntdef.h"

// FORWARD DECLARATIONS
class CContactDatabase;
class CContactIdArray;
class CContactTextDef;
class CContactItemField;
class CContactItemFieldSet;

// CLASS DECLARATION
class XQContactsPrivate : public CBase
{
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
    bool updateContactFields(long int contactId, const QList<XQContactField>& fields);
    bool removeContact(long int contactId);
    int importvCard(const QString& fileName);
    bool exportAsvCard(const QString& fileName, long int contactId) const;
    
private:
    TBool GetS60PlatformVersion(TUint& aMajor, TUint& aMinor) const;
    HBufC8* ReadPicture(const TDesC& path) const;
	void getContactIdsFromContactDBL() const;
	XQContactField::ContactFieldType qtFieldTypeFromVCardFieldType(TFieldType type) const;
    XQContactFieldPrivate::ContactFieldProperty qtFieldPropertyFromVCardFieldType(TFieldType type) const;
	TFieldType vCardFieldTypeFromQtFieldType(XQContactField::ContactFieldType type) const;
	TFieldType vCardFieldTypeFromQtFieldProperty(XQContactFieldPrivate::ContactFieldProperty property) const;
	TUid vCardMappingFromQtFieldType(XQContactField::ContactFieldType type) const;
	TDateTime fromQtDateTimeToS60DateTime(QDateTime dateTime) const;
	QDateTime fromS60DateTimeToQtDateTime(TDateTime dateTime) const;
	TBool createPictureFields(CContactItemField*& apPathField, CContactItemField*& apPictureField, const QList<XQContactField>& contactFields);
	CContactItemField* fromQtFieldToS60FieldL(const XQContactField& contactField);
	TInt fieldIndex(const CContactItemFieldSet& fieldSet, const XQContactField& contactField) const;
	static TInt WordParser(TAny *aParam);
	
public: // Data
    mutable XQContacts::Error error;
    
    mutable QList<long int> iContactIds;

private: // Data
	CContactDatabase*       ipContactDatabase;
	CContactTextDef*        ipTextDef;
	static XQContact      shared_null_contact;
	
	mutable TUint           iPlatformVersionMajor;
	mutable TUint           iPlatformVersionMinor;
};

#endif // XQCONTACTS_S60_P_H_

// End of file

