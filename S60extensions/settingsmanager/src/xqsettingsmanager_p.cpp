#include "xqsettingsmanager_p.h"
#include <centralrepository.h>
#include <e32property.h>
#include <QByteArray>

const int KRBufDefaultLength = 100; 

XQSettingsManagerPrivate::XQSettingsManagerPrivate(const XQSettingsItem& item, 
    XQSettingsManager *settingsManager) : q(settingsManager)
{
    TUid catecory;
    catecory.iUid = item.category();
    TRAP(iError,
        if (item.type() == XQSettingsItem::TypeRepository) {
            iCRepository = CRepository::NewL(catecory);
            CCenRepNotifier *cenrepNotifier = CCenRepNotifier::NewL(catecory,item.key(),*this);
            iCenRepNotifiers.Append(cenrepNotifier);
        } else if (item.type() == XQSettingsItem::TypeProperty) {
            iPubSubNotifiers.Append(CSubscriber::NewL(catecory,item.key(),*this));
        }
    )
}

XQSettingsManagerPrivate::XQSettingsManagerPrivate(
    XQSettingsManager *settingsManager) : q(settingsManager)
{   
}

XQSettingsManagerPrivate::~XQSettingsManagerPrivate()
{
    iCenRepNotifiers.ResetAndDestroy();
    iPubSubNotifiers.ResetAndDestroy();
    delete iCRepository;
}

bool XQSettingsManagerPrivate::createItem(const XQSettingsItem& item)
{
    bool retVal = false;
    TUid category;
    category.iUid = item.category();
    
    if (item.type() == XQSettingsItem::TypeRepository) {
        if (!iCRepository) {
            TRAP(iError, iCRepository = CRepository::NewL(category);)
        }
        if (item.value().type() == QVariant::Int) {
            iCRepository->Create(item.key(),item.value().toInt() !=
                    KErrNone) ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::Double) {
            iCRepository->Create(item.key(),item.value().toDouble() !=
                    KErrNone) ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::String) {
            TPtrC16 str(reinterpret_cast<const TUint16*>(
                    item.key(),item.value().toString().utf16()));
            iCRepository->Create(item.key(),str) !=
                    KErrNone ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::ByteArray){
            TPtrC8 tempPtr8((TUint8 *)(item.value().toByteArray().constData()));
            iCRepository->Create(item.key(),tempPtr8) !=
                    KErrNone ? retVal = true : retVal = false;
        } else {
            retVal = true;
        } 
    }  else if (item.type() == XQSettingsItem::TypeProperty) {
        if (item.value().type() == QVariant::Int) {
            RProperty::Define(category,item.key(),RProperty::EInt) !=
                    KErrNone ? retVal = true : retVal = false;
            setItemValue(item);
        } else if (item.value().type() == QVariant::String) {
            RProperty::Define(category,item.key(),RProperty::EText) !=
                    KErrNone ? retVal = true : retVal = false;
            setItemValue(item);
        } else if (item.value().type() == QVariant::ByteArray) {
            RProperty::Define(category,item.key(),RProperty::EByteArray) !=
                    KErrNone ? retVal = true : retVal = false;
            setItemValue(item);
        } else {
            retVal = true;
        }
    }
  
    return !retVal;   
}

bool XQSettingsManagerPrivate::deleteItem(const XQSettingsItem& item)
{
    bool retVal = false;
    TUid category;
    category.iUid = item.category();
    if (item.type() == XQSettingsItem::TypeRepository) {
        if (!iCRepository) {
            TRAP(iError, iCRepository = CRepository::NewL(category);)
        }
        iCRepository->Delete(item.key()) !=
                KErrNone ? retVal = true : retVal = false;
    } else if (item.type() == XQSettingsItem::TypeProperty) {
        RProperty::Delete(item.key()) !=
                KErrNone ? retVal = true : retVal = false; 
    }
    
    return !retVal;
}

