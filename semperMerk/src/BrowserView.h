#ifndef BROWSERVIEW_H
#define BROWSERVIEW_H

#include <QWidget>
#include <QVector>
#include <QSslError>
#include <QWebPage>
#include <QList>

class MainWindow;
class QUrl;
class MWebPage;
class MWebView;
class QWebView;
class QNetworkReply;
class QTimeLine;
class TitleBar;
class ControlStrip;
class ZoomStrip;
class PersistentCookieJar;
class Downloader;
class BrowserCache;

class BrowserView : public QWidget
{
    Q_OBJECT

    friend class MWebView;

public:
    BrowserView(MainWindow *parent = 0);
    ~BrowserView();

    QString getCurrentName();
    QString getCurrentAdress();

    void updateButtons();
public slots:
    void showMedia(const QUrl &url);
    void navigate(const QUrl &url);

private slots:
    void initialize();
    void start();
    void setProgress(int percent);
    void finish(bool);
    void updateTitleBar();
    void updateHistory(const QUrl& url);
#ifndef QT_NO_OPENSSL
    void slotSslErrors(QNetworkReply* rep,QList<QSslError>);
#endif
    void slotFrameCreated(QWebFrame * frame);
    void slotFrameInitialLayoutCompleted();
    void backView();
    void fwdView();
    void stop();
    void zoomBest();
    void reload();

    void slideFwd(int pos);
    void slideFwdDone();
    void slideBack(int pos);
    void slideBackDone();

    void onDownloadRequested (const QNetworkRequest & request);
    void onUnsupportedContent (QNetworkReply * reply);
    void unsupportedReplyFinished();

signals:
    void menuButtonClicked();
    void preferencesClicked();

protected:
    void cleanupMemory();
    void addWebview(bool appendToQ = true);

    void resizeEvent(QResizeEvent *event);
    void gotoHomePage();

protected:
    TitleBar *m_titleBar;
    QList<MWebView *> m_webQ;
    MWebView *oldWebView;
    MWebView *curWebView;
    MWebView *expiredWebView;
    MWebPage *m_webPage;
    BrowserCache* m_cache;
    PersistentCookieJar* m_cookiejar;
//    ZoomStrip *m_zoomStrip;
    ControlStrip *m_controlStrip;
    int m_progress;
    QTimeLine *m_timeLineFwd;
    QTimeLine *m_timeLineBack;

    QList<QUrl> m_history;
    int m_curHistIdx;

    Downloader* m_downloader;
    QNetworkReply* unsupReply;
    bool unsupHandled;
};

#endif // BROWSERVIEW_H
