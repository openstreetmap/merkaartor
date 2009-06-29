#ifndef MERKATOR_DOWNLOADOSM_H_
#define MERKATOR_DOWNLOADOSM_H_

class MapDocument;

class QHttp;
class QString;
class QMainWindow;
class QProgressBar;
class QLabel;
class QProgressDialog;
class QTimer;
class MainWindow;
class CoordBox;
class MapFeature;
class MapLayer;

#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtNetwork/QHttp>


class Downloader : public QObject
{
	Q_OBJECT

	public:
		Downloader(const QString& aWeb, const QString& aUser, const QString& aPwd);

		bool request(const QString& Method, const QString& URL, const QString& Out);
		bool go(const QString& url);
		QByteArray& content();
		int resultCode();
		const QString & resultText();
		const QString & errorText();
		const QString & locationText();
		QString getURLToMap();
		QString getURLToTrackPoints();
		QString getURLToFetchFull(MapFeature* aFeature);
		QString getURLToFetch(const QString& What);
		QString getURLToFetch(const QString& What, const QString& Id);
		QString getURLToCreate(const QString& What);
		QString getURLToUpdate(const QString& What, const QString& Id);
		QString getURLToDelete(const QString& What, const QString& Id);
		QString getURLToOpenChangeSet();
		QString getURLToCloseChangeSet(const QString& Id);
		void setAnimator(QProgressDialog *anAnimator, QLabel* AnimatorLabel, QProgressBar* AnimatorBar, bool anAnimate);

	public slots:
		void progress( int done, int total );
		void on_requestFinished(int Id, bool Error);
		void on_responseHeaderReceived(const QHttpResponseHeader & hdr);
		void animate();
		void on_Cancel_clicked();

	private:
		int Port;
		QHttp Request;
		QString Web, User, Password;
		QByteArray Content;
		int Result;
		QString LocationText;
		QString ResultText;
		QString ErrorText;
		int Id;
		int IdAuth;
		bool Error;
		QEventLoop Loop;
		QLabel* AnimatorLabel;
		QProgressBar* AnimatorBar;
		QTimer *AnimationTimer;
};

bool downloadOSM(QWidget* aParent, const CoordBox& aBox , MapDocument* theDocument);
bool downloadMoreOSM(QWidget* aParent, const CoordBox& aBox , MapDocument* theDocument);
bool downloadOSM(QWidget* Main, const QString& aUser, const QString& aPassword, const quint32 region , MapDocument* theDocument, MapLayer* theLayer);
bool downloadFeatures(QWidget* aParent, const QList<MapFeature*>& aDownloadList , MapDocument* theDocument);

bool checkForConflicts(MapDocument* theDocument);

#endif


