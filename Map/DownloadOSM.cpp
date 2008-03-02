#include "DownloadOSM.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Map/Coord.h"
#include "Map/ImportGPX.h"
#include "Map/ImportOSM.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/TrackSegment.h"
#include "Utils/SlippyMapWidget.h"
#include "Preferences/MerkaartorPreferences.h"

#include "GeneratedFiles/ui_DownloadMapDialog.h"

#include <QtCore/QBuffer>
#include <QtCore/QSettings>
#include <QtCore/QTimer>
#include <QtGui/QComboBox>
#include <QtGui/QMainWindow>
#include <QtGui/QMessageBox>
#include <QtGui/QProgressBar>
#include <QtGui/QProgressDialog>
#include <QtGui/QStatusBar>

#include "zlib/zlib.h"

// #define DEBUG_EVERY_CALL
// #define DEBUG_MAPCALL_ONLY
// #define DEBUG_NONGET_CALL

/* DOWNLOADER */

Downloader::Downloader(const QString& aWeb, const QString& aUser, const QString& aPwd, bool aUseProxy, const QString& aProxyHost, int aProxyPort)
: Port(80), Web(aWeb), User(aUser), Password(aPwd),
  UseProxy(aUseProxy), ProxyHost(aProxyHost), ProxyPort(aProxyPort),
  Error(false), Animator(0), AnimatorBar(0), AnimationTimer(0)
{
	int p = Web.lastIndexOf(':');
	if (p != -1)
	{
		Port = Web.right(Web.length()-(p+1)).toUInt();
		Web = Web.left(p);
	}
	Request.setHost(Web,Port);
	Request.setUser(User,Password);
//	connect(&Request,SIGNAL(done(bool)), this,SLOT(allDone(bool)));
	connect(&Request,SIGNAL(requestFinished(int, bool)),this,SLOT(on_requestFinished(int, bool)));
	connect(&Request,SIGNAL(dataReadProgress(int, int)), this,SLOT(progress(int, int)));
}


void Downloader::animate()
{
	if (AnimatorBar && AnimationTimer)
		AnimatorBar->setValue((AnimatorBar->value()+1) % AnimatorBar->maximum());
}

