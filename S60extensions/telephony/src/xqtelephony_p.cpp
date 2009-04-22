#include "xqtelephony_p.h"
#include "ccalldialer.h"

XQTelephonyPrivate::XQTelephonyPrivate(XQTelephony *telephony) : q(telephony)
{
}

XQTelephonyPrivate::~XQTelephonyPrivate()
{
    delete iCallDialer;
    delete iTelephonyMonitor;
}

bool XQTelephonyPrivate::startMonitoringLine()
{
    TRAP(iError, 
        if (!iTelephonyMonitor) {
            iTelephonyMonitor = CTelephonyMonitor::NewL(*this);
        }
        iTelephonyMonitor->StartListening();
    )
    return (iError == KErrNone);
}

void XQTelephonyPrivate::stopMonitoringLine()
{
    delete iTelephonyMonitor;
    iTelephonyMonitor == NULL;
}

void XQTelephonyPrivate::call(const QString& phoneNumber)
{
    TPtrC16 textPtr(reinterpret_cast<const TUint16*>(phoneNumber.utf16()));
    TRAP(iError,
        if (!iCallDialer) {
            iCallDialer = CCallDialer::NewL(*this);
        }
        iCallDialer->Call(textPtr);
    )
}

void XQTelephonyPrivate::TelephonyStatusChangedL(
        CTelephony::TCallStatus aStatus, const TDesC& number)
{
    QString callerNumber = QString::fromUtf16(number.Ptr(), number.Length());
    emit q->lineStatusChanged(static_cast<XQTelephony::LineStatus>(aStatus), callerNumber);
}

void XQTelephonyPrivate::CallDialedL(TInt aError)
{
    iError = aError;
    if (iError != KErrNone) {
        emit q->error(error());
    }
}

void XQTelephonyPrivate::ErrorOccuredL(TInt aError)
{
    iError = aError;
    emit q->error(error());   
}

XQTelephony::Error XQTelephonyPrivate::error()
{
    switch (iError) {
    case KErrNone:
        return XQTelephony::NoError;
    case KErrNoMemory:
        return XQTelephony::OutOfMemoryError;
    default:
        return XQTelephony::UnknownError;
    }    
}

// End of file

