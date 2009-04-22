#include "xqaudio.h"
#include "xqaudio_p.h"

/*!
    \class XQAudioRecord
    \brief The XQAudioRecord class can be used to record from microphone, ongoing call and from audio stream input.
    Example:
    \code
    XQAudioRecord *audio = new XQAudioRecord();
    
    // Use this for recording audio to file named and located as: "c:\\Data\\yyyyMMdd_hh_mm_ss.wav"
    audio->start();
    
    // Use getFileName() function to get generated filename to QString
    QString filename = audio->getFileName();
    
    //OR
    // You can also use this version to save file with specific name or destination.
    audio->start("c:\\Existing_folder\\FileNameHere.wav");
    
    // This stops recording
    audio->stop();
    \endcode
*/

/*!
    Constructs a XQAudioRecord object with the given parent.
*/
XQAudioRecord::XQAudioRecord(QObject* parent) : QObject(parent),
        d(new XQAudioRecordPrivate(this))
{
}

/*!
    This sets audio stream encoding
    
    \param encoding Sets encoding for captured audio stream.
*/
bool XQAudioRecord::setEncoding(XQAudioRecord::Encoding encoding)
{
    return d->setEncoding(encoding);
}

/*!
    Destroys the XQAudio object.
*/
XQAudioRecord::~XQAudioRecord()
{
    delete d;
}

/*!
    Starts recording
    See code snippet above.
    
    \param file Sets full filename to audio is recorded.
*/
bool XQAudioRecord::start(const QString &file)
{
    return d->start(file);
}

/*!
    Stops recording
*/
void XQAudioRecord::stop()
{
    d->stop();
}

/*!
    Returns filename
    See code snippet above.
*/
QString XQAudioRecord::fileName() const
{
    return d->fileName();
}

/*!
    Starts stream recording
    
    \sa frameRead()
*/
bool XQAudioRecord::startStream()
{
    return d->startStream();
}

/*!
    Stops stream recording
*/
void XQAudioRecord::stopStream()
{
    d->stopStream();
}

/*!
    \enum XQAudioRecord::Error
    
    This enum defines the possible errors for a XQAudioRecord object.
*/
/*! \var XQAudioRecord::Error XQAudioRecord::NoError
    No error occured.
*/
/*! \var XQAudioRecord::Error XQAudioRecord::StreamInUseCannotRecordError
    The stream is in use and can't be recorded
*/
/*! \var XQAudioRecord::Error XQAudioRecord::RecordingStreamFailedError
    The stream recording has failed.
*/
/*! \var XQAudioRecord::Error XQAudioRecord::ReadingStreamDataFromInputError
    Error in reading stream data from input.
*/
/*! \var XQAudioRecord::Error XQAudioRecord::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQAudioRecord::Error XQAudioRecord::UnknownError
    Unknown error.
*/

/*!
    \enum XQAudioRecord::Encoding
    
    This enum defines the possible encodings for a XQAudioRecord object. This function is currenty added for future implementations.
*/
/*! \var XQAudioRecord::Encoding XQAudioRecord::EncodingAMR
    Sets encoding to AMR
*/
/*! \var XQAudioRecord::Encoding XQAudioRecord::EncodingPCM
    Sets encoding to PCM
*/

/*!
    \enum XQAudioRecord::Status
    
    This enum defines the possible states for a XQAudioRecord object.
*/
/*! \var XQAudioRecord::Status XQAudioRecord::RecordingStarted
    Recording has been started.
*/
/*! \var XQAudioRecord::Status XQAudioRecord::RecordingStopped
    Recording has been stopped.
*/

/*!
    Returns current error level.
    \return Error code
*/
XQAudioRecord::Error XQAudioRecord::error() const
{
    return d->error();
}

/*!
    \fn void XQAudioRecord::statusChanged(Status status)

    This signal is emitted when the recording status has changed.

    \param status a recording status
    \sa currentStatus()
*/

/*!
    \fn void XQAudioRecord::frameRead(QByteArray frame)

    This signal is emitted when a frame of audio data has been read from the inputs and is ready to be processed.

    \param frame recorded stream frame
    \sa startRecordStream()
*/
