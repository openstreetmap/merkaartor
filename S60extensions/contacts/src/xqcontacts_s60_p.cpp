// INCLUDE FILES
#include "xqcontacts_s60_p.h"
#include "xqcontacts.h"
#include "xqcontact.h"
#include "xqcontact_p.h"

#include <cntdb.h>
#include <cntfldst.h>
#include <cntfield.h>
#include <cntfilt.h>
#include <cntitem.h>
#include <badesca.h>
#include <f32file.h>
#include <s32strm.h> 
#include <exifread.h> 
//#include <cntdef.h>

#include <QByteArray>
#include <QDateTime>
#include <QImage>
#include <QBuffer>
#include <QFile>

#define KUidContactFieldPicturePathValue 0x101f8841
const TUid KUidContactFieldPicturePath={KUidContactFieldPicturePathValue};
const int thumbnailSize = 100;

XQContactsPrivate::XQContactsPrivate()
{
    // Open the connection to the default contact database
    ipContactDatabase = CContactDatabase::OpenL();
    
	//create custom text definition to display contact item
	ipTextDef = CContactTextDef::NewL();
	//add given name field and family name field
	ipTextDef->AppendL(TContactTextDefItem(KUidContactFieldFamilyName, _L(" ")));
	ipTextDef->AppendL(TContactTextDefItem(KUidContactFieldGivenName));
	getContactIdsFromContactDBL();
}

XQContactsPrivate::~XQContactsPrivate()
{
    delete ipContactDatabase;
    delete ipTextDef;
}

int XQContactsPrivate::count() const
{
    return ipContactDatabase->CountL();
}

XQContact XQContactsPrivate::contactById(long int contactId) const
{
    CContactItem* pContact = NULL;
    TRAPD(err, pContact = ipContactDatabase->ReadContactL(contactId));
    if (err) {
        error = XQContacts::InternalError;
        return XQContact();
    }

    XQContact contact;
    contact.contactId = contactId;
    bool picturePathFound = false;

    XQContactField::ContactFieldType qtFieldType = XQContactField::ContactFieldTypeUnknown;    
    XQContactFieldPrivate::ContactFieldProperty qtFieldProperty = XQContactFieldPrivate::ContactFieldPropertyUnknown;
    CContactItemFieldSet& fields = pContact->CardFields();
    const TInt fieldCount = fields.Count();
    for (TInt i = 0; i < fieldCount; i++) {
	    CContactItemField &field = fields[i];
	    if (field.StorageType() == KStorageTypeText && field.TextStorage()->IsFull()) {
	        XQContactField newField;
	        newField.setId(field.Id());
		    TPtrC fieldText = field.TextStorage()->Text();
		    newField.setValue(QString::fromUtf16(fieldText.Ptr(),fieldText.Length()));
		    TPtrC labelText = field.Label();
		    newField.setLabel(QString::fromUtf16(labelText.Ptr(),labelText.Length()));
		    int typeCount = field.ContentType().FieldTypeCount();
		    for (int j=0; j < typeCount; j++) {
		        qtFieldType = qtFieldTypeFromVCardFieldType(field.ContentType().FieldType(j));
		        if (qtFieldType == XQContactField::ContactFieldTypeUnknown) {
		            qtFieldProperty = qtFieldPropertyFromVCardFieldType(field.ContentType().FieldType(j));
		            if (qtFieldProperty != XQContactFieldPrivate::ContactFieldPropertyUnknown) {
		                newField.d->properties.append(qtFieldProperty);
		            }
		        } else {
	                newField.setType(qtFieldType);
		        }
		    }
            /*if (qtFieldType == XQContactField::ContactFieldTypePicture) {
                picturePathFound = true;
                XQContactFieldPicture pictureField = contact.pictureField();
                if (!pictureField.isNull()) {
                    contact.removeFieldByType(XQContactField::ContactFieldTypePicture);
                }
            }*/
	        contact.addField(newField);
		} else if (field.StorageType() == KStorageTypeStore) {
            XQContactField newField;
            newField.setId(field.Id());
		    TPtrC labelText = field.Label();
		    newField.setLabel(QString::fromUtf16(labelText.Ptr(),labelText.Length()));
            int typeCount = field.ContentType().FieldTypeCount();
	        for (int j=0; j < typeCount; j++ ) {
	            qtFieldType = qtFieldTypeFromVCardFieldType(field.ContentType().FieldType(j));
	            if (qtFieldType == XQContactField::ContactFieldTypeUnknown) {
                    qtFieldProperty = qtFieldPropertyFromVCardFieldType(field.ContentType().FieldType(j));
                    if (qtFieldProperty != XQContactFieldPrivate::ContactFieldPropertyUnknown) {
                        newField.d->properties.append(qtFieldProperty);
                    }
                } else {
                    newField.setType(qtFieldType);
	            }
	        }
	        //if (!picturePathFound || qtFieldType != XQContactField::ContactFieldTypePicture) { 
	            HBufC8* pData = field.StoreStorage()->Thing();
	            newField.setValue(QByteArray((const char *)pData->Ptr(),pData->Length()));
	            contact.addField(newField);
	        //}
	    } else if (field.StorageType() == KStorageTypeDateTime) {
            XQContactField newField;
            newField.setId(field.Id());
            TPtrC labelText = field.Label();
            newField.setLabel(QString::fromUtf16(labelText.Ptr(),labelText.Length()));
            int typeCount = field.ContentType().FieldTypeCount();
            for (int j=0; j < typeCount; j++ ) {
                qtFieldType = qtFieldTypeFromVCardFieldType(field.ContentType().FieldType(j));
                if (qtFieldType != XQContactField::ContactFieldTypeUnknown) {
                    break;
                }
            }
            newField.setType(qtFieldType);
            TTime time = field.DateTimeStorage()->Time();
            newField.setValue(fromS60DateTimeToQtDateTime(time.DateTime()));
            contact.addField(newField);
        }
	    /*else if (field.StorageType() == KStorageTypeContactItemId) {
	    }*/
	}
    delete pContact;

    return contact;
}

QList<long int> XQContactsPrivate::findContactIds(QString value, XQContactField::ContactFieldType type) const
{
    // Callback function for
    TCallBack wordParserCallBack(XQContactsPrivate::WordParser);		
	
    CContactTextDef* pContactTextDef = CContactTextDef::NewLC();
	
    // determine which contact fields are to search
    TContactTextDefItem fieldToBeSearched(vCardFieldTypeFromQtFieldType(type));
    pContactTextDef->AppendL(fieldToBeSearched);
	
    CDesCArrayFlat* pFoundWordsArray = new(ELeave) CDesCArrayFlat(10);
    CleanupStack::PushL(pFoundWordsArray);
	
    TPtrC16 valueToSearch(reinterpret_cast<const TUint16*>(value.utf16()));
    // Tokenize saerch string
    TLex input(valueToSearch);
    TBuf<64> token;
    do {
        token = input.NextToken();
	    if (token.Length()) {	
	        pFoundWordsArray->AppendL(token);
	    }
    } while(token.Length());

    CContactIdArray *pContactIds = ipContactDatabase->FindInTextDefLC(*pFoundWordsArray, pContactTextDef, wordParserCallBack);
    CleanupStack::Pop(pContactIds);
    CleanupStack::PopAndDestroy(pFoundWordsArray);
    CleanupStack::PopAndDestroy(pContactTextDef);
 
    QList<long int> contactIds;
    for (int i = 0; i < pContactIds->Count(); i++ ) {
        contactIds.append((*pContactIds)[i]);
    }
 
    delete pContactIds;

    return contactIds;
}

