#ifndef XQAUDIO_P_H
#define XQAUDIO_P_H

// INCLUDES
#include <Mda\Common\Audio.h>
#include <Mda\Common\Resource.h>
#include <Mda\Client\Utility.h>
#include <MdaAudioSampleEditor.h>
#include <MdaAudioInputStream.h>

// FORWARD DECLARATIONS
class CMdaAudioClipUtility;
class XQAudioRecord;
class CMdaAudioRecorderUtility;

// CLASS DECLARATION
class XQAudioRecordPrivate : public CBase,
        public MMdaObjectStateChangeObserver,
        public MMdaAudioInputStreamCallback
{
public:
    XQAudioRecordPrivate(XQAudioRecord* qq);
    ~XQAudioRecordPrivate();
    
    enum Errors {
        ErrStreamInUseCannotRecordError = 1,
        ErrRecordingStreamFailedError,
        ErrReadingStreamDataFromInputError
    };
    
    bool setEncoding(XQAudioRecord::Encoding encoding);
    
public:
    
    void setFormat();
    void setCodec();
    
    bool start(const QString &file = NULL);
    void stop();
    QString fileName() const;
    
    bool startStream();
    void stopStream();
    
    XQAudioRecord::Error error();
    
private:
    enum TStatus {
        ENotReady,
        EOpen
    };
    
    void MoscoStateChangeEvent(CBase* aObject, TInt aPreviousState,
            TInt aCurrentState, TInt aErrorCode);
    void MaiscOpenComplete(TInt aError);
    void MaiscBufferCopied(TInt aError, const TDesC8& aBuffer);
    void MaiscRecordComplete(TInt aError);
    
    TPtr8& GetFrame(TUint aFrameIdx);
    
private:
    XQAudioRecord* q;
    
    CMdaAudioRecorderUtility* iRecorder;
    CMdaAudioInputStream* iInputStream;
    
    TUint iFrameCount;
    TUint iFrameSize;
    HBufC8* iStreamBuffer;
    
    TPtr8 iFramePtr;
    TBool iBufferOK;
    
    TFourCC iDefaultEncoding;
    TFourCC iCurrentEncoding;
    
    TMdaAudioDataSettings iStreamSettings;
    
    TStatus iInputStatus;
    
    TUint iStreamIdx;    
    TUint iStreamStart;
    TUint iStreamEnd;
    
    QString iFileName;
    
    mutable int iError;
};

#endif /*XQAUDIO_P_H*/

// End of file