void Downloader::setAnimator(QProgressDialog *anAnimator,  QProgressBar* aBar, bool anAnimate)
{
	Animator = anAnimator;
	AnimatorBar = aBar;
	if (Animator && anAnimate)
	{
		if (AnimationTimer)
			delete AnimationTimer;
		AnimationTimer = new QTimer(this);
		connect(AnimationTimer,SIGNAL(timeout()),this,SLOT(animate()));
	}
	if (Animator)
	{
		Animator->setValue(0);
		connect(Animator,SIGNAL(canceled()),this,SLOT(on_Cancel_clicked()));
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
		return Total;
	unsigned int RealSize = In.size();
	for (unsigned int i=0; i<RealSize/CHUNK+1; ++i)
	{
		unsigned int Left = RealSize-(i*CHUNK);
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
				return Total;
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
	return Total;
}




bool Downloader::go(const QString& url)
{
	if (Error) return false;
	if (AnimationTimer)
		AnimationTimer->start(200);
	Content.clear();
	QBuffer ResponseBuffer(&Content);
	ResponseBuffer.open(QIODevice::WriteOnly);
	if (UseProxy)
		Request.setProxy(ProxyHost,ProxyPort);
	QHttpRequestHeader Header("GET",url);
	Header.setValue("Accept-Encoding", "gzip,deflate");
	if (Port == 80)
		Header.setValue("Host",Web);
	else
		Header.setValue("Host",Web+':'+QString::number(Port));
	Content.clear();
	Id = Request.request(Header,QByteArray(), &ResponseBuffer);

	if (Loop.exec() == QDialog::Rejected)
	{
		Request.abort();
		return false;
	}

	if (Error)
	{
		QMessageBox::information(0,"error",Request.errorString());
	}
	if (Content.size() != (int)Request.lastResponse().contentLength())
	{
		QMessageBox::information(0,"didn't download enough",QString("%1 %2").arg(Content.size()).arg(Request.lastResponse().contentLength()));
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
	showDebug("GET", url, QByteArray() ,Content);
#endif
	Result = Request.lastResponse().statusCode();
	delete AnimationTimer;
	AnimationTimer = 0;
	return !Error;
}

bool Downloader::request(const QString& Method, const QString& URL, const QString& Data)
{
	if (Error) return false;
	QByteArray ba(Data.toUtf8());
	QBuffer Buf(&ba);

	if (UseProxy)
		Request.setProxy(ProxyHost,ProxyPort);
	QHttpRequestHeader Header(Method,URL);
	if (Port == 80)
		Header.setValue("Host",Web);
	else
		Header.setValue("Host",Web+':'+QString::number(Port));
	Content.clear();
	Id = Request.request(Header,ba);

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
	return !Error;
}

QByteArray& Downloader::content()
{
	return Content;
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
	if (Animator)
	{
		if (done < 10240)
			Animator->setLabelText(QString("Downloading from OSM (%1 bytes)").arg(done));
		else
			Animator->setLabelText(QString("Downloading from OSM (%1 kBytes)").arg(done/1024));
		if (AnimationTimer && total != 0)
		{
			delete AnimationTimer;
			AnimationTimer = 0;
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

QString Downloader::getURLToFetch(const QString &What)
{
	QString URL = QString("/api/0.4/%1?%2=");
	return URL.arg(What).arg(What);
}


QString Downloader::getURLToFetch(const QString &What, const QString& Id)
{
	QString URL = QString("/api/0.5/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToCreate(const QString &What)
{
	QString URL = QString("/api/0.5/%1/create");
	return URL.arg(What);
}

QString Downloader::getURLToUpdate(const QString &What, const QString& Id)
{
	QString URL = QString("/api/0.5/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToDelete(const QString &What, const QString& Id)
{
	QString URL = QString("/api/0.5/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToMap()
{
	QString URL = QString("/api/0.5/map?bbox=%1,%2,%3,%4");
	return URL;
}

QString Downloader::getURLToTrackPoints()
{
	QString URL = QString("/api/0.5/trackpoints?bbox=%1,%2,%3,%4&page=%5");
	return URL;
}

bool downloadOSM(QMainWindow* aParent, const QString& aWeb, const QString& aUser, const QString& aPassword, bool UseProxy, const QString& ProxyHost, int ProxyPort, const CoordBox& aBox , MapDocument* theDocument)
{
	if (checkForConflicts(theDocument))
	{
		QMessageBox::warning(aParent,MainWindow::tr("Unresolved conflicts"), MainWindow::tr("Please resolve existing conflicts first"));
		return false;
	}
	Downloader Rcv(aWeb, aUser, aPassword, UseProxy, ProxyHost, ProxyPort);
	QString URL = Rcv.getURLToMap();
	URL = URL.arg(radToAng(aBox.bottomLeft().lon())).arg(radToAng(aBox.bottomLeft().lat())).arg(radToAng(aBox.topRight().lon())).arg(radToAng(aBox.topRight().lat()));
	QProgressDialog* ProgressDialog = new QProgressDialog(aParent);
	ProgressDialog->setWindowModality(Qt::ApplicationModal);
	QProgressBar* Bar = new QProgressBar(ProgressDialog);
	Bar->setTextVisible(false);
	ProgressDialog->setBar(Bar);
	ProgressDialog->setMinimumDuration(0);
	ProgressDialog->setLabelText("Downloading from OSM (connecting)");
	ProgressDialog->setMaximum(11);
	Rcv.setAnimator(ProgressDialog,Bar,true);

	if (!Rcv.go(URL))
	{
		aParent->setCursor(QCursor(Qt::ArrowCursor));
		delete ProgressDialog;
		return false;
	}
	delete ProgressDialog;
#ifdef DEBUG_MAPCALL_ONLY
	showDebug("GET", URL,QByteArray(), Rcv.content());
#endif
	int x = Rcv.resultCode();
	switch (x)
	{
	case 200:
		break;
	case 401:
		QMessageBox::warning(aParent,MainWindow::tr("Download failed"),MainWindow::tr("Username/password invalid"));
		return false;
	default:
		QMessageBox::warning(aParent,MainWindow::tr("Download failed"),MainWindow::tr("Unexpected http status code (%1)").arg(x));
		return false;
	}
	Downloader Down(aWeb, aUser, aPassword, UseProxy, ProxyHost, ProxyPort);
	MapLayer* theLayer = new DrawingMapLayer("Download");
	bool OK = importOSM(aParent, Rcv.content(), theDocument, theLayer, &Down);
	if (!OK)
		delete theLayer;
	return OK;
}


bool downloadTracksFromOSM(QMainWindow* Main, const QString& aWeb, const QString& aUser, const QString& aPassword, bool UseProxy, const QString& ProxyHost, int ProxyPort , const CoordBox& aBox , MapDocument* theDocument)
{
	Downloader theDownloader(aWeb, aUser, aPassword, UseProxy, ProxyHost, ProxyPort);
	TrackMapLayer* trackLayer = new TrackMapLayer("Downloaded tracks");
	theDocument->add(trackLayer);
	QProgressDialog ProgressDialog(Main);
	ProgressDialog.setWindowModality(Qt::ApplicationModal);
	QProgressBar* Bar = new QProgressBar(&ProgressDialog);
	Bar->setTextVisible(false);
	ProgressDialog.setBar(Bar);
	ProgressDialog.setMinimumDuration(0);
	ProgressDialog.setMaximum(11);
	ProgressDialog.setValue(1);
	ProgressDialog.show();
	theDownloader.setAnimator(&ProgressDialog,Bar,true);
	for (unsigned int Page=0; ;++Page)
	{
		ProgressDialog.setLabelText(QString("Downloading trackpoints %1-%2").arg(Page*5000+1).arg(Page*5000+5000));
		QString URL = theDownloader.getURLToTrackPoints();
		URL = URL.arg(radToAng(aBox.bottomLeft().lon())).
				arg(radToAng(aBox.bottomLeft().lat())).
				arg(radToAng(aBox.topRight().lon())).
				arg(radToAng(aBox.topRight().lat())).
				arg(Page);
		if (!theDownloader.go(URL))
			return false;
		if (theDownloader.resultCode() != 200)
			return false;
		unsigned int Before = trackLayer->size();
		QByteArray Ar(theDownloader.content());
		bool OK = importGPX(Main, Ar, theDocument, trackLayer, true);
		if (!OK)
			return false;
		if (Before == trackLayer->size())
			break;
	}
	return true;
}


bool checkForConflicts(MapDocument* theDocument)
{
	for (FeatureIterator it(theDocument); !it.isEnd(); ++it)
		if (it.get()->lastUpdated() == MapFeature::OSMServerConflict)
			return true;
	return false;
}

bool downloadOSM(MainWindow* aParent, const CoordBox& aBox , MapDocument* theDocument)
{
	QString osmWebsite, osmUser, osmPwd, proxyHost;
	int proxyPort;
	bool useProxy;
	static bool DownloadRaw = false;

	QDialog * dlg = new QDialog(aParent);

	osmWebsite = MerkaartorPreferences::instance()->getOsmWebsite();
	osmUser = MerkaartorPreferences::instance()->getOsmUser();
	osmPwd = MerkaartorPreferences::instance()->getOsmPassword();

	useProxy = MerkaartorPreferences::instance()->getProxyUse();
	proxyHost = MerkaartorPreferences::instance()->getProxyHost();
	proxyPort = MerkaartorPreferences::instance()->getProxyPort();

	Ui::DownloadMapDialog ui;
	ui.setupUi(dlg);
	SlippyMapWidget* SlippyMap = new SlippyMapWidget(ui.groupBox);
	SlippyMap->setMinimumHeight(256);
	ui.vboxLayout1->addWidget(SlippyMap);

	QStringList DefaultBookmarks;
	DefaultBookmarks << "London" << "51.47" << "-0.20" << "51.51" << "-0.08";
//	DefaultBookmarks << "Rotterdam" << "51.89" << "4.43" << "51.93" << "4.52";
	QStringList Bookmarks(DefaultBookmarks);

	QStringList sl = MerkaartorPreferences::instance()->getBookmarks();
	if (sl.size() > 0)
		Bookmarks = sl;
	for (int i=0; i<Bookmarks.size(); i+=5)
		ui.Bookmarks->addItem(Bookmarks[i]);
	ui.IncludeTracks->setChecked(DownloadRaw);
	bool OK = true;
	if (dlg->exec() == QDialog::Accepted)
	{
		DownloadRaw = false;
		CoordBox Clip(Coord(0,0),Coord(0,0));
		if (ui.FromBookmark->isChecked())
		{
			unsigned int idx = ui.Bookmarks->currentIndex()*5+1;
			Clip = CoordBox(Coord(angToRad(Bookmarks[idx].toDouble()),angToRad(Bookmarks[idx+1].toDouble())),
				Coord(angToRad(Bookmarks[idx+2].toDouble()),angToRad(Bookmarks[idx+3].toDouble())));
		}
		else if (ui.FromView->isChecked())
		{
			Clip = aBox;
		}
		else if (ui.FromviewAndAdd->isChecked())
		{
			Clip = aBox;
			Bookmarks.insert(0,ui.NewBookmark->text());
			Bookmarks.insert(1,QString::number(radToAng(Clip.bottomLeft().lat())));
			Bookmarks.insert(2,QString::number(radToAng(Clip.bottomLeft().lon())));
			Bookmarks.insert(3,QString::number(radToAng(Clip.topRight().lat())));
			Bookmarks.insert(4,QString::number(radToAng(Clip.topRight().lon())));
			MerkaartorPreferences::instance()->setBookmarks(Bookmarks);
		}
		else if (ui.FromMap->isChecked())
		{
			QRectF R(SlippyMap->viewArea());
			Clip = CoordBox(Coord(R.x(),R.y()),Coord(R.x()+R.width(),R.y()+R.height()));
		}
		aParent->view()->setUpdatesEnabled(false);
		OK = downloadOSM(aParent,osmWebsite,osmUser,osmPwd,useProxy,proxyHost,proxyPort,Clip,theDocument);
		if (OK && ui.IncludeTracks->isChecked())
			OK = downloadTracksFromOSM(aParent,osmWebsite,osmUser,osmPwd,useProxy,proxyHost,proxyPort, Clip,theDocument);
		aParent->view()->setUpdatesEnabled(true);
		if (OK)
		{
			aParent->view()->projection().setViewport(Clip,aParent->view()->rect());
			aParent->invalidateView();

		}
	}
	delete dlg;
	return OK;
}

