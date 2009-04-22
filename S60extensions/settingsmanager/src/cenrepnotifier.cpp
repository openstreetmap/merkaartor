#include <centralrepository.h>
#include <cenrepnotifier.h>

const int KRBufDefaultLength = 100;

CCenRepNotifier* CCenRepNotifier::NewL( const TUid aUid, const TUint32 aKey,
        MCenrepNotifierObserver& aObserver )
    {
    CCenRepNotifier* self = new ( ELeave ) CCenRepNotifier( aUid, aKey, aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    CleanupStack::Pop( self );
    return self;
    }

void CCenRepNotifier::ConstructL()
    {
    iRepository = CRepository::NewL( iUid );
    CActiveScheduler::Add( this );
    iRepository->NotifyRequest( iKey, iStatus );
    SetActive();
    }

CCenRepNotifier::CCenRepNotifier( const TUid aUid, const TUint32 aKey, MCenrepNotifierObserver& aObserver ) 
    : CActive( EPriorityStandard ), iUid( aUid ), iKey( aKey ), iObserver( aObserver )
    {
    }

CCenRepNotifier::~CCenRepNotifier()
    {
    Cancel();
    delete iRepository;
    }

void CCenRepNotifier::DoCancel()
    {
    iRepository->NotifyCancel( iKey );
    }

void CCenRepNotifier::RunL()
    {
    iRepository->NotifyRequest( iKey, iStatus );
    SetActive();
    
    TInt intValue = 0;
    TReal realValue = 0.0;
    RBuf tempBuf;
    CleanupClosePushL( tempBuf );
    tempBuf.CreateL( KRBufDefaultLength );
    bool found = false;
    
    if( iRepository->Get( iKey, intValue ) == KErrNone ) 
        {
        iObserver.IntCenrepUpdatedL( iUid, iKey, intValue );
        found = true;
        }
    if( iRepository->Get( iKey, realValue ) == KErrNone ) 
        {
        iObserver.RealCenrepUpdatedL( iUid, iKey, realValue );
        found = true;
        }
    
    RBuf8 tempBuf8;
    CleanupClosePushL( tempBuf8 );
    tempBuf8.CreateL( KRBufDefaultLength );
    if ( iRepository->Get( iKey, tempBuf8 ) != KErrArgument ) {
        int ret = KErrOverflow;
        while ( ret == KErrOverflow ) {
           tempBuf8.ReAllocL( tempBuf8.MaxSize() + KRBufDefaultLength );
           ret = iRepository->Get( iKey, tempBuf8 );
        }
        if ( iRepository->Get( iKey, tempBuf8 ) == KErrNone ) {            
            iObserver.BinaryCenrepUpdatedL( iUid, iKey, tempBuf8 );
            found = true;
        }
    } else {  
        if ( iRepository->Get( iKey, tempBuf ) == KErrOverflow ) {
           int ret = KErrOverflow;
           while ( ret == KErrOverflow ) {
              tempBuf.ReAllocL( tempBuf.MaxSize() + KRBufDefaultLength );
              ret =  iRepository->Get( iKey, tempBuf );
           }
        }
        if ( iRepository->Get( iKey, tempBuf ) == KErrNone) {            
            iObserver.StrCenrepUpdatedL( iUid, iKey, tempBuf );
            found = true;
        }
    }
    CleanupStack::PopAndDestroy( 2,&tempBuf );
    
    if (!found) {
        iObserver.CenRepDeletedL( iUid, iKey );
    }
    }

TUid CCenRepNotifier::Uid() const
    {
    return iUid;
    }

TUint32 CCenRepNotifier::Key() const
    {
    return iKey;
    }
