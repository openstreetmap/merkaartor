#include "DownloadOSM.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Map/Coord.h"
#include "Map/ImportGPX.h"
#include "Map/ImportOSM.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/TrackSegment.h"

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

// #define DEBUG_EVERY_CALL
// #define DEBUG_MAPCALL_ONLY
// #define DEBUG_NONGET_CALL

/* DOWNLOADER */

Downloader::Downloader(const QString& aWeb, const QString& aUser, const QString& aPwd)
: Port(80), Web(aWeb), User(aUser), Password(aPwd), Error(false), Animator(0), AnimationTimer(0)
{
	int p = Web.lastIndexOf(':');
	if (p != -1)
	{
		Port = Web.right(Web.length()-(p+1)).toUInt();
		Web = Web.left(p);
	}
	Request.setHost(Web,Port);
	Request.setUser(User,Password);
	connect(&Request,SIGNAL(requestFinished(int, bool)), this,SLOT(finished(int, bool)));
	connect(&Request,SIGNAL(dataReadProgress(int, int)), this,SLOT(progress(int, int)));
}


void Downloader::animate()
{
	if (Animator && AnimationTimer)
		Animator->setValue((Animator->value()+1) % Animator->maximum());
}

void Downloader::setAnimator(QProgressDialog *anAnimator, bool anAnimate)
{
	Animator = anAnimator;
	if (Animator && anAnimate)
	{
		if (AnimationTimer)
			delete AnimationTimer;
		AnimationTimer = new QTimer(this);
		connect(AnimationTimer,SIGNAL(timeout()),this,SLOT(animate()));
	}
	if (Animator)
		connect(Animator,SIGNAL(canceled()),this,SLOT(on_Cancel_clicked()));
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


bool Downloader::go(const QString& url)
{
	if (Error) return false;
	if (AnimationTimer)
		AnimationTimer->start(200);
	Content.clear();
	Id = Request.get(url);
	if (Loop.exec() == QDialog::Rejected)
	{
		Request.abort();
		return false;
	}
	Content = Request.readAll();
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

void Downloader::finished(int id, bool error)
{
	if (error)
		Error = true;
	if ( (Id == id) && (Loop.isRunning()) )
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
			Animator->setMaximum(total);
		}
		if (!AnimationTimer)
			Animator->setValue(done);
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
	QString URL = QString("/api/0.4/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToCreate(const QString &What)
{
	QString URL = QString("/api/0.4/%1/create");
	return URL.arg(What);
}

QString Downloader::getURLToUpdate(const QString &What, const QString& Id)
{
	QString URL = QString("/api/0.4/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToDelete(const QString &What, const QString& Id)
{
	QString URL = QString("/api/0.4/%1/%2");
	return URL.arg(What).arg(Id);
}

QString Downloader::getURLToMap()
{
	QString URL = QString("/api/0.4/map?bbox=%1,%2,%3,%4");
	return URL;
}

QString Downloader::getURLToTrackPoints()
{
	QString URL = QString("/api/0.4/trackpoints?bbox=%1,%2,%3,%4&page=%5");
	return URL;
}

bool downloadOSM(QMainWindow* aParent, const QString& aWeb, const QString& aUser, const QString& aPassword, const CoordBox& aBox , MapDocument* theDocument)
{
	if (checkForConflicts(theDocument))
	{
		QMessageBox::warning(aParent,MainWindow::tr("Unresolved conflicts"), MainWindow::tr("Please resolve existing conflicts first"));
		return false;
	}
	Downloader Rcv(aWeb, aUser, aPassword);
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
	Rcv.setAnimator(ProgressDialog,true);
	
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
	Downloader Down(aWeb, aUser, aPassword);
	MapLayer* theLayer = new MapLayer("Download");
	bool OK = importOSM(aParent, Rcv.content(), theDocument, theLayer, &Down);
	if (!OK)
		delete theLayer;
	return OK;
}


bool downloadTracksFromOSM(QMainWindow* Main, const QString& aWeb, const QString& aUser, const QString& aPassword, const CoordBox& aBox , MapDocument* theDocument)
{
	Downloader theDownloader(aWeb, aUser, aPassword);
	MapLayer* trackLayer = new MapLayer("Downloaded tracks");
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
	theDownloader.setAnimator(&ProgressDialog,true);
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
		bool OK = importGPX(Main, Ar, theDocument, trackLayer, false);
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
	static bool DownloadRaw = true;
	QDialog * dlg = new QDialog(aParent);
	QSettings Sets;
	Sets.beginGroup("downloadosm");
	Ui::DownloadMapDialog ui;
	ui.setupUi(dlg);
	ui.Website->setText("www.openstreetmap.org");
	QStringList DefaultBookmarks;
	DefaultBookmarks << "London" << "51.47" << "-0.20" << "51.51" << "-0.08";
//	DefaultBookmarks << "Rotterdam" << "51.89" << "4.43" << "51.93" << "4.52";
	QStringList Bookmarks(DefaultBookmarks);
	QVariant V = Sets.value("bookmarks");
	if (!V.isNull())
		Bookmarks = V.toStringList();
	for (int i=0; i<Bookmarks.size(); i+=5)
		ui.Bookmarks->addItem(Bookmarks[i]);
	ui.Username->setText(Sets.value("user").toString());
	ui.Password->setText(Sets.value("password").toString());
	ui.IncludeTracks->setChecked(DownloadRaw);
	bool OK = true;
	if (dlg->exec() == QDialog::Accepted)
	{
		Sets.setValue("user",ui.Username->text());
		Sets.setValue("password",ui.Password->text());
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
			Sets.setValue("bookmarks",Bookmarks);
		}
		aParent->view()->setUpdatesEnabled(false);
		OK = downloadOSM(aParent,ui.Website->text(),ui.Username->text(),ui.Password->text(),Clip,theDocument);
		if (OK && ui.IncludeTracks->isChecked())
			OK = downloadTracksFromOSM(aParent,ui.Website->text(),ui.Username->text(),ui.Password->text(),Clip,theDocument);
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

