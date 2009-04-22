#include "xqutils.h"
#include "xqutils_p.h"

#include <DocumentHandler.h>
#include <apmstd.h>

XQUtilsPrivate::XQUtilsPrivate(XQUtils *utils): q(utils)
{
}

XQUtilsPrivate::~XQUtilsPrivate()
{
    delete iDocHandler;
}

bool XQUtilsPrivate::launchFile(const QString& filename)
{
    TRAP(iError,
        if (!iDocHandler) {
            iDocHandler = CDocumentHandler::NewL();
        }
        TPtrC16 textPtr(reinterpret_cast<const TUint16*>(filename.utf16()));
        TDataType dataType;
        User::LeaveIfError(iDocHandler->OpenFileEmbeddedL(textPtr, dataType));
    )
    return (iError == KErrNone);
}

void XQUtilsPrivate::resetInactivityTime()
{
    User::ResetInactivityTime();
}

XQUtils::Error XQUtilsPrivate::error() const
{
    switch (iError) {
    case KErrNone:
        return XQUtils::NoError;
    case KErrNoMemory:
        return XQUtils::OutOfMemoryError;
    case KUserCancel:
        return XQUtils::UserCancelledError;
    default:
        return XQUtils::UnknownError;
    }
}

// End of file
