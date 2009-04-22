#include "cdeviceinfo.h"

#include <Etel3rdParty.h>
 
CDeviceInfo* CDeviceInfo::NewL()
    {
    CDeviceInfo* self = NewLC();
    CleanupStack::Pop( self );
    return self;
    }
    
CDeviceInfo* CDeviceInfo::NewLC()
    {
    CDeviceInfo* self = new ( ELeave ) CDeviceInfo();
    CleanupStack::PushL( self );
    self->ConstructL();
    return self;
    }

void CDeviceInfo::ConstructL()
    {   
    CActiveScheduler::Add( this );
    iTelephony = CTelephony::NewL();  
    iWait = new ( ELeave ) CActiveSchedulerWait();
    }
    
CDeviceInfo::CDeviceInfo()
:CActive( EPriorityNormal ), iBatteryInfoV1Pkg( iBatteryInfoV1 ),
        iPhoneIdV1Pkg( iPhoneIdV1 ),
        iSignalStrengthV1Pckg( iSignalStrengthV1 ),
        iSubscriberIdV1Pckg( iSubscriberIdV1 )
    {
    }
 
CDeviceInfo::~CDeviceInfo()
    {
    Cancel();
    delete iTelephony;  
    }

TBuf<CTelephony::KPhoneSerialNumberSize> CDeviceInfo::imei()
    {
    Cancel();
    iTelephony->GetPhoneId( iStatus, iPhoneIdV1Pkg );
    SetActive();
    
    if ( !iWait->IsStarted() ) 
        {
        iWait->Start();
        }
    return iPhoneIdV1.iSerialNumber;
    }

TBuf<CTelephony::KPhoneModelIdSize> CDeviceInfo::model()
    {
    Cancel();
    iTelephony->GetPhoneId( iStatus, iPhoneIdV1Pkg );
    SetActive();
    
    if ( !iWait->IsStarted() ) 
        {
        iWait->Start();
        }
    return iPhoneIdV1.iModel;
    }

TBuf<CTelephony::KPhoneManufacturerIdSize> CDeviceInfo::manufacturer()
    {
    Cancel();
    iTelephony->GetPhoneId( iStatus, iPhoneIdV1Pkg );
    SetActive();
    
    if ( !iWait->IsStarted() ) 
        {
        iWait->Start();
        }
    return iPhoneIdV1.iManufacturer;
    }

TBuf<CTelephony::KIMSISize> CDeviceInfo::imsi()
    {
    Cancel();
    iTelephony->GetSubscriberId( iStatus,iSubscriberIdV1Pckg );
    SetActive();
    
    if (!iWait->IsStarted()) 
        {
        iWait->Start();
        }
    return iSubscriberIdV1.iSubscriberId;
    }

TUint CDeviceInfo::batteryLevel()
    {
    Cancel();
    iTelephony->GetBatteryInfo( iStatus,iBatteryInfoV1Pkg );
    SetActive();
    
    if ( !iWait->IsStarted() ) 
        {
        iWait->Start();
        }
    return iBatteryInfoV1.iChargeLevel;
    }

TInt32 CDeviceInfo::signalStrength()
    {
    Cancel();
    iTelephony->GetSignalStrength( iStatus, iSignalStrengthV1Pckg );
    SetActive();
    if ( !iWait->IsStarted() )
        {
        iWait->Start();
        }
    return iSignalStrengthV1.iSignalStrength;
    }
 
void CDeviceInfo::DoCancel()
    {
    iTelephony->CancelAsync( CTelephony::EGetPhoneIdCancel );
    }
 
void CDeviceInfo::RunL()
    {
    iWait->AsyncStop();
    }