XQSettingsItem XQSettingsManagerPrivate::item(const XQSettingsItem& item) const
{
    TUid category;
    category.iUid = item.category();
    if (item.type() == XQSettingsItem::TypeRepository) {
        if (!iCRepository) {
            TRAP(iError, iCRepository = CRepository::NewL(category);)
        }
        if (item.value().type() == QVariant::Int) {
            TInt tempInt;
            iCRepository->Get(item.key(), tempInt);
            return XQPropertyItem(item.category(),item.key(),tempInt);
        } else if (item.value().type() == QVariant::Double) {
            TReal tempReal;
            iCRepository->Get(item.key(), tempReal);
            return XQPropertyItem(item.category(),item.key(),tempReal);  
        } else if (item.value().type() == QVariant::String) {
            RBuf tempBuf;
            TRAP(iError, CleanupClosePushL(tempBuf);)
            TRAP(iError, tempBuf.CreateL(KRBufDefaultLength);)
            if (iCRepository->Get(item.key(), tempBuf) == KErrOverflow) {
                int ret = KErrOverflow;
                while (ret == KErrOverflow) {
                    TRAP(iError, tempBuf.ReAllocL(
                            tempBuf.MaxSize()+KRBufDefaultLength);)
                    ret = iCRepository->Get(item.key(), tempBuf);
                }
            }
            if (iCRepository->Get(item.key(), tempBuf) == KErrNone) {
                QString stringValue = QString::fromUtf16(
                        tempBuf.Ptr(),tempBuf.Length());
                CleanupStack::PopAndDestroy(&tempBuf);
                return XQPropertyItem(item.category(),item.key(),stringValue);
            }
        } else if (item.value().type() == QVariant::ByteArray) {
            RBuf8 tempBuf8;
            TRAP(iError, CleanupClosePushL(tempBuf8);)
            TRAP(iError, tempBuf8.CreateL(KRBufDefaultLength);)
            if (iCRepository->Get(item.key(), tempBuf8) == KErrOverflow) {
                int ret = KErrOverflow;
                while (ret == KErrOverflow) {
                    TRAP(iError, tempBuf8.ReAllocL(
                            tempBuf8.MaxSize()+KRBufDefaultLength);)
                    ret = iCRepository->Get(item.key(), tempBuf8);
                }
            }
            if (iCRepository->Get(item.key(), tempBuf8) == KErrNone) {
                QByteArray byteArray = QByteArray(
                        (const char *)tempBuf8.Ptr(),tempBuf8.Length());
                CleanupStack::PopAndDestroy(&tempBuf8);
                return XQPropertyItem(item.category(),item.key(),byteArray);
            }  
        } else {
            QVariant value;
            TRAP(iError, value = discoverCenRepTypeL(item);)
            return XQRepositoryItem(item.category(),item.key(),value);
        }
    } else if (item.type() == XQSettingsItem::TypeProperty) {
        if (item.value().type() == QVariant::Int) {
            TInt tempInt;
            if (RProperty::Get(category,item.key(),tempInt) == KErrNone) {
                return XQPropertyItem(item.category(),item.key(),tempInt);
            }
        } else if (item.value().type() == QVariant::String) {
            RBuf tempBuf;
            TRAP(iError, CleanupClosePushL(tempBuf);)
            TRAP(iError, tempBuf.CreateL(KRBufDefaultLength);)
            if (RProperty::Get(category,item.key(), tempBuf) == KErrOverflow) {
                int ret = KErrOverflow;
                while (ret == KErrOverflow) {
                    TRAP(iError, tempBuf.ReAllocL(
                            tempBuf.MaxSize()+KRBufDefaultLength);)
                    ret = RProperty::Get(category,item.key(), tempBuf);
                }
            }
            if (RProperty::Get(category,item.key(), tempBuf) == KErrNone) {
                QString stringValue = QString::fromUtf16(
                        tempBuf.Ptr(),tempBuf.Length());
                CleanupStack::PopAndDestroy(&tempBuf);
                return XQPropertyItem(item.category(), item.key(),stringValue);
            }   
        } else if (item.value().type() == QVariant::ByteArray) {
            RBuf8 tempBuf8;
            CleanupClosePushL(tempBuf8);
            tempBuf8.CreateL(KRBufDefaultLength);
            if (RProperty::Get(category,item.key(),
                    tempBuf8) == KErrOverflow) {
                int ret = KErrOverflow;
                while (ret == KErrOverflow) {
                    TRAP(iError, tempBuf8.ReAllocL(
                            tempBuf8.MaxSize()+KRBufDefaultLength);)
                    ret = RProperty::Get(category, item.key(), tempBuf8);
                }
            }
            TInt jep = RProperty::Get(category, item.key(), tempBuf8);
            if (RProperty::Get(category, item.key(), tempBuf8) == KErrNone) {
                QByteArray byteArray = QByteArray(
                        (const char *)tempBuf8.Ptr(),tempBuf8.Length());
                CleanupStack::PopAndDestroy(&tempBuf8);
                return XQPropertyItem(item.category(),item.key(),byteArray);
            }   
            CleanupStack::PopAndDestroy(&tempBuf8);
        } else {
            QVariant value;
            TRAP(iError, value = discoverPubSubTypeL(item);)
            return XQRepositoryItem(item.category(),item.key(),value);
        }
    }
    
    return XQSettingsItem();
}

