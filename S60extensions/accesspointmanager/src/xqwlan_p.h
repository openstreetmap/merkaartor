#ifndef XQWLAN_P_H
#define XQWLAN_P_H

class XQWLANPrivate
{
public:
    XQWLANPrivate()
        : ref(1),
          ssid(),
          networkMode(XQWLAN::WlanNetworkModeUnknown),
          securityMode(XQWLAN::WlanSecurityModeUnknown),
          visibility(false),
          usesPreSharedKey(false),
          signalStrength(-1)  {}
    XQWLANPrivate(const XQWLANPrivate &copy)
        : ref(1),
          ssid(copy.ssid),
          networkMode(copy.networkMode),
          securityMode(copy.securityMode),
          visibility(copy.visibility),
          usesPreSharedKey(copy.usesPreSharedKey),
          signalStrength(copy.signalStrength) {}
    ~XQWLANPrivate() {}
 
    QAtomicInt                 ref;
    QString                    ssid;
    QString                    mac;
    XQWLAN::WlanNetworkMode    networkMode;
    XQWLAN::WlanSecurityMode   securityMode;     
    bool                       visibility;       
    bool                       usesPreSharedKey;
    short int                  signalStrength;
};

#endif // XQWLAN_P_H