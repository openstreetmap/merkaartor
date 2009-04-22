#ifndef XQLANDMARKMANAGER_P_H
#define XQLANDMARKMANAGER_P_H

// INCLUDES
#include "xqlandmarksmanager.h"

// FORWARD DECLARATIONS
class CPosLandmarkDatabase;

// CLASS DECLARATION
class XQLandmarkManagerPrivate: public CBase
{
public:
    XQLandmarkManagerPrivate(XQLandmarkManager *landmarkManager);
    ~XQLandmarkManagerPrivate();
    
    int addLandmark(XQLandmark landmark);
    XQLandmark  landmark(int lmID) const;
    
    QList<int> landmarkIds() const;
    XQLandmarkManager::Error error() const;
    
private:
    XQLandmarkManager *q;
    mutable CPosLandmarkDatabase* iDb;
    mutable int iError;
};

#endif /*XQLANDMARKMANAGER_P_H*/

// End of file