QVariant XQSettingsManagerPrivate::discoverCenRepTypeL(
        const XQSettingsItem& item) const
{
    TInt tempInt;
    TReal tempReal;
    TUid category;
    category.iUid = item.category();

    if (!iCRepository) {
        TRAP(iError, iCRepository = CRepository::NewL(category);)
    }    
    if (iCRepository->Get(item.key(), tempInt) == KErrNone) {
        return QVariant(tempInt);
    }
    if (iCRepository->Get(item.key(), tempReal) == KErrNone) {
        return QVariant(tempReal);
    } 
    RBuf8 tempBuf8;
    CleanupClosePushL(tempBuf8);
    tempBuf8.CreateL(KRBufDefaultLength);
    if (iCRepository->Get(item.key(), tempBuf8) != KErrArgument) {
        int ret = KErrOverflow;
        while (ret == KErrOverflow) {
            tempBuf8.ReAllocL(tempBuf8.MaxSize()+KRBufDefaultLength);
            ret = iCRepository->Get(item.key(), tempBuf8);
        }
        QByteArray byteArray = QByteArray(
                (const char *)tempBuf8.Ptr(),tempBuf8.Length());
        CleanupStack::PopAndDestroy(&tempBuf8);
        return QVariant(byteArray);
    } else {  
        RBuf tempBuf;
        CleanupClosePushL(tempBuf);
        tempBuf.CreateL(KRBufDefaultLength);
        if (iCRepository->Get(item.key(), tempBuf) == KErrOverflow) {
            int ret = KErrOverflow;
            while (ret == KErrOverflow) {
                tempBuf.ReAllocL(tempBuf.MaxSize()+KRBufDefaultLength);
                ret = iCRepository->Get(item.key(), tempBuf);
            }
        }
        if (iCRepository->Get(item.key(), tempBuf) == KErrNone) {
            QString stringValue = QString::fromUtf16(tempBuf.Ptr(),tempBuf.Length());
            CleanupStack::PopAndDestroy(&tempBuf);
            return QVariant(stringValue);
        }
        CleanupStack::PopAndDestroy(&tempBuf);
    }
    CleanupStack::PopAndDestroy(&tempBuf8);
    return QVariant(QVariant::Invalid);
}

