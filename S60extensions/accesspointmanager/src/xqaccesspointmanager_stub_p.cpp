// INCLUDE FILES
#include "xqaccesspointmanager.h"
#include "xqaccesspointmanager_stub_p.h"

XQAccessPointManagerPrivate::XQAccessPointManagerPrivate()
{
}

XQAccessPointManagerPrivate::~XQAccessPointManagerPrivate()
{
}

QList<XQAccessPoint> XQAccessPointManagerPrivate::listIAPs() const
{
}

void XQAccessPointManagerPrivate::setDefaultIAP(const XQAccessPoint& iap)
{
    iap = iapId;
}

const XQAccessPoint& XQAccessPointManagerPrivate::defaultIAP() const
{
    return iap;
}

// End of file

