#include "xqlandmarksmanager.h"
#include "xqlandmarksmanager_p.h"

#include <EPos_CPosLandmarkDatabase.h> 
#include <EPos_CPosLmCategoryManager.h>
#include <LbsPosition.h> 

XQLandmarkManagerPrivate::XQLandmarkManagerPrivate(
        XQLandmarkManager *landmarkManager):q(landmarkManager)
{
}

XQLandmarkManagerPrivate::~XQLandmarkManagerPrivate()
{
    delete iDb;  
}

int XQLandmarkManagerPrivate::addLandmark(XQLandmark landmark)
{
    int landmarkId = -1;
    TRAP(iError,     
        if (!iDb) {
            iDb = CPosLandmarkDatabase::OpenL();
        }
        if (iDb->IsInitializingNeeded()) {
            ExecuteAndDeleteLD(iDb->InitializeL());
        } 
        CPosLandmark* lm = CPosLandmark::NewLC();    
    
        TPtrC textPtr(landmark.name().utf16());   
        lm->SetLandmarkNameL(textPtr);
        
        TPtrC textPtr2(landmark.description().utf16());
        lm->SetLandmarkDescriptionL(textPtr2); 
        
        TLocality pos;
        pos.SetCoordinate(landmark.latitude(),landmark.longitude());
        lm->SetPositionL(pos);
    
        landmarkId = iDb->AddLandmarkL(*lm); 
        CleanupStack::PopAndDestroy(lm);
    )
    return landmarkId;
}

XQLandmark XQLandmarkManagerPrivate::landmark(int lmID) const
{
    XQLandmark landmark;  
    TRAP(iError,
        if (!iDb) {
            iDb = CPosLandmarkDatabase::OpenL();
        }
        if (iDb->IsInitializingNeeded()) {
            ExecuteAndDeleteLD(iDb->InitializeL());
        } 

        CPosLandmark* lm = iDb->ReadLandmarkLC(lmID);
        TPtrC landmarkName;
        lm->GetLandmarkName(landmarkName);
        QString string = QString::fromUtf16(landmarkName.Ptr(), landmarkName.Length()); 
        landmark.setName(string);
        
        TLocality pos;
        lm->GetPosition(pos);
        landmark.setPosition(pos.Latitude(),pos.Longitude());
        CleanupStack::PopAndDestroy(lm);
    )
    
    return landmark;
}

QList<int> XQLandmarkManagerPrivate::landmarkIds() const
{
    QList<int> idList;
    TRAP(iError,  
        if (!iDb) {
            iDb = CPosLandmarkDatabase::OpenL();
        }
        if (iDb->IsInitializingNeeded()) {
            ExecuteAndDeleteLD(iDb->InitializeL());
        } 
    
        CPosLmItemIterator* iter = iDb->LandmarkIteratorL();    
        CleanupStack::PushL(iter);
        
        TPosLmItemId lmID = KPosLmNullItemId;
        
        while ((lmID = iter->NextL()) != KPosLmNullItemId) {   
            CPosLandmark* lm = iDb->ReadLandmarkLC(lmID);
          
            TPtrC landmarkName;  
            idList.append(lm->LandmarkId());
            CleanupStack::PopAndDestroy(lm);
        }
        
        CleanupStack::PopAndDestroy(iter);
    )
    return idList;
}

XQLandmarkManager::Error XQLandmarkManagerPrivate::error() const
{
    switch (iError) {
        case KErrNone:
        {
            return XQLandmarkManager::NoError;
        }
        case KErrNoMemory:
        {
            return XQLandmarkManager::OutOfMemoryError;
        }
        default:
        {
            return XQLandmarkManager::UnknownError;
        }
    }    
}

// End of file