QVariant XQSettingsManagerPrivate::discoverPubSubTypeL(
        const XQSettingsItem& item) const
{
    TInt tempInt;      
    TUid category;
    category.iUid = item.category();

    if (RProperty::Get(category,item.key(),tempInt) == KErrNone) {
        return QVariant(tempInt);
    }

    RBuf8 tempBuf8;
    CleanupClosePushL(tempBuf8);
    tempBuf8.CreateL(KRBufDefaultLength);
    if (RProperty::Get(category,item.key(), tempBuf8) != KErrArgument) {
        int ret = KErrOverflow;
        while (ret == KErrOverflow) {
            tempBuf8.ReAllocL(tempBuf8.MaxSize()+KRBufDefaultLength);
            ret = RProperty::Get(category,item.key(), tempBuf8);
        }
        QByteArray byteArray = QByteArray((const char *)tempBuf8.Ptr(),
                tempBuf8.Length());
        CleanupStack::PopAndDestroy(&tempBuf8);
        return QVariant(byteArray);
    } else {  
        RBuf tempBuf;
        CleanupClosePushL(tempBuf);
        tempBuf.CreateL(KRBufDefaultLength);
        if (RProperty::Get(category,item.key(), tempBuf) == KErrOverflow) {
            int ret = KErrOverflow;
            while (ret == KErrOverflow) {
               tempBuf.ReAllocL(tempBuf.MaxSize()+KRBufDefaultLength);
               ret = RProperty::Get(category,item.key(), tempBuf);
            }
        }
        if (RProperty::Get(category,item.key(), tempBuf) == KErrNone) {
            QString stringValue = QString::fromUtf16(tempBuf.Ptr(),
                tempBuf.Length());
            CleanupStack::PopAndDestroy(&tempBuf);
            return QVariant(stringValue);
        }
        CleanupStack::PopAndDestroy(&tempBuf);
    }
    CleanupStack::PopAndDestroy(&tempBuf8);
    return QVariant(QVariant::Invalid);    
}

bool XQSettingsManagerPrivate::setItemValue(const XQSettingsItem& item)
{    
    bool retVal = false;
    TUid category;
    category.iUid = item.category();
    
    if (item.type() == XQSettingsItem::TypeRepository) {
        if (!iCRepository) {
            TRAP(iError, iCRepository = CRepository::NewL(category);)
        }
        if (item.value().type() == QVariant::Int) {
            (iCRepository->Set(item.key(),item.value().toInt()) !=
                    KErrNone) ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::Double) {
            (iCRepository->Set(item.key(),item.value().toDouble()) !=
                    KErrNone) ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::String) {
            TPtrC16 str(reinterpret_cast<const TUint16*>
                    (item.value().toString().utf16()));
            (iCRepository->Set(item.key(),str) !=
                    KErrNone) ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::ByteArray){
            TPtrC8 tempPtr8((TUint8 *)
                    (item.value().toByteArray().constData()));
            iCRepository->Set(item.key(),tempPtr8) !=
                    KErrNone ? retVal = true : retVal = false;
        } else {
            retVal = true;
        } 
    } else if (item.type() == XQSettingsItem::TypeProperty) {
        if (item.value().type() == QVariant::Int) {
            RProperty::Set(category,item.key(),item.value().toInt()) !=
                    KErrNone ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::String) {
            TPtrC16 str(reinterpret_cast<const TUint16*>
                    (item.value().toString().utf16()));
            RProperty::Set(category,item.key(),str) !=
                    KErrNone ? retVal = true : retVal = false;
        } else if (item.value().type() == QVariant::ByteArray) {
            TPtrC8 tempPtr8((TUint8 *)
                    (item.value().toByteArray().constData()));
            RProperty::Set(category,item.key(),tempPtr8) !=
                    KErrNone ? retVal = true : retVal = false;        
        } else {
            retVal = true;
        } 
    }   
    
    return !retVal;   
}

bool XQSettingsManagerPrivate::startMonitoring(const XQSettingsItem& item)
{
    bool retVal = true;
    TRAP(iError,
        RBuf tempBuf;
        CleanupClosePushL(tempBuf);
        tempBuf.CreateL(KRBufDefaultLength);
        TUid catecory;
        catecory.iUid = item.category();
        if (item.type() == XQSettingsItem::TypeRepository) {
            CCenRepNotifier *cenrepNotifier =
                    CCenRepNotifier::NewL(catecory,item.key(),*this);
            iCenRepNotifiers.Append(cenrepNotifier);
        } else if (item.type() == XQSettingsItem::TypeProperty) {
            iPubSubNotifiers.Append(
                    CSubscriber::NewL(catecory,item.key(),*this));
        }
        CleanupStack::PopAndDestroy(&tempBuf);
        retVal = false;
    )
    return !retVal;    
}

