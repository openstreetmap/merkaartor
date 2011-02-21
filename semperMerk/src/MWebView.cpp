#include "MWebView.h"

#include "MouseMachine/MouseMachine.h"
#include "BrowserView.h"
#include "MyPreferences.h"

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QMenu>
#include <QGesture>

#define ZOOM_STEP 1.10

MWebPage::MWebPage(QObject * parent)
        : QWebPage(parent)
{
    setForwardUnsupportedContent(true);
}

MWebPage::~MWebPage()
{
}

QString MWebPage::userAgentForUrl ( const QUrl & url ) const
{
    if (url.toString().toLower().contains("iphone"))
        return("Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16");
//		return("Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543 Safari/419.3");

    switch (M_PREFS->getGeneralUserAgent())
    {
        case 1:  // Default symbian browser
            return("Mozilla/5.0 (SymbianOS/9.4; U; Series60/5.0 Nokia5800d-1/30.0.011; Profile/MIDP-2.1 Configuration/CLDC-1.1 ) AppleWebKit/532.4 (KHTML, like Gecko) Safari/532.4");
            break;
        case 2:  // iPhone
//			return("Mozilla/5.0 (iPhone; U; CPU like Mac OS X; en) AppleWebKit/420+ (KHTML, like Gecko) Version/3.0 Mobile/1A543 Safari/419.3");
            return("Mozilla/5.0 (iPhone; U; CPU iPhone OS 3_0 like Mac OS X; en-us) AppleWebKit/528.18 (KHTML, like Gecko) Version/4.0 Mobile/7A341 Safari/528.16");
            break;
        case 3:  // Android
//			return("Mozilla/5.0 (Linux; U; Android 1.0; en-us; generic) AppleWebKit/525.10+ (KHTML, like Gecko) Version/3.0.4 Mobile Safari/523.12.2");
            return("Mozilla/5.0 (Linux; U; Android 2.0.1; en-us;sdk Build/ES54) AppleWebKit/530.17+ (KHTML, like Gecko) Version/4.0 Mobile Safari/530.17");
            break;
        default:
            return QWebPage::userAgentForUrl(url);
            break;
    }
}

bool MWebPage::acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type)
{
//	if (QWebPage::acceptNavigationRequest(frame, request, type)) {
//		//	QNetworkProxy myProxy(QNetworkProxy::HttpProxy, "localhost", 8888);
//		//	p->m_webPage->networkAccessManager()->setProxy(myProxy);
//		QNetworkProxyQuery npq(request.url());
//		QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
//		if (listOfProxies.size())
//			networkAccessManager()->setProxy(listOfProxies[0]);
//		return true;
//	} else
//		return false;

    QNetworkProxyQuery npq(request.url());
    QList<QNetworkProxy> listOfProxies = QNetworkProxyFactory::systemProxyForQuery(npq);
    if (listOfProxies.size())
        networkAccessManager()->setProxy(listOfProxies[0]);

//	QNetworkProxy myProxy(QNetworkProxy::HttpProxy, "localhost", 8888);
//	networkAccessManager()->setProxy(myProxy);

    if (request.url().scheme() == "rtsp") {
        emit rtspRequested(request.url());
        return false;
    }
    return QWebPage::acceptNavigationRequest(frame, request, type);

}

/**************************************/

MWebView::MWebView(BrowserView * parent)
    : QWebView(parent), m_BrowserView(parent), doubleClickHold(false), possibleSingleClick(false), charm(0)
    , m_zoomedIn(false), m_zoomedOut(false), m_initZoom(1.0)
{
    setAttribute(Qt::WA_AcceptTouchEvents);
    grabGesture(Qt::PinchGesture);

    QTimer::singleShot(0, this, SLOT(initialize()));
}

void MWebView::initialize()
{
    setMouseTracking(false);
    charm = new MouseMachine(this, MouseMachine::AutoScroll | MouseMachine::SignalTap | MouseMachine::SignalScroll);
    charm->setObjectName("Browser_charm");
    connect(charm, SIGNAL(scroll(QPoint,QPoint)), SLOT(slotScroll(QPoint,QPoint)));
    connect(charm, SIGNAL(singleTap(QPoint)), SLOT(slotClicked(QPoint)));
    connect(charm, SIGNAL(doubleTap(QPoint)), SLOT(slotDoubleClicked(QPoint)));
    connect(charm, SIGNAL(tapAndHold(QPoint)), SLOT(slotTapAndHold(QPoint)));
}