QList<XQContact> XQContactsPrivate::findContacts(QString value, XQContactField::ContactFieldType type) const
{
    // Callback function for
    TCallBack wordParserCallBack(XQContactsPrivate::WordParser);		
	
    CContactTextDef* pContactTextDef = CContactTextDef::NewLC();
	
    // determine which contact fields are to search
    TContactTextDefItem fieldToBeSearched(vCardFieldTypeFromQtFieldType(type));
    pContactTextDef->AppendL(fieldToBeSearched);
	
    CDesCArrayFlat* pFoundWordsArray = new(ELeave) CDesCArrayFlat(10);
    CleanupStack::PushL(pFoundWordsArray);
	
    TPtrC16 valueToSearch(reinterpret_cast<const TUint16*>(value.utf16()));
    // Tokenize saerch string
    TLex input(valueToSearch);
    TBuf<64> token;
    do {
        token = input.NextToken();
	    if (token.Length()) {	
	        pFoundWordsArray->AppendL(token);
	    }
    } while(token.Length());

    CContactIdArray *pContactIds = ipContactDatabase->FindInTextDefLC(*pFoundWordsArray, pContactTextDef, wordParserCallBack);
    CleanupStack::Pop(pContactIds);
    CleanupStack::PopAndDestroy(pFoundWordsArray);
    CleanupStack::PopAndDestroy(pContactTextDef);
 
    QList<XQContact> contactList;
    XQContact contact;
    for (int i = 0; i < pContactIds->Count(); i++ ) {
        contact = contactById((*pContactIds)[i]);
        contactList.append(contact);
    }

    delete pContactIds;

    return contactList;
}


QString XQContactsPrivate::contactNameById(long int contactId) const
{
    TBuf<256> buf;
    ipContactDatabase->ReadContactTextDefL(contactId, buf, ipTextDef);
    return QString::fromUtf16(buf.Ptr(),buf.Length());
}

TBool XQContactsPrivate::createPictureFields(CContactItemField*& apPathField, CContactItemField*& apPictureField, const QList<XQContactField>& contactFields)
{
    CContactItemField* pTempPathField = NULL;
    CContactItemField* pTempPictureField = NULL;

    QString picturePath;
    QByteArray picture;

    // Search picture path and/or picture binary data fields
    for (int i = 0; i < contactFields.count(); i++) {
        if (contactFields[i].type() == XQContactField::ContactFieldTypePicture) {
            if (contactFields[i].value().type() == QVariant::String) {
                picturePath = contactFields[i].value().toString();
            } else if (contactFields[i].value().canConvert<QByteArray>()) {
                picture = contactFields[i].value().toByteArray();
            }
        }
    }
    
    if (picturePath.isNull() && picture.isNull()) {
        // Picture fields were not found
        return EFalse;
    }

    TPtrC16 value(KNullDesC);
    TUint major = 3;
    TUint minor = 1;
    GetS60PlatformVersion(major, minor);

    // 1. Add picture path field to Contact
    if ((major == 3 && minor > 1) || (major > 3)) {
        // S60 version is >= 3.2
        // Support for picture path exists
        if (!picturePath.isNull()) {
            // Add picture path field to Contact
            pTempPathField = CContactItemField::NewLC(KStorageTypeText);
            value.Set(reinterpret_cast<const TUint16*>(picturePath.utf16()));
            pTempPathField->TextStorage()->SetTextL(value);
            pTempPathField->SetMapping(KUidContactFieldPicturePath);
            pTempPathField->AddFieldTypeL(KUidContactFieldPicture);
        } else if (!picture.isNull()) {
            // Store picture to gallery & add picture path field to contact
            /*
            pTempPathField = CContactItemField::NewLC(KStorageTypeText);
            //TODO: value.Set(reinterpret_cast<const TUint16*>(contactFields[i].value().toString().utf16()));
            //TODO: pTempPathField->TextStorage()->SetTextL(value);
            pTempPathField->SetMapping(KUidContactFieldPicturePath);
            pTempPathField->AddFieldTypeL(KUidContactFieldPicture);
            */
        }
    }

    // 2. Add picture thumbnail field to Contact
    pTempPictureField = CContactItemField::NewLC(KStorageTypeStore);
    pTempPictureField->ResetStore();
    if ((major == 3 && minor > 1) || (major > 3)) {
        // S60 version is >= 3.2
        pTempPictureField->SetMapping(KUidContactFieldVCardMapLOGO);
    } else {
        // S60 version is < 3.2
        pTempPictureField->SetMapping(KUidContactFieldVCardMapPHOTO);
    }
    pTempPictureField->AddFieldTypeL(KUidContactFieldPicture);
    if (!picture.isNull()) {
        // Picture data was given
        TPtr8 tmpPicture8((unsigned char *)picture.data(),picture.size()); 
        pTempPictureField->StoreStorage()->SetThingL(tmpPicture8);
        CleanupStack::Pop(pTempPictureField);
    } else if (!picturePath.isNull()) {
        // Only path to picture was given
        // Try to read picture data using given path
        value.Set(reinterpret_cast<const TUint16*>(picturePath.utf16()));
        HBufC8* pPicture = ReadPicture(value);
        if (pPicture) {
            // Picture data was successfully read
            // => Store picture data to Contact field
            CleanupStack::PushL(pPicture);
            pTempPictureField->StoreStorage()->SetThingL(*pPicture);
            CleanupStack::PopAndDestroy(pPicture); 
            CleanupStack::Pop(pTempPictureField);
        }
        else {
            // Picture was not found
            // Picture data can not be saved to Contact field
            CleanupStack::PopAndDestroy(pTempPictureField);
            pTempPictureField = NULL;
        }
    }
    if (pTempPathField) {
        CleanupStack::Pop(pTempPathField);
    }
    apPathField = pTempPathField;
    apPictureField = pTempPictureField;
    
    return ETrue;
}

