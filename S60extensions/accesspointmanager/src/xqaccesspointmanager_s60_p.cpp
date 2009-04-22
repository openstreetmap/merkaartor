// INCLUDE FILES
#include "xqaccesspointmanager_s60_p.h"
#include "xqwlan_p.h"

#include <e32base.h>
#include <commdb.h>
#include <es_sock.h>
#include <centralrepository.h>
#include <rconnmon.h>
#include <profileenginesdkcrkeys.h>
#include <favouritesdb.h>
#include <aputils.h>
#include <apaccesspointitem.h>
#include <apdatahandler.h>
#include <stdapis/sys/socket.h>
#include <stdapis/net/if.h>
#include <wlanmgmtclient.h> // from WLAN Management API Plugin
#include <wlanscaninfo.h>   // from WLAN Management API Plugin
#include <wlanmgmtcommon.h> // from WLAN Management API Plugin

//  CONSTANTS
#ifdef __WLAN_WEP256_ENABLED
    const TUint KValidWepKeyLengths[] = { 64, 128, 256 };   
    // WEP 256 is enabled -> max. length of key is 58 characters
    const TUint KMaxWepKeyLen = 58; 
#else
    const TUint KValidWepKeyLengths[] = { 64, 128 };
    // WEP 128 is enabled -> max. length of key is 26 characters
    const TUint KMaxWepKeyLen = 26; 
#endif  //__WLAN_WEP256_ENABLED

#ifdef __SERIES60_32__ 
    const TUint KWpaKeyMaxLength = 64;
#else
    const TUint KWpaKeyMaxLength = 63;
#endif
    
#define XQ_WLAN_SERVICE                _S("WLANServiceTable")
#define XQ_WLAN_SERVICE_ID             _S("ServiceID")                 // to which iap these settings belong to
#define XQ_WLAN_SECURITY_MODE          _S("WlanSecurityMode")          // Encryption type
#define XQ_WLAN_HEX_WEP_KEY1           _S("WlanWepKey1InHex")
#define XQ_WLAN_HEX_WEP_KEY2           _S("WlanWepKey2InHex")
#define XQ_WLAN_HEX_WEP_KEY3           _S("WlanWepKey3InHex")
#define XQ_WLAN_HEX_WEP_KEY4           _S("WlanWepKey4InHex")
#define XQ_WLAN_WEP_KEY1               _S("Wlan_Wep_Key1")
#define XQ_WLAN_WEP_KEY2               _S("Wlan_Wep_Key2")
#define XQ_WLAN_WEP_KEY3               _S("Wlan_Wep_Key3")
#define XQ_WLAN_WEP_KEY4               _S("Wlan_Wep_Key4")    
#define XQ_WLAN_WEP_KEY1_FORMAT        _S("WlanWepKey1Format")
#define XQ_WLAN_WEP_KEY2_FORMAT        _S("WlanWepKey2Format")
#define XQ_WLAN_WEP_KEY3_FORMAT        _S("WlanWepKey3Format")
#define XQ_WLAN_WEP_KEY4_FORMAT        _S("WlanWepKey4Format")
#define XQ_WLAN_WEP_INDEX              _S("WlanWepKeyIndex")        
#define XQ_WLAN_ENABLE_WPA_PSK         _S("UseWpaPreSharedKey")
#define XQ_WLAN_WPA_PRE_SHARED_KEY     _S("WlanWpaPreSharedKey")       // Shared key
#define XQ_WLAN_WPA_KEY_LENGTH         _S("WlanWpaKeyLength")          // Key length
#define XQ_WLAN_AUTHENTICATION_MODE3_2 _S("Wlan_Authentication_Mode")
#define XQ_WLAN_AUTHENTICATION_MODE    _S("WlanAuthenticationMode")


const TInt KUsedBitsAcsii    = 8;
const TInt KUsedBitsHex      = 4;
const TInt KFirstWepKey      = 0;
const TUint KWepKeyHeaderLen = 24; // the wep key contains 24 bit overhead

const TUint8 KWlan802Dot11SsidIE = 0;

void RollbackCommsDb(TAny* aCommsDb)
    {
    STATIC_CAST(CCommsDatabase*, aCommsDb)->RollbackTransaction();
    }

void CleanupRollbackPushL(CCommsDatabase& aCommsDb)
    {
    CleanupStack::PushL(TCleanupItem(RollbackCommsDb, &aCommsDb));
    }

void CancelView(TAny* aView)
{
    STATIC_CAST(CCommsDbTableView*, aView)->CancelRecordChanges();
}

