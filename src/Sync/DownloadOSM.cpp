#include "DownloadOSM.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Coord.h"
#include "ImportGPX.h"
#include "ImportExportGdal.h"
#include "ImportOSM.h"
#include "Document.h"
#include "Layer.h"
#include "Feature.h"
#include "TrackSegment.h"
#include "SlippyMapWidget.h"
#include "MerkaartorPreferences.h"
#include "OsmLink.h"

#include "IProgressWindow.h"

#include <ui_DownloadMapDialog.h>

#include <QBuffer>
#include <QTimer>
#include <QComboBox>
#include <QMessageBox>
#include <QProgressBar>
#include <QLabel>
#include <QProgressDialog>
#include <QStatusBar>
#include <QInputDialog>

#include <zlib.h>

// #define DEBUG_EVERY_CALL
// #define DEBUG_MAPCALL_ONLY
// #define DEBUG_NONGET_CALL

/* DOWNLOADER */

Downloader::Downloader(const QString& aUser, const QString& aPwd)
: User(aUser), Password(aPwd),
  Id(0),Error(false), AnimatorLabel(0), AnimatorBar(0), AnimationTimer(0)
{
    //IdAuth = Request.setUser(User.toUtf8(), Password.toUtf8());
//	connect(&Request,SIGNAL(done(bool)), this,SLOT(allDone(bool)));
    connect(&Request,SIGNAL(requestFinished(int, bool)),this,SLOT(on_requestFinished(int, bool)));
    connect(&Request,SIGNAL(dataReadProgress(int, int)), this,SLOT(progress(int, int)));
    connect(&Request, SIGNAL(responseHeaderReceived(const QHttpResponseHeader &)), this, SLOT(on_responseHeaderReceived(const QHttpResponseHeader &)));
}


void Downloader::animate()
{
    if (AnimatorBar && AnimationTimer)
        AnimatorBar->setValue((AnimatorBar->value()+1) % AnimatorBar->maximum());
}

void Downloader::setAnimator(QProgressDialog *anAnimator, QLabel* anAnimatorLabel, QProgressBar* anAnimatorBar, bool anAnimate)
{
    if (AnimationTimer)
        delete AnimationTimer;

    AnimatorLabel = anAnimatorLabel;
    AnimatorBar = anAnimatorBar;
    if (AnimatorBar && anAnimate)
    {
        AnimationTimer = new QTimer(this);
        connect(AnimationTimer,SIGNAL(timeout()),this,SLOT(animate()));
    }
    if (AnimatorBar)
    {
        AnimatorBar->setValue(0);
        if (anAnimator)
            connect(anAnimator,SIGNAL(canceled()),this,SLOT(on_Cancel_clicked()));
        qApp->processEvents();
    }
}

void Downloader::on_Cancel_clicked()
{
    Error = true;
    if (Loop.isRunning())
        Loop.exit(QDialog::Rejected);
}

#include "QtGui/QTextBrowser"

void showDebug(const QString& Method, const QString& URL, const QString& Sent, const QByteArray& arr)
{
    static int Download = 0;
    ++Download;
    QTextBrowser* b = new QTextBrowser;
    b->setWindowTitle(Method + " " + URL+" -- "+QString::number(Download));
    if (Sent.length())
    {
        b->append(Sent+QString("\n"));
        b->append(" <======================== "+QString("\n"));
        b->append(" ========================> "+QString("\n"));
    }
    b->append(QString::fromUtf8(arr));
    b->setAttribute(Qt::WA_DeleteOnClose,true);
    b->show();
    b->raise();
}


#define CHUNK 4096