CContactItemField* XQContactsPrivate::fromQtFieldToS60FieldL(const XQContactField& contactField)
{
    XQContactField::ContactFieldType qtFieldType = contactField.type();
    if (qtFieldType == XQContactField::ContactFieldTypePicture) {
        return NULL;
    }
    CContactItemField* pField = NULL;
    TPtrC16 value(KNullDesC);
    if (qtFieldType != XQContactField::ContactFieldTypeUnknown) {
        QVariant qtValue = contactField.value();
        // 1. Create field and store data in correct format to the field
        if (qtFieldType == XQContactField::ContactFieldTypeBirthday) {
             if (qtValue.canConvert<QDateTime>()) {
                 pField = CContactItemField::NewLC(KStorageTypeDateTime);
                 pField->DateTimeStorage()->SetTime(fromQtDateTimeToS60DateTime(qtValue.toDateTime()));
             } else {
                 return NULL;
             }
        } /*else if (qtFieldType == XQContactField::ContactFieldTypeRingTone) {
             if (qtValue.canConvert<QByteArray>()) {
                 pField = CContactItemField::NewLC(KStorageTypeStore);
                 pField->ResetStore();
                 TPtr8 tmpRingTone8((unsigned char *)qtValue.toByteArray().data(),qtValue.toByteArray().size()); 
                 pField->StoreStorage()->SetThingL(tmpRingTone8);
             } else {
                 return NULL;
             }
        }*/ else if (qtFieldType == XQContactField::ContactFieldTypeAnniversary) {
            if (qtValue.canConvert<QDateTime>()) {
                pField = CContactItemField::NewLC(KStorageTypeDateTime);
                pField->DateTimeStorage()->SetTime(fromQtDateTimeToS60DateTime(qtValue.toDateTime()));
            } else {
                return NULL;
            }
        } else {
             pField = CContactItemField::NewLC(KStorageTypeText);
             value.Set(reinterpret_cast<const TUint16*>(contactField.value().toString().utf16()));
             pField->TextStorage()->SetTextL(value);
        }
        
        // 2. Set field mapping & field type
        //    and add properties to field
        if (pField) {
            pField->SetMapping(vCardMappingFromQtFieldType(qtFieldType));
            TFieldType vCardFieldType = vCardFieldTypeFromQtFieldType(qtFieldType);
            if (vCardFieldType != KUidContactFieldNone) {
                pField->AddFieldTypeL(vCardFieldType);
            }
            if (!contactField.d->properties.isEmpty()) {
                // Add properties
                XQContactFieldPrivate::ContactFieldProperty qtFieldProperty;
                QList<XQContactFieldPrivate::ContactFieldProperty>& properties = contactField.d->properties;
                for (int j=0;j < properties.count(); j++) {
                    qtFieldProperty = properties[j];
                    pField->AddFieldTypeL(vCardFieldTypeFromQtFieldProperty(qtFieldProperty));
                }
            }
        }
    }
    if (pField) {
        CleanupStack::Pop(pField);
    }
    
    return pField;
}

long int XQContactsPrivate::addContact(const XQContact & contact)
{
    if (contact.fields().count() == 0) {
        return 0;
    }
    
    QString picturePath;
    bool pictureFound = false;

    CContactItem* pCard = CContactCard::NewL();
    CleanupStack::PushL(pCard);

    CContactItemField* pField;
    TPtrC16 value(KNullDesC);
    const QList<XQContactField>& contactFields = contact.fields();
    for (int i = 0; i < contactFields.count(); i++) {
        // Note: fromQtFieldToS60FieldL skips picture fields
        pField = fromQtFieldToS60FieldL(contactFields[i]);
        if (pField) {
            CleanupStack::PushL(pField);
            pCard->AddFieldL(*pField);
            CleanupStack::Pop(pField);
        }
    }
    
    CContactItemField* picturePathField;
    CContactItemField* pictureDataField;
    if (createPictureFields(picturePathField, pictureDataField, contactFields)) {
        CleanupStack::PushL(picturePathField);
        CleanupStack::PushL(pictureDataField);
        if (picturePathField) {
            pCard->AddFieldL(*picturePathField);
        }
        if (pictureDataField) {
            pCard->AddFieldL(*pictureDataField);
        }
        CleanupStack::Pop(pictureDataField);
        CleanupStack::Pop(picturePathField);
    }
    
    long int contactId = ipContactDatabase->AddNewContactL(*pCard);
    CleanupStack::PopAndDestroy(pCard); 

    getContactIdsFromContactDBL();
    return contactId;
}

HBufC8* XQContactsPrivate::ReadPicture(const TDesC& path) const
{
    HBufC8* pPicture = NULL; 
    RFs fs;
    RFile file;
    TRAPD(error,
        // Try to read image file
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        User::LeaveIfError(file.Open(fs, path, EFileRead));
        CleanupClosePushL(file);
        TInt size = 0;
        file.Size(size);
        pPicture = HBufC8::NewLC(size);
        TPtr8 picturePtr(pPicture->Des()); 
        User::LeaveIfError(file.Read(picturePtr));
        CleanupStack::Pop(pPicture);
        CleanupStack::PopAndDestroy(&file); 
        CleanupStack::PopAndDestroy(&fs); 
    )
    if (error != KErrNone) {
        // Could not read image => return NULL
        return NULL;
    }
    
    if (pPicture) {
        TRAP(error,  
            // Try to read thumbnail from image EXIF data
            CExifRead* pExifRead = CExifRead::NewL(pPicture->Des());
            CleanupStack::PushL(pExifRead);
            HBufC8* pPictureThumbnail = pExifRead->GetThumbnailL();
            CleanupStack::PopAndDestroy(pExifRead);
            delete pPicture;
            pPicture = NULL;
            if (pPictureThumbnail != NULL) {
                pPicture = pPictureThumbnail;
            }
        )
        if (error != KErrNone || !pPicture) {
            // Could not read thumbnail from image EXIF data
            // => Create thumbnail for image
            delete pPicture;
            QImage image;
            if (image.load(QString::fromUtf16(path.Ptr(),path.Length()))) {
                QImage scaledImage = image.scaled(QSize(thumbnailSize, thumbnailSize),
                                                        Qt::KeepAspectRatio,
                                                        Qt::SmoothTransformation);
                QByteArray byteArray;
                QBuffer buffer(&byteArray);
                buffer.open(QIODevice::WriteOnly);
                scaledImage.save(&buffer,"jpg");
                TPtr8 tmpPicture8((unsigned char *)byteArray.data(),byteArray.size());
                tmpPicture8.SetLength(byteArray.size());
                pPicture = tmpPicture8.Alloc();
            }
            
        }
    }

    return pPicture;
}

TBool XQContactsPrivate::GetS60PlatformVersion(TUint& aMajor, TUint& aMinor) const
{
    if (iPlatformVersionMajor != 0) {
        aMajor = iPlatformVersionMajor;
        aMinor = iPlatformVersionMinor;
        return ETrue;
    }

    RFs fs;
    if (fs.Connect() != KErrNone) {
        return EFalse;
    }
    CleanupClosePushL(fs);
     
    // Obtain the version number
    TFindFile fileFinder = fs;
    CDir* pResult;
 
    _LIT(KS60ProductIDFile, "Series60v*.sis");
    _LIT(KROMInstallDir, "z:\\system\\install\\");
    
    if (fileFinder.FindWildByDir(KS60ProductIDFile, KROMInstallDir, pResult) != KErrNone) {
        CleanupStack::PopAndDestroy(&fs);
        return EFalse;
    }
    CleanupStack::PushL(pResult);
    
    // Sort the file names so that the newest platforms are first
    if (pResult->Sort(ESortByName | EDescending) != KErrNone) {
        CleanupStack::PopAndDestroy(pResult);
        CleanupStack::PopAndDestroy(&fs);
        return EFalse;
    }
 
    // Parse the version numbers from the file name (e.g. Series60v3_1.sis)
    aMajor = (*pResult)[0].iName[9] - '0';
    aMinor = (*pResult)[0].iName[11] - '0';
    CleanupStack::PopAndDestroy(pResult);
    CleanupStack::PopAndDestroy(&fs);
    
    iPlatformVersionMajor = aMajor;
    iPlatformVersionMinor = aMinor;
    return ETrue;
}