void CleanupCancelPushL(CCommsDbTableView& aView)
{
    CleanupStack::PushL(TCleanupItem(CancelView, &aView));
}

XQAccessPointManagerPrivate::XQAccessPointManagerPrivate()
{
    if (iOpenCLibrary.Load(_L("libc")) == KErrNone) {
        iDynamicSetdefaultif = (TOpenCSetdefaultifFunction)iOpenCLibrary.Lookup(564);
    }
    ipCommsDB = CCommsDatabase::NewL(EDatabaseTypeIAP);
}

XQAccessPointManagerPrivate::~XQAccessPointManagerPrivate()
{
    iOpenCLibrary.Close();
    delete ipCommsDB;
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::accessPoints() const
{
    QList<XQAccessPoint> aps; 
    TRAPD(error, aps = accessPointsL());
    return aps;
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::accessPointsL() const
{
    QList<XQAccessPoint> aps;
    XQAccessPoint ap;

    //open internet accesspoint table
    CCommsDbTableView* pDbTView = ipCommsDB->OpenTableLC(TPtrC(IAP));

    // Loop through all IAPs
    TUint32 apId = 0;
    TInt retVal = pDbTView->GotoFirstRecord();
    while (retVal == KErrNone) {
        pDbTView->ReadUintL(TPtrC(COMMDB_ID), apId);
        ap = accessPointById(apId);
        if (!ap.isNull()) {
            aps << ap;
        }
        retVal = pDbTView->GotoNextRecord();
    }

    CleanupStack::PopAndDestroy(pDbTView);
    
    return aps;
}

XQAccessPoint XQAccessPointManagerPrivate::accessPointById(unsigned long int id) const
{
    XQAccessPoint ap;
    TRAPD(error, ap = accessPointByIdL(id));
    return ap;
}

XQAccessPoint XQAccessPointManagerPrivate::accessPointByIdL(unsigned long int id) const
{
    XQAccessPoint ap;

    CApDataHandler* pDataHandler = CApDataHandler::NewLC(*ipCommsDB); 
    
    CApAccessPointItem* pAPItem = CApAccessPointItem::NewLC(); 
    pDataHandler->AccessPointDataL(id,*pAPItem);
    TBuf<KCommsDbSvrMaxColumnNameLength> name;
    pAPItem->ReadTextL(EApIapName, name);
    const HBufC* pGprsName = pAPItem->ReadConstLongTextL(EApGprsAccessPointName);

    ap = XQAccessPoint(QString::fromUtf16(name.Ptr(),name.Length()),id);
    ap.setGprsName(QString::fromUtf16(pGprsName->Ptr(),pGprsName->Length()));
    
    switch (pAPItem->BearerTypeL()) {
        case EApBearerTypeCSD:      
            ap.setModemBearer(XQAccessPoint::ModemBearerCSD);
            break;
        case EApBearerTypeGPRS:
            ap.setModemBearer(XQAccessPoint::ModemBearerGPRS);
            break;
        case EApBearerTypeHSCSD:
            ap.setModemBearer(XQAccessPoint::ModemBearerHSCSD);
            break;
        case EApBearerTypeCDMA:
            ap.setModemBearer(XQAccessPoint::ModemBearerCDMA);
            break;
        case EApBearerTypeWLAN:
            ap.setModemBearer(XQAccessPoint::ModemBearerWLAN);
            break;
        case EApBearerTypeLAN:
            ap.setModemBearer(XQAccessPoint::ModemBearerLAN);
            break;
        case EApBearerTypeLANModem:
            ap.setModemBearer(XQAccessPoint::ModemBearerLANModem);
            break;
        default: ap.setModemBearer(XQAccessPoint::ModemBearerUnknown);
    }        
    
    CleanupStack::PopAndDestroy(pAPItem);
    CleanupStack::PopAndDestroy(pDataHandler);
    
    return ap;
}

bool XQAccessPointManagerPrivate::setDefaultAccessPoint(const XQAccessPoint& iap)
{
    if (!iDynamicSetdefaultif || iap.isNull()) {
        return false;
    }

    QList<XQAccessPoint> iaps = accessPoints();
    bool found = false; 
    for (int i=0; i < iaps.count(); i++) {
        if (iaps[i].id() == iap.id()) {
            found = true;
            break;
        }
    }
    if (!found) {
        return false;
    }

    // Use name of the IAP to set default IAP
    QByteArray nameAsByteArray = iap.name().toUtf8(); 
    ifreq ifr;
    strcpy(ifr.ifr_name, nameAsByteArray.constData());

    int error = iDynamicSetdefaultif(&ifr);
    
    if (error) {
        return false;
    }
    iUserSetDefaultAccessPoint = iap;
    return true;
}

const XQAccessPoint& XQAccessPointManagerPrivate::defaultAccessPoint() const
{
    return iUserSetDefaultAccessPoint;
}

bool XQAccessPointManagerPrivate::isSetDefaultAccessPointSupported() const
{
    if (!iDynamicSetdefaultif) {
        return false;
    }
    return true;
}

XQAccessPoint XQAccessPointManagerPrivate::systemAccessPoint() const
{
    XQAccessPoint ap;
    TRAPD(error, ap = systemAccessPointL());
    return ap;
}

XQAccessPoint XQAccessPointManagerPrivate::systemAccessPointL() const
{
    XQAccessPoint iap;

    RSocketServ serv;
    TInt retVal = serv.Connect();
    if (retVal != KErrNone) {
        return iap;
    }
    CleanupClosePushL(serv);
    RConnection conn;
    retVal = conn.Open(serv);
    if (retVal != KErrNone) {
        CleanupStack::PopAndDestroy(&serv);
        return iap;
    }
    CleanupClosePushL(conn);
    retVal = conn.Start();
    if (retVal != KErrNone) {
        CleanupStack::PopAndDestroy(&conn);
        CleanupStack::PopAndDestroy(&serv);
        return iap;
    }
    
    _LIT(KSetting, "IAP\\Id");
    unsigned long int iapId = 0;
    conn.GetIntSetting(KSetting, iapId);

    CleanupStack::PopAndDestroy(&conn);
    CleanupStack::PopAndDestroy(&serv);

    QList<XQAccessPoint> iaps = accessPoints();
    for (int i=0; i < iaps.count(); i++) {
        if (iaps[i].id() == iapId) {
            iap = iaps[i];
            break;
        }
    }
    
    return iap;
}

XQAccessPoint XQAccessPointManagerPrivate::preferredAccessPoint() const
{
    XQAccessPoint ap;
    TRAPD(error, ap = preferredAccessPointL());
    return ap;
}

XQAccessPoint XQAccessPointManagerPrivate::preferredAccessPointL() const
{
    XQAccessPoint iap;

    // 1. Check active connections
    QList<XQAccessPoint> iaps = activeAccessPoints();
    if (iaps.count() > 0) {
        // Active connections found
        // Try to find active WLAN connection
        for (int i=0; i < iaps.count(); i++) {
            if (iaps[i].modemBearer() == XQAccessPoint::ModemBearerWLAN) {
                iap = iaps[0];
                break;
            }
        }
        if (iap.isNull()) {
            // Active WLAN connection was not found
            // => Use first reported active IAP
            iap = iaps[0];
        }
    }
    
    iaps = availableAccessPoints();
    if (iap.isNull()) {
        // 2. Check available WLAN IAPS
        // Try to find WLAN IAP from available IAPS
        for (int i=0; i < iaps.count(); i++) {
            if (iaps[i].modemBearer() == XQAccessPoint::ModemBearerWLAN) {
                iap = iaps[i];
                break;
            }
        }
    }
    
    if (iap.isNull()) {
        // 3. Check GPRS/3G
        // Check whether device is offline or online
        TInt currentProfileId;
        CRepository* pRepository = CRepository::NewL(KCRUidProfileEngine);
        pRepository->Get(KProEngActiveProfile, currentProfileId);
        delete pRepository;
        if (currentProfileId != 5) { // 5 = Off-line profile
            for (int i=0; i < iaps.count(); i++) {
                if (iaps[i].gprsName() == QString("internet")) {
                    iap = iaps[i];
                    break;
                }
            }
            if (iap.isNull() && iaps.count() > 0) {
                iap = iaps[0];
            }
        }
    }
    
    return iap;
}

QList<XQWLAN> XQAccessPointManagerPrivate::availableWLANs() const
{
    QList<XQWLAN> wlans; 
    TRAPD(error, wlans = availableWLANsL());
    return wlans;
}

QList<XQWLAN> XQAccessPointManagerPrivate::availableWLANsL() const
{
    QList<XQWLAN> wlans;
    
    RConnectionMonitor monitor;
    monitor.ConnectL();
    CleanupClosePushL(monitor);
    
    TRequestStatus status;
    TPckgBuf<TConnMonNetworkNames> pkgNetworks;
    monitor.GetPckgAttribute(EBearerIdWLAN, 0, KNetworkNames, pkgNetworks, status);
    User::WaitForRequest( status ) ;

    if (status.Int() == KErrNone) {
        for(TUint i=0; i<pkgNetworks().iCount; i++)
        {
            XQWLAN wlan;
            wlan.d = new XQWLANPrivate();

            TBuf<1000> buf16;
            buf16.Copy(pkgNetworks().iNetwork[i].iName);
            wlan.d->ssid = QString::fromUtf16(buf16.Ptr(),buf16.Length());

            wlan.d->signalStrength = pkgNetworks().iNetwork[i].iSignalStrength;

            wlans << wlan;
        }
    }

    CleanupStack::PopAndDestroy(&monitor);
    
    return wlans;
}

QList<XQWLAN> XQAccessPointManagerPrivate::availableWLANs2() const
{
    QList<XQWLAN> wlans; 
    TRAPD(error, wlans = availableWLANs2L());
    return wlans;
}

QList<XQWLAN> XQAccessPointManagerPrivate::availableWLANs2L() const
{
    QList<XQWLAN> wlans;

    TWlanConnectionSecurityMode securityMode;
    TUint8 ieLen = 0;
    const TUint8* ieData = NULL;
    TBuf<1000> buf16;
    TUint16 capability;
    TInt lanType;
    TWlanBssid bssid;
    TInt retVal;
    
    CWlanScanInfo* pScanInfo = CWlanScanInfo::NewL();
    CleanupStack::PushL(pScanInfo);
    CWlanMgmtClient* pMgmtClient = CWlanMgmtClient::NewL();
    CleanupStack::PushL(pMgmtClient);
    pMgmtClient->GetScanResults(*pScanInfo);

    for(pScanInfo->First(); !pScanInfo->IsDone(); pScanInfo->Next() ) {
        XQWLAN wlan;
        wlan.d = new XQWLANPrivate();
    
        securityMode = pScanInfo->SecurityMode();
        switch (securityMode) {
            case EWlanConnectionSecurityOpen:
                wlan.d->securityMode = XQWLAN::WlanSecurityModeOpen;
                break;
            case EWlanConnectionSecurityWep:
                wlan.d->securityMode = XQWLAN::WlanSecurityModeWep;
                break;
            case EWlanConnectionSecurity802d1x:
                wlan.d->securityMode = XQWLAN::WlanSecurityMode802_1x;
                break;
            case EWlanConnectionSecurityWpa:
                wlan.d->securityMode = XQWLAN::WlanSecurityModeWpa;
                break;
            case EWlanConnectionSecurityWpaPsk:
                wlan.d->securityMode = XQWLAN::WlanSecurityModeWpa2;
                break;
        }
        
        //In general if the least significant bit of the capability field
        //is 1 then the network is of "Infrastructure Type" or else if the
        //capability field is 0 then the network is of "Ad-hoc type"
        capability = pScanInfo->Capability();
        lanType = capability & 1;
        if (lanType == 1) {
            wlan.d->networkMode = XQWLAN::WlanNetworkModeInfra;
        } else {
            wlan.d->networkMode = XQWLAN::WlanNetworkModeAdhoc;
        }
        
        wlan.d->signalStrength = pScanInfo->RXLevel();
        
        retVal = pScanInfo->InformationElement(KWlan802Dot11SsidIE, ieLen, &ieData);
        if (retVal == KErrNone) {
            TPtrC8 ptr(ieData, ieLen);
            buf16.Copy(ptr);
            wlan.d->ssid = QString::fromUtf16(buf16.Ptr(),buf16.Length());
        }
        
        pScanInfo->Bssid(bssid);
        buf16.SetLength(0);
        for(TInt i = 0; i < bssid.Length(); i++) {
            buf16.AppendFormat(_L("%02x:"), bssid[i]);
        }
        if (buf16.Length() > 0) {
            buf16.Delete(buf16.Length()-1, 1);
        }
        wlan.d->mac = QString::fromUtf16(buf16.Ptr(),buf16.Length());
        
        wlans << wlan;
    }

    CleanupStack::PopAndDestroy(pMgmtClient);    
    CleanupStack::PopAndDestroy(pScanInfo);
    
    return wlans;    
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::availableAccessPoints() const
{
    QList<XQAccessPoint> aps; 
    TRAPD(error, aps = availableAccessPointsL());
    return aps;
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::availableAccessPointsL() const
{
    QList<XQAccessPoint> iaps;

    RConnectionMonitor monitor;
    monitor.ConnectL();
    CleanupClosePushL(monitor);
    
    TRequestStatus status;
    TPckgBuf<TConnMonNetworkNames> pkgNetworks;
    TConnMonIapInfoBuf iapBuf;
    monitor.GetPckgAttribute(EBearerIdAll, 0, KIapAvailability, iapBuf, status);
    User::WaitForRequest(status) ;

    if (status.Int() == KErrNone) {
        XQAccessPoint ap;
        for(TUint i=0; i<iapBuf().iCount; i++) {
            ap = accessPointById(iapBuf().iIap[i].iIapId);
            if (!ap.isNull()) {
                iaps << ap;
            }
        }
    }
    CleanupStack::PopAndDestroy(&monitor);
    
    return iaps;
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::activeAccessPoints() const
{
    QList<XQAccessPoint> aps; 
    TRAPD(error, aps = activeAccessPointsL());
    return aps;
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::activeAccessPointsL() const
{
    QList<XQAccessPoint> iaps;

    RConnectionMonitor monitor;
    monitor.ConnectL();
    CleanupClosePushL(monitor);

    TRequestStatus status;
    TUint connectionCount;
    monitor.GetConnectionCount(connectionCount, status);
    User::WaitForRequest(status);
    
    TUint connectionId;
    TUint subConnectionCount;
    TUint apId;
    if (status.Int() == KErrNone) {
        XQAccessPoint ap;
        for (TInt i = 1; i <= connectionCount; i++) {
            monitor.GetConnectionInfo(i, connectionId, subConnectionCount);
            monitor.GetUintAttribute(connectionId, subConnectionCount, KIAPId, apId, status);
            User::WaitForRequest(status);
            ap = accessPointById(apId);
            if (!ap.isNull()) {
                iaps << ap;
            }
        }
    }
    CleanupStack::PopAndDestroy(&monitor);

    return iaps;
}

unsigned long int XQAccessPointManagerPrivate::createWLANAccessPoint(const QString& name,
                                                                       const XQWLAN& wlan,
                                                                       const QString& preSharedKey)
{
    TPtrC16 nameDesc(reinterpret_cast<const TUint16*>(name.utf16()));
    TPtrC16 preSharedKeyDesc(reinterpret_cast<const TUint16*>(preSharedKey.utf16()));
    unsigned long int newIAPId = 0;
    TRAPD(error, newIAPId = createAccessPointL(nameDesc,wlan,preSharedKeyDesc));
    if (error != KErrNone) {
        newIAPId = 0;
    }
    return newIAPId;
}


void XQAccessPointManagerPrivate::storeWPADataL(const TInt aIapId, const TDesC& aPresharedKey, const XQWLAN& aWlan)
{
    CCommsDbTableView* wLanServiceTable;
    
    CApUtils* apUtils = CApUtils::NewLC(*ipCommsDB);
    TUint32 iapId = apUtils->IapIdFromWapIdL(aIapId);
    CleanupStack::PopAndDestroy(apUtils);

    TUint32 serviceId;

    CCommsDbTableView* iapTable = ipCommsDB->OpenViewMatchingUintLC(TPtrC(IAP),
                                                                    TPtrC(COMMDB_ID),
                                                                    iapId);
    User::LeaveIfError(iapTable->GotoFirstRecord());
    iapTable->ReadUintL(TPtrC(IAP_SERVICE), serviceId);
    CleanupStack::PopAndDestroy( iapTable );

    wLanServiceTable = ipCommsDB->OpenViewMatchingUintLC(TPtrC(XQ_WLAN_SERVICE),
                                                         TPtrC(XQ_WLAN_SERVICE_ID),
                                                         serviceId );
    TInt errorCode = wLanServiceTable->GotoFirstRecord();
    if (errorCode == KErrNone) {
        User::LeaveIfError(wLanServiceTable->UpdateRecord());
    } else {
        TUint32 dummyUid(0);
        User::LeaveIfError(wLanServiceTable->InsertRecord(dummyUid));
        wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_SERVICE_ID), aIapId);
    }
    CleanupCancelPushL(*wLanServiceTable);

    TBool usesPsk(aWlan.usesPreSharedKey());
    // Save WPA Mode
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_ENABLE_WPA_PSK), usesPsk); 
    
    // Save security mode
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_SECURITY_MODE),
                                 fromQtSecurityModeToS60SecurityMode(aWlan.securityMode())); 

    // Save PreShared Key
    TBuf8<KWpaKeyMaxLength> keyWPA;
    
    //convert to 8 bit
    keyWPA.Copy(aPresharedKey);
    wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WPA_PRE_SHARED_KEY),keyWPA);

    // Check and save PreShared Key Length
    TInt len(keyWPA.Length());
        
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WPA_KEY_LENGTH),len);

    User::LeaveIfError(wLanServiceTable->PutRecordChanges());

    CleanupStack::Pop(wLanServiceTable); // table rollback...
    CleanupStack::PopAndDestroy(wLanServiceTable);
}

