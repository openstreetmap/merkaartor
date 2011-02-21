#include "GpsFix.h"

#ifndef _MOBILE
#include <QFile>
#include <QTimer>
#endif

GpsFix::GpsFix(QObject *parent) :
    QObject(parent)
  , m_accuracy(999)

{
#ifdef _MOBILE
    src = QGeoPositionInfoSource::createDefaultSource(this);
#else
    src = new QNmeaPositionInfoSource(QNmeaPositionInfoSource::SimulationMode, this);
    src->setDevice(new QFile("D:/srcS60/MerkLog/qml/MerkLog/nmealog.txt"));
    QTimer::singleShot(5000, this, SLOT(onUpdateTimeout()));
#endif
    if (src) {
        src->setUpdateInterval(1000);
        src->startUpdates();
        connect(src, SIGNAL(updateTimeout()), SLOT(onUpdateTimeout()));
        connect(src, SIGNAL(positionUpdated(QGeoPositionInfo)), SLOT(onPositionUpdated(QGeoPositionInfo)));
    }
}

void GpsFix::onUpdateTimeout()
{
#ifdef _MOBILE
    m_accuracy = 999;
#else
    m_accuracy = 100;
#endif
    emit accuracyChanged(m_accuracy);
}

void GpsFix::onPositionUpdated(const QtMobility::QGeoPositionInfo &update)
{
    m_timestamp = update.timestamp().toTime_t();
    emit timestampChanged(m_timestamp);
    m_latitude = update.coordinate().latitude();
    emit latitudeChanged(m_latitude);
    m_longitude = update.coordinate().longitude();
    emit longitudeChanged(m_longitude);
    m_altitude = update.coordinate().altitude();
    emit altitudeChanged(m_altitude);

    if (update.hasAttribute(QGeoPositionInfo::GroundSpeed)) {
        m_speed = update.attribute(QGeoPositionInfo::GroundSpeed);
        emit speedChanged(m_speed);
    }
    if (update.hasAttribute(QGeoPositionInfo::HorizontalAccuracy)) {
        m_accuracy = qRound(update.attribute(QGeoPositionInfo::HorizontalAccuracy));
        emit accuracyChanged(m_accuracy);
    }
    if (update.hasAttribute(QGeoPositionInfo::Direction)) {
        m_heading = qRound(update.attribute(QGeoPositionInfo::Direction));
        emit headingChanged(m_heading);
    }
}

int GpsFix::getUpdateInterval() const
{
    if (src)
        return src->updateInterval();
    else
        return 0;
}

void GpsFix::setUpdateInterval(int arg)
{
    if (src)
        src->setUpdateInterval(arg);
}
