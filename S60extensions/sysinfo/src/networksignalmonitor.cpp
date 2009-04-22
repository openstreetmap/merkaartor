#include "networksignalmonitor.h"

CNetworkSignalMonitor::~CNetworkSignalMonitor()
{
    Cancel();
    delete iTelephony;
}
 
void CNetworkSignalMonitor::ConstructL()
{
    iTelephony = CTelephony::NewL();
    iTelephony->NotifyChange( iStatus, CTelephony::ESignalStrengthChange, iSigStrengthV1Pckg );
    SetActive();
}
 
CNetworkSignalMonitor::CNetworkSignalMonitor(MNetworkSignalObserver& aObserver)
    : CActive( EPriorityStandard ), iObserver( aObserver ), iSigStrengthV1Pckg( iSigStrengthV1 )
{
    CActiveScheduler::Add(this);
}
 
void CNetworkSignalMonitor::RunL()
{
    iObserver.SignalStatusL( iSigStrengthV1.iSignalStrength, iSigStrengthV1.iBar );
     iTelephony->NotifyChange( iStatus, CTelephony::ESignalStrengthChange, iSigStrengthV1Pckg ); 
    SetActive();

}
 
void CNetworkSignalMonitor::DoCancel()
{
    iTelephony->CancelAsync( CTelephony::ESignalStrengthChangeCancel );
}
