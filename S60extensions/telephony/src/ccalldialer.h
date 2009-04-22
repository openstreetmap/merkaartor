#ifndef CCALLDIALER_H
#define CCALLDIALER_H

#include <Etel3rdParty.h>
 
class MDialObserver
    {
    public:
        virtual void CallDialedL( TInt aError ) = 0;
    };
        
class CCallDialer : public CActive
    { 
public:
    static CCallDialer* NewL( MDialObserver& aObserver );
    static CCallDialer* NewLC( MDialObserver& aObserver );
    CCallDialer( MDialObserver& aObserver );
    ~CCallDialer();
    
    void Call( const TDesC& aNumber );
    
private: 
    void ConstructL();
    
private:
    void RunL();
    void DoCancel();
    
private:
    MDialObserver&          iObserver;
    CTelephony*             iTelephony;
    CTelephony::TCallId             iCallId;
    CTelephony::TCallParamsV1           iCallParams;
    CTelephony::TCallParamsV1Pckg       iCallParamsPckg;
    };

#endif /* CCALLDIALER_H */
