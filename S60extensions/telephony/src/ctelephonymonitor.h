#ifndef CTELEPHONYMONITOR_H
#define CTELEPHONYMONITOR_H

#include <e32base.h>
#include <Etel3rdParty.h>
 
class MTelephonyStatusObserver
    {
    public:
        virtual void TelephonyStatusChangedL( CTelephony::TCallStatus aStatus,
                const TDesC& aNumber ) = 0;
        virtual void ErrorOccuredL( TInt aError ) = 0;
    };

class CTelephonyMonitor : public CActive
    {
public:
    CTelephonyMonitor( MTelephonyStatusObserver& aObserver );
    static CTelephonyMonitor* NewL( MTelephonyStatusObserver& aObserver );
    static CTelephonyMonitor* NewLC( MTelephonyStatusObserver& aObserver );
    ~CTelephonyMonitor();
    void StartListening();
    CTelephony::TTelNumber GetNumber();
 
private:
    void ConstructL();
 
private:    
    void RunL();
    void DoCancel();
    
private:
    CTelephony* iTelephony;
    CTelephony::TCallStatusV1 iLineStatus;
    CTelephony::TCallStatusV1Pckg iLineStatusPkg;
    MTelephonyStatusObserver& iTelephonyStatusObserver;
    CTelephony::TCallInfoV1 iCallInfoV1;
    CTelephony::TRemotePartyInfoV1 iRemotePartyInfoV1;
    CTelephony::TCallSelectionV1 iCallSelectionV1;
    CTelephony::TRemotePartyInfoV1Pckg iRemotePartyInfoV1Pckg;
    CTelephony::TCallSelectionV1Pckg iCallSelectionV1Pckg;
    CTelephony::TCallInfoV1Pckg iCallInfoV1Pckg;
    };

#endif /* CTELEPHONYMONITOR_H */
