#include "ccalldialer.h"

CCallDialer* CCallDialer::NewL( MDialObserver& aObserver )
    {
    CCallDialer* self = CCallDialer::NewLC( aObserver );
    CleanupStack::Pop( self );
    return self;
    }
 
CCallDialer* CCallDialer::NewLC( MDialObserver& aObserver )
    {
    CCallDialer* self = new ( ELeave ) CCallDialer( aObserver );
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

CCallDialer::~CCallDialer()
{
    Cancel();
    delete iTelephony;
}

void CCallDialer::ConstructL()
    {
    iTelephony = CTelephony::NewL();
    }
 
CCallDialer::CCallDialer( MDialObserver& aObserver )
: CActive( EPriorityNormal ), iObserver( aObserver ),
iCallParamsPckg( iCallParams )
    {
    CActiveScheduler::Add( this );
    }

void CCallDialer::Call( const TDesC& aNumber )
    {
    CTelephony::TTelNumber telNumber( aNumber ); 
    iCallParams.iIdRestrict = CTelephony::ESendMyId;    
    iTelephony->DialNewCall( iStatus, iCallParamsPckg, telNumber, iCallId );
    SetActive();  
    }
 
void CCallDialer::RunL()
    {
    iObserver.CallDialedL( iStatus.Int() );
    }
 
void CCallDialer::DoCancel()
    {
    iTelephony->CancelAsync( CTelephony::EDialNewCallCancel );
    }
