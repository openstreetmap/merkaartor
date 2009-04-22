#ifndef CFLIGHTMODECHECKER_H
#define CFLIGHTMODECHECKER_H

#include <e32base.h>
#include <Etel3rdParty.h>

class CFlightModeChecker : public CActive
    { 
public:
    CFlightModeChecker();
    ~CFlightModeChecker();
    bool IsFlightMode();

private:
    void RunL();
    void DoCancel();

private:
    CTelephony* iTelephony;
    CTelephony::TFlightModeV1 iFlightModeV1;
    CTelephony::TFlightModeV1Pckg iFlightModeV1Pckg;
    
    CActiveSchedulerWait *iWait;
    };
#endif /* CFLIGHTMODECHECKER_H */