TInt XQContactsPrivate::fieldIndex(const CContactItemFieldSet& fieldSet,
                                     const XQContactField& contactField) const
{
    const TInt fieldCount = fieldSet.Count();
    if (contactField.id() > 0) {
        for (TInt i = 0; i < fieldCount; i++) {
            if (contactField.id() == fieldSet[i].Id()) {
                return i;
            }
        }
    }
    return KErrNotFound;
}

bool XQContactsPrivate::updateContact(const XQContact & contact)
{
    if ((contact.fields().count() == 0) || contact.isNull() || contact.id() == 0) {
        return false;
    }

    long int contactId = contact.id();
    CContactItem* pContact = NULL;
    TRAPD(err, pContact = ipContactDatabase->OpenContactL(contactId));
    if (err) {
        error = XQContacts::InternalError;
        return false;
    }

    bool picturePathOk = false;
    bool pictureOk = false;
    QList<XQContactField> contactFields = contact.fields();
    CContactItemField* picturePathField;
    CContactItemField* pictureDataField;
    createPictureFields(picturePathField, pictureDataField, contactFields);
    CleanupStack::PushL(picturePathField);
    CleanupStack::PushL(pictureDataField);
    
    CContactItemField* pField;
    TPtrC16 value(KNullDesC);
    CContactItemFieldSet& fields = pContact->CardFields();
    const TInt fieldCount = fields.Count();
    bool found;
    for (int i=0; i<fields.Count(); i++) {
        found = false;
        for (int j = 0; j < contactFields.count(); j++) {
            if (fields[i].Id() == contactFields[j].id()) {
                found = true;
                if (contactFields[j].type() == XQContactField::ContactFieldTypeBirthday || 
                    contactFields[j].type() == XQContactField::ContactFieldTypeAnniversary) {
                     if (contactFields[j].value().canConvert<QDateTime>()) {
                         fields[i].DateTimeStorage()->SetTime(fromQtDateTimeToS60DateTime(contactFields[j].value().toDateTime()));
                     }
                } /*else if (qtFieldType == XQContactField::ContactFieldTypeRingTone) {
                        if (qtValue.canConvert<QByteArray>()) {
                        pField = CContactItemField::NewLC(KStorageTypeStore);
                        pField->ResetStore();
                        TPtr8 tmpRingTone8((unsigned char *)qtValue.toByteArray().data(),qtValue.toByteArray().size()); 
                        pField->StoreStorage()->SetThingL(tmpRingTone8);
                     }
                 }
                }*/ else if (contactFields[j].type() == XQContactField::ContactFieldTypePicture) {
                    if (fields[i].StorageType() == KStorageTypeText) {
                        if (picturePathField) {
                            fields[i].TextStorage()->SetTextL(picturePathField->TextStorage()->Text());
                            picturePathOk = true;
                        }
                    } else if (fields[i].StorageType() == KStorageTypeStore) {
                        if (pictureDataField) {
                            fields[i].StoreStorage()->SetThingL(*pictureDataField->StoreStorage()->Thing());
                            pictureOk = true;
                        }
                    }
                } else {
                    value.Set(reinterpret_cast<const TUint16*>(contactFields[j].value().toString().utf16()));
                    fields[i].TextStorage()->SetTextL(value);
                }
                // Remove handled field from contactFields array
                contactFields.removeAt(j);
            }
        }
        if (!found) {
            fields.Remove(i);
            i--;
        }
    }

    for (int i = 0; i < contactFields.count(); i++) {
        // Note: fromQtFieldToS60FieldL skips picture fields
        pField = fromQtFieldToS60FieldL(contactFields[i]);
        if (pField) {
            CleanupStack::PushL(pField);
            pContact->AddFieldL(*pField);
            CleanupStack::Pop(pField);
        }
    }
    
    if (!picturePathOk && picturePathField) {
        pContact->AddFieldL(*picturePathField);
    }
    if (!pictureOk && pictureDataField) {
        pContact->AddFieldL(*pictureDataField);
    }
    CleanupStack::Pop(pictureDataField);
    CleanupStack::Pop(picturePathField);
    
    TRAP(err, ipContactDatabase->CommitContactL(*pContact));
    delete pContact;	
    
    return true;
}

bool XQContactsPrivate::updateContactFields(long int contactId, const QList<XQContactField>& fields)
{
    
}

bool XQContactsPrivate::removeContact(long int contactId)
{
    TRAPD(err, 
            ipContactDatabase->DeleteContactL(contactId);
            getContactIdsFromContactDBL();
          );
    if (err) {
        error = XQContacts::InternalError;
        return false;
    }
    return true;
}

void XQContactsPrivate::getContactIdsFromContactDBL() const
	{
	CCntFilter* pFilter = CCntFilter::NewLC();

	// Get all contact items (no groups, templates...)
	pFilter->SetContactFilterTypeALL(EFalse);
	pFilter->SetContactFilterTypeCard(ETrue);
	ipContactDatabase->FilterDatabaseL(*pFilter);

	CContactIdArray* pContactIds = CContactIdArray::NewLC(pFilter->iIds);
	iContactIds.clear();
	for ( int i = 0; i < pContactIds->Count(); i++ ) {
	    iContactIds.append((*pContactIds)[i]);	
	}

	CleanupStack::PopAndDestroy(pContactIds);
	CleanupStack::PopAndDestroy(pFilter);
	}

TInt XQContactsPrivate::importvCard(const QString& fileName)
	{
	TPtrC16 filename(reinterpret_cast<const TUint16*>(fileName.utf16()));
	
	// Create a file to read contacts
	RFs fileSession;
	User::LeaveIfError(fileSession.Connect());
	CleanupClosePushL(fileSession);

	RFile file;
	if (file.Open(fileSession, filename, EFileRead) != KErrNone) {
		CleanupStack::PopAndDestroy(&fileSession);
		return KErrGeneral;
	}
	CleanupClosePushL(file);

    RFileReadStream inputFileStream(file);
    CleanupClosePushL(inputFileStream);
	
	TBool result = EFalse;
	TUid p1;
	p1.iUid = KVersitEntityUidVCard;
	CArrayPtr<CContactItem>* imported = 
	               ipContactDatabase->ImportContactsL(p1,
			                                          inputFileStream,
                                                      result,
                                                      CContactDatabase::ETTFormat);
	imported->ResetAndDestroy();
	delete imported;

    CleanupStack::PopAndDestroy(&inputFileStream);
    CleanupStack::PopAndDestroy(&file);
    CleanupStack::PopAndDestroy(&fileSession);
    
	return result?KErrNone:KErrGeneral;
	}

