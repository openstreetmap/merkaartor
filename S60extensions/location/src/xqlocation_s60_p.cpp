// INCLUDE FILES
#include <e32base.h>
#include <math.h>
#include <LbsSatellite.h>
#include <LbsCommon.h>

#include "xqlocation.h"
#include "xqlocation_s60_p.h"

_LIT(KXQLocationRequestor,"XQLocation"); //The name of the requestor
static const int KXQLocationDefaultInterval = 1000; // 1000 milliseconds = 1 second

XQLocationPrivate::XQLocationPrivate(XQLocation* apParent)
    : updateInterval(KXQLocationDefaultInterval), // default update interval is one second 
      ipParent(apParent),
      iSingleUpdate(EFalse)
{
}

XQLocationPrivate::~XQLocationPrivate()
{
    delete ipPositionerObserver;
    delete ipPositionServerObserver;
}

void XQLocationPrivate::requestUpdate()
{
    iSingleUpdate = ETrue;
    startUpdates();
}

void XQLocationPrivate::startUpdates(int msec)
{
    if (updateInterval != msec) {
        if (msec < 0) {
            msec = 0;
        }
        updateInterval = msec;
        ipPositionerObserver->SetUpdateInterval(updateInterval);
    }
    startUpdates();
}

void XQLocationPrivate::startUpdates()
{
    iSingleUpdate = EFalse;
    XQLocation::Error error = openConnectionToPositioner();
    if (error != XQLocation::NoError) {
        DeliverError(error);
        return;
    }
    if (ipPositionerObserver->UpdateInterval() != updateInterval) {
        error = ipPositionerObserver->SetUpdateInterval(updateInterval);
        if (error != XQLocation::NoError) {
            DeliverError(error);
            return;
        }
    }
    
    firstStatusUpdate = true;
    iPreviousPosition = TPosition();
    iPreviousSpeed = 0;
    iPreviousNumSatellitesInView = -1;
    iPreviousNumSatellitesUsed = -1;

    ipPositionServerObserver->StartListening();
}

void XQLocationPrivate::stopUpdates()
{
    ipPositionServerObserver->StopListening();
    ipPositionerObserver->StopListening();
}

XQLocation::Error XQLocationPrivate::openConnectionToPositioner()
{
    XQLocation::Error error = XQLocation::NoError; 

    if (!ipPositionerObserver)
    	{
    	// Create Observer for PositionServer
        TRAPD(err, ipPositionServerObserver = CPositionServerObserver::NewL(this));
        if (err != KErrNone) {
            return XQLocation::OutOfMemoryError;
        }
        error = ipPositionServerObserver->Init();
        if (error != XQLocation::NoError) {
            delete ipPositionServerObserver;
            ipPositionServerObserver = NULL;
            return error;
        }
    	
    	// Create Observer for Positioner
        TRAP(err, ipPositionerObserver = CPositionerObserver::NewL(this));
        if (err != KErrNone) {
            delete ipPositionServerObserver;
            ipPositionServerObserver = NULL;
            return XQLocation::OutOfMemoryError;
        }
        
        RPositionServer& posServer = ipPositionServerObserver->PositionServer();
        error = ipPositionerObserver->Init(posServer,updateInterval);
        if (error != XQLocation::NoError) {
            delete ipPositionerObserver;
            ipPositionerObserver = NULL;
            delete ipPositionServerObserver;
            ipPositionServerObserver = NULL;
            return error;
        }
        
        speedAvailable = false;
        satelliteInfoAvailable = false;
        TPositionModuleInfo moduleInfo;
        err = posServer.GetModuleInfoById(ipPositionServerObserver->PositionModuleId(), moduleInfo);
        if (err == KErrNone) {
            TPositionModuleInfo::TCapabilities caps = moduleInfo.Capabilities();
            if (caps & TPositionModuleInfo::ECapabilitySpeed) {
                speedAvailable = true;
            }
            if (caps & TPositionModuleInfo::ECapabilitySatellite) {
                satelliteInfoAvailable = true;
            }
        }
    }
    
    return error;
}

void XQLocationPrivate::closeConnectionToPositioner()
{
    delete ipPositionerObserver;
    ipPositionerObserver = NULL;
    delete ipPositionServerObserver;
    ipPositionServerObserver = NULL;
}

