#include "xqlandmarksmanager.h"
#include "xqlandmarksmanager_p.h"

/*!
    \class XQLandmarkManager

    \brief The XQLandmarkManager class is used to control landmarks.
    
    Example:
    \code
    XQLandmarkManager *landmarkManager = new XQLandmarkManager(this);
    XQLandmark landmark;
    landmark.setPosition(61.4500, 23.8502);
    landmark.setName("Test landmark");
    landmarkManager->addLandmark(landmark);

    QList<int> ids = landmarkManager->landmarkIds();
    QListWidget *listWidget = new QListWidget(this);
    // List all landmarks to the list widget
    for (int i=0; i<ids.count(); i++)
    {
        listWidget->addItem(landmarkManager->landmark(ids.value(i)).name());
    }
    \endcode
*/

/*!
    Constructs a XQLandmarkManager object with the given parent.
    \sa addLandmark
*/
XQLandmarkManager::XQLandmarkManager(QObject *parent)
 : QObject(parent), d(new XQLandmarkManagerPrivate(this))
{
}

/*!
    Destroys the XQLandmarkManager object.
*/
XQLandmarkManager::~XQLandmarkManager()
{
    delete d;
}

/*!
    \enum XQLandmarkManager::Error

    This enum defines the possible errors for a XQLandmarkManager object.
*/
/*! \var XQLandmarkManager::Error XQLandmarkManager::NoError
    No error occured.
*/
/*! \var XQLandmarkManager::Error XQLandmarkManager::OutOfMemoryError
    Not enough memory.
*/
/*! \var XQLandmarkManager::Error XQLandmarkManager::UnknownError
    Unknown error.
*/

/*!
    Adds a new landmark to the database 
        
    \param landmark A landmark object
    \return The ID of the new landmark.
*/
int XQLandmarkManager::addLandmark(XQLandmark landmark)
{
    return d->addLandmark(landmark); 
}

/*!
    Gets a landmark specified by the id 
        
    \param id Landmark id
    \return landmark object
    \sa landmarkIds 
*/
XQLandmark XQLandmarkManager::landmark(int id) const
{
    return d->landmark(id);
}

/*!
    Get a list of ids found in the landmark database
        
    \return List of ids
    \sa landmark
*/
QList<int> XQLandmarkManager::landmarkIds() const
{
    return d->landmarkIds();
}

/*!
    Returns current error level.
    \return Error code
*/
XQLandmarkManager::Error XQLandmarkManager::error() const
{
    return d->error();
}

/*!
    \class XQLandmark

    \brief The XQLandmark is a container class for the landmark.
    The XQLandmark contains functions for setting and getting landmark attributes.
*/

/*!
    Constructs a XQLandmark object with the given parent.
*/
XQLandmark::XQLandmark(): landmarkname(""), latitudeValue(0),
        longitudeValue(0), landmarkDescription("")
{
}

/*!
    Destroys the XQLandmark object.
*/
XQLandmark::~XQLandmark()
{
}

/*!
    Set name for the landmark
    
    \param name Name for the landmark    
    \sa landmarkName
*/
void XQLandmark::setName(QString name)
{
    landmarkname = name;
}

/*!
    Get name of the landmark
        
    \return The name of the landmark
    \sa setLandmarkName 
*/
QString XQLandmark::name() const
{
    return landmarkname;
}

/*!
    Set position for the landmark
        
    \param latitude Latitude value
    \param longitude Longitude value
    \sa latitude, longitude 
*/
void XQLandmark::setPosition(qreal latitude, qreal longitude)
{
    latitudeValue = latitude;
    longitudeValue = longitude;
}

/*!
    Get latitude value of the landmark
        
    \return latitude
    \sa setPosition, longitude
*/
qreal XQLandmark::latitude() const
{
    return latitudeValue;
}

/*!
    Get longitude value of the landmark
        
    \return longitude
    \sa setPosition, latitude 
*/
qreal XQLandmark::longitude() const
{
    return longitudeValue;
}

/*!
    Set description of the landmark
        
    \param description Description for the landmark
    \sa landmarkDescription 
*/
void XQLandmark::setDescription(QString description)
{
    landmarkDescription = description;
}

/*!
    Get landmark description
        
    \return Description of the landmark
    \sa setLandmarkDescription
*/
QString XQLandmark::description() const
{
    return landmarkDescription;
}

// End of file
