#ifndef XQVIEWFINDERWIDGET_H
#define XQVIEWFINDERWIDGET_H

// INCLUDES
#include <QWidget>

// FORWARD DECLARATIONS
class XQCamera;
class XQViewFinderWidgetPrivate;

// CLASS DECLARATION
class XQViewFinderWidget : public QWidget
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        CameraNotSetError,
        UnknownError = -1
    };
    
    XQViewFinderWidget(QWidget *parent = 0);
    ~XQViewFinderWidget();
    
    void setCamera(XQCamera& cameraObject);
    void setViewfinderSize(QSize size);
    XQViewFinderWidget::Error error() const;
    
public Q_SLOTS:
    bool start();
    void stop();
    
protected:
    void paintEvent(QPaintEvent *event);

public Q_SLOTS:
    void setImage(const QImage &image);
    
private:
    friend class XQViewFinderWidgetPrivate;
    XQViewFinderWidgetPrivate *d;
};

#endif  //XQVIEWFINDERWIDGET_H
