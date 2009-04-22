#ifndef XQPROFILEPRIVATE_H
#define XQPROFILEPRIVATE_H

// INCLUDES
#include "XQProfile.h"

// FORWARD DECLARATIONS
class MProEngEngine;

// CLASS DECLARATION
class XQProfilePrivate: public CBase
{
public:
    XQProfilePrivate(XQProfile *profile);
    ~XQProfilePrivate();
    
    XQProfile::Profile activeProfile() const;
    bool isSilent() const;
    bool isFlightMode() const;
    bool setActiveProfile(XQProfile::Profile profile);
    bool setSilent();
    bool setRingingTone(QString path, XQProfile::Profile profile);
    bool setVideoCallTone(QString path, XQProfile::Profile profile);
    bool setMessageAlertTone(QString path, XQProfile::Profile profile);
    bool setEmailAlertTone(QString path, XQProfile::Profile profile);
    bool setVibratingAlert(bool vibratingAlert, XQProfile::Profile profileId);
    bool setWarningAndGameTones(bool warningAndGameTones, XQProfile::Profile profileId);
    bool setRingingType(XQProfile::RingingType type, XQProfile::Profile profileId);
    bool setTextToSpeech(bool textToSpeech, XQProfile::Profile profileId);
    bool setRingingVolume(XQProfile::RingingVolume ringingVolume, XQProfile::Profile profileId);
    bool setKeypadVolume(XQProfile::KeypadVolume keypadVolume, XQProfile::Profile profileId);
    XQProfile::Error error();

private:
    XQProfile *q;
    MProEngEngine* iProfileEngine;
    mutable int iError;
};

#endif /*XQPROFILEPRIVATE_H*/

// End of file

