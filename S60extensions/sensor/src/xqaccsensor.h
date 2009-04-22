#ifndef XQACCELERATIONSENSOR_H
#define XQACCELERATIONSENSOR_H

// INCLUDES
#include "xqsensor.h"

// FORWARD DECLARATIONS
class XQAccelerationSensorPrivate;
class XQAbstractAccelerationSensorFilter;

// CLASS DECLARATION
class XQAccelerationSensor: public XQSensor
{
    Q_OBJECT

public:
    XQAccelerationSensor(QObject *parent = 0);
    virtual ~XQAccelerationSensor();
    void addFilter(XQAbstractAccelerationSensorFilter &filter);
    QList<XQAbstractAccelerationSensorFilter *>& filters();

    int xAcceleration() const;
    int yAcceleration() const;
    int zAcceleration() const;

private:
    friend class XQAccelerationSensorPrivate;
};


class XQAbstractAccelerationSensorFilter
{
public:
    virtual bool filter(int &xAcceleration, int &yAcceleration, int &zAcceleration) = 0;
};

class XQAccelerationDataPostFilter : public XQAbstractAccelerationSensorFilter
{
protected:  //from XQAbstractAccelerationSensorFilter
    bool filter(int &xAcceleration, int &yAcceleration, int &zAcceleration);
};

#endif /*XQACCELERATIONSENSOR_H*/

// End of file
