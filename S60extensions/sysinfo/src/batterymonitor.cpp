#include <batterymonitor.h>

CBatteryMonitor::~CBatteryMonitor()
{
    Cancel();
    delete iTelephony;
}
 
void CBatteryMonitor::ConstructL()
{
    iTelephony = CTelephony::NewL();
    iTelephony->GetBatteryInfo( iStatus, iBatteryInfoV1Pckg );
    SetActive();
}
 
CBatteryMonitor::CBatteryMonitor( MBatteryObserver& aObserver )
    : CActive( EPriorityStandard ), iObserver( aObserver ), iBatteryInfoV1Pckg( iBatteryInfoV1 )
{
    CActiveScheduler::Add(this);
}
 
void CBatteryMonitor::RunL()
{
    iObserver.BatteryLevelL( iBatteryInfoV1.iChargeLevel, iBatteryInfoV1.iStatus );
    iTelephony->NotifyChange( iStatus, CTelephony::EBatteryInfoChange, iBatteryInfoV1Pckg );   
    SetActive();
}
 
void CBatteryMonitor::DoCancel()
{
    iTelephony->CancelAsync( CTelephony::EBatteryInfoChangeCancel );
}
