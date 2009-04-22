#ifndef XQDEVICEORIENTATION_H
#define XQDEVICEORIENTATION_H

#include <QObject>

class XQDeviceOrientationPrivate;

class XQDeviceOrientation : public QObject
{
    Q_OBJECT

public:
    enum DisplayOrientation
    {
        OrientationUndefined = 0,
        OrientationDisplayUp,
        OrientationDisplayDown,
        OrientationDisplayLeftUp,
        OrientationDisplayRightUp,
        OrientationDisplayUpwards,
        OrientationDisplayDownwards
    };

public:
    XQDeviceOrientation(QObject *parent = 0);
    virtual ~XQDeviceOrientation();

    void setResolution(int resolution);
    int resolution() const;

    int xRotation() const;
    int yRotation() const;
    int zRotation() const;
    XQDeviceOrientation::DisplayOrientation orientation() const;

Q_SIGNALS:
    void rotationChanged(int xRotation, int yRotation, int zRotation);
    void orientationChanged(XQDeviceOrientation::DisplayOrientation orientation);

private:
    friend class XQDeviceOrientationPrivate;
    XQDeviceOrientationPrivate* d;
};

#endif /*XQDEVICEORIENTATION_H*/

// End of file
