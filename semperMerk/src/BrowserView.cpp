#include "BrowserView.h"

#include <QtWidgets>
#include <QtNetwork>
#include <QNetworkCookieJar>
#include <QTimeLine>
#include <QMessageBox>
#include <QWebFrame>
#include <Phonon>

#include "MyPreferences.h"

#include "MainWindow.h"
#include "MWebView.h"
#include "ControlStrip.h"
#include "TitleBar.h"
#include "ZoomStrip.h"
#include "Downloader.h"
#include "BrowserCache.h"

class PersistentCookieJar : public QNetworkCookieJar {
public:
    PersistentCookieJar(QObject *parent) : QNetworkCookieJar(parent) { load(); }
    ~PersistentCookieJar() { save(); }

public:
    void save()
    {
        QList<QNetworkCookie> list = allCookies();
        QByteArray data;
        foreach (QNetworkCookie cookie, list) {
            if (!cookie.isSessionCookie()) {
                data.append(cookie.toRawForm());
                data.append("\n");
            }
        }
        QSettings settings;
        settings.setValue("Cookies",data);
    }

    void load()
    {
        QSettings settings;
        QByteArray data = settings.value("Cookies").toByteArray();
        setAllCookies(QNetworkCookie::parseCookies(data));
    }
};

/***************************/

BrowserView::BrowserView(MainWindow *parent)
    : QWidget(parent)
    , m_titleBar(0)
    , oldWebView(0)
    , curWebView(0)
    , expiredWebView(0)
    , m_webPage(0)
    , m_progress(0)
    , m_curHistIdx(-1)
    , m_downloader(0)
    , m_cache(0)
{
    curWebView = new MWebView(this);
    curWebView->settings()->setAttribute(QWebSettings::PluginsEnabled, false);
    m_webPage = new MWebPage(curWebView);
    m_webPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(m_webPage, SIGNAL(linkClicked(QUrl)), this, SLOT(navigate(QUrl)));
    connect(m_webPage, SIGNAL(frameCreated (QWebFrame *)), this, SLOT(slotFrameCreated (QWebFrame *)));
    connect(m_webPage, SIGNAL(windowCloseRequested()), SLOT(backView()));
#ifndef QT_NO_OPENSSL
    connect(m_webPage->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(slotSslErrors(QNetworkReply*,QList<QSslError>)));
#endif
    connect(m_webPage, SIGNAL(downloadRequested (const QNetworkRequest &)), SLOT(onDownloadRequested (const QNetworkRequest &)));
    connect(m_webPage, SIGNAL(unsupportedContent (QNetworkReply *)), SLOT(onUnsupportedContent (QNetworkReply *)));

    m_cookiejar = new PersistentCookieJar(this);
    if (!M_PREFS->getCachingInstantHistorySize()) {
        m_cache = new BrowserCache(this);
        m_webPage->networkAccessManager()->setCache(m_cache);
    }
    m_webPage->networkAccessManager()->setCookieJar(m_cookiejar);
    QWebFrame *frame = m_webPage->mainFrame();
    connect(frame, SIGNAL(initialLayoutCompleted()), SLOT(slotFrameInitialLayoutCompleted()));
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    curWebView->setPage(m_webPage);

    m_titleBar = new TitleBar(this);
//	m_zoomStrip = new ZoomStrip(this);
    m_controlStrip = parent->controlStrip();

//	QVBoxLayout *layout = new QVBoxLayout(this);
//	layout->setSpacing(0);
//	layout->setContentsMargins(0, 0, 0, 0);
//	setLayout(layout);
//	layout->addWidget(m_titleBar);
//	layout->addWidget(curWebView);
//	layout->addWidget(m_controlStrip);
//

    m_timeLineFwd = new QTimeLine(800, this);
    m_timeLineFwd->setCurveShape(QTimeLine::EaseInOutCurve);
    m_timeLineFwd->setUpdateInterval(100);

    m_timeLineBack = new QTimeLine(800, this);
    m_timeLineBack->setCurveShape(QTimeLine::EaseInOutCurve);
    m_timeLineBack->setUpdateInterval(100);

    QTimer::singleShot(0, this, SLOT(initialize()));
}

BrowserView::~BrowserView()
{
    delete m_cookiejar;
}

