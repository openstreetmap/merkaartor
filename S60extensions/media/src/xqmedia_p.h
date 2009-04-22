#ifndef XQMEDIA_P_H
#define XQMEDIA_P_H

// INCLUDES
#include "xqmedia.h"
#include <MCLFOperationObserver.h>

#include <e32base.h>
#include <CLFContentListing.hrh>

// FORWARD DECLARATIONS
class MCLFContentListingEngine;
class MCLFItemListModel;
class CCEOperationObserver;
class QStringList;
class CModelOwner;

// CLASS DECLARATION
class XQMediaPrivate: public CBase, public MCLFOperationObserver
{
public:
    XQMediaPrivate(XQMedia *vibra);
    ~XQMediaPrivate();
    
    bool fetch(XQMedia::MediaType type);   
    QImage thumbnail(QString path) const;
    QImage videoThumbnail(QString path) const;
    XQMedia::Error error() const;
    
private: // From MCLFOperationObserver
    void HandleOperationEventL(TCLFOperationEvent aOperationEvent, TInt /*aError*/);
    
private:
    QStringList listMedia(XQMedia::MediaType type);
    
private:
    XQMedia *q;
    MCLFContentListingEngine* iEngine;
    RPointerArray<CModelOwner> iListModelArray;
    XQMedia::MediaType iMediaType;
    mutable int iError;
};

class MCLFItemListModel;

class CModelOwner: public CBase
    {
    public:
        static CModelOwner* NewL( TCLFMediaType type, MCLFItemListModel *itemListModel );
        ~CModelOwner();
    
    private:
        CModelOwner( TCLFMediaType type, MCLFItemListModel *itemListModel );
    
    public:
        const TCLFMediaType Type();
        MCLFItemListModel* Model();
        
    private: 
        TCLFMediaType iModelType;
        MCLFItemListModel *iModel;
    };


#endif /*XQMEDIA_P_H*/

// End of file
