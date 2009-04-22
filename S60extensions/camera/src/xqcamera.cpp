#include "xqcamera.h"
#include "xqcamera_p.h"

/*!
    \class XQCamera
    \brief The XQCamera class can be used to take photos with the device's onboard camera.
    
    Example:
    \code
    XQCamera *camera = new XQCamera(this);    
    connect(captureButton,SIGNAL(clicked()),camera,SLOT(capture()));
    connect(camera,SIGNAL(captureCompleted(QByteArray)),this,SLOT(handleCapturedImage(QByteArray)));
    camera->open(0);
    \endcode
*/

/*!
    \enum XQCamera::Error

    This enum defines the possible errors for a XQCamera object.
*/
/*! \var XQCamera::Error XQCamera::NoError
    No error occured.
*/
/*! \var XQCamera::Error XQCamera::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQCamera::Error XQCamera::UnknownError
    Unknown error.
*/

/*!
    Constructs a XQCamera object with the given parent.
*/
XQCamera::XQCamera(QObject *parent)
    : QObject(parent), d(new XQCameraPrivate(this))
{
}

/*!
    Destroys the XQCamera object.
*/
XQCamera::~XQCamera()
{
    delete d;
}

/*!
    Opens the camera. This function also reserves and powers on the camera.
    Signal cameraReady is emitted when the camera is ready to be used.
    
    \param index Camera index to be opened. Default is 0.
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQCamera::Error that indicates which error occurred
    \sa error()
*/
bool XQCamera::open(int index)
{
    return d->open(index);
}

/*!
    Sets the size request for QImage that will be generated when capturing the
    actual images from the camera.
    \param size Requested image size
*/
void XQCamera::setCaptureSize(QSize size)
{
    d->setCaptureSize(size);
}

/*!
    Starts capturing the actual image. Whenever the image is ready
    captureCompleted is emitted.
    
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQCamera::Error that indicates which error occurred
    \sa captureCompleted(), error()
*/
bool XQCamera::capture()
{
    return d->capture();
}

/*!
    Closes the camera. This function also releases and powers off the camera.
*/
void XQCamera::close()
{
    d->close();
}

/*!
    Starts focusing the camera. Signal focused() is emitted when focus procedure
    is completed.
    
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQCamera::Error that indicates which error occurred
    \sa focused()
*/
bool XQCamera::focus()
{
    return d->focus();
}

/*!
    Cancels focus procedure.
*/
void XQCamera::cancelFocus()
{
    d->cancelFocus();
}

/*!
    Returns the number of cameras available in the phone 
*/
int XQCamera::camerasAvailable()
{
    return XQCameraPrivate::camerasAvailable();
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError
    \return Error code
*/
XQCamera::Error XQCamera::error() const
{
    return d->error();
}
    

/*!
    \fn void XQCamera::cameraReady();
    
    This signal is emitted when the camera is ready to be used. The image may
    be from viewfinder of actual capturing.
*/

/*!
    \fn void XQCamera::focused();
    
    This signal is emitted when the focus procedure has completed.
*/

/*!
    \fn void XQCamera::captureCompleted(QByteArray image);
    
    This signal is emitted when the image has been captured.
    \param image Captured image. Note that the buffer is released right after
    this function call.
*/

// End of file
