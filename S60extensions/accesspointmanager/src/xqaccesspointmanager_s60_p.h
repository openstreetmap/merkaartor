#ifndef XQACCESSPOINTMANAGER_S60_P_H
#define XQACCESSPOINTMANAGER_S60_P_H

// INCLUDES
#include "xqaccesspointmanager.h"
#include <apengineconsts.h>

// FORWARD DECLARATIONS
class TWLanInfo;
class CCommsDatabase;
class CCommsDbTableView;

typedef int(*TOpenCSetdefaultifFunction)(const struct ifreq*);

// CLASS DECLARATION
class XQAccessPointManagerPrivate : public CBase
{
public:
    enum TWepKeyLength {
#ifdef __WLAN_WEP256_ENABLED
            E64Bits,
            E128Bits,
            E256Bits
#else
            E64Bits,
            E128Bits
#endif  //__WLAN_WEP256_ENABLED
    };    
    
    XQAccessPointManagerPrivate();
    ~XQAccessPointManagerPrivate();

    QList<XQAccessPoint> accessPoints() const;
    QList<XQAccessPoint> availableAccessPoints() const;
    QList<XQAccessPoint> activeAccessPoints() const;

    XQAccessPoint systemAccessPoint() const;
    XQAccessPoint preferredAccessPoint() const;

    XQAccessPoint accessPointById(unsigned long int id) const;
    
    bool setDefaultAccessPoint(const XQAccessPoint& accessPoint);
    const XQAccessPoint& defaultAccessPoint() const;
    bool isSetDefaultAccessPointSupported() const;

    QList<XQWLAN> availableWLANs() const;
    QList<XQWLAN> availableWLANs2() const;

    unsigned long int createWLANAccessPoint(const QString& name,
                                            const XQWLAN& wlan,
                                            const QString& preSharedKey);    
private:
    QList<XQAccessPoint> accessPointsL() const;
    QList<XQAccessPoint> availableAccessPointsL() const;
    QList<XQAccessPoint> activeAccessPointsL() const;
    QList<XQWLAN> availableWLANsL() const;
    QList<XQWLAN> availableWLANs2L() const;

    XQAccessPoint systemAccessPointL() const;
    XQAccessPoint preferredAccessPointL() const;

    void storeWPADataL(const TInt aIapId, const TDesC& aPresharedKey, const XQWLAN& aWlan);    
    void storeWEPDataL(const TInt aIapId, const TDesC& aPresharedKey);    
    TUint32 makeIapL(const TDesC& aIapName, const XQWLAN& wlan);    
    TUint32 createAccessPointL(const TDesC& aAccessPointName, const XQWLAN& wlan,
                               const TDesC& aPresharedKey);
    void asciiToHex(const TDesC8& aSource,HBufC8*& aDest);
    TBool validWepKeyLength(const TDesC& aKey, TBool& aHex, TWepKeyLength &aKeyLen);    
    TBool isHex(const TDesC& aKey);
    
    void wlanInfo(XQWLAN& wlan);

    XQAccessPoint accessPointByIdL(unsigned long int id) const;
    
    XQWLAN::WlanNetworkMode fromS60NetworkModeToQtNetworkMode(TWlanNetMode aNetworkMode);
    XQWLAN::WlanSecurityMode fromS60SecurityModeToQtSecurityMode(TWlanSecMode aNetworkMode);
    TWlanNetMode fromQtNetworkModeToS60NetworkMode(XQWLAN::WlanNetworkMode aNetworkMode);
    TWlanSecMode fromQtSecurityModeToS60SecurityMode(XQWLAN::WlanSecurityMode aNetworkMode);
    
    TBool s60PlatformVersion(TUint& aMajor, TUint& aMinor) const;
    TBool isS60VersionGreaterThan3_1() const;
    
private: // Data
    CCommsDatabase*            ipCommsDB;
    XQAccessPoint              iUserSetDefaultAccessPoint;
    RLibrary                   iOpenCLibrary;
    TOpenCSetdefaultifFunction iDynamicSetdefaultif;
    mutable TUint              iPlatformVersionMajor;
    mutable TUint              iPlatformVersionMinor;
};

#endif /*XQACCESSPOINTMANAGER_S60_P_H*/

// End of file