void MWebView::slotScroll(QPoint oldPos, QPoint newPos)
{
    QWebFrame *frame = page()->mainFrame();
    frame->setScrollPosition(frame->scrollPosition() - (newPos - oldPos));
}

void MWebView::slotClicked(QPoint pos)
{
    QWebFrame* f = page()->frameAt(pos);
    if (f) {
        QWebHitTestResult wh = f->hitTestContent(pos);
        if (!wh.linkUrl().isEmpty()) {
    //        QString code = "$('a').each( function () { $(this).css('background-color', 'yellow') } )";
    //        QString code = QString("$(\":contains('%1')\").css('background-color', 'yellow')").arg(wh.linkUrl().toString());
    //        f->evaluateJavaScript(code);
        }
    }
    QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonPress,
                                          pos, Qt::LeftButton,
                                          Qt::LeftButton, Qt::NoModifier);
    QMouseEvent *event2 = new QMouseEvent(QEvent::MouseButtonRelease,
                                          pos, Qt::LeftButton,
                                          Qt::LeftButton, Qt::NoModifier);

    QApplication::postEvent(page(), event1);
    QApplication::postEvent(page(), event2);
}

void MWebView::slotDoubleClicked(QPoint pos)
{
    QWebFrame *frame = page()->mainFrame();

    QPoint p = frame->scrollPosition();
    qDebug() << p;
    qDebug() << zoomFactor();

    qreal targetZoom;
    if (m_zoomedIn || m_zoomedOut) {
        targetZoom = m_initZoom;
        m_zoomedIn = false;
        m_zoomedOut = false;
    } else {
        targetZoom = m_initZoom * 2.0;
        m_zoomedIn = true;
    }
    QPointF p2 =  p * (targetZoom / zoomFactor());
    QPointF pos2 = pos * (targetZoom / zoomFactor());
    p2 += pos2 - pos;

    setZoomFactor(targetZoom);

    frame->setScrollPosition(p2.toPoint());
}

void MWebView::slotTapAndHold(QPoint pos)
{
//	QContextMenuEvent *event1 = new QContextMenuEvent(QContextMenuEvent::Other, pos);
//	QApplication::postEvent(page(), event1);
    page()->updatePositionDependentActions(pos);
    QMenu* ctxMenu = page()->createStandardContextMenu();
    ctxMenu->exec(mapToGlobal(pos));
}

void MWebView::zoomIn()
{
    double currentZoom = zoomFactor() * ZOOM_STEP;
    setZoomFactor(currentZoom);
}

void MWebView::zoomOut()
{
    double currentZoom = zoomFactor() / ZOOM_STEP;
    setZoomFactor(currentZoom);
}

void MWebView::zoomBest()
{
    QWebFrame *frame = page()->mainFrame();

    QPoint lastZoomedPoint = frame->scrollPosition();

    double currentZoom = (double)width() / page()->mainFrame()->contentsSize().width() * zoomFactor();
    setZoomFactor(currentZoom);
    m_zoomedOut = true;
    m_zoomedIn = false;

    frame->setScrollPosition(lastZoomedPoint);
}

bool MWebView::isZoomed()
{
    return (zoomFactor() != m_initZoom);
}

void MWebView::zoom100()
{
    setZoomFactor(m_initZoom);
}

QWebView *MWebView::createWindow(QWebPage::WebWindowType type)
{
    Q_UNUSED(type);

    m_BrowserView->addWebview();
    return qobject_cast<QWebView *>(m_BrowserView->curWebView);
}

bool MWebView::event(QEvent *event)
{
    switch(event->type()){

    case QEvent::TouchBegin:
        //accepting touch begin allows us to get touch updates
        return true;
        break;

    case QEvent::Gesture:
        return gestureEvent(static_cast<QGestureEvent*>(event));
        break;

    default:
        return QWebView::event(event);
    }
}

bool MWebView::gestureEvent(QGestureEvent *event)
{
    qDebug() << "gesture";
    if (QGesture *pinch = event->gesture(Qt::PinchGesture)){
         pinchTriggered(static_cast<QPinchGesture *>(pinch));
    }
    return true;
}

void MWebView::pinchTriggered(QPinchGesture *gesture)
{
    qDebug() << "pinch";
    QPinchGesture::ChangeFlags changeFlags = gesture->changeFlags();
    if (changeFlags & QPinchGesture::ScaleFactorChanged) {
      qreal value = gesture->scaleFactor();
               qreal zoom = value*zoomFactor();

    if(zoom < 2 && zoom > 0.5){
                 qDebug()<< "zooming";
                 setZoomFactor(zoom);
           }
    }
}
