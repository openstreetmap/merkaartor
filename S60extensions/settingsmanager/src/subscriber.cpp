#include "subscriber.h"
 
const int KRBufDefaultLength = 100;

CSubscriber::CSubscriber( const TUid aUid, const TUint32 aKey, MSubscriberObserver& aObserver ) 
    : CActive(EPriorityStandard), iUid( aUid ), iKey( aKey ), iObserver( aObserver)
    {
    }
 
CSubscriber* CSubscriber::NewL( const TUid aUid, const TUint32 aKey, MSubscriberObserver& aObserver )
    {
    CSubscriber* self = new ( ELeave ) CSubscriber( aUid, aKey, aObserver );
    CleanupStack::PushL(self);
    self->ConstructL();
    CleanupStack::Pop(self);
    return self;
    }
 
void CSubscriber::ConstructL()
    {
    User::LeaveIfError( iProperty.Attach( iUid, iKey ) );
    CActiveScheduler::Add( this );
    iProperty.Subscribe( iStatus );
    SetActive();
    }
 
CSubscriber::~CSubscriber()
    {
    Cancel();
    iProperty.Close();
    }

void CSubscriber::DoCancel()
    {
    iProperty.Cancel();
    }
 
void CSubscriber::RunL()
    {
    iProperty.Subscribe( iStatus );
    SetActive();
    
    TInt intValue = 0;
    RBuf tempBuf;
    CleanupClosePushL( tempBuf );
    tempBuf.CreateL( KRBufDefaultLength );
    bool found = false;
    
    if( iProperty.Get( intValue ) == KErrNone ) 
        {
        iObserver.IntPropertyUpdatedL( iUid, iKey, intValue );
        found = true;
        }
    
    RBuf8 tempBuf8;
    CleanupClosePushL( tempBuf8 );
    tempBuf8.CreateL( KRBufDefaultLength );
    if ( iProperty.Get( tempBuf8 ) != KErrArgument ) {
        int ret = KErrOverflow;
        while ( ret == KErrOverflow ) {
           tempBuf8.ReAllocL( tempBuf8.MaxSize() + KRBufDefaultLength );
           ret = iProperty.Get( tempBuf8 );
        }
        if ( iProperty.Get( tempBuf8 ) == KErrNone ) {            
            iObserver.BinaryPropertyUpdatedL( iUid, iKey, tempBuf8 );
            found = true;
        }
    } else {  
        if ( iProperty.Get(tempBuf) == KErrOverflow ) {
           int ret = KErrOverflow;
           while ( ret == KErrOverflow ) {
              tempBuf.ReAllocL( tempBuf.MaxSize() + KRBufDefaultLength );
              ret =  iProperty.Get( tempBuf );
           }
        }
        if (iProperty.Get( tempBuf ) == KErrNone) {            
            iObserver.StrPropertyUpdatedL( iUid, iKey, tempBuf );
            found = true;
        }
    }
    CleanupStack::PopAndDestroy( 2,&tempBuf );
    
    if (!found) {
        iObserver.PropertyDeletedL( iUid, iKey );
    }
    }

TUid CSubscriber::Uid() const
    {
    return iUid;
    }

TUint32 CSubscriber::Key() const
    {
    return iKey;
    }
 
//  End of File