bool XQContactsPrivate::exportAsvCard(const QString& fileName, long int contactId) const
	{
	TPtrC16 filename(reinterpret_cast<const TUint16*>(fileName.utf16()));

	RFs fileSession;
	User::LeaveIfError(fileSession.Connect());
	CleanupClosePushL(fileSession);

	RFile file;
	if (file.Replace(fileSession, filename, EFileWrite) != KErrNone) {
		CleanupStack::PopAndDestroy(&fileSession);
		return false;
	}
	CleanupClosePushL(file);

	//open file
    RFileWriteStream outputFileStream(file);
    CleanupClosePushL(outputFileStream);
	
	CContactIdArray* exportContacts = CContactIdArray::NewL();
	CleanupStack::PushL(exportContacts);
	exportContacts->AddL(contactId);

	TUid p1;
	p1.iUid = KVersitEntityUidVCard;
	ipContactDatabase->ExportSelectedContactsL(p1,
								               *exportContacts,
								               outputFileStream,
								               CContactDatabase::EExcludeUid);

	CleanupStack::PopAndDestroy(exportContacts);
	
    CleanupStack::PopAndDestroy(&outputFileStream);
    CleanupStack::PopAndDestroy(&file);
    CleanupStack::PopAndDestroy(&fileSession);
    
    return true;
	}

XQContactField::ContactFieldType XQContactsPrivate::qtFieldTypeFromVCardFieldType(TFieldType type) const
{
    switch (type.iUid) {
        case KUidContactFieldAddressValue:                  return XQContactField::ContactFieldTypeAddress;
        case KUidContactFieldPostOfficeValue:               return XQContactField::ContactFieldTypePostOffice;
        case KUidContactFieldExtendedAddressValue:          return XQContactField::ContactFieldTypeExtendedAddress;
        case KUidContactFieldLocalityValue:                 return XQContactField::ContactFieldTypeLocality;
        case KUidContactFieldRegionValue:                   return XQContactField::ContactFieldTypeRegion;
        case KUidContactFieldPostCodeValue:                 return XQContactField::ContactFieldTypePostCode;
        case KUidContactFieldCountryValue:                  return XQContactField::ContactFieldTypeCountry;
        case KUidContactFieldCompanyNameValue:              return XQContactField::ContactFieldTypeCompanyName;
        //case KUidContactFieldCompanyNamePronunciationValue: return XQContactField::ContactFieldTypeCompanyNamePronunciation;
        case KUidContactFieldPhoneNumberValue:              return XQContactField::ContactFieldTypePhoneNumber;
        case KUidContactFieldGivenNameValue:                return XQContactField::ContactFieldTypeGivenName;
        case KUidContactFieldFamilyNameValue:               return XQContactField::ContactFieldTypeFamilyName;
        //case KUidContactFieldGivenNamePronunciationValue:   return XQContactField::ContactFieldTypeGivenNamePronunciation;
        //case KUidContactFieldFamilyNamePronunciationValue:  return XQContactField::ContactFieldTypeFamilyNamePronunciation;
        case KUidContactFieldAdditionalNameValue:           return XQContactField::ContactFieldTypeAdditionalName;
        case KUidContactFieldSuffixNameValue:               return XQContactField::ContactFieldTypeSuffixName;
        case KUidContactFieldPrefixNameValue:               return XQContactField::ContactFieldTypePrefixName;
        //case KUidContactFieldHiddenValue:                   return XQContactField::ContactFieldTypeHidden;
        case KUidContactFieldEMailValue:                    return XQContactField::ContactFieldTypeEMail;
        //case KUidContactFieldMsgValue:                      return XQContactField::ContactFieldTypeMsg;
        //case KUidContactFieldSmsValue:                      return XQContactField::ContactFieldTypeSms;
        //case KUidContactFieldFaxValue:                      return XQContactField::ContactFieldTypeFax;
        //case KUidContactFieldDefinedTextValue:              return XQContactField::ContactFieldTypeDefinedText;
        case KUidContactFieldNoteValue:                     return XQContactField::ContactFieldTypeNote;
        case KUidContactFieldBirthdayValue:                 return XQContactField::ContactFieldTypeBirthday;
        case KUidContactFieldUrlValue:                      return XQContactField::ContactFieldTypeUrl;
        //case KUidContactFieldStorageInlineValue:            return XQContactField::ContactFieldTypeStorageInline;
        //case KUidContactFieldTemplateLabelValue:            return XQContactField::ContactFieldTypeTemplateLabel;
        case KUidContactFieldPictureValue:                  return XQContactField::ContactFieldTypePicture;
        case KUidContactFieldPicturePathValue:              return XQContactField::ContactFieldTypePicture; // TODO:
        //case KUidContactFieldRingToneValue:                 return XQContactField::ContactFieldTypeRingTone;
        //case KUidContactsVoiceDialFieldValue:               return XQContactField::ContactFieldTypeVoiceDial;
        case KUidContactFieldNoneValue:                     return XQContactField::ContactFieldTypeUnknown; //None
        case KUidContactFieldJobTitleValue:                 return XQContactField::ContactFieldTypeJobTitle;
        //case KUidContactFieldICCSlotValue:                  return XQContactField::ContactFieldTypeICCSlot;
        //case KUidContactFieldICCPhonebookValue:             return XQContactField::ContactFieldTypeICCPhonebook;
        //case KUidContactFieldICCGroupValue:                 return XQContactField::ContactFieldTypeICCGroup;
        //case KUidContactFieldIMAddressValue:                return XQContactField::ContactFieldTypeIMAddress;
        case KUidContactFieldSecondNameValue:               return XQContactField::ContactFieldTypeSecondName;
        case KUidContactFieldSIPIDValue:                    return XQContactField::ContactFieldTypeSIPID;
        case KUidContactFieldAssistantValue:                return XQContactField::ContactFieldTypeAssistant;
        case KUidContactFieldAnniversaryValue:              return XQContactField::ContactFieldTypeAnniversary;
        case KUidContactFieldSpouseValue:                   return XQContactField::ContactFieldTypeSpouse;
        case KUidContactFieldChildrenValue:                 return XQContactField::ContactFieldTypeChildren;
        //case KUidContactFieldClassValue:                    return XQContactField::ContactFieldTypeClass;
        case KUidContactFieldDepartmentNameValue:           return XQContactField::ContactFieldTypeDepartmentName;
    }
    return XQContactField::ContactFieldTypeUnknown;
}

