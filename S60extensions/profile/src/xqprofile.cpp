#include "XQProfile.h"
#include "XQProfile_p.h"

/*!
    \class XQProfile
    \brief The XQProfile class can be used for getting/setting active profile.
    Some information about the current profile is also provided (e.g. silent and flightmode status).
    
    Example:
    \code
    XQProfile *profile = new XQProfile(this);
    profile->setActiveProfile(XQProfile::ProfileSilent);
    \endcode
*/

/*!
    Constructs a XQProfile object with the given parent.
    Call error() to get a value of XQProfile::Error that indicates if any error happend during instantiating.
    \sa profile(), setProfile(), error()
*/
XQProfile::XQProfile(QObject *parent):
    QObject(parent), d(new XQProfilePrivate(this))
{
}

/*!
    Destroys the XQProfile object.
*/
XQProfile::~XQProfile()
{
    delete d;
}

/*!
    \enum XQProfile::Error
    This enum defines the possible errors for a XQProfile object.
*/
/*! \var XQProfile::Error XQProfile::NoError
    No error occured.
*/
/*! \var XQProfile::Error XQProfile::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQProfile::Error XQProfile::NotFoundError
    Profile not found.
*/
/*! \var XQProfile::Error XQProfile::UnknownError
    Unknown error.
*/




/*!
    \enum XQProfile::Profile
    This enum defines the possible profiles which can be set via this API. 
    Note! Custom profiles isn't supported.
*/
/*! \var XQProfile::Profile XQProfile::ProfileGeneral
    General profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfileSilent
    Silent profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfileMeeting
    Meeting profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfileOutdoor
    Outdoor profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfilePager
    Pager profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfileOffLine
    Offline profile.
*/
/*! \var XQProfile::Profile XQProfile::ProfileDrive
    Drive profile.
*/

/*!
    Returns active profile
    
    \sa setProfile()
*/
XQProfile::Profile XQProfile::activeProfile() const
{
    return d->activeProfile();    
}

/*! 
    Return whether this profile is silent or not. 

    A profile being silent means that either the ringing type is silent or all the alert tones are set to "None".
    
    \sa setSilent() 
*/
bool XQProfile::isSilent() const
{
    return d->isSilent();
}

/*! 
    Return whether the flight mode is on or off 
    All network-related request are disable in case of flight mode is "off"
    Call error() to get a value of XQProfile::Error that indicates if any error happend 
    during this function call.
    
    \sa error()
*/
bool XQProfile::isFlightMode() const
{
    return d->isFlightMode();
} 

/*! 
    Sets the active profile.

    \param profile Possible profiles are defined as enumerations: XQProfile::Profile
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa profile(), error() 
*/
bool XQProfile::setActiveProfile(XQProfile::Profile profile)
{
    return d->setActiveProfile(profile);
}

/*! 
    Sets the current ring tone to silent
    
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa isSilent(), error() 
*/
bool XQProfile::setSilent()
{  
    return d->setSilent();
}

/*! 
    Sets ringing tone
    
    \param path Path to the ringing tone
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setRingingTone(QString path, XQProfile::Profile profile)
{
    return d->setRingingTone(path, profile);
}

/*! 
    Sets video call tone
    
    \param path Path to the video call tone
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setVideoCallTone(QString path, XQProfile::Profile profile)
{
    return d->setVideoCallTone(path, profile);
}

/*! 
    Sets message alert tone
    
    \param path Path to the message alert tone
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setMessageAlertTone(QString path, XQProfile::Profile profile)
{
    return d->setMessageAlertTone(path, profile);
}

/*! 
    Sets email alert tone
    
    \param path Path to the message alert tone
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setEmailAlertTone(QString path, XQProfile::Profile profile)
{
    return d->setEmailAlertTone(path, profile);
}

/*! 
    Sets vibrating alert on/orff

    \param vibratingAlert True if vibrating will be enabled, otherwise false
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setVibratingAlert(bool vibratingAlert, XQProfile::Profile profile)
{
    return d->setVibratingAlert(vibratingAlert, profile);
}

/*! 
    Sets game and warning tones on/off
    
    \param warningAndGameTones True if game and warning tones will be enabled, otherwise false
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setWarningAndGameTones(bool warningAndGameTones, XQProfile::Profile profile)
{
    return d->setWarningAndGameTones(warningAndGameTones, profile);
}

/*!
    Sets ringing type
    
    \param type Ringing type (e.g. ringing tone is played only once)
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setRingingType(XQProfile::RingingType type, XQProfile::Profile profile)
{
    return d->setWarningAndGameTones(type, profile);
}

/*! 
    Sets text to speech feature on/off
    
    \param textToSpeech True if text to speech feature will be enabled, otherwise false
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setTextToSpeech(bool textToSpeech, XQProfile::Profile profile)
{
    return d->setTextToSpeech(textToSpeech, profile);
}

/*! 
    Sets ringing volume
    
    \param ringingVolume Predefined ringing volume level
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error() 
*/
bool XQProfile::setRingingVolume(XQProfile::RingingVolume ringingVolume, XQProfile::Profile profile)
{
    return d->setRingingVolume(ringingVolume, profile);
}

/*! 
    Sets keypad volume
    
    \param keypadVolume Predefined keypad volume level
    \param profile Profile to be modified
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQProfile::Error that indicates which error occurred
    \sa error()
*/
bool XQProfile::setKeypadVolume(XQProfile::KeypadVolume keypadVolume, XQProfile::Profile profile)
{
    return d->setKeypadVolume(keypadVolume, profile);
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError.
    \return Error code
*/
XQProfile::Error XQProfile::error() const
{
    return d->error();
}
