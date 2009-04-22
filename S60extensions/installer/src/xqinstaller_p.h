#ifndef XQINSTALLER_P_H
#define XQINSTALLER_P_H

// INCLUDES
#include "xqinstaller.h"
#include <SWInstApi.h>
#include <SWInstDefs.h>

// FORWARD DECLARATIONS
class QString;
class QFile;

// CLASS DECLARATION
class XQInstallerPrivate: public CActive
{

public:
    enum State {
        ERemove,
        EInstall
    };
    
    XQInstallerPrivate(XQInstaller *installer);
    ~XQInstallerPrivate();
    
    bool install(const QString& file, XQInstaller::Drive drive);
    bool remove(uint uid);
    QMap<QString, uint> applications() const;

public:
    XQInstaller::Error error();
    
protected:  
    void DoCancel();
    void RunL();

private:
    XQInstaller *q;
    mutable int iError;
    
    SwiUI::RSWInstSilentLauncher iLauncher;
    SwiUI::TInstallOptions iOptions;
    SwiUI::TInstallOptionsPckg iOptionsPckg;  
    
    SwiUI::TUninstallOptions iUninstallOptions;
    SwiUI::TUninstallOptionsPckg iUninstallOptionsPckg;
    XQInstallerPrivate::State iState;
    TFileName iFileName;
    bool iLauncherConnected;
};

#endif /*XQINSTALLER_P_H*/

// End of file