TFieldType XQContactsPrivate::vCardFieldTypeFromQtFieldType(XQContactField::ContactFieldType type) const
{
    switch (type) {
        case XQContactField::ContactFieldTypeAddress:                  return KUidContactFieldAddress;
        case XQContactField::ContactFieldTypePostOffice:               return KUidContactFieldPostOffice;
        case XQContactField::ContactFieldTypeExtendedAddress:          return KUidContactFieldExtendedAddress;
        case XQContactField::ContactFieldTypeLocality:                 return KUidContactFieldLocality;
        case XQContactField::ContactFieldTypeRegion:                   return KUidContactFieldRegion;
        case XQContactField::ContactFieldTypePostCode:                 return KUidContactFieldPostcode;
        case XQContactField::ContactFieldTypeCountry:                  return KUidContactFieldCountry;
        case XQContactField::ContactFieldTypeCompanyName:              return KUidContactFieldCompanyName;
        //case XQContactField::ContactFieldTypeCompanyNamePronunciation: return KUidContactFieldCompanyNamePronunciation;
        case XQContactField::ContactFieldTypePhoneNumber:              return KUidContactFieldPhoneNumber;
        case XQContactField::ContactFieldTypeGivenName:                return KUidContactFieldGivenName;
        case XQContactField::ContactFieldTypeFamilyName:               return KUidContactFieldFamilyName;
        //case XQContactField::ContactFieldTypeGivenNamePronunciation:   return KUidContactFieldGivenNamePronunciation;
        //case XQContactField::ContactFieldTypeFamilyNamePronunciation:  return KUidContactFieldFamilyNamePronunciation;
        case XQContactField::ContactFieldTypeAdditionalName:           return KUidContactFieldAdditionalName;
        case XQContactField::ContactFieldTypeSuffixName:               return KUidContactFieldSuffixName;
        case XQContactField::ContactFieldTypePrefixName:               return KUidContactFieldPrefixName;
        //case XQContactField::ContactFieldTypeHidden:                   return KUidContactFieldHidden;
        case XQContactField::ContactFieldTypeEMail:                    return KUidContactFieldEMail;
        //case XQContactField::ContactFieldTypeMsg:                      return KUidContactFieldMsg;
        //case XQContactField::ContactFieldTypeSms:                      return KUidContactFieldSms;
        //case XQContactField::ContactFieldTypeFax:                      return KUidContactFieldFax; //TODO:
        //case XQContactField::ContactFieldTypeDefinedText:              return KUidContactFieldDefinedText;
        case XQContactField::ContactFieldTypeNote:                     return KUidContactFieldNote;
        case XQContactField::ContactFieldTypeBirthday:                 return KUidContactFieldBirthday;
        case XQContactField::ContactFieldTypeUrl:                      return KUidContactFieldUrl;
        //case XQContactField::ContactFieldTypeStorageInline:            return KUidContactStorageInline;
        //case XQContactField::ContactFieldTypeTemplateLabel:            return KUidContactFieldTemplateLabel;
        case XQContactField::ContactFieldTypePicture:                  return KUidContactFieldPicture;
        //case XQContactField::ContactFieldTypePicturePath:              return KUidContactFieldPicturePath; // TODO:
        //case XQContactField::ContactFieldTypeRingTone:                 return KUidContactFieldRingTone;
        //case XQContactField::ContactFieldTypeVoiceDial:                return KUidContactsVoiceDialField;
        case XQContactField::ContactFieldTypeJobTitle:                 return KUidContactFieldJobTitle;
        //case XQContactField::ContactFieldTypeICCSlot:                  return KUidContactFieldICCSlot;
        //case XQContactField::ContactFieldTypeICCPhonebook:             return KUidContactFieldICCPhonebook;
        //case XQContactField::ContactFieldTypeICCGroup:                 return KUidContactFieldICCGroup;
        //case XQContactField::ContactFieldTypeIMAddress:                return KUidContactFieldIMAddress;
        case XQContactField::ContactFieldTypeSecondName:               return KUidContactFieldSecondName;
        case XQContactField::ContactFieldTypeSIPID:                    return KUidContactFieldSIPID;
        case XQContactField::ContactFieldTypeAssistant:                return KUidContactFieldAssistant;
        case XQContactField::ContactFieldTypeAnniversary:              return KUidContactFieldAnniversary;
        case XQContactField::ContactFieldTypeSpouse:                   return KUidContactFieldSpouse;
        case XQContactField::ContactFieldTypeChildren:                 return KUidContactFieldChildren;
        //case XQContactField::ContactFieldTypeClass:                    return KUidContactFieldClass;
        case XQContactField::ContactFieldTypeDepartmentName:           return KUidContactFieldDepartmentName;
        //case XQContactField::ContactFieldTypeAgent:                    return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeFormattedName:            return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeGeo:                      return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeKey:                      return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeLabel:                    return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeMailer:                   return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeRole:                     return KUidContactFieldNone;
        //case XQContactField::ContactFieldTypeSound:                    return KUidContactFieldNone;
        case XQContactField::ContactFieldTypeUnknown:                  return TUid(); // TODO:
    }
    return TUid();
}

TUid XQContactsPrivate::vCardFieldTypeFromQtFieldProperty(XQContactFieldPrivate::ContactFieldProperty property) const
{
    switch (property) {
        case XQContactFieldPrivate::ContactFieldPropertyHome:  return KUidContactFieldVCardMapHOME;
        case XQContactFieldPrivate::ContactFieldPropertyWork:  return KUidContactFieldVCardMapWORK;
        case XQContactFieldPrivate::ContactFieldPropertyPref:  return KUidContactFieldVCardMapPREF;
        case XQContactFieldPrivate::ContactFieldPropertyVoice: return KUidContactFieldVCardMapVOICE;
        case XQContactFieldPrivate::ContactFieldPropertyCell:  return KUidContactFieldVCardMapCELL;
        case XQContactFieldPrivate::ContactFieldPropertyPager: return KUidContactFieldVCardMapPAGER;
        case XQContactFieldPrivate::ContactFieldPropertyBbs:   return KUidContactFieldVCardMapBBS;
        case XQContactFieldPrivate::ContactFieldPropertyModem: return KUidContactFieldVCardMapMODEM;
        case XQContactFieldPrivate::ContactFieldPropertyCar:   return KUidContactFieldVCardMapCAR;
        case XQContactFieldPrivate::ContactFieldPropertyIsdn:  return KUidContactFieldVCardMapISDN;
        case XQContactFieldPrivate::ContactFieldPropertyVideo: return KUidContactFieldVCardMapVIDEO;
        case XQContactFieldPrivate::ContactFieldPropertyMsg:   return KUidContactFieldVCardMapMSG;
        case XQContactFieldPrivate::ContactFieldPropertyFax:   return KUidContactFieldVCardMapUnknown; // TODO:
        case XQContactFieldPrivate::ContactFieldPropertyPoc:   return KUidContactFieldVCardMapPOC;
        case XQContactFieldPrivate::ContactFieldPropertySwis:  return KUidContactFieldVCardMapSWIS;
        case XQContactFieldPrivate::ContactFieldPropertyVoip:  return KUidContactFieldVCardMapVOIP;
        case XQContactFieldPrivate::ContactFieldPropertyX509:  return KUidContactFieldVCardMapX509;
        case XQContactFieldPrivate::ContactFieldPropertyPGP:   return KUidContactFieldVCardMapPGP;
        case XQContactFieldPrivate::ContactFieldPropertyDom:   return KUidContactFieldVCardMapDOM;
        case XQContactFieldPrivate::ContactFieldPropertyGif:   return KUidContactFieldVCardMapGIF;
        case XQContactFieldPrivate::ContactFieldPropertyCgm:   return KUidContactFieldVCardMapCGM;
        case XQContactFieldPrivate::ContactFieldPropertyWmf:   return KUidContactFieldVCardMapWMF;
        case XQContactFieldPrivate::ContactFieldPropertyBmp:   return KUidContactFieldVCardMapBMP;
        case XQContactFieldPrivate::ContactFieldPropertyDib:   return KUidContactFieldVCardMapDIB;
        case XQContactFieldPrivate::ContactFieldPropertyPs:    return KUidContactFieldVCardMapPS;
        case XQContactFieldPrivate::ContactFieldPropertyPmb:   return KUidContactFieldVCardMapPMB;
        case XQContactFieldPrivate::ContactFieldPropertyPdf:   return KUidContactFieldVCardMapPDF;
        case XQContactFieldPrivate::ContactFieldPropertyPict:  return KUidContactFieldVCardMapPICT;
        case XQContactFieldPrivate::ContactFieldPropertyTiff:  return KUidContactFieldVCardMapTIFF;
        case XQContactFieldPrivate::ContactFieldPropertyJpeg:  return KUidContactFieldVCardMapJPEG;
        case XQContactFieldPrivate::ContactFieldPropertyMet:   return KUidContactFieldVCardMapMET;
        case XQContactFieldPrivate::ContactFieldPropertyMpeg:  return KUidContactFieldVCardMapMPEG;
        case XQContactFieldPrivate::ContactFieldPropertyMpeg2: return KUidContactFieldVCardMapMPEG2;
        case XQContactFieldPrivate::ContactFieldPropertyAvi:   return KUidContactFieldVCardMapAVI;
        case XQContactFieldPrivate::ContactFieldPropertyQTime: return KUidContactFieldVCardMapQTIME;
        case XQContactFieldPrivate::ContactFieldPropertyUnknown: return TUid(); // TODO:
    }
    return TUid();
}

