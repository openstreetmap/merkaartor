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

/* DOWNLOADER */

Downloader::Downloader(OsmServer server)
: server(server),
  currentReply(0),Error(false), AnimatorLabel(0), AnimatorBar(0), AnimationTimer(0)
{
    /* FIXME: Handle finished request. */
    //connect(&netManager,SIGNAL(finished(QNetworkReply*)),this,SLOT(on_requestFinished(QNetworkReply*)));
}


void Downloader::animate()
{
    if (AnimatorBar && AnimationTimer)
        AnimatorBar->setValue((AnimatorBar->value()+1) % AnimatorBar->maximum());
}

void Downloader::setAnimator(QProgressDialog *anAnimator, QLabel* anAnimatorLabel, QProgressBar* anAnimatorBar, bool anAnimate)
{
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


bool Downloader::go(const QUrl& url) {
    return request("GET", url, QString());
}

bool Downloader::request(const QString& theMethod, const QUrl& url, const QString& theData) {
    if (Error) return false;

    qDebug() << "Downloader::request:" << server->baseUrl().resolved(url);

    QNetworkRequest req(server->baseUrl().resolved(url));

    req.setRawHeader(QByteArray("Content-Type"), QByteArray("text/xml"));
    req.setRawHeader(QByteArray("User-Agent"), USER_AGENT.toLatin1());

    QByteArray dataArray(theData.toUtf8());
    currentReply = server->sendRequest(req, theMethod.toLatin1(), dataArray);
    connect(currentReply, &QNetworkReply::finished, this, [this] {
            qDebug()  << "Downloader::request: received response";
            if (currentReply->error()) {
                Error = true;
                qDebug() << "Downloader::request: received response with code"
                    << currentReply->error() << ", message" << currentReply->errorString();
                qDebug() << "Body: " << currentReply->readAll();
                Loop.exit(QDialog::Rejected);
            } else {
                Loop.exit(QDialog::Accepted);
            }
    }
    );

    if (AnimationTimer) {
        AnimationTimer->start(200);
        connect(currentReply,SIGNAL(downloadProgress(qint64, qint64)), this,SLOT(progress(qint64, qint64)));
    }

    if (Loop.exec() == QDialog::Rejected)
        return false;

    if (AnimationTimer)
        SAFE_DELETE(AnimationTimer);

    /* Test for redirections */
    QVariant redir = currentReply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if (redir.isValid()) {
        LocationText = redir.toString();
        if (!LocationText.isEmpty()) {
            QUrl newUrl(LocationText);
            return request(theMethod, newUrl, theData);
        }
    }

    /* We will log the error, but reporting shall be done at the call site. */
    if (currentReply->error()) {
        qDebug() << "Downloader::request: received response with code"
            << currentReply->error() << ", message" << currentReply->errorString();
    }


    /* Read the data */
    Content = currentReply->readAll();
    Result = currentReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    ResultText = currentReply->errorString();
    ErrorText = currentReply->rawHeader("Error");
    return !Error;
}

QByteArray& Downloader::content()
{
    return Content;
}

void Downloader::progress(qint64 done, qint64 total)
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
    return QString("/api/0.6/changeset/create");
}

QString Downloader::getURLToCloseChangeSet(const QString& Id)
{
    return QString("/api/0.6/changeset/%1/close").arg(Id);
}

QString Downloader::getURLToUploadDiff(QString changesetId)
{
    return QString("/api/0.6/changeset/%1/upload").arg(changesetId);
}

QString Downloader::getURLToFetch(const QString &What)
{
    QString URL = QString("/api/0.6/%1?%2=");
    return URL.arg(What).arg(What);
}

QString Downloader::getURLToFetchFull(IFeature::FId id)
{
    QString type;
	QString URL;
    if (id.type & IFeature::Point) {
        type = "node";
		URL = QString("/api/0.6/%1/%2");
	} else {
		if (id.type & IFeature::LineString)
			type = "way";
		if (id.type & IFeature::OsmRelation)
			type = "relation";
		URL = QString("/api/0.6/%1/%2/full");
	}

    return URL.arg(type).arg(id.numId);
}

QString Downloader::getURLToFetchFull(Feature* aFeature)
{
    return getURLToFetchFull(aFeature->id());
}