void BrowserView::initialize()
{
    QPalette pal = curWebView->palette();
    pal.setBrush(QPalette::Base, Qt::white);
    curWebView->setPalette(pal);

    connect(curWebView, SIGNAL(loadStarted()), SLOT(start()));
    connect(curWebView, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(curWebView, SIGNAL(loadFinished(bool)), SLOT(finish(bool)));
    connect(curWebView, SIGNAL(urlChanged(QUrl)), SLOT(updateHistory(QUrl)));

    connect(curWebView, SIGNAL(loadStarted()), m_controlStrip, SLOT(slotLoadingStarted()));
    connect(curWebView, SIGNAL(loadFinished(bool)), m_controlStrip, SLOT(slotLoadingFinished()));

    curWebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    curWebView->setFocus();

    curWebView->setGeometry(0, 0, width(), height());
    curWebView->show();

    gotoHomePage();

    connect(m_timeLineFwd, SIGNAL(frameChanged(int)), SLOT(slideFwd(int)));
    connect(m_timeLineFwd, SIGNAL(finished()), SLOT(slideFwdDone()));
    connect(m_timeLineBack, SIGNAL(frameChanged(int)), SLOT(slideBack(int)));
    connect(m_timeLineBack, SIGNAL(finished()), SLOT(slideBackDone()));

    m_controlStrip->backAct->setEnabled(false);
    m_controlStrip->forwardAct->setEnabled(false);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::JavascriptEnabled, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::PluginsEnabled, true);
}

void BrowserView::cleanupMemory()
{
}

void BrowserView::addWebview(bool appendToQ)
{
    if (appendToQ) {
        oldWebView = curWebView;
        m_webQ.append(oldWebView);
        if (m_webQ.size() > M_PREFS->getCachingInstantHistorySize()) {
            expiredWebView = m_webQ.takeFirst();
            expiredWebView->deleteLater();
            if (expiredWebView == oldWebView)
                oldWebView = NULL;
            expiredWebView = NULL;
        }

        if (m_timeLineFwd->state() == QTimeLine::NotRunning) {
            m_timeLineFwd->setFrameRange(curWebView->pos().x(), curWebView->pos().x() + curWebView->width());
            m_timeLineFwd->start();
        }
    }

//	dynamic_cast<PersistentCookieJar*>(m_webPage->networkAccessManager()->cookieJar())->save();

    curWebView = NULL;
    m_webPage = NULL;
    curWebView = new MWebView(this);
    if (!curWebView) {
        cleanupMemory();
    }
    m_webPage = new MWebPage(curWebView);
    if (!m_webPage) {
        cleanupMemory();
    }
    if (!curWebView || !m_webPage) {
        QMessageBox::critical(0, tr("Memory Allocation error"), tr("Cannot allocate memory.\nBailing out..."));
    }

    m_webPage->setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    connect(m_webPage, SIGNAL(linkClicked(QUrl)), this, SLOT(navigate(QUrl)));
#ifndef QT_NO_OPENSSL
    connect(m_webPage->networkAccessManager(), SIGNAL(sslErrors(QNetworkReply*,QList<QSslError>)), this, SLOT(slotSslErrors(QNetworkReply*,QList<QSslError>)));
#endif
    connect(m_webPage, SIGNAL(frameCreated (QWebFrame *)), this, SLOT(slotFrameCreated (QWebFrame *)));
    connect(m_webPage, SIGNAL(downloadRequested (const QNetworkRequest &)), SLOT(onDownloadRequested (const QNetworkRequest &)));
    connect(m_webPage, SIGNAL(unsupportedContent (QNetworkReply *)), SLOT(onUnsupportedContent (QNetworkReply *)));
    connect(m_webPage, SIGNAL(rtspRequested(QUrl)), SLOT(showMedia (QUrl)));
    if (!M_PREFS->getCachingInstantHistorySize()) {
        if (!m_cache)
            m_cache = new BrowserCache(this);
        m_webPage->networkAccessManager()->setCache(m_cache);
    }
    m_webPage->networkAccessManager()->setCookieJar(m_cookiejar);
    m_cookiejar->setParent(0);
    QWebFrame *frame = m_webPage->mainFrame();
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
    connect(frame, SIGNAL(initialLayoutCompleted()), SLOT(slotFrameInitialLayoutCompleted()));
    curWebView->setPage(m_webPage);

    QPalette pal = curWebView->palette();
    pal.setBrush(QPalette::Base, Qt::white);
    curWebView->setPalette(pal);

    connect(curWebView, SIGNAL(loadStarted()), SLOT(start()));
    connect(curWebView, SIGNAL(loadProgress(int)), SLOT(setProgress(int)));
    connect(curWebView, SIGNAL(loadFinished(bool)), SLOT(finish(bool)));
    connect(curWebView, SIGNAL(urlChanged(QUrl)), SLOT(updateHistory(QUrl)));

    connect(curWebView, SIGNAL(loadStarted()), m_controlStrip, SLOT(slotLoadingStarted()));
    connect(curWebView, SIGNAL(loadFinished(bool)), m_controlStrip, SLOT(slotLoadingFinished()));

    curWebView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
//    curWebView->setFocus();

    curWebView->setGeometry(0, 0, width(), height());

    curWebView->show();
    if (oldWebView)
        curWebView->stackUnder(oldWebView);
}