void XQAccessPointManagerPrivate::storeWEPDataL(const TInt aIapId, const TDesC& aPresharedKey)
{
    CCommsDbTableView* wLanServiceTable;

    CApUtils* apUtils = CApUtils::NewLC(*ipCommsDB);
    TUint32 iapId = apUtils->IapIdFromWapIdL(aIapId);
    CleanupStack::PopAndDestroy(apUtils);

    TUint32 serviceId;

    CCommsDbTableView* iapTable = ipCommsDB->OpenViewMatchingUintLC(TPtrC(IAP),
                                                                    TPtrC(COMMDB_ID),
                                                                    iapId);
                            
    User::LeaveIfError(iapTable->GotoFirstRecord());
    iapTable->ReadUintL(TPtrC(IAP_SERVICE), serviceId);
    CleanupStack::PopAndDestroy(iapTable);

    wLanServiceTable = ipCommsDB->OpenViewMatchingUintLC(TPtrC(XQ_WLAN_SERVICE),
                                                         TPtrC(XQ_WLAN_SERVICE_ID),
                                                         serviceId);
    TInt errorCode = wLanServiceTable->GotoFirstRecord();
    if (errorCode == KErrNone) {
        User::LeaveIfError(wLanServiceTable->UpdateRecord());
    }
    else {
        TUint32 dummyUid = 0;
        User::LeaveIfError(wLanServiceTable->InsertRecord(dummyUid));
        wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_SERVICE_ID), aIapId);
    }
    
    CleanupCancelPushL(*wLanServiceTable);

    // Save index of key in use
    TUint32 keyInUse(KFirstWepKey);
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WEP_INDEX), keyInUse);

    // Save authentication mode
    TUint32 auth(0); // set to open... 
    if (isS60VersionGreaterThan3_1()) {  
        //TODO: wLanServiceTable->WriteUintL(TPtrC(NU_WLAN_AUTHENTICATION_MODE), auth);
    } else {
        wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_AUTHENTICATION_MODE), auth);
    }
    // not we need to convert the key.... to 8bit and to hex... and again detect the required bits..
    TBuf8<KMaxWepKeyLen> key;
    
    //convert to 8 bit
    key.Copy(aPresharedKey);

    TBool useHex(EFalse);
    TWepKeyLength keyLength;
    TBool validKey = validWepKeyLength(aPresharedKey, useHex, keyLength);
    
    if (!useHex) {
        // Must be converted to hexa and stored as a hexa
        // Ascii key is half the length of Hex
        HBufC8* buf8Conv = HBufC8::NewLC(key.Length() * 2);
        asciiToHex(key, buf8Conv);
        
        if (isS60VersionGreaterThan3_1()) {  
            wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_HEX_WEP_KEY1), buf8Conv->Des());
        } else {
            wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WEP_KEY1), buf8Conv->Des());
        }
        CleanupStack::PopAndDestroy(buf8Conv);
    } else if (isHex(aPresharedKey)) {
        //already in hexa format
        if (isS60VersionGreaterThan3_1()) {  
            wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_HEX_WEP_KEY1), key);
        } else {
            wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WEP_KEY1), key);
        }
    }
  
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WEP_KEY1_FORMAT), useHex);
  
    key.Zero();
    // write default values to the rest of the columns
    if (isS60VersionGreaterThan3_1()) {  
        wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_HEX_WEP_KEY2), 
                                     key);
    } else { 
        wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WEP_KEY2), 
                                     key );
    }
    // Save third WEP key
    if (isS60VersionGreaterThan3_1()) {  
        wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_HEX_WEP_KEY3), 
                                     key);
    } else { 
        wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WEP_KEY3), 
                                     key);
    }
    // Save fourth WEP key
    if (isS60VersionGreaterThan3_1()) {  
        //TODO: wLanServiceTable->WriteTextL(TPtrC(NU_WLAN_WEP_KEY4), 
        //                             key);
    } else {
        wLanServiceTable->WriteTextL(TPtrC(XQ_WLAN_WEP_KEY4), 
                                     key);
    }
    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WEP_KEY2_FORMAT), 
                                 (TUint32&)useHex);

    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WEP_KEY3_FORMAT), 
                                 (TUint32&)useHex);

    wLanServiceTable->WriteUintL(TPtrC(XQ_WLAN_WEP_KEY4_FORMAT), 
                                 (TUint32&)useHex);
 
    wLanServiceTable->PutRecordChanges();

    CleanupStack::Pop(wLanServiceTable); // table rollback...
    CleanupStack::PopAndDestroy(wLanServiceTable);
}