bool XQSettingsManagerPrivate::stopMonitoring(const XQSettingsItem& item)
{
    bool retVal = true;
    TUid category;
    category.iUid = item.category();
    if (item.type() == XQSettingsItem::TypeRepository) {
        for (int i = 0; i < iCenRepNotifiers.Count(); i++) {
            if (iCenRepNotifiers[i]->Key() == item.key() &&
                    iCenRepNotifiers[i]->Key() == item.key()) {
                iCenRepNotifiers.Remove(i);
                retVal = false;
            }
        } 
    }  else if (item.type() == XQSettingsItem::TypeProperty) {
        for (int i = 0; i < iPubSubNotifiers.Count(); i++) {
            if (iPubSubNotifiers[i]->Key() == item.key() &&
                    iPubSubNotifiers[i]->Key() == item.key()) {
                iPubSubNotifiers.Remove(i);
                retVal = false;
            }
        } 
    }
    
    return !retVal;
}

void XQSettingsManagerPrivate::IntPropertyUpdatedL(TUid aUid,
        TInt32 aKey, TInt aValue)
{
    emit q->valueChanged(XQPropertyItem(aUid.iUid, aKey, QVariant(aValue)));
}

void XQSettingsManagerPrivate::StrPropertyUpdatedL(TUid aUid,
        TInt32 aKey, const TDesC& aValue)
{
    emit q->valueChanged(XQPropertyItem(aUid.iUid, aKey,
            QVariant(QString::fromUtf16(aValue.Ptr(),aValue.Length()))));
}

void XQSettingsManagerPrivate::BinaryPropertyUpdatedL(TUid aUid, TInt32 aKey,
        const TDesC8& aValue)
{
    emit q->valueChanged(XQPropertyItem(aUid.iUid, aKey, QVariant(QByteArray(
            (const char *)aValue.Ptr(),aValue.Length())))); 
}

void XQSettingsManagerPrivate::PropertyDeletedL(TUid aUid, TInt32 aKey)
{
    emit q->itemDeleted(XQPropertyItem(aUid.iUid, aKey));
}

void XQSettingsManagerPrivate::IntCenrepUpdatedL(TUid aUid, TUint32 aKey,
        TInt aValue)
{
    emit q->valueChanged(XQRepositoryItem(aUid.iUid,aKey,QVariant(aValue)));
}

void XQSettingsManagerPrivate::StrCenrepUpdatedL(TUid aUid, TUint32 aKey,
        const TDesC& aValue)
{
    emit q->valueChanged(XQRepositoryItem(aUid.iUid,aKey,
    QVariant(QString::fromUtf16(aValue.Ptr(),aValue.Length()))));
}

void XQSettingsManagerPrivate::RealCenrepUpdatedL(TUid aUid, TUint32 aKey,
        TReal aValue)
{
    emit q->valueChanged(XQRepositoryItem(aUid.iUid,aKey,QVariant(aValue)));
}

void XQSettingsManagerPrivate::BinaryCenrepUpdatedL(TUid aUid, TUint32 aKey,
        const TDesC8& aValue)
{
    emit q->valueChanged(XQRepositoryItem(aUid.iUid, aKey, QVariant(QByteArray(
            (const char *)aValue.Ptr(),aValue.Length()))));
}

void XQSettingsManagerPrivate::CenRepDeletedL(TUid aUid, TUint32 aKey)
{
    emit q->itemDeleted(XQRepositoryItem(aUid.iUid, aKey));   
}

XQSettingsManager::Error XQSettingsManagerPrivate::error() const
{
    switch (iError) {
        case KErrNone:
            return XQSettingsManager::NoError;
        case KErrNoMemory:
            return XQSettingsManager::OutOfMemoryError;
        case KErrNotFound:
            return XQSettingsManager::NotFoundError;
        case KErrAlreadyExists:
            return XQSettingsManager::AlreadyExistsError;
        default:
            return XQSettingsManager::UnknownError;
    }    
}

// End of file