void XQLocationPrivate::PositionServerReady()
{
    ipPositionerObserver->StartListening();
}

void XQLocationPrivate::DeliverError(XQLocation::Error aError)
{
    emit ipParent->error(aError);
}

void XQLocationPrivate::DeliverPositionServerResults(TPositionModuleStatus aModuleStatus)
{
    XQLocation::DeviceStatus st;

    switch (aModuleStatus.DeviceStatus()) {
        case TPositionModuleStatus::EDeviceError:
        	// This state is used to indicate that there are problems when using the device.
        	// For example, there may be hardware errors. It should not be confused with
        	// complete loss of data quality (see TDataQualityStatus), which indicates that
        	// the device is functioning correctly, but is currently unable to obtain position
        	// information. The error state is reported if the device cannot be successfully
        	// brought on line. For example, the positioning module may have been unable to
        	// communicate with the device or it is not responding as expected.

        	st = XQLocation::StatusError;
            break;
        case TPositionModuleStatus::EDeviceDisabled:
        	// Although the device may be working properly, it has been taken off line and is
        	// regarded as being unavailable to obtain position information. This is generally
        	// done by the user via the control panel. In this state, Mobile Location FW will
        	// not use the device.

        	st = XQLocation::StatusDisabled;
            break;
        case TPositionModuleStatus::EDeviceInactive:
        	// This state signifies that the device is not being used by Mobile Location FW.
        	// This typically occurs because there are no clients currently obtaining positions
        	// from it.

        	st = XQLocation::StatusInactive;
            break;
        case TPositionModuleStatus::EDeviceInitialising:
        	// This is a transient state. The device is being brought out of the Inactive state
        	// but has not reached either the Ready or Standby modes. The initializing state
        	// occurs when the positioning module is first selected to provide a client
        	// application with location information.

        	st = XQLocation::StatusInitialising;
            break;
        case TPositionModuleStatus::EDeviceStandBy:
        	// This state indicates that the device has entered the Sleep or Power save mode.
        	// This signifies that the device is online, but it is not actively retrieving
        	// position information. A device generally enters this mode when the next position
        	// update is not required for some time and it is more efficient to partially power
        	// down.
        	// Note: Not all positioning modules support this state, particularly when there is
        	//       external hardware. 

        	st = XQLocation::StatusStandBy;
            break;
        case TPositionModuleStatus::EDeviceReady:
        	// The positioning device is online and is ready to retrieve position information.

        	st = XQLocation::StatusReady;
            break;
        case TPositionModuleStatus::EDeviceActive:
        	// The positioning device is actively in the process of retrieving position
        	// information.
        	// Note: Not all positioning modules support this state, particularly when there is
        	//       external hardware. 

        	st = XQLocation::StatusActive;
            break;
        default:
        	// EDeviceUnknown
        	// This is not a valid state and must never be reported.
        	
        	st = XQLocation::StatusUnknown;
            break;
    }
    if (st != status || firstStatusUpdate) {
        status = st;
        emit ipParent->statusChanged(status);
    }

    XQLocation::DataQuality dq;
    switch (aModuleStatus.DataQualityStatus()) {
        case TPositionModuleStatus::EDataQualityLoss:
            // This state indicates that the accuracy and contents of the position information
        	// has been completely compromised. It is no longer possible to return any coherent
        	// data. This situation occurs if the device has lost track of all the transmitters
        	// (for example, satellites or base stations). It should be noted that although it
        	// is currently not possible to obtain position information, the device may still be
        	// functioning correctly. This state should not be confused with a device error
        	// (see TDeviceStatus above).
        	
        	dq = XQLocation::DataQualityLoss;
            break;
        case TPositionModuleStatus::EDataQualityPartial:
            // The state signifies that there has been a partial degradation in the available
        	// position information. In particular, it is not possible to provide the required
        	// (or expected) quality of information. This situation may occur if the device has
        	// lost track of one of the transmitters (for example, satellites or base stations).
        	
        	dq = XQLocation::DataQualityPartial;
            break;
        case TPositionModuleStatus::EDataQualityNormal:
            // The positioning device is functioning as expected.

        	dq = XQLocation::DataQualityNormal;
            break;
        default:
        	// EDataQualityUnknown
            // This is an unassigned valued. This state should only be reported during an event
        	// indicating that a positioning module has been removed.

        	dq = XQLocation::DataQualityUnknown;
            break;
    }
    if (dq != dataQuality || firstStatusUpdate) {
        dataQuality = dq;
        emit ipParent->dataQualityChanged(dataQuality);
    }

    firstStatusUpdate = false;
}

