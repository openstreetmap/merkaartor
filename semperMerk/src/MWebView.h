#ifndef MWEBVIEW_H
#define MWEBVIEW_H

#include <QtWebKit>

class MouseMachine;
class BrowserView;
class QPinchGesture;

class MWebPage : public QWebPage
{
    Q_OBJECT

public:
    MWebPage(QObject * parent = 0);
    ~MWebPage();

protected:
    QString userAgentForUrl ( const QUrl & url ) const;
    bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

signals:
    void rtspRequested(QUrl url);
};

class MWebView : public QWebView
{
    Q_OBJECT

public:
    MWebView(BrowserView * parent = 0);

private slots:
    void initialize();

protected:
//	virtual void mouseDoubleClickEvent (QMouseEvent * event);
//  virtual void mousePressEvent ( QMouseEvent * event );
//	virtual void mouseReleaseEvent ( QMouseEvent * event );
//	virtual void mouseMoveEvent ( QMouseEvent * event );
//	virtual void paintEvent ( QPaintEvent * event );
    virtual QWebView *createWindow(QWebPage::WebWindowType type);

    bool event(QEvent *event);
    bool gestureEvent(QGestureEvent *event);
    void pinchTriggered(QPinchGesture *gesture);

protected slots:
//	void mouseRelease ();
    void slotScroll(QPoint oldPos, QPoint newPos);
    void slotClicked(QPoint pos);
    void slotDoubleClicked(QPoint pos);
    void slotTapAndHold(QPoint pos);

public slots:
    void zoomIn();
    void zoomOut();
    void zoomBest();
    void zoom100();

public:
    bool isZoomed();

private:
    BrowserView* m_BrowserView;
    bool doubleClickHold;
    bool possibleSingleClick;
    double scaleRatio;
    double curZoom;
    QPoint lastMouse;
    MouseMachine *charm;
    QPixmap* theImage;
    QMouseEvent * curEvent;

    qreal m_initZoom;
    bool m_zoomedIn;
    bool m_zoomedOut;
};

#endif // MWEBVIEW_H
