#ifndef XQAUDIO_H
#define XQAUDIO_H

// INCLUDES
#include <QObject>

class XQAudioRecordPrivate;

// CLASS DECLARATION
class XQAudioRecord : public QObject
{
    Q_OBJECT
public:
    XQAudioRecord(QObject *parent = 0);
    ~XQAudioRecord();
    
    enum Encoding {
        EncodingAMR = 0,
        EncodingPCM
    };
    
    enum Error {
        NoError = 0,
        StreamInUseCannotRecordError,
        RecordingStreamFailedError,
        ReadingStreamDataFromInputError,
        OutOfMemoryError,
        UnknownError = -1
    };
    
    enum Status {
        RecordingStarted = 0,
        RecordingStopped
    };
    
    bool setEncoding(XQAudioRecord::Encoding encoding);
    QString fileName() const;
    
    XQAudioRecord::Error error() const;
    
public Q_SLOTS:
    bool start(const QString &file = NULL);
    void stop();

    bool startStream();
    void stopStream();
    
Q_SIGNALS:
    void statusChanged(XQAudioRecord::Status status);
    void frameRead(QByteArray frame);
    
private:
    friend class XQAudioRecordPrivate;
    XQAudioRecordPrivate* d;
};

#endif // XQAUDIO_H

// End of file
