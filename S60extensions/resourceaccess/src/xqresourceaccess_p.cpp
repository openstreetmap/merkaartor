#include "xqresourceaccess_p.h"
#include "xqresourceaccess.h"

#include <f32file.h>
#include <bautils.h>

_LIT(KFilePath, "c:\\resource\\apps\\");

XQResourceAccessPrivate::XQResourceAccessPrivate(
    XQResourceAccess *resourceAccess) : q(resourceAccess), iError(KErrNone), iResourceFileOpen(false)
{
}

XQResourceAccessPrivate::~XQResourceAccessPrivate()
{
    iResourceFile.Close();
    iFsSession.Close();
}

QString XQResourceAccessPrivate::loadStringFromResourceFile(int resourceId) const
{
    QString resourceString;
    TRAP(iError,
        HBufC8 *dataBuffer = iResourceFile.AllocReadL(resourceId);
        resourceString = QString::fromUtf16(reinterpret_cast<const TUint16*>
                (dataBuffer->Des().Ptr()), dataBuffer->Length() / 2);
        delete dataBuffer;
    )
    return resourceString;
}

bool XQResourceAccessPrivate::openResourceFile(QString fileName)
{
    if (iResourceFileOpen) {
        iError = KErrInUse;
        return false;
    }
    TPtrC textPtr(reinterpret_cast<const TUint16*>(fileName.utf16()));
    
    TRAP(iError,
        TFileName filePath(textPtr);
        User::LeaveIfError(iFsSession.Connect());
        if (BaflUtils::FileExists(iFsSession, filePath)) {
            BaflUtils::NearestLanguageFile(iFsSession, filePath);
            iResourceFile.OpenL(iFsSession, filePath);
        } else {
            TFileName file(KFilePath);
            file.Append(filePath);

            if (BaflUtils::FileExists(iFsSession, file)) {
                BaflUtils::NearestLanguageFile(iFsSession, file);
                iResourceFile.OpenL(iFsSession, file);
            } else {
                User::Leave(KErrPathNotFound);
            }
        }
        iResourceFile.ConfirmSignatureL(0);
        iResourceFileOpen = true;
    )
    return (iError == KErrNone);
}

XQResourceAccess::Error XQResourceAccessPrivate::error() const
{
    switch (iError) {
    case KErrNone:
        return XQResourceAccess::NoError;
    case KErrNoMemory:
        return XQResourceAccess::OutOfMemoryError;
    case KErrPathNotFound:
        return XQResourceAccess::FileNotFoundError;
    case KErrNotFound:
        return XQResourceAccess::ResourceNotFoundError;
    case KErrInUse:
        return XQResourceAccess::ResourceFileOpenError;
    default:
        return XQResourceAccess::UnknownError; 
    }
}