TUint32 XQAccessPointManagerPrivate::makeIapL(const TDesC& aIapName, const XQWLAN& aWlan)
{
    TUint32 iapId(0);
    
    CApAccessPointItem* apItem = CApAccessPointItem::NewLC();

    apItem->SetBearerTypeL(EApBearerTypeWLAN);
    if (aIapName.Length()) {
        apItem->SetNamesL(aIapName);
        apItem->WriteTextL(EApWlanNetworkName, aIapName);
    } else {
        TPtrC16 ssidDesc(reinterpret_cast<const TUint16*>(aWlan.name().utf16()));
        apItem->SetNamesL(ssidDesc);
        apItem->WriteTextL(EApWlanNetworkName, ssidDesc);
    }
    // set the wlan security mode
    apItem->WriteUint(EApWlanSecurityMode,
                      fromQtSecurityModeToS60SecurityMode(aWlan.securityMode()));
    
    // set the wlan network structure...
    apItem->WriteUint(EApWlanNetworkMode,
                      fromQtNetworkModeToS60NetworkMode(aWlan.networkMode())) ;
    
    apItem->WriteBool(EApWlanScanSSID, !aWlan.isVisible()) ;
    
    CApDataHandler* dataHandler = CApDataHandler::NewLC(*ipCommsDB);
    if (apItem->SanityCheckOk()) {
        iapId = dataHandler->CreateFromDataL(*apItem);
    }

    CleanupStack::PopAndDestroy( dataHandler );
    CleanupStack::PopAndDestroy( apItem );
    
    return iapId;
}

