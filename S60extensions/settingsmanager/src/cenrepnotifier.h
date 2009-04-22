#ifndef CENREPNOTIFYHANDLER_H
#define CENREPNOTIFYHANDLER_H

#include <e32base.h>

class CRepository;

class MCenrepNotifierObserver
    {
    public:
       virtual void IntCenrepUpdatedL( TUid aUid, TUint32 aKey, TInt aValue ) = 0;
       virtual void RealCenrepUpdatedL( TUid aUid, TUint32 aKey, TReal aValue ) = 0;
       virtual void StrCenrepUpdatedL( TUid aUid, TUint32 aKey, const TDesC& aValue ) = 0;
       virtual void BinaryCenrepUpdatedL( TUid aUid, TUint32 aKey, const TDesC8& aValue ) = 0;
       virtual void CenRepDeletedL( TUid aUid, TUint32 aKey ) = 0;
    };

class CCenRepNotifier : public CActive
    { 
    public:
        static CCenRepNotifier* NewL( const TUid aUid, const TUint32 aKey, MCenrepNotifierObserver& aObserver );
        virtual ~CCenRepNotifier();
        TUid Uid() const;
        TUint32 Key() const;
    
    private:
        CCenRepNotifier( const TUid aUid, const TUint32 aKey, MCenrepNotifierObserver& aObserver );
        void ConstructL();

    private:
        void RunL();
        void DoCancel();
              
    private:
        CRepository* iRepository;
        const TUid iUid;
        const TUint32 iKey;
        MCenrepNotifierObserver& iObserver;
    };


#endif /* CENREPNOTIFYHANDLER_H */
