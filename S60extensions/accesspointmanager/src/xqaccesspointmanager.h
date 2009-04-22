#ifndef XQACCESSPOINTMANAGER_H
#define XQACCESSPOINTMANAGER_H

#include <QObject>
#include <QString>
#include <QList>

// FORWARD DECLARATIONS
class XQAccessPointManagerPrivate;
class XQAccessPoint;
class XQAccessPointPrivate;
class XQWLANPrivate;
class XQWLAN;

class XQAccessPointManager : public QObject
{
    Q_OBJECT

public:
    XQAccessPointManager(QObject *parent = 0);
    ~XQAccessPointManager();

    QList<XQAccessPoint> accessPoints() const;
    QList<XQAccessPoint> availableAccessPoints() const;
    QList<XQAccessPoint> activeAccessPoints() const;
    
    XQAccessPoint systemAccessPoint() const;
    XQAccessPoint preferredAccessPoint() const;

    XQAccessPoint accessPointById(unsigned long int id) const;
    
    bool removeAccessPoint(const XQAccessPoint& iap);
    
    const XQAccessPoint& defaultAccessPoint() const;
    bool isSetDefaultAccessPointSupported() const;
    
    QList<XQWLAN> availableWLANs() const;    

public Q_SLOTS:
    bool setDefaultAccessPoint(const XQAccessPoint& iap);

private: //Data
	XQAccessPointManagerPrivate* d;
};

class XQAccessPoint
{
public:
    enum ModemBearer {
        ModemBearerCSD,
        ModemBearerGPRS,
        ModemBearerHSCSD,
        ModemBearerCDMA,
        ModemBearerWLAN,
        ModemBearerLAN,
        ModemBearerLANModem,
        ModemBearerUnknown = -1
    };

	XQAccessPoint();
	XQAccessPoint(const QString& iapName, unsigned long int iapId);
	XQAccessPoint(const XQAccessPoint &other);
    ~XQAccessPoint();

    void setId(unsigned long int iapId);
    unsigned long int id() const;
    void setName(const QString& name);
    QString name() const;
    void setGprsName(const QString& name);
    QString gprsName() const;
    void setModemBearer(XQAccessPoint::ModemBearer bearer);
    XQAccessPoint::ModemBearer modemBearer() const;
    
    bool isNull() const;
    XQAccessPoint& operator=(const XQAccessPoint& other);

private:
    void detach();

private:
    XQAccessPointPrivate* d;
};

class XQWLAN
{
public:
    enum WlanNetworkMode
        {
        WlanNetworkModeAdhoc   = 0,
        WlanNetworkModeInfra   = 1,
        WlanNetworkModeUnknown = -1
        };
    
    enum WlanSecurityMode
        {
        WlanSecurityModeOpen    = 1,
        WlanSecurityModeWep     = 2,
        WlanSecurityMode802_1x  = 4,
        WlanSecurityModeWpa     = 8,
        WlanSecurityModeWpa2    = 16,
        WlanSecurityModeUnknown = -1
        };

    XQWLAN();
    XQWLAN(const XQWLAN &other);
    ~XQWLAN();

    QString name() const;
    QString macAddress() const;
    short int signalStrength() const;
    bool usesPreSharedKey() const;
    WlanNetworkMode networkMode() const;
    WlanSecurityMode securityMode() const;
    bool isVisible() const;

    bool isNull() const;
    XQWLAN& operator=(const XQWLAN& other);
    
private:
    void detach();

private:
    XQWLANPrivate* d;
    
    friend class XQAccessPointManagerPrivate;
};

#endif // XQACCESSPOINTMANAGER_H

// End of file

