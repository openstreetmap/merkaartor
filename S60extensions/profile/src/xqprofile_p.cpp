#include "XQProfile_p.h"
#include "CFlightModeChecker.h"
#include <mproengengine.h> 
#include <proengfactory.h>
#include <mproengprofile.h>
#include <MProEngProfileName.h> 
#include <MProEngToneSettings.h>
#include <MProEngTones.h>
#include <FeatDiscovery.h>
#include <featureinfo.h>

XQProfilePrivate::XQProfilePrivate(XQProfile *profile): q(profile) 
{
    TRAP(iError,
        iProfileEngine = ProEngFactory::NewEngineL();   
    )
}

XQProfilePrivate::~XQProfilePrivate()
{
    if(iProfileEngine) {
        iProfileEngine->Release();
    }
}

XQProfile::Profile XQProfilePrivate::activeProfile() const
{
    MProEngProfile* activeProfile;
    TRAP(iError, activeProfile = iProfileEngine->ActiveProfileL();)
    
    XQProfile::Profile currentProfile = static_cast<XQProfile::Profile>(activeProfile->ProfileName().Id());
    activeProfile->Release();
    return currentProfile;
}

bool XQProfilePrivate::isSilent() const
{
    MProEngProfile* activeProfile;
    TRAP(iError, activeProfile = iProfileEngine->ActiveProfileL();) 
    bool isSilent = activeProfile->IsSilent();
    activeProfile->Release();
    return isSilent;
}

bool XQProfilePrivate::isFlightMode() const
{
    CFlightModeChecker* flightModeChecker;
    TRAP(iError, flightModeChecker = new (ELeave) CFlightModeChecker();)
    bool isFlightModeEnabled;
    isFlightModeEnabled = flightModeChecker->IsFlightMode();
    delete flightModeChecker;
    flightModeChecker = NULL;
    return isFlightModeEnabled;
}

bool XQProfilePrivate::setActiveProfile(XQProfile::Profile profile)
{
    TRAP(iError,iProfileEngine->SetActiveProfileL(profile);)
    return (iError == KErrNone);
}

bool XQProfilePrivate::setSilent()
{
    MProEngProfile* activeProfile;
    TRAP(iError, activeProfile = iProfileEngine->ActiveProfileL();
        User::LeaveIfError(activeProfile->ToneSettings().SetRingingType(EProfileRingingTypeSilent));
        activeProfile->CommitChangeL();
    )
    activeProfile->Release();
    return (iError == KErrNone);
}

bool XQProfilePrivate::setRingingTone(QString path, XQProfile::Profile profileId)
{
    TRAP(iError,
        TPtrC filePath(reinterpret_cast<const TUint16*>(path.utf16()));
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ProfileTones().SetRingingTone1L(filePath);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setVideoCallTone(QString path, XQProfile::Profile profileId)
{
    TRAP(iError,
        TPtrC filePath(reinterpret_cast<const TUint16*>(path.utf16()));
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ProfileTones().SetVideoCallRingingToneL(filePath);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);  
}

bool XQProfilePrivate::setMessageAlertTone(QString path, XQProfile::Profile profileId)
{
    TRAP(iError,
        TPtrC filePath(reinterpret_cast<const TUint16*>(path.utf16()));
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ProfileTones().SetMessageAlertToneL(filePath);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);  
}

bool XQProfilePrivate::setEmailAlertTone(QString path, XQProfile::Profile profileId)
{
    TRAP(iError,
        TPtrC filePath(reinterpret_cast<const TUint16*>(path.utf16()));
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ProfileTones().SetEmailAlertToneL(filePath);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);  
}

bool XQProfilePrivate::setVibratingAlert(bool vibratingAlert, XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetVibratingAlert(vibratingAlert);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setWarningAndGameTones(bool warningAndGameTones, XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetWarningAndGameTones(warningAndGameTones);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setRingingType(XQProfile::RingingType type, XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetRingingType(static_cast<TProfileRingingType>(type));
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setTextToSpeech(bool textToSpeech, XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetTextToSpeech(textToSpeech);
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setRingingVolume(XQProfile::RingingVolume ringingVolume, 
    XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetRingingVolume(static_cast<TProfileRingingVolume>(ringingVolume));
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

bool XQProfilePrivate::setKeypadVolume(XQProfile::KeypadVolume keypadVolume, 
    XQProfile::Profile profileId)
{
    TRAP(iError,
        MProEngProfile* profile = iProfileEngine->ProfileLC(profileId);
        profile->ToneSettings().SetKeypadVolume(static_cast<TProfileKeypadVolume>(keypadVolume));
        profile->CommitChangeL();
        CleanupStack::PopAndDestroy(profile);
    )
    return (iError == KErrNone);
}

XQProfile::Error XQProfilePrivate::error()
{
    switch (iError) {
    case KErrNone: 
        return XQProfile::NoError;
    case KErrNoMemory: 
        return XQProfile::OutOfMemoryError;
    case KErrNotFound:
        return XQProfile::NotFoundError;
    default: 
        return XQProfile::UnknownError;
    }    
}

