#ifndef XQPROFILE_H
#define XQPROFILE_H

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQProfilePrivate;

// CLASS DECLARATION
class XQProfile : public QObject
{

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        NotFoundError,
        UnknownError = -1
    };
    
    enum Profile {
        ProfileGeneral   = 0,
        ProfileSilent,
        ProfileMeeting,
        ProfileOutdoor,
        ProfilePager,
        ProfileOffLine,
        ProfileDrive
    };
    
    enum KeypadVolume {
        KeypadVolumeOff,   
        KeypadVolumeLevel1,   
        KeypadVolumeLevel2,   
        KeypadVolumeLevel3
    };
     
    enum RingingType {  
        RingingTypeRinging,  
        RingingTypeAscending,  
        RingingTypeRingingOnce,  
        RingingTypeBeepOnce,  
        RingingTypeSilent
    };
     
    enum RingingVolume {
        RingingVolumeLevel1,   
        RingingVolumeLevel2,   
        RingingVolumeLevel3,   
        RingingVolumeLevel4,   
        RingingVolumeLevel5,  
        RingingVolumeLevel6,   
        RingingVolumeLevel7,   
        RingingVolumeLevel8,   
        RingingVolumeLevel9,   
        RingingVolumeLevel10
    };
     
    XQProfile(QObject *parent = 0);
    ~XQProfile();
    
    XQProfile::Profile activeProfile() const;
    bool isSilent() const;
    bool setSilent();
    bool isFlightMode() const;
    bool setActiveProfile(XQProfile::Profile profile);
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

    XQProfile::Error error() const;

private:
    friend class XQProfilePrivate;
    XQProfilePrivate *d;
};

#endif /*XQPROFILE_H*/

// End of file