void XQLocationPrivate::DeliverPositionerResults(TPositionSatelliteInfo aPositionInfo)
{
    TPosition pos; 
    aPositionInfo.GetPosition(pos);
    
    // Handle speed reporting
    float speed = 0;
    if (speedAvailable) {
        // Positioning module is able to offer speed information
        TCourse course;
        aPositionInfo.GetCourse(course);
        speed = course.Speed();
        if (isnan(speed)) {
            speed = 0;
        }
    } else {
        // Positioning module does not offer speed information
        // => Speed is calculated using position information & timestamps
        TTime posTime;
        TTimeIntervalSeconds interval;
        for (int i = iPositions.Count()-1 ; i >= 0; i--) {
            if (pos.Time().SecondsFrom(iPositions[i].Time(),interval) == KErrNone) {
                if (interval.Int() > 10) {
                    pos.Speed(iPositions[i], speed);
                    break;
                }
            }
        }
        while (iPositions.Count() > 0) {
            if (pos.Time().SecondsFrom(iPositions[0].Time(),interval) == KErrNone) {
                if (interval.Int() > 60) {
                    iPositions.Remove(0);
                } else {
                    break;
                }
            }
        }
        if (iPositions.Count() > 0) {
    	    if (pos.Time().SecondsFrom(iPositions[iPositions.Count()-1].Time(),interval) == KErrNone) {
                if (interval.Int() > 1) {
                    iPositions.Append(pos);
                }
        	}
        } else {
            iPositions.Append(pos);
        }
        // Accept speed from range 0.01 m/s (0.036 km/h) to 200 m/s (720 km/h)  
        if (speed < 0.01 || speed > 200) {
            speed = 0;
        }
    }
    if (speed != iPreviousSpeed) {
        emit ipParent->speedChanged(speed);
    }
    iPreviousSpeed = speed;
    
    // Handle satellite information reporting
    if (satelliteInfoAvailable) {
        if (aPositionInfo.NumSatellitesInView() != iPreviousNumSatellitesInView) {
            emit ipParent->numberOfSatellitesInViewChanged(aPositionInfo.NumSatellitesInView());
        }
        iPreviousNumSatellitesInView = aPositionInfo.NumSatellitesInView(); 

		if (aPositionInfo.NumSatellitesUsed() != iPreviousNumSatellitesUsed) { 
            emit ipParent->numberOfSatellitesUsedChanged(aPositionInfo.NumSatellitesUsed());
        }
        iPreviousNumSatellitesUsed = aPositionInfo.NumSatellitesUsed();
    }
    
    // Handle position information reporting
    if (iPreviousPosition.Latitude() != pos.Latitude() ||
        iPreviousPosition.Longitude() != pos.Longitude() ||
        iPreviousPosition.Altitude() != pos.Altitude()) {
        if (!isnan(pos.Latitude()) || !isnan(pos.Longitude()) || !isnan(pos.Altitude())) {
            emit ipParent->locationChanged(pos.Latitude(),pos.Longitude(),pos.Altitude(),speed);
        }
        
        if (iPreviousPosition.Latitude() != pos.Latitude()) {
            if (!isnan(pos.Latitude())) {
                emit ipParent->latitudeChanged(pos.Latitude(),pos.HorizontalAccuracy());
            }
        }
        if (iPreviousPosition.Longitude() != pos.Longitude()) {
            if (!isnan(pos.Longitude())) {
                emit ipParent->longitudeChanged(pos.Longitude(),pos.HorizontalAccuracy());
            }
        }
        if (iPreviousPosition.Altitude() != pos.Altitude()) {
            if (!isnan(pos.Altitude())) {
                emit ipParent->altitudeChanged(pos.Altitude(),pos.VerticalAccuracy());
            }
        }
    }
    iPreviousPosition = pos;
    
    if (iSingleUpdate) {
        stopUpdates();
        iSingleUpdate = EFalse;
    }
}


/*********************************************
 *
 * CPositionServerObserver
 *
 *********************************************/

CPositionServerObserver* CPositionServerObserver::NewL(XQLocationPrivate* apParent)
{
    CPositionServerObserver* self = new (ELeave) CPositionServerObserver(apParent);
    return self;
}

