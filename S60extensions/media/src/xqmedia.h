#ifndef XQMEDIA_H
#define XQMEDIA_H

// INCLUDES
#include <QObject>
#include <QStringList>

// FORWARD DECLARATIONS
class XQMediaPrivate;
class QImage;

// CLASS DECLARATION
class XQMedia : public QObject
{
    Q_OBJECT

public:   
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        UnknownError = -1
    };
    
    enum MediaType {
        MediaTypeMusic          = 0x1,
        MediaTypeSound          = 0x2,
        MediaTypeImage          = 0x3,
        MediaTypeVideo          = 0x4
    };

    XQMedia(QObject *parent = 0);
    ~XQMedia();
    
    bool fetch(XQMedia::MediaType type);
    QImage thumbnail(QString path) const;
    QImage videoThumbnail(QString path) const;
    
    XQMedia::Error error() const;
 
Q_SIGNALS:
    void listAvailable(QStringList stringList);
    
private:
    friend class XQMediaPrivate;
    XQMediaPrivate *d;
};

#endif // XQMEDIA_H

// End of file

