#ifndef XQRESOURCEACCESSPRIVATE_H
#define XQRESOURCEACCESSPRIVATE_H

// INCLUDES
#include "xqresourceaccess.h"
#include "xqresourceaccess_p.h"
#include <barsc.h>

// CLASS DECLARATION
class XQResourceAccessPrivate: public CBase
{
public:
    XQResourceAccessPrivate(XQResourceAccess *resourceAccess);
    ~XQResourceAccessPrivate();
    
    QString loadStringFromResourceFile(int resourceId) const;
    bool openResourceFile(QString file);    
    XQResourceAccess::Error error() const;

private:
    XQResourceAccess *q;
    RFs iFsSession;
    RResourceFile iResourceFile;
    mutable int iError;
    bool iResourceFileOpen;
};

#endif /*XQRESOURCEACCESSPRIVATE_H*/

// End of file
