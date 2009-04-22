#ifndef XQUTILS
#define XQUTILS

// INCLUDES
#include <QObject>

// FORWARD DECLARATIONS
class XQUtilsPrivate;

// CLASS DECLARATION
class XQUtils : public QObject
{
     Q_OBJECT
public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        UserCancelledError,
        UnknownError = -1
    };
    
    XQUtils(QObject *parent = 0);
    ~XQUtils();
    
    bool launchFile(const QString& filename);
    XQUtils::Error error() const;
    
public Q_SLOTS:
    void resetInactivityTime();
	
private:
    XQUtilsPrivate *d;
};

#endif // XQUTILS

// End of file
