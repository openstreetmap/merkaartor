#ifndef XQLOCATION_STUB_P_H_
#define XQLOCATION_STUB_P_H_

// INCLUDES
#include "private/qobject_p.h"
#include "xqlocation.h"

// CLASS DECLARATION
class XQLocationPrivate: public QObjectPrivate
{
    Q_DECLARE_PUBLIC(XQLocation)

public:
    XQLocationPrivate();
    ~XQLocationPrivate();
    
    void requestUpdate();
    void startUpdates();
    void startUpdates(int msec);
    void stopUpdates();
    
    XQLocation::Error OpenConnectionToPositioner();
    void CloseConnectionToPositioner();

public: // Data
    int                          updateInterval;
    XQLocation::DeviceStatus     status;
    XQLocation::DataQuality      dataQuality;
    int                          error;
};

#endif /*XQLOCATION_S60_STUB_P_H_*/

// End of file
