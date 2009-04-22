#ifndef XQRESOURCEACCESS_H
#define XQRESOURCEACCESS_H

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQResourceAccessPrivate;

// CLASS DECLARATION
class XQResourceAccess : public QObject
{
    Q_OBJECT

public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        FileNotFoundError,
        ResourceNotFoundError,
        ResourceFileOpenError,
        UnknownError = -1
    };
    
    XQResourceAccess(QObject *parent = 0);
    ~XQResourceAccess();
    
    bool openResourceFile(QString fileName);
    QString loadStringFromResourceFile(int resourceId) const;
    
    XQResourceAccess::Error error() const;

private:
    XQResourceAccessPrivate *d;
};

#endif /*XQRESOURCEACCESS_H*/

// End of file
