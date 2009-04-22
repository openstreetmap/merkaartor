#ifndef XQACCESSPOINTMANAGER_STUB_P_H
#define XQACCESSPOINTMANAGER_STUB_P_H

// INCLUDES
#include "xqaccesspointmanager.h"

// FORWARD DECLARATIONS

// CLASS DECLARATION
class XQAccessPointManagerPrivate : public CBase
{
public:
    XQAccessPointManagerPrivate();
    ~XQAccessPointManagerPrivate();

    QList<XQAccessPoint> listIAPs() const;
    void setDefaultIAP(const XQAccessPoint& iap);
    const XQAccessPoint& defaultIAP() const;

private: // Data
    unsigned long int iap;
};

#endif /*XQACCESSPOINTMANAGER_S60_STUB_P_H*/

// End of file
