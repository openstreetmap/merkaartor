#ifndef MERKATOR_DOWNLOADOSM_H_
#define MERKATOR_DOWNLOADOSM_H_

class Document;

class QHttp;
class QString;
class QMainWindow;
class QProgressBar;
class QLabel;
class QProgressDialog;
class QTimer;
class MainWindow;
class CoordBox;
class Feature;
class Layer;

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
        QString getURLToFetchFull(QString id);
        QString getURLToFetchFull(Feature* aFeature);
        QString getURLToFetch(const QString& What);
        QString getURLToFetch(const QString& What, const QString& Id);
        QString getURLToCreate(const QString& What);
        QString getURLToUpdate(const QString& What, const QString& Id);
        QString getURLToDelete(const QString& What, const QString& Id);
        QString getURLToOpenChangeSet();
        QString getURLToCloseChangeSet(const QString& Id);
        QString getURLToUploadDiff(QString changesetId);
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

bool downloadOSM(MainWindow* Main, const CoordBox& aBox , Document* theDocument);
bool downloadMoreOSM(MainWindow* Main, const CoordBox& aBox , Document* theDocument);
bool downloadOSM(QWidget* Main, const QString& aUser, const QString& aPassword, const quint32 region , Document* theDocument, Layer* theLayer);
bool downloadFeatures(MainWindow* Main, const QList<Feature*>& aDownloadList , Document* theDocument);
bool downloadFeature(MainWindow* Main, const QString& id, Document* theDocument, Layer* theLayer=NULL);
bool downloadFeatures(MainWindow* Main, const QList<QString>& aDownloadList, Document* theDocument, Layer* theLayer=NULL);

bool checkForConflicts(Document* theDocument);

#endif


