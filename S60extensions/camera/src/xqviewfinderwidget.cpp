#include "xqviewfinderwidget.h"
#include "xqviewfinderwidget_p.h"

#include <xqcamera.h>
#include <QPainter>

/*!
    \class XQViewFinderWidget
    \brief The XQViewFinderWidget class can be used with the co-operation of the XQCamera to show viewfinder.
    
    Example:
    \code
    XQCamera *camera = new XQCamera(this);
    XQViewFinderWidget *viewFinder = new XQViewFinderWidget(this);
    viewFinder->setCamera(*camera);
    viewFinder->setViewfinderSize(QSize(128, 96));
    connect(captureButton, SIGNAL(clicked()), camera,SLOT(capture()));
    connect(camera, SIGNAL(captureCompleted(QByteArray)), this, SLOT(handleCapture(QByteArray)));
    connect(camera, SIGNAL(cameraReady()), viewFinder, SLOT(start()));
    camera->open(0);
    \endcode
*/

/*!
    Constructs a XQViewFinderWidget object with the given parent.
*/
XQViewFinderWidget::XQViewFinderWidget(QWidget *parent)
    : QWidget(parent), d(new XQViewFinderWidgetPrivate(this))
{
}

/*!
    Destroys the XQViewFinderWidget object.
*/
XQViewFinderWidget::~XQViewFinderWidget()
{
    delete d;
}

/*!
    \enum XQViewFinderWidget::Error

    This enum defines the possible errors for a XQViewFinderWidget object.
*/
/*! \var XQViewFinderWidget::Error XQViewFinderWidget::NoError
    No error occured.
*/
/*! \var XQViewFinderWidget::Error XQViewFinderWidget::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQViewFinderWidget::Error XQViewFinderWidget::CameraNotSetError
    Camera isn't set.
    \sa setCamera()
*/
/*! \var XQViewFinderWidget::Error XQViewFinderWidget::UnknownError
    Unknown error.
*/

/*!
    Starts the viewfinder.
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQViewFinderWidget::Error that indicates which error occurred.
    \sa stop(), error()
*/
bool XQViewFinderWidget::start()
{
    return d->start();
}

/*!
    Stops the viewfinder.
    \sa start() 
*/
void XQViewFinderWidget::stop()
{
    return d->stop();
}


void XQViewFinderWidget::paintEvent(QPaintEvent *event)
{
    d->paintEvent(event);
}

/*!
    Sets the image to the viewfinder widget.
*/
void XQViewFinderWidget::setImage(const QImage &image)
{
    d->setImage(image);
}

/*!
    Sets the camera object 
    \param cameraObject Reference to camera object
*/
void XQViewFinderWidget::setCamera(XQCamera& cameraObject)
{
    d->setCamera(cameraObject);
}

/*!
    Sets the size request for QImage that will be generated from the
    viewfinder.
    \param size Requested image size
    \sa start()
*/
void XQViewFinderWidget::setViewfinderSize(QSize size)
{
    d->setViewfinderSize(size);
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError
    \return Error code
*/
XQViewFinderWidget::Error XQViewFinderWidget::error() const
{
    return d->error();
}