void BrowserView::gotoHomePage()
{
//    curWebView->setHtml("<big><b>semperWeb</b></big><p>by SemperPax</p>");
    curWebView->setUrl(QUrl("qrc:/html/home.html"));
}

void BrowserView::stop()
{
    curWebView->stop();
}

void BrowserView::zoomBest()
{
    curWebView->zoomBest();
}

void BrowserView::reload()
{
    curWebView->reload();
}

void BrowserView::backView()
{
    if (m_curHistIdx) {
        m_curHistIdx--;
    } else {
        gotoHomePage();
        return;
    }

    if (m_webQ.size()) {
        oldWebView = curWebView;
        curWebView = m_webQ.takeLast();
        curWebView->show();
        expiredWebView = oldWebView;
        oldWebView->stackUnder(oldWebView);
    } else {
        if (M_PREFS->getCachingInstantHistorySize() > 0) {
            oldWebView = curWebView;
            expiredWebView = curWebView;
            addWebview(false);
            curWebView->stackUnder(m_titleBar);
            curWebView->move(curWebView->pos().x() + curWebView->width(), curWebView->pos().y());
        }
        QNetworkRequest req(m_history[m_curHistIdx]);
        req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::AlwaysCache);
        curWebView->page()->mainFrame()->load(req);

    }
    if (m_timeLineBack->state() == QTimeLine::NotRunning && M_PREFS->getCachingInstantHistorySize() > 0) {
        m_timeLineBack->setFrameRange(curWebView->pos().x(), curWebView->pos().x() - curWebView->width());
        m_timeLineBack->start();
    }

    m_controlStrip->backAct->setEnabled((m_curHistIdx >= 0));
    m_controlStrip->forwardAct->setEnabled(true);
}

void BrowserView::fwdView()
{
    if (m_curHistIdx < m_history.size()-1) {
        m_curHistIdx++;
    } else
        return;

    if (M_PREFS->getCachingInstantHistorySize() > 0) {
        addWebview();
    }
    curWebView->page()->mainFrame()->load(m_history[m_curHistIdx]);

    m_controlStrip->backAct->setEnabled(true);
    m_controlStrip->forwardAct->setEnabled((m_curHistIdx < m_history.size()-1));
}

void BrowserView::updateButtons()
{
    m_controlStrip->backAct->setEnabled((m_curHistIdx >= 0));
    m_controlStrip->forwardAct->setEnabled((m_curHistIdx < m_history.size()-1));
}

void BrowserView::slideBack(int pos)
{
    if (oldWebView)
        oldWebView->move(-(oldWebView->size().width() - pos), 0);
    curWebView->move(pos, 0);
//	update();
}

void BrowserView::slideBackDone()
{
    if (oldWebView)
        oldWebView->hide();
    if (expiredWebView) {
        expiredWebView->deleteLater();
        expiredWebView = NULL;
    }
}

void BrowserView::slideFwd(int pos)
{
    curWebView->move(-(curWebView->size().width() - pos), 0);
    if (oldWebView)
        oldWebView->move(pos, 0);
//	update();
}

void BrowserView::slideFwdDone()
{
    if (oldWebView)
        oldWebView->hide();
    if (expiredWebView) {
        expiredWebView->deleteLater();
        expiredWebView = NULL;
    }
}

void BrowserView::start()
{
    m_progress = 0;
    updateTitleBar();
    //m_titleBar->setText(curWebView->url().toString());
}

void BrowserView::setProgress(int percent)
{
    m_progress = percent;
    updateTitleBar();
    //m_titleBar->setText(QString("Loading %1%").arg(percent));
}

void BrowserView::updateTitleBar()
{
    QUrl url = curWebView->url();
    m_titleBar->setHost(url.host());
    m_titleBar->setTitle(curWebView->title());
    m_titleBar->setProgress(m_progress);
}

void BrowserView::updateHistory(const QUrl& url)
{
    if (url.toString() == "about:blank")
        return;
    if (m_curHistIdx < m_history.size()-1 && url == m_history[m_curHistIdx])
        return;

    m_history.erase(m_history.begin()+m_curHistIdx+1, m_history.end());
    m_history.append(url);
    m_curHistIdx++;

    updateButtons();
    updateTitleBar();
}

void BrowserView::finish(bool ok)
{
//    m_webPage->setViewportSize(m_webPage->mainFrame()->contentsSize());

    m_progress = 0;
    updateTitleBar();

    // TODO: handle error
    if (!ok && !unsupHandled) {
        m_titleBar->setTitle("Loading failed.");
        return;
    }

    // Force hidding any scrollbar
    QWebElement body = m_webPage->currentFrame()->findFirstElement("body");
    if (!body.isNull())
        body.setStyleProperty("overflow", "hidden !important");

#if !defined(QT_NO_DEBUG_OUTPUT) && !defined(Q_OS_SYMBIAN)
    QFile f("c:/semperWeb.html");
    f.open(QIODevice::WriteOnly);
    f.write(m_webPage->mainFrame()->toHtml().toUtf8());
    f.close();
#endif
}

