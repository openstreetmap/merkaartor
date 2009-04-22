#ifndef BATTERYMONITOR_H
#define BATTERYMONITOR_H

#include <Etel3rdParty.h>
 
class MBatteryObserver
    {
    public:
         virtual void BatteryLevelL( TUint aChargeLevel, 
             CTelephony::TBatteryStatus aBatteryStatus ) = 0;
    };
    
class CBatteryMonitor : public CActive
  { 
public:
    CBatteryMonitor( MBatteryObserver& aObserver );
    void ConstructL();
    ~CBatteryMonitor();
    
private:    
    void RunL();
    void DoCancel();
    
private:
    MBatteryObserver& iObserver;
    CTelephony* iTelephony;
    CTelephony::TBatteryInfoV1 iBatteryInfoV1;
    CTelephony::TBatteryInfoV1Pckg iBatteryInfoV1Pckg;
   };

#endif /* BATTERYMONITOR_H */