QByteArray gzipDecode(const QByteArray& In)
{
    QByteArray Total;

    int ret;
    unsigned have;
    z_stream strm;
    char in[CHUNK+2];
    char out[CHUNK+2];
    /* allocate inflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    ret = inflateInit2(&strm,15+32);
    if (ret != Z_OK)
       {
               (void)inflateEnd(&strm);
        return Total;
       }
    int RealSize = In.size();
    for (int i=0; i<RealSize/CHUNK+1; ++i)
    {
        int Left = RealSize-(i*CHUNK);
        if (Left > CHUNK)
            Left = CHUNK;
        memcpy(in,In.constData()+(i*CHUNK),Left);
        strm.avail_in = Left;
        strm.next_in = reinterpret_cast<unsigned char*>(in);

        /* run inflate() on input until output buffer not full */
        do
        {
            strm.avail_out = CHUNK;
            strm.next_out = reinterpret_cast<unsigned char*>(out);
            ret = inflate(&strm, Z_NO_FLUSH);
            if (ret == Z_STREAM_ERROR)
                       {
                               (void)inflateEnd(&strm);
                return Total;
                       }
            switch (ret)
            {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    (void)inflateEnd(&strm);
                    return Total;
            }
            have = CHUNK - strm.avail_out;
            out[have] = 0;
            Total.append(QByteArray(out,have));
        } while (strm.avail_out == 0);
    }
       (void)inflateEnd(&strm);
    return Total;
}




bool Downloader::go(const QUrl& url)
{
    if (Error) return false;

    Request.setProxy(M_PREFS->getProxy(url));
    Request.setHost(url.host(),url.port(80));

    if (AnimationTimer)
        AnimationTimer->start(200);
    Content.clear();
    QBuffer ResponseBuffer(&Content);
    ResponseBuffer.open(QIODevice::WriteOnly);
    QString sReq = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    qDebug() << "Downloader::go: " << url << sReq;
    QHttpRequestHeader Header("GET",sReq);
    Header.setValue("Accept-Encoding", "gzip,deflate");
    Header.setValue("Host",url.host()+':'+QString::number(url.port(80)));
    Header.setValue("User-Agent", USER_AGENT);
    Content.clear();
    Id = Request.request(Header,QByteArray(), &ResponseBuffer);

    if (Loop.exec() == QDialog::Rejected)
    {
        Request.abort();
        return false;
    }

    if (Error)
    {
        QMessageBox::information(0,tr("error"),Request.errorString());
    }
    if (Request.lastResponse().hasContentLength() && Content.size() != (int)Request.lastResponse().contentLength())
    {
        QMessageBox::information(0,tr("didn't download enough"),QString("%1 %2").arg(Content.size()).arg(Request.lastResponse().contentLength()));
    }

    if (Request.lastResponse().hasKey("Content-encoding"))
    {
        QString t(Request.lastResponse().value("Content-encoding"));
        if (t == "gzip")
        {
            QByteArray Uncompressed(gzipDecode(Content));
            Content = Uncompressed;
        }
    }

#ifdef DEBUG_EVERY_CALL
    showDebug("GET", Web.path() + url, QByteArray() ,Content);
#endif
    SAFE_DELETE(AnimationTimer);
    LocationText = Request.lastResponse().value("Location");

    Result = Request.lastResponse().statusCode();
    switch (Result) {
    case 301:
    case 302:
    case 307: {
            qDebug() << "New location: " << LocationText;
            if (!LocationText.isEmpty()) {
                QUrl aURL(LocationText);
                return go(aURL);
            }
            break;
        }
    }

    ResultText = Request.lastResponse().reasonPhrase();
    ErrorText = Request.lastResponse().value("Error");
    return !Error;
}