void BrowserView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    int h1 = m_titleBar->sizeHint().height();

    m_titleBar->resize(width(), h1);
    curWebView->resize(width(), height());

//    int zw = m_zoomStrip->sizeHint().width();
//    int zh = m_zoomStrip->sizeHint().height();
//    m_zoomStrip->move(width() - zw, (height() - zh) / 2);
}

void BrowserView::showMedia(const QUrl &url)
{
    QWidget *dummy = new QWidget(parentWidget());
    dummy->resize(parentWidget()->size());
    dummy->raise();
    dummy->show();
//    Phonon::MediaObject *mediaObject = new Phonon::MediaObject(this);
//    Phonon::VideoWidget *videoWidget = new Phonon::VideoWidget(dummy);
//    Phonon::createPath(mediaObject, videoWidget);
//    qDebug() << url.toString();
//    Phonon::MediaSource source = Phonon::MediaSource(url.toString());
//    mediaObject->setCurrentSource(source);
//    mediaObject->play();
    Phonon::VideoPlayer *player =
             new Phonon::VideoPlayer(Phonon::VideoCategory, dummy);
    player->play(url);
}

void BrowserView::navigate(const QUrl &url)
{
//    if (url.toString().endsWith("3gp")) {
//        showMedia(url);
//        return;
//    }
//	curWebView->setZoomFactor(INIT_ZOOM);
    QUrl referer = curWebView->page()->mainFrame()->url();
    if (M_PREFS->getCachingInstantHistorySize() > 0) {
        addWebview();
    }
    QNetworkRequest req(url);
    req.setRawHeader(QString("Referer").toLatin1(), referer.toString().toLatin1());
    req.setAttribute(QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferCache);
    unsupHandled = false;
    curWebView->load(req);
}

#ifndef QT_NO_OPENSSL
void BrowserView::slotSslErrors(QNetworkReply* rep,QList<QSslError>)
{
    rep->ignoreSslErrors();
}
#endif

void BrowserView::slotFrameCreated(QWebFrame * frame)
{
    frame->setScrollBarPolicy(Qt::Vertical, Qt::ScrollBarAlwaysOff);
    frame->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);

    connect(frame, SIGNAL(initialLayoutCompleted()), SLOT(slotFrameInitialLayoutCompleted()));
}

void BrowserView::slotFrameInitialLayoutCompleted()
{
}

QString BrowserView::getCurrentName()
{
    return curWebView->title();
}

QString BrowserView::getCurrentAdress()
{
    return curWebView->url().toString();
}

void BrowserView::onUnsupportedContent (QNetworkReply * reply)
{
    if (!reply)
         return;

    updateHistory(reply->url());

    if (reply->error() != QNetworkReply::NoError)
        return;

    if (reply->header(QNetworkRequest::ContentTypeHeader).isValid() != true)
        return;

    if (reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower() == "image/pjpeg") {
        unsupReply = reply;
        unsupReply->setParent(this);
        if (unsupReply->isFinished())
            unsupportedReplyFinished();
        else {
            connect(unsupReply, SIGNAL(finished()), SLOT(unsupportedReplyFinished()));
        }
        unsupHandled = true;
        return;
    }

    if (reply->header(QNetworkRequest::ContentTypeHeader).toString().toLower().startsWith("video")) {
        showMedia(reply->url());
        unsupHandled = true;
        return;
    }

    qDebug() << "Unsuported: " << reply->url() << reply->header(QNetworkRequest::ContentTypeHeader);

    if (QMessageBox::question(0, tr("Launch File"), tr("Do you really want to launch this file?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok) {
        m_downloader = new Downloader(this, reply);
        unsupHandled = true;
    } else
        reply->abort();

//	backView();
}

void BrowserView::onDownloadRequested (const QNetworkRequest & request)
{
    m_downloader = new Downloader(this, request, (QNetworkCookieJar *)m_cookiejar);
    m_downloader->go();
    //FIXME When to delete the Downloader? Async -> no now...
}

void BrowserView::unsupportedReplyFinished()
{
    QByteArray ba = unsupReply->readAll();
    curWebView->setContent(ba, QString("image/jpeg"));
//    QFile* m_out = new QFile("c:/tmp.jpg");
//    m_out->open(QIODevice::WriteOnly);
//    m_out->write(unsupReply->readAll());
//    m_out->close();

    unsupReply->close();
}