XQContactFieldPrivate::ContactFieldProperty XQContactsPrivate::qtFieldPropertyFromVCardFieldType(TFieldType type) const
{
    switch (type.iUid) {
        case KIntContactFieldVCardMapHOME:    return XQContactFieldPrivate::ContactFieldPropertyHome;
        case KIntContactFieldVCardMapWORK:    return XQContactFieldPrivate::ContactFieldPropertyWork;
        case KIntContactFieldVCardMapPREF:    return XQContactFieldPrivate::ContactFieldPropertyPref;
        case KIntContactFieldVCardMapVOICE:   return XQContactFieldPrivate::ContactFieldPropertyVoice;
        case KIntContactFieldVCardMapCELL:    return XQContactFieldPrivate::ContactFieldPropertyCell;
        case KIntContactFieldVCardMapPAGER:   return XQContactFieldPrivate::ContactFieldPropertyPager;
        case KIntContactFieldVCardMapBBS:     return XQContactFieldPrivate::ContactFieldPropertyBbs;
        case KIntContactFieldVCardMapMODEM:   return XQContactFieldPrivate::ContactFieldPropertyModem;
        case KIntContactFieldVCardMapCAR:     return XQContactFieldPrivate::ContactFieldPropertyCar;
        case KIntContactFieldVCardMapISDN:    return XQContactFieldPrivate::ContactFieldPropertyIsdn;
        case KIntContactFieldVCardMapVIDEO:   return XQContactFieldPrivate::ContactFieldPropertyVideo;
        case KIntContactFieldVCardMapMSG:     return XQContactFieldPrivate::ContactFieldPropertyMsg;
        case KIntContactFieldVCardMapUnknown: return XQContactFieldPrivate::ContactFieldPropertyFax; // TODO:
        case KIntContactFieldVCardMapPOC:     return XQContactFieldPrivate::ContactFieldPropertyPoc;
        case KIntContactFieldVCardMapSWIS:    return XQContactFieldPrivate::ContactFieldPropertySwis;
        case KIntContactFieldVCardMapVOIP:    return XQContactFieldPrivate::ContactFieldPropertyVoip;
        case KIntContactFieldVCardMapX509:    return XQContactFieldPrivate::ContactFieldPropertyX509;
        case KIntContactFieldVCardMapPGP:     return XQContactFieldPrivate::ContactFieldPropertyPGP;
        case KIntContactFieldVCardMapDOM:     return XQContactFieldPrivate::ContactFieldPropertyDom;
        case KIntContactFieldVCardMapGIF:     return XQContactFieldPrivate::ContactFieldPropertyGif;
        case KIntContactFieldVCardMapCGM:     return XQContactFieldPrivate::ContactFieldPropertyCgm;
        case KIntContactFieldVCardMapWMF:     return XQContactFieldPrivate::ContactFieldPropertyWmf;
        case KIntContactFieldVCardMapBMP:     return XQContactFieldPrivate::ContactFieldPropertyBmp;
        case KIntContactFieldVCardMapDIB:     return XQContactFieldPrivate::ContactFieldPropertyDib;
        case KIntContactFieldVCardMapPS:      return XQContactFieldPrivate::ContactFieldPropertyPs;
        case KIntContactFieldVCardMapPMB:     return XQContactFieldPrivate::ContactFieldPropertyPmb;
        case KIntContactFieldVCardMapPDF:     return XQContactFieldPrivate::ContactFieldPropertyPdf;
        case KIntContactFieldVCardMapPICT:    return XQContactFieldPrivate::ContactFieldPropertyPict;
        case KIntContactFieldVCardMapTIFF:    return XQContactFieldPrivate::ContactFieldPropertyTiff;
        case KIntContactFieldVCardMapJPEG:    return XQContactFieldPrivate::ContactFieldPropertyJpeg;
        case KIntContactFieldVCardMapMET:     return XQContactFieldPrivate::ContactFieldPropertyMet;
        case KIntContactFieldVCardMapMPEG:    return XQContactFieldPrivate::ContactFieldPropertyMpeg;
        case KIntContactFieldVCardMapMPEG2:   return XQContactFieldPrivate::ContactFieldPropertyMpeg2;
        case KIntContactFieldVCardMapAVI:     return XQContactFieldPrivate::ContactFieldPropertyAvi;
        case KIntContactFieldVCardMapQTIME:   return XQContactFieldPrivate::ContactFieldPropertyQTime;

    }
    return XQContactFieldPrivate::ContactFieldPropertyUnknown;
}


