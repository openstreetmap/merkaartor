#ifndef XQLANDMARKMANAGER_H
#define XQLANDMARKMANAGER_H

// INCLUDES
#include <QObject>
#include <QStringlist>

// FORWARD DECLARATIONS
class XQLandmark;
class XQLandmarkManagerPrivate;

// CLASS DECLARATION
class XQLandmarkManager : public QObject
{
    Q_OBJECT
     
public:
    enum Error {
        NoError = 0,
        OutOfMemoryError,
        UnknownError = -1
    };
    
    XQLandmarkManager(QObject *parent = 0);
    ~XQLandmarkManager();

    int addLandmark(XQLandmark landmark);
    XQLandmark landmark(int id) const;
    
    QList<int> landmarkIds() const;
    XQLandmarkManager::Error error() const;
    
private:
    XQLandmarkManagerPrivate *d;
};

class XQLandmark
{
     
public:
    XQLandmark();
    ~XQLandmark();
    
    void setName(QString name);
    void setPosition(qreal latitude, qreal longitude);
    void setDescription(QString description);

    qreal latitude() const;
    qreal longitude() const;
    QString name() const;
    QString description() const;
    
private:
    QString landmarkname;
    qreal latitudeValue;
    qreal longitudeValue;
    QString landmarkDescription;
};

#endif // XQLANDMARKMANAGER_H

// End of file