bool Downloader::request(const QString& Method, const QUrl& url, const QString& Data, bool FireForget)
{
    if (Error) return false;

    Request.setProxy(M_PREFS->getProxy(url));
    Request.setHost(url.host(),url.port(80));
    qDebug() << "Downloader::request: " << url;

    QByteArray ba(Data.toUtf8());
    QBuffer Buf(&ba);

    QString sReq = url.toString(QUrl::RemoveScheme | QUrl::RemoveAuthority);
    QHttpRequestHeader Header(Method,sReq);
    Header.setValue("Host",url.host()+':'+QString::number(url.port(80)));
    Header.setValue("User-Agent", USER_AGENT);
    Header.setValue("Content-Type", "text/xml");

    QString auth = QString("%1:%2").arg(User).arg(Password);
    QByteArray ba_auth = auth.toUtf8().toBase64();
    Header.setValue("Authorization", QString("Basic %1").arg(QString(ba_auth)));

    Content.clear();
    Id = Request.request(Header,ba);

    if (FireForget)
        return true;

    if (Loop.exec() == QDialog::Rejected)
    {
        Request.abort();
        return false;
    }
    Content = Request.readAll();
    if (Request.lastResponse().hasKey("Content-encoding"))
    {
        QString t(Request.lastResponse().value("Content-encoding"));
        if (t == "gzip")
        {
            QByteArray Uncompressed(gzipDecode(Content));
            Content = Uncompressed;
        }
    }
#ifdef DEBUG_NONGET_CALL
    showDebug(Method,URL,Data,Content);
#endif
    Result = Request.lastResponse().statusCode();
    ResultText = Request.lastResponse().reasonPhrase();
    ErrorText = Request.lastResponse().value("Error");
    return !Error;
}

QByteArray& Downloader::content()
{
    return Content;
}

void Downloader::on_responseHeaderReceived(const QHttpResponseHeader & hdr)
{
    //switch (hdr.statusCode()) {
    //	case 200:
    //		break;
    //	case 406:
    //		QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","Duplicate key"));
    //		break;
    //	case 413:
    //		QMessageBox::critical(NULL,QApplication::translate("MerkaartorPreferences","Preferences upload failed"), QApplication::translate("MerkaartorPreferences","More than 150 preferences"));
    //		break;
    //	default:
    //		qDebug() << hdr.statusCode();
    //		qDebug() << hdr.reasonPhrase();
    //		break;
    //}

    qDebug() << "Downloader::on_responseHeaderReceived: " << hdr.statusCode() << hdr.reasonPhrase();
}

void Downloader::on_requestFinished(int anId, bool anError)
{
    if (anError)
        Error = true;
    if ( (anId == Id) && Loop.isRunning() )
        Loop.exit(QDialog::Accepted);
}

void Downloader::progress(int done, int total)
{
    if (AnimatorLabel && AnimatorBar)
    {
        if (done < 10240)
            AnimatorLabel->setText(tr("Downloading from OSM (%n bytes)", "", done));
        else
            AnimatorLabel->setText(tr("Downloading from OSM (%n kBytes)", "", (done/1024)));
        if (AnimationTimer && total != 0)
        {
            SAFE_DELETE(AnimationTimer);
            AnimatorBar->setMaximum(total);
        }
        if (!AnimationTimer && AnimatorBar)
            AnimatorBar->setValue(done);
    }
}

int Downloader::resultCode()
{
    return Result;
}

const QString &Downloader::resultText()
{
    return ResultText;
}

const QString &Downloader::errorText()
{
    return ErrorText;
}

const QString &Downloader::locationText()
{
    return LocationText;
}

QString Downloader::getURLToOpenChangeSet()
{
    return QString("/changeset/create");
}

QString Downloader::getURLToCloseChangeSet(const QString& Id)
{
    return QString("/changeset/%1/close").arg(Id);
}

QString Downloader::getURLToUploadDiff(QString changesetId)
{
    return QString("/changeset/%1/upload").arg(changesetId);
}

QString Downloader::getURLToFetch(const QString &What)
{
    QString URL = QString("/%1?%2=");
    return URL.arg(What).arg(What);
}

QString Downloader::getURLToFetchFull(IFeature::FId id)
{
    QString type;
    if (id.type & IFeature::Point)
        type = "node";
    else if (id.type & IFeature::LineString)
        type = "way";
    else if (id.type & IFeature::OsmRelation)
        type = "relation";

    QString URL = QString("/%1/%2/full");
    return URL.arg(type).arg(id.numId);
}

