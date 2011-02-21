#ifndef GPSFIX_H
#define GPSFIX_H

#include <qmobilityglobal.h>
#include <qgeopositioninfosource.h>
#include <qgeosatelliteinfosource.h>
#include <qnmeapositioninfosource.h>
#include <QGeoPositionInfo>

// Use the QtMobility namespace
QTM_USE_NAMESPACE

class GpsFix : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int updateInterval READ getUpdateInterval WRITE setUpdateInterval)
    Q_PROPERTY(int timestamp READ getTimtstamp NOTIFY timestampChanged)
    Q_PROPERTY(qreal latitude READ getLatitude NOTIFY latitudeChanged)
    Q_PROPERTY(qreal longitude READ getLongitude NOTIFY longitudeChanged)
    Q_PROPERTY(qreal speed READ getSpeed NOTIFY speedChanged)
    Q_PROPERTY(qreal altitude READ getAltitude NOTIFY altitudeChanged)
    Q_PROPERTY(int accuracy READ getAccuracy NOTIFY accuracyChanged)
    Q_PROPERTY(int heading READ getHeading NOTIFY headingChanged)

public:
    explicit GpsFix(QObject *parent = 0);

    int getUpdateInterval() const;

    qreal getLatitude() const
    {
        return m_latitude;
    }

    qreal getLongitude() const
    {
        return m_longitude;
    }

    qreal getSpeed() const
    {
        return m_speed;
    }

    qreal getAltitude() const
    {
        return m_altitude;
    }

    int getAccuracy() const
    {
        return m_accuracy;
    }

    int getHeading() const
    {
        return m_heading;
    }

    int getTimtstamp() const
    {
        return m_timestamp;
    }

    signals:

    void latitudeChanged(qreal arg);

    void longitudeChanged(qreal arg);

    void speedChanged(qreal arg);

    void altitudeChanged(qreal arg);

    void accuracyChanged(int arg);

    void headingChanged(int arg);

    void timestampChanged(int arg);

    public slots:
    void onUpdateTimeout();
    void onPositionUpdated ( const QGeoPositionInfo & update ) ;

    void setUpdateInterval(int arg);

    private:
#ifdef _MOBILE
    QGeoPositionInfoSource* src;
#else
    QNmeaPositionInfoSource* src;
#endif
    int m_accuracy;
    int m_heading;
    qreal m_latitude;
    qreal m_longitude;
    qreal m_speed;
    qreal m_altitude;
    int m_timestamp;
};

#endif // GPSFIX_H
