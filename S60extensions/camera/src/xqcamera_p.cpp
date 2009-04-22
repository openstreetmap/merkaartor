#include "xqcamera.h"
#include "xqcamera_p.h"
#include <cameraengine.h>
#include <fbs.h>

XQCameraPrivate::XQCameraPrivate(XQCamera *qq) : q(qq), iCaptureSize(QSize(640, 480))
{
}

XQCameraPrivate::~XQCameraPrivate()
{
    delete iCameraEngine;
}

bool XQCameraPrivate::open(int index)
{
    TRAP(iError,
        iCameraEngine = CCameraEngine::NewL(index, 0, this);
        iCameraEngine->ReserveAndPowerOn();
        )
    return (iError == KErrNone);
}

void XQCameraPrivate::setCaptureSize(QSize size)
{
    iCaptureSize = size;
}

bool XQCameraPrivate::capture()
{
    TSize size(iCaptureSize.width(), iCaptureSize.height());
    TRAP(iError,
        iCameraEngine->PrepareL(size);
        iCameraEngine->CaptureL();
    )
    return (iError == KErrNone);
}

void XQCameraPrivate::close()
{
    iCameraEngine->ReleaseAndPowerOff();
}

bool XQCameraPrivate::focus()
{
    TRAP(iError, iCameraEngine->StartFocusL();)
    return (iError == KErrNone);
}

void XQCameraPrivate::cancelFocus()
{
    iCameraEngine->FocusCancel();
}

void XQCameraPrivate::MceoCameraReady()
{
    emit q->cameraReady();
}

void XQCameraPrivate::MceoFocusComplete()
{
    emit q->focused();
}

void XQCameraPrivate::MceoCapturedDataReady(TDesC8* aData)
{
    emit q->captureCompleted(QByteArray::fromRawData((const char *)aData->Ptr(), aData->Length()));
    iCameraEngine->ReleaseImageBuffer();
}

void XQCameraPrivate::MceoCapturedBitmapReady(CFbsBitmap* /*aBitmap*/)
{
}

void XQCameraPrivate::MceoViewFinderFrameReady(CFbsBitmap& aFrame)
{
    if (iVFProcessor) {
        int bytesPerLine = aFrame.ScanLineLength(iViewFinderSize.width(),
            aFrame.DisplayMode());
        QImage image((uchar *)aFrame.DataAddress(), iViewFinderSize.width(), 
        iViewFinderSize.height(), bytesPerLine, QImage::Format_RGB32);
        iVFProcessor->ViewFinderFrameReady(image);     
        iCameraEngine->ReleaseViewFinderBuffer();
    }
}

void XQCameraPrivate::MceoHandleError(TCameraEngineError /*aErrorType*/,
    TInt aError)
{   
    iError = aError;
    emit q->error(error());
}

void XQCameraPrivate::setVFProcessor(MVFProcessor* VFProcessor)
{
    iVFProcessor = VFProcessor;
}

int XQCameraPrivate::camerasAvailable()
{
    return CCameraEngine::CamerasAvailable();
}

XQCamera::Error XQCameraPrivate::error() const
{
    switch (iError) {
    case KErrNone:
        return XQCamera::NoError;
    case KErrNoMemory:
        return XQCamera::OutOfMemoryError;
    default:
        return XQCamera::UnknownError;
    }    
}

// End of file