CPositionServerObserver::CPositionServerObserver(XQLocationPrivate* apParent)
    : CActive(CActive::EPriorityStandard),
      ipParent(apParent)
{
    CActiveScheduler::Add(this);
}


CPositionServerObserver::~CPositionServerObserver()
{
    Cancel();
    iPositionServer.Close();
}

XQLocation::Error CPositionServerObserver::Init()
{
    // Connect to the position server
    TInt error = iPositionServer.Connect();

    // Connection failed
    if (error != KErrNone) {
         return XQLocation::OpenError;
    }

    error = iPositionServer.GetDefaultModuleId(iPositionModuleId);
    // Can not found default module
    if (error != KErrNone) {
         return XQLocation::DefaultModuleNotFoundError;
    }
    
    return XQLocation::NoError;
}

void CPositionServerObserver::StartListening()
{
    if (!IsActive()) {
        // Try to get initial module status and deliver status
        TInt error = iPositionServer.GetModuleStatus(iModuleStatus, iPositionModuleId);
        if (error == KErrNone) {
            if ((iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceReady ||
                 iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceActive ||
                 iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceInitialising)) {
                // Status is Initializing, Active or Ready
	            // => Start listening positioner signals immediately
                ipParent->PositionServerReady();
            }
            ipParent->DeliverPositionServerResults(iModuleStatus);
        }
	
        TPositionModuleStatusEventBase::TModuleEvent requestedEvents =
                   TPositionModuleStatusEventBase::EEventDeviceStatus | 
                   TPositionModuleStatusEventBase::EEventDataQualityStatus;
        iModuleStatusEvent.SetRequestedEvents(requestedEvents);

        iPositionServer.NotifyModuleStatusEvent(iModuleStatusEvent,iStatus);
        SetActive();
    }
}

void CPositionServerObserver::StopListening()
{
    Cancel();
}

TPositionModuleId CPositionServerObserver::PositionModuleId()
{
    return iPositionModuleId;
}

RPositionServer& CPositionServerObserver::PositionServer()
{
    return iPositionServer;
}

void CPositionServerObserver::RunL()
{
    switch (iStatus.Int()) {
        case KErrNone:
            iModuleStatusEvent.GetModuleStatus(iModuleStatus);
            ipParent->DeliverPositionServerResults(iModuleStatus);
        
            if ((iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceReady ||
                 iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceActive ||
                 iModuleStatus.DeviceStatus() == TPositionModuleStatus::EDeviceInitialising)) {
        	    // Status is Initializing, Active or Ready
        	    // => Start listening position signals
                ipParent->PositionServerReady();
            }

            // Continue listening PositionServer status change
            iPositionServer.NotifyModuleStatusEvent(iModuleStatusEvent,iStatus);
            SetActive();
            break;
        case KErrNotFound:
        	// The specified module does not exist.
            ipParent->DeliverError(XQLocation::ModuleNotFoundError);
            break;
        case KErrCancel:
            // Request has been successfully cancelled.
            break;
        default:
            // Unrecoverable errors:
        	//     KErrArgument - if the requested event mask is zero (i.e. no events have been requested). 

            ipParent->DeliverError(XQLocation::InternalError);
            break;
    }
}

TInt CPositionServerObserver::RunError(TInt /*aError*/)
{
    ipParent->DeliverError(XQLocation::InternalError);
    return KErrNone;
}

void CPositionServerObserver::DoCancel()
{
	iPositionServer.CancelRequest(EPositionServerNotifyModuleStatusEvent);
}


/*********************************************
 *
 * CPositionerObserver
 *
 *********************************************/

CPositionerObserver* CPositionerObserver::NewL(XQLocationPrivate* apParent)
{
    CPositionerObserver* self = new (ELeave) CPositionerObserver(apParent);
    return self;
}

CPositionerObserver::CPositionerObserver(XQLocationPrivate* apParent)
    : CActive(CActive::EPriorityStandard),
      ipParent(apParent)
{
    CActiveScheduler::Add(this);
}

CPositionerObserver::~CPositionerObserver()
{
    Cancel();
    iPositioner.Close();
}