TUint32 XQAccessPointManagerPrivate::createAccessPointL(const TDesC& aAccessPointName,
                                                          const XQWLAN& aWlan, 
                                                          const TDesC& aPresharedKey)
{
    TUint32 iapId;

    if (isS60VersionGreaterThan3_1()) {
        if (fromQtSecurityModeToS60SecurityMode(aWlan.securityMode()) != EOpen )
            {
            User::LeaveIfError(ipCommsDB->BeginTransaction());
            }
    } else {
        User::LeaveIfError(ipCommsDB->BeginTransaction());
    }

    CleanupRollbackPushL(*ipCommsDB);
    
    iapId = makeIapL(aAccessPointName, aWlan);
    switch (fromQtSecurityModeToS60SecurityMode(aWlan.securityMode()))
        {
        case EWep:
            {
            storeWEPDataL(iapId, aPresharedKey);
            break;
            }
        case E802_1x:
        case EWpa:
        case EWpa2:
            {
            storeWPADataL(iapId, aPresharedKey, aWlan);
            break;
            }
        case EOpen: // fall through on purpose though we're done for open network
        default:
            break;
            }

    CleanupStack::Pop(); // db rollback..

    if (isS60VersionGreaterThan3_1()) { 
        if (fromQtSecurityModeToS60SecurityMode(aWlan.securityMode()) != EOpen) {
            ipCommsDB->CommitTransaction();
        }
    } else {
        ipCommsDB->CommitTransaction();
    }

    CApUtils* apUtils = CApUtils::NewLC(*ipCommsDB);
    TUint32 iapTableiapId = apUtils->IapIdFromWapIdL(iapId);
    CleanupStack::PopAndDestroy( apUtils );
    return iapTableiapId;   
}

