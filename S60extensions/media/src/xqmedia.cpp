#include "xqmedia.h"
#include "xqmedia_p.h"
#include <QImage>

/*!
    \class XQMedia

    \brief The XQMedia class can be used for retrieving lists of music, image, video and sound files located in the gallery

    Example:
    \code
    XQMedia *media = new XQMedia(this);
    media->fetch(XQMedia::MediaTypeImage);
    connect(media, SIGNAL(listAvailable(QStringList)), this, SLOT(handleFileList(QStringList)));
    \endcode
*/

/*!
    Constructs a XQMedia object with the given parent
    \sa fetch()
*/
XQMedia::XQMedia(QObject *parent)
 : QObject(parent), d(new XQMediaPrivate(this))
{
}

/*!
    Destroys the XQMedia object.
*/
XQMedia::~XQMedia()
{
    delete d;
}

/*!
    \enum XQMedia::Error

    This enum defines the possible errors for a XQMedia object.
*/
/*! \var XQMedia::Error XQMedia::NoError
    No error occured.
*/
/*! \var XQMedia::Error XQMedia::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQMedia::Error XQMedia::UnknownError
    Unknown error.
*/

/*!
    \enum XQMedia::MediaType

    This enum defines the possible media types for a XQMedia object:
*/
/*! \var XQMedia::MediaType XQMedia::MediaTypeMusic
    Musics
*/
/*! \var XQMedia::MediaType XQMedia::MediaTypeSound
    Sounds
*/
/*! \var XQMedia::MediaType XQMedia::MediaTypeImage
    Images
*/
/*! \var XQMedia::MediaType XQMedia::MediaTypeVideo
    Videos
*/

/*!
    Retrieves list of media files, type given as parameter. When retrieving is ready it's signalled via listAvailable signal.
    
    \param  type Media type
    \return If false is returned, an error has occurred. Call error() to get a value of 
    XQMedia::Error that indicates which error occurred
    \sa error()
*/
bool XQMedia::fetch(XQMedia::MediaType type)
{
    return d->fetch(type);
}

/*!
    Gets thumbnail from the Exif data.
    Note! If returned image is null call error() to get a value of 
    XQMedia::Error that indicates which error occurred
    \param  path Absolute file path
    \return Thumbnail image
    \sa error()
*/
QImage XQMedia::thumbnail(QString path) const
{
    return d->thumbnail(path);
}

/*!
    Gets thumbnail from the first frame of the video.
    
    \param  path Absolute file path
    \return Thumbnail image
*/
QImage XQMedia::videoThumbnail(QString path) const
{
    return d->videoThumbnail(path);
}

/*!
    Returns the type of error that occurred if the latest function call failed; otherwise returns NoError.
    \return Error code
*/
XQMedia::Error XQMedia::error() const
{
    return d->error();
}

/*!
    \fn void XQMedia::listAvailable(QStringList stringList)

    This signal is emitted when retrieving has completed.
    
    \param stringList The string list of the files
    \sa fetch()
*/

// End of file


