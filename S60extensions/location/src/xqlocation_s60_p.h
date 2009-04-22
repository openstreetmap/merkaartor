#ifndef XQLOCATION_S60_P_H
#define XQLOCATION_S60_P_H

// INCLUDES
#include "xqlocation.h"
#include <Lbs.h>
#include <LbsSatellite.h> 
#include <e32cmn.h> //RArray

// FORWARD DECLARATIONS
class CPositionServerObserver;
class CPositionerObserver;

// CLASS DECLARATION
class XQLocationPrivate: public CBase
{
public:
    XQLocationPrivate(XQLocation* apParent);
    ~XQLocationPrivate();
    
    void requestUpdate();
    void startUpdates();
    void startUpdates(int msec);
    void stopUpdates();    

    XQLocation::Error openConnectionToPositioner();
    void closeConnectionToPositioner();

private:
	void PositionServerReady();
    void DeliverError(XQLocation::Error aError);
    void DeliverPositionServerResults(TPositionModuleStatus aModuleStatus);
    void DeliverPositionerResults(TPositionSatelliteInfo aPositionInfo);
    
public: // Data
	int                        updateInterval;
    XQLocation::DeviceStatus   status;
    XQLocation::DataQuality    dataQuality;
    bool                       firstStatusUpdate;
    bool                       speedAvailable;
    bool                       satelliteInfoAvailable;

private: // Data
	XQLocation*                ipParent;

	bool                       iSingleUpdate;
    RArray<TPosition>          iPositions;
    
    TPosition                  iPreviousPosition;
    float                      iPreviousSpeed;
    int                        iPreviousNumSatellitesInView;
    int                        iPreviousNumSatellitesUsed;

    CPositionServerObserver*   ipPositionServerObserver;
    CPositionerObserver*       ipPositionerObserver;
    
    friend class CPositionServerObserver;
    friend class CPositionerObserver;
};

class CPositionServerObserver : public CActive
{
public:
    static CPositionServerObserver* NewL(XQLocationPrivate* apParent);
    ~CPositionServerObserver();
    
    XQLocation::Error Init();
    void StartListening();
    void StopListening();
    
    TPositionModuleId PositionModuleId();
    RPositionServer& PositionServer();
    
private: // From CActive
    void RunL();
    TInt RunError(TInt aError);
    void DoCancel();

private: 
	CPositionServerObserver(XQLocationPrivate* apParent);

private:
    RPositionServer            iPositionServer;
    TPositionModuleId          iPositionModuleId;
    TPositionModuleStatus      iModuleStatus;
    TPositionModuleStatusEvent iModuleStatusEvent;
    XQLocationPrivate*         ipParent;
};

class CPositionerObserver : public CActive
{
public:
    static CPositionerObserver* NewL(XQLocationPrivate* apParent);
    ~CPositionerObserver();

    XQLocation::Error Init(RPositionServer &aPosServer, TInt aUpdateInterval);
    void StartListening();
    void StopListening();

    XQLocation::Error SetUpdateInterval(TInt aUpdateInterval);
    TInt UpdateInterval();
    
private: // From CActive
    void RunL();
    TInt RunError(TInt aError);
    void DoCancel();

private:
   CPositionerObserver(XQLocationPrivate* apParent);

private:
   TInt                   iUpdateInterval;
   RPositioner            iPositioner;
   TPositionSatelliteInfo iPositionInfo;
   TPositionUpdateOptions iPositionUpdateOptions;
   XQLocationPrivate*     ipParent;
};

#endif /*XQLOCATION_S60_P_H*/

// End of file