void XQAccessPointManagerPrivate::asciiToHex(const TDesC8& aSource, 
                                               HBufC8*& aDest)
{
    _LIT(hex, "0123456789ABCDEF");
    TInt size = aSource.Size();
    TPtr8 ptr = aDest->Des();
    for (TInt ii = 0; ii < size; ii++) {
        TText8 ch = aSource[ii];
        ptr.Append( hex()[(ch/16)&0x0f] );
        ptr.Append( hex()[ch&0x0f] );
    }
}
    
TBool XQAccessPointManagerPrivate::validWepKeyLength(const TDesC& aKey, 
                                                       TBool& aHex,
                                                       TWepKeyLength &aKeyLen)
{
    TInt keyLen = aKey.Length();
    if (keyLen == 0) {
        return EFalse;
    }
    TInt arrayCount = sizeof(KValidWepKeyLengths) / sizeof(TUint);
    
    for (TInt i(0) ; i < arrayCount ; i++) {
        if (keyLen * KUsedBitsAcsii == (KValidWepKeyLengths[i] - KWepKeyHeaderLen)) {
            aKeyLen = (TWepKeyLength)i;
            aHex = EFalse;
            return ETrue;
        }
        if (keyLen * KUsedBitsHex == (KValidWepKeyLengths[i] - KWepKeyHeaderLen)) {
            aKeyLen = (TWepKeyLength)i;
            aHex = ETrue;
            return ETrue;
        }
    } 
    return EFalse;  
}
    
