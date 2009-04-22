#ifndef SUBSCRIBER_H
#define SUBSCRIBER_H

#include <e32base.h>
#include <e32property.h>

class MSubscriberObserver
    {
    public:
        virtual void IntPropertyUpdatedL( TUid aUid, TInt32 aKey, TInt aValue ) = 0;
        virtual void StrPropertyUpdatedL( TUid aUid, TInt32 aKey, const TDesC& aValue ) = 0;
        virtual void BinaryPropertyUpdatedL( TUid aUid, TInt32 aKey, const TDesC8& aValue ) = 0;
        virtual void PropertyDeletedL( TUid aUid, TInt32 aKey ) = 0;
    };
 
class CSubscriber : public CActive
    {
    public:
        static CSubscriber* NewL( const TUid aUid, const TUint32 aKey, MSubscriberObserver& aNotifier );
        virtual ~CSubscriber();
        TUid Uid() const;
        TUint32 Key() const;
            
    private:
        CSubscriber( const TUid aUid, const TUint32 aKey, MSubscriberObserver& aNotifier );
        void ConstructL();
    
    private:
        void RunL();
        void DoCancel();
      
    private:
        RProperty               iProperty;
        const TUid              iUid;
        const TUint32           iKey;
        MSubscriberObserver&  iObserver; 
    };
 
#endif /*SUBSCRIBER_H*/
