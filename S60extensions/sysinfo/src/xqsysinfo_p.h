#ifndef XQSYSINFOPRIVATE_H
#define XQSYSINFOPRIVATE_H

// INCLUDES
#include "XQsysinfo.h"
#include "xqsysInfo_p.h"

#include "networksignalmonitor.h"
#include "batterymonitor.h"

#include <etel3rdparty.h>

// FORWARD DECLARATIONS
class CDeviceInfo;

// CLASS DECLARATION
class XQSysInfoPrivate: public QObject, public CBase, public MNetworkSignalObserver, 
    public MBatteryObserver
{
    Q_OBJECT
public:
    XQSysInfoPrivate(XQSysInfo *sysInfo);
    ~XQSysInfoPrivate();
    
    XQSysInfo::Language currentLanguage() const;
    QString imei() const;
    QString imsi() const;
    QString softwareVersion() const;
    QString model() const;
    QString manufacturer() const;
    uint batteryLevel() const;
    int signalStrength() const;                  
    QString browserVersion() const;
    
    int memory() const;
    qlonglong diskSpace(XQSysInfo::Drive drive);
    bool isDiskSpaceCritical(XQSysInfo::Drive drive) const;
    bool isNetwork() const;
    
    static bool isSupported(int featureId);
    
    XQSysInfo::Error error() const;
    
private: // From MNetworkSignalObserver
    void SignalStatusL(TInt32 aStrength, TInt8 aBars);

private: // From MBatteryObserver
    void BatteryLevelL(TUint aChargeLevel, CTelephony::TBatteryStatus aBatteryStatus);
    
private:
    XQSysInfo *q;
    CDeviceInfo* iDeviceInfo;
    
    mutable int iError;
};

#endif /*XQSYSINFOPRIVATE_H*/

// End of file

