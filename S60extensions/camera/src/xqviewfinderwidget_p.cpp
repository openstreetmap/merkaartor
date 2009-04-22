#include "xqviewfinderwidget.h"
#include "xqviewfinderwidget_p.h"
#include <QPainter>
#include <cameraengine.h>

const int KErrCameraNotSet = -51;

XQViewFinderWidgetPrivate::XQViewFinderWidgetPrivate(XQViewFinderWidget *qq) : q(qq)
{
}

XQViewFinderWidgetPrivate::~XQViewFinderWidgetPrivate()
{
}

bool XQViewFinderWidgetPrivate::start()
{
    if (!iCamera) {
        iError = KErrCameraNotSet;
        return false;
    }
    TRAP(iError,
        TSize size(iCamera->d->iViewFinderSize.width(), iCamera->d->iViewFinderSize.height());
        iCamera->d->iCameraEngine->StartViewFinderL(size);
    )
    return (iError == KErrNone);
}

void XQViewFinderWidgetPrivate::stop()
{
    iCamera->d->iCameraEngine->StopViewFinder();
}

void XQViewFinderWidgetPrivate::ViewFinderFrameReady(const QImage& image)
{
    setImage(image);
}

void XQViewFinderWidgetPrivate::setCamera(XQCamera& cameraObject)
{
    iCamera = &cameraObject;
    iCamera->d->setVFProcessor(this);
}

void XQViewFinderWidgetPrivate::setImage(const QImage &image)
{
    iPixmapImage = QPixmap::fromImage(image);
    q->repaint();
}

void XQViewFinderWidgetPrivate::setViewfinderSize(QSize size)
{
    iCamera->d->iViewFinderSize = size;
}

void XQViewFinderWidgetPrivate::paintEvent(QPaintEvent * /*event*/)
{
    QPainter painter(q);
    QPoint point(q->pos());
    painter.drawPixmap(point, iPixmapImage.scaled(q->size(), Qt::KeepAspectRatio));
}

XQViewFinderWidget::Error XQViewFinderWidgetPrivate::error() const
{
    switch (iError) {
    case KErrNone:
        return XQViewFinderWidget::NoError;
    case KErrNoMemory:
        return XQViewFinderWidget::OutOfMemoryError;
    case KErrCameraNotSet:
        return XQViewFinderWidget::CameraNotSetError;
    default:
        return XQViewFinderWidget::UnknownError;
}    
}

// End of file