XQLocation::Error CPositionerObserver::Init(RPositionServer &aPosServer, TInt aUpdateInterval)
{
    // Open sub-session to positioning server.
    TPositionModuleId positionModuleId;
    aPosServer.GetDefaultModuleId(positionModuleId);
    TInt error = iPositioner.Open(aPosServer,positionModuleId);

    // Opening of a sub-session failed
    if (error != KErrNone) {
        return XQLocation::OpenError;
    }

    // Set position requestor
    error = iPositioner.SetRequestor(CRequestor::ERequestorService,
                                     CRequestor::EFormatApplication,
                                     KXQLocationRequestor);

    // The requestor could not be set
    if (error != KErrNone) {
        iPositioner.Close();
        return XQLocation::OpenError;
    }

    XQLocation::Error err = SetUpdateInterval(aUpdateInterval);
    // The update interval could not be set
    if (err != XQLocation::NoError) {
        iPositioner.Close();
        return err;
    }
    
    return XQLocation::NoError;
}

XQLocation::Error CPositionerObserver::SetUpdateInterval(TInt aUpdateInterval)
{
    iUpdateInterval = aUpdateInterval;
    iPositionUpdateOptions.SetUpdateInterval(TTimeIntervalMicroSeconds(iUpdateInterval * 1000));
    iPositionUpdateOptions.SetAcceptPartialUpdates(ETrue);
    if (iPositioner.SetUpdateOptions(iPositionUpdateOptions) != KErrNone) {
        return XQLocation::InvalidUpdateIntervalError; 
    }
    return XQLocation::NoError;
}

TInt CPositionerObserver::UpdateInterval()
{
    return iUpdateInterval;
}

void CPositionerObserver::StartListening()
{
    if (!IsActive()) {
        iPositioner.NotifyPositionUpdate(iPositionInfo,iStatus);
        SetActive();
    }
}

void CPositionerObserver::StopListening()
{
    Cancel();
}

void CPositionerObserver::RunL()
{
    switch (iStatus.Int()) {
        case KPositionPartialUpdate:
        	// Position information has been retrieved but it is incomplete.
        case KErrNone:
            // NotifyPositionUpdate successfully completed

            ipParent->DeliverPositionerResults(iPositionInfo);

            iPositioner.NotifyPositionUpdate(iPositionInfo,iStatus);
            SetActive();
            break;
        case KPositionQualityLoss:
            //  Positioning module is unable to return any position information.

            ipParent->DeliverPositionerResults(iPositionInfo);

            iPositioner.NotifyPositionUpdate(iPositionInfo,iStatus);
            SetActive();
            break;
        case KErrAccessDenied:
            // Requestor information has not been specified or 
            // privacy check has failed.
    
            ipParent->DeliverError(XQLocation::AccessDeniedError);
            break;
        case KErrNotFound:
            // Currently used module is invalid. A previously correct module
            // may become invalid if the positioning module has been
            // uninstalled or disabled by user.

            ipParent->DeliverError(XQLocation::ModuleNotFoundError);
            break;
        case KErrTimedOut:
            // requested location information could not be retrieved within
            // the maximum period as specified in the current update options
            // or time out happens in entities that are involved in retrieving
            // the location fix.

            iPositioner.NotifyPositionUpdate(iPositionInfo,iStatus);
            SetActive();
            break;
        case KErrArgument:
            // Positioning module is unable to support the type of the class
            // passed in aPosInfo. All positioning modules are required to
            // support both TPositionInfo and HPositionGenericInfo.

            ipParent->DeliverError(XQLocation::InternalError);
            break;
        case KErrPositionBufferOverflow:
            // There is insufficient space to return the required information
            // back to the client. This situation can occur when using
            // HPositionGenericInfo if the application has not allocated a
            // large enough buffer.

            ipParent->DeliverError(XQLocation::InternalError);
            break;
        case KErrCancel:
            // Request was successfully cancelled.

            break;
        case KErrUnknown:
            ipParent->DeliverError(XQLocation::UnknownError);
            break;
        default:
            // Unrecoverable errors.
            ipParent->DeliverError(XQLocation::InternalError);
            break;
    }
}

TInt CPositionerObserver::RunError(TInt /*aError*/)
{
    ipParent->DeliverError(XQLocation::InternalError);
    return KErrNone;
}

void CPositionerObserver::DoCancel()
{
    iPositioner.CancelRequest(EPositionerNotifyPositionUpdate);
}

// End of file