TBool XQAccessPointManagerPrivate::isHex(const TDesC& aKey)
{
    TBool err( ETrue );
    
    for ( TInt i = 0; i < aKey.Length(); i++ ) {
        TChar c( aKey[i] );
        if ( !c.IsHexDigit() ) {
            err = EFalse;
            break;
        }
    }

    return err;
}

XQWLAN::WlanNetworkMode XQAccessPointManagerPrivate::fromS60NetworkModeToQtNetworkMode(TWlanNetMode aNetworkMode)
{
    switch (aNetworkMode) {
        case EAdhoc: return XQWLAN::WlanNetworkModeAdhoc;
        case EInfra: return XQWLAN::WlanNetworkModeInfra;
    }
    return XQWLAN::WlanNetworkModeUnknown;
}

XQWLAN::WlanSecurityMode XQAccessPointManagerPrivate::fromS60SecurityModeToQtSecurityMode(TWlanSecMode aNetworkMode)
{
    switch (aNetworkMode) {
        case EOpen:   return XQWLAN::WlanSecurityModeOpen;
        case EWep:    return XQWLAN::WlanSecurityModeWep;
        case E802_1x: return XQWLAN::WlanSecurityMode802_1x;
        case EWpa:    return XQWLAN::WlanSecurityModeWpa;
        case EWpa2:   return XQWLAN::WlanSecurityModeWpa2;
    }
    return XQWLAN::WlanSecurityModeUnknown;
}

