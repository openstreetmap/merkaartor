#ifndef MERKATOR_DOWNLOADOSM_H_
#define MERKATOR_DOWNLOADOSM_H_

class MapDocument;

class QHttp;
class QString;
class QMainWindow;
class QProgressBar;
class QProgressDialog;
class QTimer;
class MainWindow;
class CoordBox;

#include <QtCore/QByteArray>
#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtNetwork/QHttp>


class Downloader : public QObject
{
	Q_OBJECT

	public:
		Downloader(const QString& aWeb, const QString& aUser, const QString& aPwd, bool UseProxy, const QString& aProxyHost, int aProxyPort);

		bool request(const QString& Method, const QString& URL, const QString& Out);
		bool go(const QString& url);
		QByteArray& content();
		int resultCode();
		const QString & resultText();
		QString getURLToMap();
		QString getURLToTrackPoints();
		QString getURLToFetch(const QString& What);
		QString getURLToFetch(const QString& What, const QString& Id);
		QString getURLToCreate(const QString& What);
		QString getURLToUpdate(const QString& What, const QString& Id);
		QString getURLToDelete(const QString& What, const QString& Id);
		void setAnimator(QProgressDialog* Animator, QProgressBar* AnimatorBar, bool anAnimate);

	public slots:
		void progress( int done, int total );
		void on_requestFinished(int Id, bool Error);
		void animate();
		void on_Cancel_clicked();

	private:
		unsigned int Port;
		QHttp Request;
		QString Web, User, Password;
		bool UseProxy;
		QString ProxyHost;
		int ProxyPort;
		QByteArray Content;
		int Result;
		QString ResultText;
		int Id;
		bool Error;
		QEventLoop Loop;
		QProgressDialog* Animator;
		QProgressBar* AnimatorBar;
		QTimer *AnimationTimer;
};

bool downloadOSM(MainWindow* aParent, const CoordBox& aBox , MapDocument* theDocument);
bool downloadMoreOSM(MainWindow* aParent, const CoordBox& aBox , MapDocument* theDocument);

bool checkForConflicts(MapDocument* theDocument);

#endif


