#ifndef XQVIEWFINDERWIDGET_P_H
#define XQVIEWFINDERWIDGET_P_H

// INCLUDES
#include <e32base.h>
#include "xqviewfinderwidget.h"
#include "xqcamera.h"
#include "xqcamera_p.h"

// FORWARD DECLARATIONS
class XQViewFinderWidget;

// CLASS DECLARATION
class XQViewFinderWidgetPrivate: public CBase, public MVFProcessor
{
    
public:
    XQViewFinderWidgetPrivate(XQViewFinderWidget *qq);
    ~XQViewFinderWidgetPrivate();

public:
    void setCamera(XQCamera& cameraObject);
    bool start();
    void stop();
    void setImage(const QImage &image);
    void setViewfinderSize(QSize size);
    XQViewFinderWidget::Error error() const;

protected:
    void paintEvent(QPaintEvent *event);
    
private: //from MVFProcessor
    void ViewFinderFrameReady(const QImage& image);

private:
    friend class XQViewFinderWidget; 
    XQCamera *iCamera;
    XQViewFinderWidget *q;
    QPixmap iPixmapImage;
    int iError;
};

#endif /*XQVIEWFINDERWIDGET_P_H*/

// End of file
