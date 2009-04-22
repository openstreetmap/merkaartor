#ifndef XQSETTINGSMANAGER_P_H
#define XQSETTINGSMANAGER_P_H

// INCLUDES
#include "xqsettingsmanager.h"
#include "xqsettingsitem.h"
#include <e32std.h>
#include "subscriber.h"
#include "cenrepnotifier.h"

// FORWARD DECLARATIONS
class QVariant;

// CLASS DECLARATION
class XQSettingsManagerPrivate:  public CBase, public MCenrepNotifierObserver, public MSubscriberObserver
{
public:
    XQSettingsManagerPrivate(XQSettingsManager *settingsmanager);
    XQSettingsManagerPrivate(const XQSettingsItem& item, XQSettingsManager *settingsmanager);
    ~XQSettingsManagerPrivate();
    
    bool createItem(const XQSettingsItem& item);
    bool deleteItem(const XQSettingsItem& item);
    XQSettingsItem item(const XQSettingsItem& item) const;
    bool setItemValue(const XQSettingsItem& item);
    bool startMonitoring(const XQSettingsItem& item);
    bool stopMonitoring(const XQSettingsItem& item); 
    XQSettingsManager::Error error() const;
    
private:  // From MSubscriberObserver
    void IntPropertyUpdatedL(TUid aUid, TInt32 aKey, TInt aValue);
    void StrPropertyUpdatedL(TUid aUid, TInt32 aKey, const TDesC& aValue);
    void BinaryPropertyUpdatedL(TUid aUid, TInt32 aKey, const TDesC8& aValue);
    void PropertyDeletedL(TUid aUid, TInt32 aKey);
    
private: // From MCenrepNotifierObserver
    void IntCenrepUpdatedL(TUid aUid, TUint32 aKey, TInt aValue);
    void RealCenrepUpdatedL(TUid aUid, TUint32 aKey, TReal aValue);
    void StrCenrepUpdatedL(TUid aUid, TUint32 aKey, const TDesC& aValue);
    void BinaryCenrepUpdatedL(TUid aUid, TUint32 aKey, const TDesC8& aValue);
    void CenRepDeletedL(TUid aUid, TUint32 aKey);
    
private:
    QVariant discoverCenRepTypeL(const XQSettingsItem& item) const;
    QVariant discoverPubSubTypeL(const XQSettingsItem& item) const;
    
private:
    XQSettingsManager *q;
    mutable CRepository *iCRepository;
    RPointerArray<CCenRepNotifier> iCenRepNotifiers;
    RPointerArray<CSubscriber> iPubSubNotifiers;
    mutable int iError;
};

#endif /*XQSETTINGSMANAGER_P_H*/

// End of file

