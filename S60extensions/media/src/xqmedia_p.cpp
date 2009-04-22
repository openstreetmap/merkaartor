#include "xqmedia.h"
#include "xqmedia_p.h"

#include <ContentListingFactory.h>
#include <MCLFContentListingEngine.h>
#include <MCLFItemListModel.h>
#include <MCLFModifiableItem.h>
#include <CLFContentListing.hrh>
#include <f32file.h> 
#include <ExifRead.h> 

#include <QtGui/QImage>

XQMediaPrivate::XQMediaPrivate(XQMedia *media) : q(media)
{
}

XQMediaPrivate::~XQMediaPrivate()
{
    iListModelArray.ResetAndDestroy();
    delete iEngine;
}

bool XQMediaPrivate::fetch(XQMedia::MediaType type)
{
    iMediaType = type;
    TRAP(iError,
        if (!iEngine) {
            iEngine = ContentListingFactory::NewContentListingEngineLC();
                CleanupStack::Pop();
        }     
        CModelOwner *modelOwner = CModelOwner::NewL(static_cast<TCLFMediaType>(type),
                iEngine->CreateListModelLC(*this));
        CleanupStack::Pop();
        
        RArray<TInt> array;
        CleanupClosePushL(array);
        array.AppendL(static_cast<TCLFMediaType>(type));
        
        if (iListModelArray.Count() == 0) {
            iListModelArray.Append(modelOwner);
            iListModelArray[0]->Model()->SetWantedMediaTypesL(array.Array());
            iListModelArray[0]->Model()->RefreshL();
        } else {
            bool found = false;
            for (int i = 0; i<iListModelArray.Count(); i++) {
                if (iListModelArray[i]->Type() == modelOwner->Type()) {
                    emit q->listAvailable(listMedia(iMediaType));
                    found = true;
                }
            }
            if (!found) {
                iListModelArray.Append(modelOwner);
                iListModelArray[iListModelArray.Count()-1]->Model()->
                        SetWantedMediaTypesL(array.Array());
                iListModelArray[iListModelArray.Count()-1]->Model()->RefreshL();
            }
        }
        CleanupStack::PopAndDestroy(&array);
    )

    return (iError == KErrNone);
}

QStringList XQMediaPrivate::listMedia(XQMedia::MediaType type)
{
    QStringList fileList;
    
    for (int i = 0; i<iListModelArray.Count(); i++) {     
        if (iListModelArray[i]->Type() == static_cast<TCLFMediaType>(type)) {
            int items = iListModelArray[i]->Model()->ItemCount();
            for (int j = 0; j < iListModelArray[i]->Model()->ItemCount(); j++) {
                const MCLFItem& myItem = iListModelArray[i]->Model()->Item(j);
                
                TPtrC fileName;
                iError = iListModelArray[i]->Model()->
                    Item(j).GetField(ECLFFieldIdFileNameAndPath, fileName);
                if (iError == KErrNone) {
                    QString path = QString::fromUtf16(fileName.Ptr(),
                            fileName.Length());
                    fileList.append(path);
                }
            }
            return fileList;
        }
    }
        
    return fileList;
}

void XQMediaPrivate::HandleOperationEventL(TCLFOperationEvent aOperationEvent,
        TInt /*aError*/ )
{
    if (aOperationEvent == ECLFRefreshComplete) {
        emit q->listAvailable(listMedia(iMediaType));
    }
}

QImage XQMediaPrivate::thumbnail(QString path) const
{
    QImage image;   
    RFile file;
    
    TPtrC pathPtr(reinterpret_cast<const TUint16*>(path.utf16()));
    RFs fs;
    
    TRAP(iError,
        User::LeaveIfError(fs.Connect());
        CleanupClosePushL(fs);
        User::LeaveIfError(file.Open(fs, pathPtr.AllocL()->Des(), EFileRead));
        CleanupClosePushL(file);
        TInt size = 0;
        file.Size(size);
        HBufC8* exif = HBufC8::NewLC(size);
        TPtr8 bufferDes(exif->Des()); 
        User::LeaveIfError(file.Read(bufferDes));
        
        CExifRead* read = CExifRead::NewL(exif->Des());
        CleanupStack::PushL(read);
        
        HBufC8* data = read->GetThumbnailL();
        CleanupStack::PushL(data);
        
        image = QImage::fromData(data->Ptr(), data->Size());
        
        CleanupStack::PopAndDestroy(5, &fs);
    )  
    return image; 
}

QImage XQMediaPrivate::videoThumbnail(QString /*path*/) const
{
    //TBD
    return QImage();
}

XQMedia::Error XQMediaPrivate::error() const
{
    switch (iError) {
        case KErrNone:
            return XQMedia::NoError;
        case KErrNoMemory:
            return XQMedia::OutOfMemoryError;
        default:
            return XQMedia::UnknownError;
    }    
}

CModelOwner::CModelOwner( TCLFMediaType type, MCLFItemListModel *itemListModel ):
    iModelType( type ), iModel( itemListModel )
    {
    }

CModelOwner* CModelOwner::NewL( TCLFMediaType type, MCLFItemListModel *itemListModel )
    {
    CModelOwner* self = new ( ELeave ) CModelOwner( type, itemListModel );
    return self;
    }

CModelOwner::~CModelOwner()
    {
    delete iModel;
    }

const TCLFMediaType CModelOwner::Type()
    {
    return iModelType;
    }

MCLFItemListModel* CModelOwner::Model()
    {
    return iModel;
    }

// End of file
