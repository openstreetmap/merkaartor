#ifndef NETWORKSIGNALMONITOR_H
#define NETWORKSIGNALMONITOR_H

#include <Etel3rdParty.h> 
 
class MNetworkSignalObserver
    {
    public:
        virtual void SignalStatusL( TInt32 aStrength, TInt8 aBars ) = 0;
    };
      
class CNetworkSignalMonitor : public CActive
    { 
public:
    CNetworkSignalMonitor( MNetworkSignalObserver& aObserver );
    void ConstructL();
    ~CNetworkSignalMonitor();
    
private:
    void RunL();
    void DoCancel();
    
private:
    MNetworkSignalObserver& iObserver;
    CTelephony* iTelephony;
    CTelephony::TSignalStrengthV1 iSigStrengthV1;
    CTelephony::TSignalStrengthV1Pckg iSigStrengthV1Pckg;
    };

#endif /* NETWORKCSIGNALMONITOR_H */