TWlanNetMode XQAccessPointManagerPrivate::fromQtNetworkModeToS60NetworkMode(XQWLAN::WlanNetworkMode aNetworkMode)
{
    switch (aNetworkMode) {
        case XQWLAN::WlanNetworkModeAdhoc: return EAdhoc;
        case XQWLAN::WlanNetworkModeInfra: return EInfra;
    }
    
    return EAdhoc;
}

TWlanSecMode XQAccessPointManagerPrivate::fromQtSecurityModeToS60SecurityMode(XQWLAN::WlanSecurityMode aNetworkMode)
{
    switch (aNetworkMode) {
        case XQWLAN::WlanSecurityModeOpen:   return EOpen;
        case XQWLAN::WlanSecurityModeWep:    return EWep;
        case XQWLAN::WlanSecurityMode802_1x: return E802_1x;
        case XQWLAN::WlanSecurityModeWpa:    return EWpa;
        case XQWLAN::WlanSecurityModeWpa2:   return EWpa2;
    }
    
    return EOpen;
}

TBool XQAccessPointManagerPrivate::s60PlatformVersion(TUint& aMajor, TUint& aMinor) const
{
    if (iPlatformVersionMajor != 0) {
        aMajor = iPlatformVersionMajor;
        aMinor = iPlatformVersionMinor;
        return ETrue;
    }

    RFs fs;
    if (fs.Connect() != KErrNone) {
        return EFalse;
    }
    CleanupClosePushL(fs);
     
    // Obtain the version number
    TFindFile fileFinder = fs;
    CDir* pResult;
 
    _LIT(KS60ProductIDFile, "Series60v*.sis");
    _LIT(KROMInstallDir, "z:\\system\\install\\");
    
    if (fileFinder.FindWildByDir(KS60ProductIDFile, KROMInstallDir, pResult) != KErrNone) {
        CleanupStack::PopAndDestroy(&fs);
        return EFalse;
    }
    CleanupStack::PushL(pResult);
    
    // Sort the file names so that the newest platforms are first
    if (pResult->Sort(ESortByName | EDescending) != KErrNone) {
        CleanupStack::PopAndDestroy(pResult);
        CleanupStack::PopAndDestroy(&fs);
        return EFalse;
    }
 
    // Parse the version numbers from the file name (e.g. Series60v3_1.sis)
    aMajor = (*pResult)[0].iName[9] - '0';
    aMinor = (*pResult)[0].iName[11] - '0';
    CleanupStack::PopAndDestroy(pResult);
    CleanupStack::PopAndDestroy(&fs);
    
    iPlatformVersionMajor = aMajor;
    iPlatformVersionMinor = aMinor;
    return ETrue;
}

TBool XQAccessPointManagerPrivate::isS60VersionGreaterThan3_1() const
{
    TUint major = 3;
    TUint minor = 1;
    s60PlatformVersion(major, minor);
    if ((major == 3 && minor > 1) || (major > 3)) {
        return true;
    }
    return false;
}

// End of file

