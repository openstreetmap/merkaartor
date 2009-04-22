#ifndef XQTELEPHONY_P_H
#define XQTELEPHONY_P_H

// INCLUDES
#include "xqtelephony.h"
#include "ccalldialer.h"
#include "ctelephonymonitor.h"

// CLASS DECLARATION
class XQTelephonyPrivate: public CBase, public MDialObserver, public MTelephonyStatusObserver
{
public:
    XQTelephonyPrivate(XQTelephony *telephony);
    ~XQTelephonyPrivate();
    
    void call(const QString& phoneNumber);
    bool startMonitoringLine();
    void stopMonitoringLine();
    XQTelephony::Error error();

private: //From MDialObserver
    void CallDialedL(TInt aError);
    
private: // From MTelephonyStatusObserver
    void TelephonyStatusChangedL(CTelephony::TCallStatus aStatus,
            const TDesC& aNumber);
    void ErrorOccuredL(TInt aError);
    
private:
    XQTelephony *q;
    CCallDialer* iCallDialer;
    CTelephonyMonitor* iTelephonyMonitor;
    int iError;
};

#endif /*XQTELEPHONY_P_H*/

// End of file

