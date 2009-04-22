#ifndef CIMEIREADER_H
#define CIMEIREADER_H

#include <e32base.h>
#include <Etel3rdParty.h>

class CDeviceInfo : public CActive
{
public:
    enum States
    {
        EModel, 
        EManufacturer, 
        ESerialnumber,
    };
    
    static CDeviceInfo* NewL();
    static CDeviceInfo* NewLC();
    ~CDeviceInfo();

    TBuf<CTelephony::KPhoneSerialNumberSize> imei();
    TBuf<CTelephony::KPhoneModelIdSize> model();
    TBuf<CTelephony::KPhoneManufacturerIdSize> manufacturer();
    TBuf<CTelephony::KIMSISize> imsi();
    TUint batteryLevel();
    TInt32 signalStrength();
    
protected:  
    void DoCancel();
    void RunL();
    
private:
    CDeviceInfo();
    void ConstructL();

private:
    CTelephony*                 iTelephony;
        
    CTelephony::TBatteryInfoV1Pckg iBatteryInfoV1Pkg; 
    CTelephony::TBatteryInfoV1 iBatteryInfoV1;
    
    CTelephony::TPhoneIdV1      iPhoneIdV1;    
    CTelephony::TPhoneIdV1Pckg  iPhoneIdV1Pkg;
        
    CTelephony::TSignalStrengthV1Pckg iSignalStrengthV1Pckg; 
    CTelephony::TSignalStrengthV1 iSignalStrengthV1;
    
    CTelephony::TSubscriberIdV1Pckg iSubscriberIdV1Pckg;
    CTelephony::TSubscriberIdV1 iSubscriberIdV1; 
    
    CActiveSchedulerWait *iWait;
    
    bool phoneIdFetched;
};

#endif /* CIMEIREADER_H */
