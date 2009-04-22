#ifndef XQINSTALLER_H
#define XQINSTALLER_H

// INCLUDES
#include <QObject>
#include <QMap>

class XQInstallerPrivate;

// CLASS DECLARATION
class XQInstaller : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        AlreadyInUseError,
        UserCancelError, 
        PackageNotSupportedError,
        SecurityFailureError,
        MissingDependencyError,
        NoRightsError,
        BusyError,
        AccessDeniedError,
        UpgradeError,
        UnknownError = -1
    };
    
    enum Drive {
        DriveA,   DriveB,   DriveC,   DriveD,   DriveE,
        DriveF,   DriveG,   DriveH,   DriveI,   DriveJ,
        DriveK,   DriveL,   DriveM,   DriveN,   DriveO, 
        DriveP,   DriveQ,   DriveR,   DriveS,   DriveT,
        DriveU,   DriveV,   DriveW,   DriveX,   DriveY,
        DriveZ
    };

    XQInstaller(QObject *parent = 0);
    ~XQInstaller();
    
    bool install(const QString& file, XQInstaller::Drive drive = XQInstaller::DriveC);
    QMap<QString, uint> applications() const;
    bool remove(uint uid);

    XQInstaller::Error error() const;
    
Q_SIGNALS:
    void applicationInstalled();
    void applicationRemoved();
    void error(XQInstaller::Error);
    
private:
    friend class XQInstallerPrivate;
    XQInstallerPrivate *d;
};

#endif // XQINSTALLER_H

// End of file