QString Downloader::getURLToFetchFull(Feature* aFeature)
{
    return getURLToFetchFull(aFeature->id());
}

QString Downloader::getURLToFetch(const QString &What, const QString& Id)
{
    QString URL = QString("/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToCreate(const QString &What)
{
    QString URL = QString("/%1/create");
    return URL.arg(What);
}

QString Downloader::getURLToUpdate(const QString &What, const QString& Id)
{
    QString URL = QString("/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToDelete(const QString &What, const QString& Id)
{
    QString URL = QString("/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToMap()
{
    QString URL("/map?bbox=%1,%2,%3,%4");
    return URL;
}

QString Downloader::getURLToTrackPoints()
{
    QString URL = QString("/trackpoints?bbox=%1,%2,%3,%4&page=%5");
    return URL;
}

bool downloadOSM(QWidget* aParent, const QUrl& theUrl, const QString& aUser, const QString& aPassword, Document* theDocument, Layer* theLayer)
{
    Downloader Rcv(aUser, aPassword);

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(aParent);
    if (aProgressWindow) {

        QProgressDialog* dlg = aProgressWindow->getProgressDialog();
        if (dlg) {
            dlg->setWindowTitle(QApplication::translate("Downloader","Downloading..."));
            dlg->setWindowFlags(dlg->windowFlags() & ~Qt::WindowContextHelpButtonHint);
            dlg->setWindowFlags(dlg->windowFlags() | Qt::MSWindowsFixedSizeDialogHint);
        }

        QProgressBar* Bar = aProgressWindow->getProgressBar();
        Bar->setTextVisible(false);
        Bar->setMaximum(11);

        QLabel* Lbl = aProgressWindow->getProgressLabel();
        Lbl->setText(QApplication::translate("Downloader","Downloading from OSM (connecting)"));

        if (dlg)
            dlg->show();

        Rcv.setAnimator(dlg, Lbl, Bar, true);
    }
    if (!Rcv.go(theUrl))
    {
#ifndef _MOBILE
        aParent->setCursor(QCursor(Qt::ArrowCursor));
#endif
        return false;
    }
#ifdef DEBUG_MAPCALL_ONLY
    showDebug("GET", URL,QByteArray(), Rcv.content());
#endif
    int x = Rcv.resultCode();
    switch (x)
    {
    case 200:
        break;
    case 301:
    case 302:
    case 307: {
        QString aWeb = Rcv.locationText();
        if (!aWeb.isEmpty()) {
            QUrl aURL(aWeb);
            return downloadOSM(aParent, aURL, aUser, aPassword, theDocument, theLayer);
        } else {
            QString msg = QApplication::translate("Downloader","Unexpected http status code (%1)\nServer message is '%2'").arg(x).arg(Rcv.resultText());
            if (!Rcv.errorText().isEmpty())
                msg += QApplication::translate("Downloader", "\nAPI message is '%1'").arg(Rcv.errorText());
            QMessageBox::warning(aParent,QApplication::translate("Downloader","Download failed"), msg);
            return false;
        }
        break;
    }
    case 401:
        QMessageBox::warning(aParent,QApplication::translate("Downloader","Download failed"),QApplication::translate("Downloader","Username/password invalid"));
        return false;
    default:
        QString msg = QApplication::translate("Downloader","Unexpected http status code (%1)\nServer message is '%2'").arg(x).arg(Rcv.resultText());
        if (!Rcv.errorText().isEmpty())
            msg += QApplication::translate("Downloader", "\nAPI message is '%1'").arg(Rcv.errorText());
        QMessageBox::warning(aParent,QApplication::translate("Downloader","Download failed"), msg);
        return false;
    }
    Downloader Down(aUser, aPassword);
    bool OK = importOSM(aParent, Rcv.content(), theDocument, theLayer, &Down);
    return OK;
}

bool downloadOSM(QWidget* aParent, const QString& aWeb, const QString& aUser, const QString& aPassword, const CoordBox& aBox , Document* theDocument, Layer* theLayer)
{
    if (checkForConflicts(theDocument))
    {
        QMessageBox::warning(aParent,QApplication::translate("Downloader","Unresolved conflicts"), QApplication::translate("Downloader","Please resolve existing conflicts first"));
        return false;
    }
    Downloader Rcv(aUser, aPassword);
    QString URL = Rcv.getURLToMap();
    URL = URL.arg(aBox.bottomLeft().x(), 0, 'f').arg(aBox.bottomLeft().y(), 0, 'f').arg(aBox.topRight().x(), 0, 'f').arg(aBox.topRight().y(), 0, 'f');

    QUrl theUrl(aWeb+URL);
    return downloadOSM(aParent, theUrl, aUser, aPassword, theDocument, theLayer);
}

bool downloadTracksFromOSM(QWidget* Main, const QString& aWeb, const QString& aUser, const QString& aPassword, const CoordBox& aBox , Document* theDocument)
{
    Downloader theDownloader(aUser, aPassword);
    QList<TrackLayer*> theTracklayers;
    //TrackMapLayer* trackLayer = new TrackMapLayer(QApplication::translate("Downloader","Downloaded tracks"));
    //theDocument->add(trackLayer);

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(Main);
    if (!aProgressWindow)
        return false;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    dlg->setWindowTitle(QApplication::translate("Downloader","Parsing..."));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    Bar->setTextVisible(false);
    Bar->setMaximum(11);

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

    if (dlg)
        dlg->show();

    theDownloader.setAnimator(dlg,Lbl,Bar,true);
    for (int Page=0; ;++Page)
    {
        Lbl->setText(QApplication::translate("Downloader","Downloading trackpoints %1-%2").arg(Page*5000+1).arg(Page*5000+5000));
        QString URL = theDownloader.getURLToTrackPoints();
        URL = URL.arg(aBox.bottomLeft().x()).
                arg(aBox.bottomLeft().y()).
                arg(aBox.topRight().x()).
                arg(aBox.topRight().y()).
                arg(Page);
        QUrl theUrl(aWeb+URL);
        if (!theDownloader.go(theUrl))
            return false;
        if (theDownloader.resultCode() != 200)
            return false;
        int Before = theTracklayers.size();
        QByteArray Ar(theDownloader.content());
        bool OK = importGPX(Main, Ar, theDocument, theTracklayers, true);
        if (!OK)
            return false;
        if (Before == theTracklayers.size())
            break;
        theTracklayers[theTracklayers.size()-1]->setName(QApplication::translate("Downloader", "Downloaded track - nodes %1-%2").arg(Page*5000+1).arg(Page*5000+5000));
    }
    return true;
}


bool checkForConflicts(Document* theDocument)
{
    for (FeatureIterator it(theDocument); !it.isEnd(); ++it) {
        if (it.get()->lastUpdated() == Feature::OSMServerConflict)
            return true;
    }
    return false;
}

bool downloadFeatures(MainWindow* Main, const QList<Feature*>& aDownloadList , Document* theDocument)
{
    QList<IFeature::FId> list;
    foreach (Feature* F, aDownloadList) {
        list << F->id();
    }

    bool ok = downloadFeatures(Main, list, theDocument, NULL);

    return ok;
}

bool downloadFeature(MainWindow* Main, const IFeature::FId& id, Document* theDocument, Layer* theLayer)
{
    QList<IFeature::FId> list;
    list << id;
    bool ok = downloadFeatures(Main, list, theDocument, theLayer);

    return ok;
}

bool downloadFeatures(MainWindow* Main, const QList<IFeature::FId>& idList , Document* theDocument, Layer* theLayer)
{
    if (!theLayer) {
        if (!theDocument->getLastDownloadLayer()) {
            theLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
            theDocument->add(theLayer);
        } else
            theLayer = (Layer*)theDocument->getLastDownloadLayer();
    }

    QString osmWebsite, osmUser, osmPwd;

    osmWebsite = M_PREFS->getOsmApiUrl();
    osmUser = M_PREFS->getOsmUser();
    osmPwd = M_PREFS->getOsmPassword();

    if (Main)
        Main->view()->setUpdatesEnabled(false);

    bool OK = true;
    Downloader Rcv(osmUser, osmPwd);

    for (int i=0; i<idList.size(); ++i) {
        QString URL = Rcv.getURLToFetchFull(idList[i]);
        QUrl theUrl(osmWebsite+URL);

        downloadOSM(Main, theUrl, osmUser, osmPwd, theDocument, theLayer);
    }

    if (Main)
        Main->view()->setUpdatesEnabled(true);
    if (OK)
    {
        if (Main)
            Main->invalidateView();
    } else
    {
        if (theLayer != theDocument->getLastDownloadLayer()) {
            theDocument->remove(theLayer);
            delete theLayer;
        }
    }
    return OK;
}

bool downloadMapdust(MainWindow* Main, const CoordBox& aBox, Document* theDocument, SpecialLayer* theLayer)
{
    QUrl url;

    url.setUrl(M_PREFS->getMapdustUrl());

    if (Main)
        Main->view()->setUpdatesEnabled(false);

    Downloader theDownloader("", "");

    SpecialLayer* trackLayer = theLayer;
    if (!trackLayer) {
        trackLayer = new SpecialLayer(QApplication::translate("Downloader","MapDust"), Layer::MapDustLayer);
        trackLayer->setUploadable(false);
        theDocument->add(trackLayer);
    }

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(Main);
    if (!aProgressWindow)
        return false;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    dlg->setWindowTitle(QApplication::translate("Downloader","Parsing..."));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    Bar->setTextVisible(false);
    Bar->setMaximum(11);

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

    if (dlg)
        dlg->show();

    theDownloader.setAnimator(dlg,Lbl,Bar,true);
    Lbl->setText(QApplication::translate("Downloader","Downloading points"));
    url.addQueryItem("t", COORD2STRING(aBox.topRight().y()));
    url.addQueryItem("l", COORD2STRING(aBox.bottomLeft().x()));
    url.addQueryItem("b", COORD2STRING(aBox.bottomLeft().y()));
    url.addQueryItem("r", COORD2STRING(aBox.topRight().x()));

    if (!theDownloader.go(url))
        return false;
    if (theDownloader.resultCode() != 200)
        return false;
    QByteArray Ar(theDownloader.content());
    ImportExportGdal gdal(theDocument);
    bool OK = gdal.import(trackLayer, Ar, false);

    if (Main)
        Main->view()->setUpdatesEnabled(true);
    if (OK) {
        if (Main)
            Main->invalidateView();
    }
    return OK;
}

bool downloadOpenstreetbugs(MainWindow* Main, const CoordBox& aBox, Document* theDocument, SpecialLayer* theLayer)
{
    QUrl osbUrl;

    osbUrl.setUrl(M_PREFS->getOpenStreetBugsUrl());
    osbUrl.setPath(osbUrl.path() + "getGPX");

    if (Main)
        Main->view()->setUpdatesEnabled(false);

    Downloader theDownloader("", "");

    QList<TrackLayer*> theTracklayers;
    SpecialLayer* trackLayer = theLayer;
    if (!trackLayer) {
        SpecialLayer* trackLayer = new SpecialLayer(QApplication::translate("Downloader","OpenStreetBugs"),Layer::OsmBugsLayer);
        trackLayer->setUploadable(false);
        theDocument->add(trackLayer);
    }
    theTracklayers << trackLayer;

    IProgressWindow* aProgressWindow = dynamic_cast<IProgressWindow*>(Main);
    if (!aProgressWindow)
        return false;

    QProgressDialog* dlg = aProgressWindow->getProgressDialog();
    dlg->setWindowTitle(QApplication::translate("Downloader","Parsing..."));

    QProgressBar* Bar = aProgressWindow->getProgressBar();
    Bar->setTextVisible(false);
    Bar->setMaximum(11);

    QLabel* Lbl = aProgressWindow->getProgressLabel();
    Lbl->setText(QApplication::translate("Downloader","Parsing XML"));

    if (dlg)
        dlg->show();

    theDownloader.setAnimator(dlg,Lbl,Bar,true);
    Lbl->setText(QApplication::translate("Downloader","Downloading points"));
    osbUrl.addQueryItem("t", COORD2STRING(aBox.topRight().y()));
    osbUrl.addQueryItem("l", COORD2STRING(aBox.bottomLeft().x()));
    osbUrl.addQueryItem("b", COORD2STRING(aBox.bottomLeft().y()));
    osbUrl.addQueryItem("r", COORD2STRING(aBox.topRight().x()));
    osbUrl.addQueryItem("open", "yes");

    if (!theDownloader.go(osbUrl))
        return false;
    if (theDownloader.resultCode() != 200)
        return false;
    QByteArray Ar(theDownloader.content());
    bool OK = importGPX(Main, Ar, theDocument, theTracklayers, true);

    if (Main)
        Main->view()->setUpdatesEnabled(true);
    if (OK) {
        if (Main)
            Main->invalidateView();
    }
    return OK;
}

bool downloadMoreOSM(MainWindow* Main, const CoordBox& aBox , Document* theDocument)
{
    Layer* theLayer;
    if (!theDocument->getLastDownloadLayer()) {
        theLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
        theDocument->add(theLayer);
    } else
        theLayer = (Layer*)theDocument->getLastDownloadLayer();

    QString osmWebsite, osmUser, osmPwd;

    osmWebsite = M_PREFS->getOsmApiUrl();
    osmUser = M_PREFS->getOsmUser();
    osmPwd = M_PREFS->getOsmPassword();

    Main->view()->setUpdatesEnabled(false);

    bool OK = true;
    OK = downloadOSM(Main,osmWebsite,osmUser,osmPwd,aBox,theDocument,theLayer);
    Main->view()->setUpdatesEnabled(true);
    if (OK)
    {
        theDocument->setLastDownloadLayer(theLayer);
        theDocument->addDownloadBox(theLayer, aBox);
        // Don't jump around on Download More
        // aParent->view()->projection().setViewport(aBox,aParent->view()->rect());
        Main->invalidateView();
    } else
    {
        if (theLayer != theDocument->getLastDownloadLayer()) {
            theDocument->remove(theLayer);
            delete theLayer;
        }
    }
    return OK;
}

bool downloadOSM(MainWindow* Main, const CoordBox& aBox , Document* theDocument)
{
    QString osmWebsite, osmUser, osmPwd;
    static bool DownloadRaw = false;

    QDialog * dlg = new QDialog(Main);

    osmWebsite = M_PREFS->getOsmApiUrl();
    osmUser = M_PREFS->getOsmUser();
    osmPwd = M_PREFS->getOsmPassword();

    Ui::DownloadMapDialog ui;
    ui.setupUi(dlg);
    SlippyMapWidget* SlippyMap = new SlippyMapWidget(ui.groupBox);
#ifndef _MOBILE
    SlippyMap->setMinimumHeight(256);
#endif
    CoordBox Clip(aBox);
    SlippyMap->setViewportArea(Clip);
    ui.verticalLayout->addWidget(SlippyMap);
    QObject::connect(SlippyMap, SIGNAL(redraw()), ui.FromMap, SLOT(toggle()));
    BookmarkListIterator i(*(M_PREFS->getBookmarks()));
    while (i.hasNext()) {
        i.next();
        if (i.value().deleted == false)
            ui.Bookmarks->addItem(i.key());
    }
    ui.edXapiUrl->setText(QString("*[bbox=%1,%2,%3,%4]").arg(aBox.bottomLeft().x(), 0, 'f').arg(aBox.bottomLeft().y(), 0, 'f').arg(aBox.topRight().x(), 0, 'f').arg(aBox.topRight().y(), 0, 'f'));
    ui.IncludeTracks->setChecked(DownloadRaw);
    ui.ResolveRelations->setChecked(M_PREFS->getResolveRelations());
    bool OK = true, retry = true, directAPI = false;
    QString directUrl;
    while (retry) {
        retry = false;
#ifdef _MOBILE
        dlg->setWindowState(Qt::WindowMaximized);
#endif
        if (dlg->exec() == QDialog::Accepted)
        {
            DownloadRaw = false;
            if (ui.FromBookmark->isChecked())
            {
                Clip = M_PREFS->getBookmarks()->value(ui.Bookmarks->currentText()).Coordinates;
            }
            else if (ui.FromView->isChecked())
            {
                Clip = aBox;
            }
            else if (ui.FromLink->isChecked()) {
                QString link = ui.Link->text();

                if (link.contains("/api/")) {
                    directAPI=true;
                    directUrl = link;
                } else if (link.contains("/browse/")) {
                    QString tag("/browse/");
                    int ix = link.lastIndexOf(tag) + tag.length();
                    directUrl = M_PREFS->getOsmApiUrl();
                    if (!directUrl.endsWith("/")) directUrl += "/";
                    directUrl += link.right(link.length() - ix);
                    if (!directUrl.endsWith("/")) directUrl += "/";
                    directUrl += "full";
                    directAPI=true;
                } else if (link.startsWith("way") || link.startsWith("node") || link.startsWith("relation")) {
                    directUrl = M_PREFS->getOsmApiUrl();
                    if (!directUrl.endsWith("/")) directUrl += "/";
                    directUrl += link;
                    directAPI=true;
                } else {
                    OsmLink ol(link);
                    Clip = ol.getCoordBox();
                    if (Clip.isNull() || Clip.isEmpty())
                        retry = true;
                }
            }
            else if (ui.FromXapi->isChecked())
            {
                directAPI = true;
                directUrl = M_PREFS->getXapiUrl();
                if (!directUrl.endsWith("/")) directUrl += "/";
                directUrl += ui.edXapiUrl->text();
            }
            else if (ui.FromMap->isChecked())
            {
                QRectF R(SlippyMap->viewArea());
                Clip = CoordBox(Coord(R.x(), R.y()), Coord(R.x()+R.width(), R.y()+R.height()));
            }
            if (retry) continue;
            Main->view()->setUpdatesEnabled(false);
            Layer* theLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
            theDocument->add(theLayer);
            M_PREFS->setResolveRelations(ui.ResolveRelations->isChecked());
            if (directAPI) {
                if (ui.FromXapi->isChecked())
                    theLayer->setUploadable(false);
                OK = downloadOSM(Main,QUrl(QUrl::fromEncoded(directUrl.toAscii())),osmUser,osmPwd,theDocument,theLayer);
            }
            else
                OK = downloadOSM(Main,osmWebsite,osmUser,osmPwd,Clip,theDocument,theLayer);
            if (OK && ui.IncludeTracks->isChecked())
                OK = downloadTracksFromOSM(Main,osmWebsite,osmUser,osmPwd, Clip,theDocument);
            Main->view()->setUpdatesEnabled(true);
            if (OK)
            {
                theDocument->setLastDownloadLayer(theLayer);
                theDocument->addDownloadBox(theLayer, Clip);
#ifndef _MOBILE
                if (directAPI)
                    Main->on_viewZoomAllAction_triggered();
                else
#endif
                    Main->view()->setViewport(Clip,Main->view()->rect());
                Main->invalidateView();
            } else {
                retry = true;
                theDocument->remove(theLayer);
                SAFE_DELETE(theLayer);
            }
        }
    }
    delete dlg;
    return OK;
}

