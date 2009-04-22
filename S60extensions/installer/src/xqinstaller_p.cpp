#include "xqinstaller.h"
#include "xqinstaller_p.h"
#include <f32file.h>
#include <apgcli.h>

XQInstallerPrivate::XQInstallerPrivate(XQInstaller *installer) 
    : CActive(EPriorityNormal), q(installer), iOptionsPckg(iOptions), 
      iUninstallOptionsPckg(iUninstallOptions), iLauncherConnected(false)
{
    CActiveScheduler::Add(this);  
}

XQInstallerPrivate::~XQInstallerPrivate()
{
    Cancel();
    if (iLauncherConnected) {
        iLauncher.Close();
    }
}

bool XQInstallerPrivate::install(const QString& file, XQInstaller::Drive drive)
{
    int asciiValue = 10;  // = 'A'
    TRAP(iError,
        if (!iLauncherConnected) {
            User::LeaveIfError(iLauncher.Connect());
            iLauncherConnected = true;
        }
        if (IsActive()) {
            User::Leave(KErrInUse);
        }
        
        iState = XQInstallerPrivate::EInstall;
        iOptions.iUpgrade = SwiUI::EPolicyAllowed;
        iOptions.iOCSP = SwiUI::EPolicyNotAllowed;
        iOptions.iDrive = TChar(asciiValue+drive);
        iOptions.iUntrusted = SwiUI::EPolicyAllowed; 
        iOptions.iCapabilities = SwiUI::EPolicyAllowed;  
        iOptions.iOptionalItems = SwiUI::EPolicyAllowed;  
        iOptions.iOverwrite = SwiUI::EPolicyAllowed; 
        TPtrC16 fileName(reinterpret_cast<const TUint16*>(file.utf16()));
        iFileName = fileName;
        iLauncher.SilentInstall(iStatus, iFileName, iOptionsPckg);
        SetActive();
    )
    return (iError == KErrNone);
}

bool XQInstallerPrivate::remove(uint uid)
{
    TRAP(iError,
        if (!iLauncherConnected) {
            User::LeaveIfError(iLauncher.Connect());
            iLauncherConnected = true;
        }
        if (IsActive()) {
            User::Leave(KErrInUse);
        }

        iState = XQInstallerPrivate::ERemove;
    
        iLauncher.SilentUninstall(iStatus,TUid::Uid(uid),
            iUninstallOptionsPckg, SwiUI::KSisxMimeType);
        SetActive();
    )
    return (iError == KErrNone);
}

QMap<QString, uint> XQInstallerPrivate::applications() const
{
    RApaLsSession lsSession;
    QMap<QString, uint> applications; 
      
    // Connect to application architecture server
    TRAP(iError,
        User::LeaveIfError(lsSession.Connect());
        CleanupClosePushL(lsSession);
        
        TApaAppInfo appInfo;
        lsSession.GetAllApps();  
        
        while (lsSession.GetNextApp(appInfo) == KErrNone) {
            TApaAppCapabilityBuf capability;
            User::LeaveIfError(lsSession.GetAppCapability(capability,
                appInfo.iUid));
            if (appInfo.iCaption.Length() > 0 && !capability().iAppIsHidden) {
                QString fullName = QString::fromUtf16(
                    appInfo.iCaption.Ptr(), appInfo.iCaption.Length());
                applications.insert(fullName, (TUint)appInfo.iUid.iUid);
            }
        }
        CleanupStack::PopAndDestroy(&lsSession);
    )
      
    return applications; 
}

void XQInstallerPrivate::DoCancel()
{
    if (iState == XQInstallerPrivate::EInstall) {
        iLauncher.CancelAsyncRequest(SwiUI::ERequestSilentInstall);
    } else if (iState == XQInstallerPrivate::ERemove) {
        iLauncher.CancelAsyncRequest(SwiUI::ERequestSilentUninstall);
    }
}
 
void XQInstallerPrivate::RunL()
{
    if (iStatus.Int() == KErrNone) {
        if (iState == XQInstallerPrivate::EInstall) {
            emit q->applicationInstalled();
        } else if (iState == XQInstallerPrivate::ERemove) {
            emit q->applicationRemoved();
        }
    } else {
        iError = iStatus.Int();
        emit q->error(error());
    }
}

XQInstaller::Error XQInstallerPrivate::error()
{
    switch (iError) {
    case KErrNone:
        return XQInstaller::NoError;
    case SwiUI::KSWInstErrInsufficientMemory:
    case KErrNoMemory:
        return XQInstaller::OutOfMemoryError;
    case SwiUI::KSWInstErrFileInUse:
    case KErrInUse:
        return XQInstaller::AlreadyInUseError;
    case SwiUI::KSWInstErrUserCancel:
        return XQInstaller::UserCancelError;  
    case SwiUI::KSWInstErrPackageNotSupported:
        return XQInstaller::PackageNotSupportedError; 
    case SwiUI::KSWInstErrSecurityFailure:
        return XQInstaller::SecurityFailureError; 
    case SwiUI::KSWInstErrMissingDependency:
        return XQInstaller::MissingDependencyError; 
    case SwiUI::KSWInstErrNoRights:
        return XQInstaller::NoRightsError;
    case SwiUI::KSWInstErrBusy:
        return XQInstaller::BusyError;
    case SwiUI::KSWInstErrAccessDenied:
        return XQInstaller::AccessDeniedError;
    case SwiUI::KSWInstUpgradeError:
        return XQInstaller::UpgradeError;
    case SwiUI::KSWInstErrGeneralError:
    default:
        return XQInstaller::UnknownError;
    }    
}
 
// End of file
