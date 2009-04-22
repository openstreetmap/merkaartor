#include <e32base.h>
#include <Etel3rdParty.h>

#include "cflightmodechecker.h"

CFlightModeChecker::CFlightModeChecker()
    : CActive( EPriorityStandard ),
      iFlightModeV1Pckg( iFlightModeV1 )
    {
    iTelephony = CTelephony::NewL();
    CActiveScheduler::Add( this );
    iWait = new ( ELeave ) CActiveSchedulerWait();
    }

CFlightModeChecker::~CFlightModeChecker()
    {
    delete iTelephony;
    delete iWait;
    }

bool CFlightModeChecker::IsFlightMode()
    {
    Cancel();
    iTelephony->GetFlightMode( iStatus, iFlightModeV1Pckg );
    SetActive();
    
    if (!iWait->IsStarted()) 
        {
        iWait->Start();
        }
    
    bool flightMode = false;
    if( iFlightModeV1.iFlightModeStatus == CTelephony::EFlightModeOn )
        {
        flightMode = true;
        }
    return flightMode;
    }

void CFlightModeChecker::RunL()
    {
    iWait->AsyncStop();
    }

void CFlightModeChecker::DoCancel()
    {
    iTelephony->CancelAsync( CTelephony::EGetFlightModeCancel );
    }