TUid XQContactsPrivate::vCardMappingFromQtFieldType(XQContactField::ContactFieldType type) const
{
    switch (type) {
        case XQContactField::ContactFieldTypeAddress:                  return KUidContactFieldVCardMapADR;
        case XQContactField::ContactFieldTypePostOffice:               return KUidContactFieldVCardMapPOSTOFFICE;
        case XQContactField::ContactFieldTypeExtendedAddress:          return KUidContactFieldVCardMapEXTENDEDADR;
        case XQContactField::ContactFieldTypeLocality:                 return KUidContactFieldVCardMapLOCALITY;
        case XQContactField::ContactFieldTypeRegion:                   return KUidContactFieldVCardMapREGION;
        case XQContactField::ContactFieldTypePostCode:                 return KUidContactFieldVCardMapPOSTCODE;
        case XQContactField::ContactFieldTypeCountry:                  return KUidContactFieldVCardMapCOUNTRY;
        case XQContactField::ContactFieldTypeCompanyName:              return KUidContactFieldVCardMapORG;
        //case XQContactField::ContactFieldTypeCompanyNamePronunciation: return KUidContactFieldVCardMapORGPronunciation;
        case XQContactField::ContactFieldTypePhoneNumber:              return KUidContactFieldVCardMapTEL;
        case XQContactField::ContactFieldTypeGivenName:                return KUidContactFieldVCardMapUnusedN;
        case XQContactField::ContactFieldTypeFamilyName:               return KUidContactFieldVCardMapUnusedN;
        //case XQContactField::ContactFieldTypeGivenNamePronunciation:   return KUidContactFieldVCardMapNotRequired;
        //case XQContactField::ContactFieldTypeFamilyNamePronunciation:  return KUidContactFieldVCardMapUnusedN;
        case XQContactField::ContactFieldTypeAdditionalName:           return KUidContactFieldVCardMapUnusedN;
        case XQContactField::ContactFieldTypeSuffixName:               return KUidContactFieldVCardMapUnusedN;
        case XQContactField::ContactFieldTypePrefixName:               return KUidContactFieldVCardMapUnusedN;
        //case XQContactField::ContactFieldTypeHidden:                   return KUidContactFieldHidden; //TODO:
        case XQContactField::ContactFieldTypeEMail:                    return KUidContactFieldVCardMapEMAILINTERNET;
        //case XQContactField::ContactFieldTypeMsg:                      return KUidContactFieldVCardMapTEL; //TODO:
        //case XQContactField::ContactFieldTypeSms:                      return KUidContactFieldVCardMapTEL; //TODO:
        //case XQContactField::ContactFieldTypeFax:                      return KUidContactFieldVCardMapTEL; //TODO:
        //case XQContactField::ContactFieldTypeDefinedText:              return KUidContactFieldDefinedText; //TODO:
        case XQContactField::ContactFieldTypeNote:                     return KUidContactFieldVCardMapNOTE;
        case XQContactField::ContactFieldTypeBirthday:                 return KUidContactFieldVCardMapBDAY;
        case XQContactField::ContactFieldTypeUrl:                      return KUidContactFieldVCardMapURL;
        //case XQContactField::ContactFieldTypeStorageInline:            return KUidContactStorageInline; //TODO:
        //case XQContactField::ContactFieldTypeTemplateLabel:            return KUidContactFieldTemplateLabel; //TODO:
        case XQContactField::ContactFieldTypePicture:                  return KUidContactFieldVCardMapLOGO;
        //case XQContactField::ContactFieldTypePicturePath:              return KUidContactFieldPicturePath; //TODO:
        //case XQContactField::ContactFieldTypeRingTone:                 return KUidContactFieldVCardMapUnusedN; //TODO:
        //case XQContactField::ContactFieldTypeVoiceDial:                return KUidContactFieldVCardMapTEL; //TODO:
        case XQContactField::ContactFieldTypeJobTitle:                 return KUidContactFieldVCardMapTITLE;
        //case XQContactField::ContactFieldTypeICCSlot:                  return KUidContactFieldICCSlot; //TODO:
        //case XQContactField::ContactFieldTypeICCPhonebook:             return KUidContactFieldICCPhonebook; //TODO:
        //case XQContactField::ContactFieldTypeICCGroup:                 return KUidContactFieldICCGroup; //TODO:
        //case XQContactField::ContactFieldTypeIMAddress:                return KUidContactFieldVCardMapWV;
        case XQContactField::ContactFieldTypeSecondName:               return KUidContactFieldVCardMapSECONDNAME;
        case XQContactField::ContactFieldTypeSIPID:                    return KUidContactFieldVCardMapSIPID;
        case XQContactField::ContactFieldTypeAssistant:                return KUidContactFieldVCardMapAssistant;
        case XQContactField::ContactFieldTypeAnniversary:              return KUidContactFieldVCardMapAnniversary;
        case XQContactField::ContactFieldTypeSpouse:                   return KUidContactFieldVCardMapSpouse;
        case XQContactField::ContactFieldTypeChildren:                 return KUidContactFieldVCardMapChildren;
        //case XQContactField::ContactFieldTypeClass:                    return KUidContactFieldVCardMapClass;
        case XQContactField::ContactFieldTypeDepartmentName:           return KUidContactFieldVCardMapDepartment;
        //case XQContactField::ContactFieldTypeAgent:                    return KUidContactFieldVCardMapAGENT;
        //case XQContactField::ContactFieldTypeFormattedName:            return KUidContactFieldVCardMapUnusedFN;
        //case XQContactField::ContactFieldTypeGeo:                      return KUidContactFieldVCardMapGEO;
        //case XQContactField::ContactFieldTypeKey:                      return KUidContactFieldVCardMapKEY;
        //case XQContactField::ContactFieldTypeLabel:                    return KUidContactFieldVCardMapLABEL;
        //case XQContactField::ContactFieldTypeMailer:                   return KUidContactFieldVCardMapMAILER;
        //case XQContactField::ContactFieldTypeRole:                     return KUidContactFieldVCardMapROLE;
        //case XQContactField::ContactFieldTypeSound:                    return KUidContactFieldVCardMapSOUND;
        case XQContactField::ContactFieldTypeUnknown:                  return TUid(); // TODO:
    }
    return TUid();
}

TDateTime XQContactsPrivate::fromQtDateTimeToS60DateTime(QDateTime dateTime) const
{
    QDate date = dateTime.date();
    QTime time = dateTime.time();
    TMonth month = EJanuary;
    switch (date.month()) {
        case 1:
            month = EJanuary;
            break;
        case 2:
            month = EFebruary;
            break;
        case 3:
            month = EMarch;
            break;
        case 4:
            month = EApril; 
            break;
        case 5:
            month = EMay; 
            break;
        case 6:
            month = EJune; 
            break;
        case 7:
            month = EJuly; 
            break;
        case 8:
            month = EAugust; 
            break;
        case 9:
            month = ESeptember; 
            break;
        case 10:
            month = EOctober; 
            break;
        case 11:
            month = ENovember; 
            break;
        case 12:
            month = EDecember; 
            break;
    }
    return TDateTime(date.year(),month,date.day()-1,time.hour(),time.minute(),time.second(),time.msec());
}

QDateTime XQContactsPrivate::fromS60DateTimeToQtDateTime(TDateTime dateTime) const
{
    int month = 1;
    switch (dateTime.Month()) {
        case EJanuary:
            month = 1;
            break;
        case EFebruary:
            month = 2;
            break;
        case EMarch:
            month = 3;
            break;
        case EApril:
            month = 4; 
            break;
        case EMay:
            month = 5; 
            break;
        case EJune:
            month = 6; 
            break;
        case EJuly:
            month = 7; 
            break;
        case EAugust:
            month = 8; 
            break;
        case ESeptember:
            month = 9; 
            break;
        case EOctober:
            month = 10; 
            break;
        case ENovember:
            month = 11; 
            break;
        case EDecember:
            month = 12; 
            break;
    }
    QDate date(dateTime.Year(),month,dateTime.Day()+1);
    QTime time(dateTime.Hour(),dateTime.Minute(),dateTime.Second(),dateTime.MicroSecond());
    
    return QDateTime(date,time);
}

TInt XQContactsPrivate::WordParser(TAny *aParam)
{
    SFindInTextDefWordParser *aSFindInTextDefWordParser = static_cast<SFindInTextDefWordParser *>(aParam);
	
    TLex Input(*aSFindInTextDefWordParser->iSearchString);
    TBuf<64> Token;
    TInt TokenCount = 0;
    do {
        Token = Input.NextToken();
        if(Token.Length())	{
            aSFindInTextDefWordParser->iWordArray->AppendL(Token);
            ++TokenCount;
        }
    } while(Token.Length());

    return TokenCount;
}

// End of file

