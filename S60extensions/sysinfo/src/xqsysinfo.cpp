#include "XQSysInfo.h"
#include "XQSysInfo_p.h"

/*!
    \class XQSysInfo

    \brief The XQSysInfo class is used to retrieve information about the phone (Version, free memory, IMEI, etc.)
    
    Example:
    \code
    XQSysInfo *sysInfo = new XQSysInfo(this);
    QLabel *imeiLabel = new QLabel("Imei: "+sysInfo->imei());
    QLabel *versionLabel = new QLabel("Version: "+sysInfo->softwareVersion());
    QLabel *imsiLabel = new QLabel("Imsi: "+sysInfo->imsi());
    QLabel *modelLabel = new QLabel("Model: "+sysInfo->model());
    QLabel *manufacturerLabel = new QLabel("Manufacturer: "+sysInfo->manufacturer());
    QLabel *signalStrengthLabel = new QLabel("Signal strenght: "+QString::number(sysInfo->signalStrength()));
    QLabel *batteryLevelLabel = new QLabel("Battery level: "+QString::number(sysInfo->batteryLevel()));
    QLabel *diskSpaceLabel = new QLabel("Free space (c:): "+QString::number(sysInfo->diskSpace(XQSysInfo::CDrive)));
    \endcode
*/

/*!
    Constructs a XQSysInfo object with the given parent.
*/
XQSysInfo::XQSysInfo(QObject *parent):
    QObject(parent), d(new XQSysInfoPrivate(this))
{
}

/*!
    Destroys the XQSysInfo object.
*/
XQSysInfo::~XQSysInfo()
{
    delete d;
}

/*!
    \enum XQSysInfo::Error
    This enum defines the possible errors for a XQSysInfo object.
*/
/*! \var XQSysInfo::Error XQSysInfo::NoError
    No error occured.
*/
/*! \var XQSysInfo::Error XQSysInfo::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQSysInfo::Error XQSysInfo::IncorrectDriveError
    Wrong drive letter.
*/
/*! \var XQSysInfo::Error XQSysInfo::DriveNotFoundError
    Drive cannot be found.
*/
/*! \var XQSysInfo::Error XQSysInfo::UnknownError
    Unknown error.
*/

/*!
    \enum XQSysInfo::Drive

    This enum defines the possible drive letters to be used.
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveA 
    Drive A
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveB
    Drive B
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveC
    Drive C
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveD
    Drive D
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveE
    Drive E
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveF
    Drive F
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveG
    Drive G
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveH
    Drive H
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveI
    Drive I
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveJ
    Drive J
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveK
    Drive K
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveL
    Drive L
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveM
    Drive M
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveN 
    Drive N
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveO
    Drive O
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveP
    Drive P
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveQ
    Drive Q
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveR
    Drive R
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveS
    Drive S
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveT
    Drive T
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveU
    Drive U
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveV
    Drive V
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveW    
    Drive W
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveX
    Drive X
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveY
    Drive Y
*/
/*! \var XQSysInfo::Drive XQSysInfo::DriveZ
    Drive Z
*/

/*!
    Current language 

    \return Current language
*/
XQSysInfo::Language XQSysInfo::currentLanguage() const
{
    return d->currentLanguage();
}

/*!
    Imei

    \return Imei
*/
QString XQSysInfo::imei() const
{
    return d->imei();
}

/*!
    Imsi

    \return Imsi
*/
QString XQSysInfo::imsi() const
{
    return d->imsi();
}

/*!
    Software version

    \return Software version
*/
QString XQSysInfo::softwareVersion() const
{
    return d->softwareVersion();
}

/*!
    Phone model

    \return Phone model
*/
QString XQSysInfo::model() const
{
    return d->model();
}

/*!
    Manufacturer of the phone

    \return Manufacturer of the phone
*/
QString XQSysInfo::manufacturer() const
{
    return d->manufacturer();
}

/*!
    Current battery level

    \return current battery level in percents
*/
uint XQSysInfo::batteryLevel() const
{
    return d->batteryLevel();
}

/*!
    Network signal strenght

    \return Network signal strenght in percents
*/
int XQSysInfo::signalStrength() const
{
    return d->signalStrength();
}

/*!
    Browser version
    NOTE: No implementation in the alpha release

    \return Browser version
*/
QString XQSysInfo::browserVersion() const
{
    return d->browserVersion();
}

/*!
    Free disk space

    \param drive Drive letter as enumeration
    \return Free disk space in bytes
*/
qlonglong XQSysInfo::diskSpace(XQSysInfo::Drive drive) const
{
    return d->diskSpace(drive);
}

/*!
    Helper function to check is disk space in critical contidition

    \param drive Drive letter as enumeration
    \return True, if memory has enabled, otherwise false
*/
bool XQSysInfo::isDiskSpaceCritical(XQSysInfo::Drive drive) const
{
    return d->isDiskSpaceCritical(drive);
}

/*!
    Free RAM memory in bytes

    \return Free RAM memory in bytes
*/
int XQSysInfo::memory() const
{
    return d->memory();
}

/*!
    Checks if the specific feature is supported in the environment.
    
    \param featureId Feature id
     \return True, if the feature has supported, otherwise false
*/
bool XQSysInfo::isSupported(int featureId)
{
    return XQSysInfoPrivate::isSupported(featureId);
}
/*!
    Returns current error level.
    \return Error code
*/
XQSysInfo::Error XQSysInfo::error() const
{
    return d->error();
}

/*!
    Checks if the network is available

    \return True, if the networks is available, otherwise false
*/
bool XQSysInfo::isNetwork() const
{
    return d->isNetwork();
}

/*!
    \fn void XQSysInfo::networkSignalChanged(ulong signalStrength)
    
    This signal is emitted when signal strength is changed.

    \param signalStrength New signal strength value
    \sa signalStrength()
*/

/*!
    \fn void XQSysInfo::batteryLevelChanged(uint batteryLevel)

    This signal is emitted when battery level is changed.

    \param batteryLevel New battery level
    \sa batteryLevel()
*/