QString Downloader::getURLToFetch(const QString &What, const QString& Id)
{
    QString URL = QString("/api/0.6/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToCreate(const QString &What)
{
    QString URL = QString("/api/0.6/%1/create");
    return URL.arg(What);
}

QString Downloader::getURLToUpdate(const QString &What, const QString& Id)
{
    QString URL = QString("/api/0.6/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToDelete(const QString &What, const QString& Id)
{
    QString URL = QString("/api/0.6/%1/%2");
    return URL.arg(What).arg(Id);
}

QString Downloader::getURLToMap()
{
    QString URL("/api/0.6/map?bbox=%1,%2,%3,%4");
    return URL;
}

QString Downloader::getURLToTrackPoints()
{
    QString URL = QString("/api/0.6/trackpoints?bbox=%1,%2,%3,%4&page=%5");
    return URL;
}

bool downloadOSM(QWidget* aParent, const QUrl& theUrl, OsmServer server, Document* theDocument, Layer* theLayer)
{
    Downloader Rcv(server);

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
        /** TODO: Let the caller set the cursor. */
        aParent->setCursor(QCursor(Qt::ArrowCursor));
#endif
    }
    int x = Rcv.resultCode();
    qDebug() << "DownloadOSM: Received OSM API response code:" << x << ", processing.";

    /** We will parse the error codes and display the appropriate messages. */
    switch (x)
    {
    case 200:
        qDebug() << "DownloadOSM: Response OK, processing.";
        break;
    case 301:
    case 302:
    case 307: {
        qDebug() << "FIXME: Relocation to: " << Rcv.locationText();
                  /*
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
        */
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
    Downloader Down(server);
    bool OK = importOSM(aParent, Rcv.content(), theDocument, theLayer, &Down);
    return OK;
}

bool downloadOSM(QWidget* aParent, OsmServer server, const CoordBox& aBox , Document* theDocument, Layer* theLayer)
{
    if (checkForConflicts(theDocument))
    {
        QMessageBox::warning(aParent,QApplication::translate("Downloader","Unresolved conflicts"), QApplication::translate("Downloader","Please resolve existing conflicts first"));
        return false;
    }
    Downloader Rcv(server);
    QString URL = Rcv.getURLToMap();

    if ((fabs(aBox.bottomLeft().x()) < 180.0 && fabs(aBox.topRight().x()) > 180.0) 
     || (fabs(aBox.bottomLeft().x()) > 180.0 && fabs(aBox.topRight().x()) < 180.0)) {
        /* Check for +-180 meridian, and split query in two if necessary */
        int sign = (aBox.bottomLeft().x() > 0) ? 1 : -1;
        CoordBox q1 = aBox, q2 = aBox;
        if (aBox.bottomLeft().x() > 0) {
            q1.setRight(180*sign);
            q2.setLeft(-180*sign);
            q2.setRight(q2.right()-360);
        } else {
            q1.setLeft(180*sign);
            q2.setRight(-180*sign);
            q2.setLeft(q2.left()+360);
        }
        return downloadOSM(aParent, server, q1, theDocument, theLayer)
            && downloadOSM(aParent, server, q2, theDocument, theLayer);

    } else {
        /* Normal code path */
        URL = URL.arg(aBox.bottomLeft().x(), 0, 'f').arg(aBox.bottomLeft().y(), 0, 'f').arg(aBox.topRight().x(), 0, 'f').arg(aBox.topRight().y(), 0, 'f');
        QUrl theUrl(URL);
        return downloadOSM(aParent, theUrl, server, theDocument, theLayer);
    }
}

bool downloadTracksFromOSM(QWidget* Main, OsmServer server, const CoordBox& aBox , Document* theDocument)
{
    Downloader theDownloader(server);
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
        QUrl theUrl(URL);
        if (!theDownloader.go(theUrl))
            return false;
        if (theDownloader.resultCode() != 200)
            return false;
        int Before = theTracklayers.size();
        QByteArray Ar(theDownloader.content());
        bool OK = ImportGPX::import(Main, Ar, theDocument, theTracklayers, ImportGPX::Option::MakeSegmented | ImportGPX::Option::DetectAnonymizedSegments);
        if (!OK)
            return false;
        if (Before == theTracklayers.size())
            break;
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


    if (Main)
        Main->view()->setUpdatesEnabled(false);

    bool OK = true;
    Downloader Rcv(M_PREFS->getOsmServer());

    for (int i=0; i<idList.size(); ++i) {
        QString URL = Rcv.getURLToFetchFull(idList[i]);
        QUrl theUrl(URL);

        downloadOSM(Main, theUrl, M_PREFS->getOsmServer(), theDocument, theLayer);
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

bool downloadMoreOSM(MainWindow* Main, const CoordBox& aBox , Document* theDocument)
{
    Layer* theLayer;
    if (!theDocument->getLastDownloadLayer()) {
        theLayer = new DrawingLayer(QApplication::translate("Downloader","%1 download").arg(QDateTime::currentDateTime().toString(Qt::ISODate)));
        theDocument->add(theLayer);
    } else
        theLayer = (Layer*)theDocument->getLastDownloadLayer();

    OsmServer server = M_PREFS->getOsmServer();

    Main->view()->setUpdatesEnabled(false);

    bool OK = true;
    OK = downloadOSM(Main,server,aBox,theDocument,theLayer);
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

    OsmServer server = M_PREFS->getOsmServer();

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

                if (link.contains("/api/0.6/")) {
                    directAPI=true;
                    directUrl = link;
                } else if (link.contains("/browse/")) {
                    QString tag("/browse/");
                    int ix = link.lastIndexOf(tag) + tag.length();
                    if (!directUrl.endsWith("/")) directUrl += "/";
                    directUrl += link.right(link.length() - ix);
                    if (!directUrl.endsWith("/")) directUrl += "/";
                    directUrl += "full";
                    directAPI=true;
                } else if (link.startsWith("way") || link.startsWith("node") || link.startsWith("relation")) {
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
                //if (!directUrl.endsWith("/")) directUrl += "/";
                directUrl += ui.edXapiUrl->text();
            }
            else if (ui.FromMap->isChecked())
            {
                QRectF R(SlippyMap->selectedArea());
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
                OK = downloadOSM(Main,QUrl(QUrl::fromEncoded(directUrl.toLatin1())),server,theDocument,theLayer);
            }
            else
                OK = downloadOSM(Main,server,Clip,theDocument,theLayer);
            if (OK && ui.IncludeTracks->isChecked())
                OK = downloadTracksFromOSM(Main,server, Clip,theDocument);
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

