#include <QDateTime>
#include <QTextStream>
#include <MdaAudioInputStream.h>
#include <MdaAudioSampleEditor.h>

#include <mmf\common\mmfutilities.h>
#include <Mda\Client\Utility.h>
#include <BAUTILS.H>
#include <f32file.h>
#include <s32file.h>

#include "xqaudio.h"
#include "xqaudio_p.h"

const QString KDefaultFilePath = "C:\\Data\\";

const TInt KFrameCountAMR = 128;
const TInt KFrameSizeAMR = 14;

const TInt KFrameCountPCM = 100;
const TInt KFrameSizePCM = 4096;

XQAudioRecordPrivate::XQAudioRecordPrivate(XQAudioRecord *qq) : q(qq),
        iFrameCount(KFrameCountAMR), iFrameSize(KFrameSizeAMR),
        iStreamBuffer(0), iFramePtr(0,0), iBufferOK(EFalse)
{
}

XQAudioRecordPrivate::~XQAudioRecordPrivate()
{
    if (iRecorder)
    {
        iRecorder->Stop();
        iRecorder->Close();
        delete iRecorder;
    }
    if (iInputStream)
    {
        if (iInputStatus!=ENotReady)
        {
            iInputStream->Stop();
        }
        delete iInputStream;
    }    
    delete iStreamBuffer;
}

bool XQAudioRecordPrivate::start(const QString& file)
{
    TRAP(iError, iRecorder = CMdaAudioRecorderUtility::NewL(*this));
    if (iError != KErrNone)
    {
        return false;
    }
    
    iFileName = file;
    
    if (file.isNull())
    {
        QString dateTime = 
                QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss");
        iFileName = KDefaultFilePath + dateTime + QString(".wav");
    }
    
    TPtrC16 str(reinterpret_cast<const TUint16*>(iFileName.utf16()));
    
    TRAP(iError, iRecorder->OpenFileL(str));
    iRecorder->SetGain(iRecorder->MaxGain());
    return (iError == KErrNone);
}

void XQAudioRecordPrivate::stop()
{
    if (iRecorder)
    {
        iRecorder->Stop();
        iRecorder->Close();
    }
    emit q->statusChanged(XQAudioRecord::RecordingStopped);
}

QString XQAudioRecordPrivate::fileName() const
{
    return iFileName;
}

void XQAudioRecordPrivate::MoscoStateChangeEvent(CBase* aObject,
        TInt aPreviousState, TInt aCurrentState, TInt /*aErrorCode*/)
{
	if (aObject == iRecorder)
    {
		switch(aCurrentState)
		{
            case CMdaAudioClipUtility::EOpen:
            {
                if(aPreviousState == CMdaAudioClipUtility::ENotReady)
                {
                    iRecorder->SetGain(iRecorder->MaxGain());
                    TRAP(iError,
                        iRecorder->RecordL());
                    emit q->statusChanged(XQAudioRecord::RecordingStarted);
                }
                break;
            }
            default:
            {
                break;
            }
		}
	}
}

bool XQAudioRecordPrivate::startStream()
{
    TRAP(iError,iInputStream = CMdaAudioInputStream::NewL(*this));
    if (iError != KErrNone)
    {
        return false;
    }
    iDefaultEncoding = iInputStream->DataType();
    iCurrentEncoding = iDefaultEncoding;

    TRAP(iError, iStreamBuffer = HBufC8::NewMaxL(iFrameSize * iFrameCount));
    
    if (iInputStatus!=ENotReady)
    {
        iError = ErrStreamInUseCannotRecordError;
        return (iError == KErrNone);
    }
    iInputStream->Open(&iStreamSettings);
    return (iError == KErrNone);
}

void XQAudioRecordPrivate::stopStream()
{
    iInputStream->Stop();
    emit q->statusChanged(XQAudioRecord::RecordingStopped);
}

TPtr8& XQAudioRecordPrivate::GetFrame(TUint aFrameIdx)
{
    iFramePtr.Set((TUint8*)(iStreamBuffer->Ptr() + (aFrameIdx * iFrameSize)),
                               iFrameSize,
                               iFrameSize);
    return iFramePtr;
}

void XQAudioRecordPrivate::MaiscOpenComplete(TInt aError)
{
    if (aError==KErrNone && iInputStream)
    {   
        iStreamSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate8000Hz;
        iStreamSettings.iChannels = TMdaAudioDataSettings::EChannelsMono;
        TRAP(iError, iInputStream->SetDataTypeL(KMMFFourCCCodeAMR));
        iInputStream->SetAudioPropertiesL(iStreamSettings.iSampleRate,
                iStreamSettings.iChannels);
        iInputStream->SetGain(iInputStream->MaxGain());
        iInputStream->SetPriority(EPriorityNormal, EMdaPriorityPreferenceTime);        
        iStreamBuffer->Des().FillZ(iFrameCount * iFrameSize);
        iStreamIdx=0;
        iInputStream->ReadL(GetFrame(iStreamIdx));
    }
}

void XQAudioRecordPrivate::MaiscBufferCopied(TInt aError,
        const TDesC8& aBuffer)
{
    if (aError==KErrNone)
    {
        TRAP(iError, iInputStream->ReadL(GetFrame(iStreamIdx)));
        emit q->frameRead(QByteArray((const char*)aBuffer.Ptr(),
                aBuffer.Size()));
    }
}

void XQAudioRecordPrivate::MaiscRecordComplete(TInt aError)
{
    iInputStatus = ENotReady;
    if (aError==KErrNone)
    {
        // normal stream closure
    }
    else
    {
        // completed with error(s)
    }
}

XQAudioRecord::Error XQAudioRecordPrivate::error()
{
    switch (iError)
    {
        case KErrNone:
        {
            return XQAudioRecord::NoError;
        }
        case KErrNoMemory:
        {
            return XQAudioRecord::OutOfMemoryError;
        }
        case ErrRecordingStreamFailedError:
        {
            return XQAudioRecord::RecordingStreamFailedError;
        }
        case ErrReadingStreamDataFromInputError:
        {
            return XQAudioRecord::ReadingStreamDataFromInputError;
        }
        default:
        {
            return XQAudioRecord::UnknownError;
        }
    }    
}

bool XQAudioRecordPrivate::setEncoding(XQAudioRecord::Encoding encoding)
{
    bool retVal = true;
    if (encoding == XQAudioRecord::EncodingAMR)
    {
        TRAP(iError, iInputStream->SetDataTypeL(KMMFFourCCCodeAMR));
        if (iError != KErrNone)
        {
            iCurrentEncoding = iDefaultEncoding;
        }
        else
        {
            iCurrentEncoding = KMMFFourCCCodeAMR;
            iFrameCount = KFrameCountAMR;
            iFrameSize = KFrameSizeAMR;
            retVal = false;
        }
    }
    else if (encoding == XQAudioRecord::EncodingPCM)
    {
        TRAP(iError, iInputStream->SetDataTypeL(KMMFFourCCCodePCM16));
        if (iError != KErrNone)
        {
            iCurrentEncoding = iDefaultEncoding;
        }
        else
        {
            iCurrentEncoding = KMMFFourCCCodePCM16;
            iFrameCount = KFrameCountPCM;
            iFrameSize = KFrameSizePCM;
            retVal = false;
        }
    }
    return retVal;
}
