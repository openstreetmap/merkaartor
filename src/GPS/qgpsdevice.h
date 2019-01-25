/***************************************************************************
 *   Copyright (C) 2005 by Robin Gingras                                   *
 *   neozenkai@cox.net                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef QGPS_DEVICE_H
#define QGPS_DEVICE_H

#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QFile>

class QString;
class QMutex;
class QextSerialPort;
class QFile;

class QGPSDevice;
// We want these slots to be executed within the thread represented by
// QGPSDDevice. Since that class itself lives in the main thread, we need
// a forwarder that does live there as the receiver.

class GPSSlotForwarder : public QObject
{
    Q_OBJECT

public:
    GPSSlotForwarder(QGPSDevice* Target);


public slots:
    void onLinkReady();
    void onDataAvailable();
    void onStop();
    void checkDataAvailable();

private:
    QGPSDevice* Target;
};

class QGPSDevice : public QThread
{
    Q_OBJECT

public:

    QGPSDevice();

    virtual bool openDevice() = 0;
    virtual bool closeDevice() = 0;

    enum FixMode
    {
        FixAuto,
        FixManual
    };

    enum FixStatus
    {
        StatusActive,
        StatusVoid
    };

    enum FixType
    {
        FixUnavailable,
        FixInvalid,
        Fix2D,
        Fix3D
    };

    enum CardinalDirection
    {
        CardinalNorth,
        CardinalSouth,
        CardinalEast,
        CardinalWest,
        CardinalNone
    };

    void setDevice(QString new_device) { cur_device = new_device; }

    QString device()    { return cur_device;    }

    QDateTime	dateTime()	{ return cur_datetime;		}

    int   fixQuality()      { return cur_fixQuality;    }
    int   numSatellites()   { return cur_numSatellites; }
    FixType		fixType()		{ return cur_fixType; }
    FixMode		fixMode()		{ return cur_fixMode; }
    FixStatus	fixStatus()		{ return cur_fixStatus; }

    qreal latitude()        { return cur_latitude;      }
    qreal longitude()       { return cur_longitude;     }
    qreal altitude()        { return cur_altitude;      }
    qreal heading()         { return cur_heading;       }
    qreal speed()           { return cur_speed;         }
    qreal variation()       { return cur_variation;     }
    qreal dillution()       { return cur_dillution;     }

    CardinalDirection latCardinal()     { return cur_latCardinal;   }
    CardinalDirection longCardinal()    { return cur_longCardinal;  }
    CardinalDirection varCardinal()     { return cur_varCardinal;   }

    bool isActiveSat(int prn);
    void satInfo(int index, int &elev, int &azim, int &snr);

    // some convinience functions

    int latDegrees();
    int latMinutes();
    int latSeconds();

    int longDegrees();
    int longMinutes();
    int longSeconds();

public slots:

    virtual void startDevice();
    virtual void stopDevice();

signals:

    void  updatePosition(qreal latitude, qreal longitude, QDateTime time, qreal altitude, qreal speed, qreal heading);
    void  updateStatus();
    void doStopDevice();


protected:

    virtual void checkDataAvailable() {};
    virtual void run() = 0;

    int     fd;
    bool    stopLoop;

    QMutex  *mutex;

    // functions to set various properties - private

    void setLatitude(qreal new_latitude)                    { cur_latitude      = new_latitude;     }
    void setLongitude(qreal new_longitude)                  { cur_longitude     = new_longitude;    }
    void setAltitude(qreal new_altitude)                    { cur_altitude      = new_altitude;     }
    void setHeading(qreal new_heading)                      { cur_heading       = new_heading;      }
    void setSpeed(qreal new_speed)                          { cur_speed         = new_speed;        }
    void setVariation(qreal new_variation)                  { cur_variation     = new_variation;    }
    void setLatCardinal(CardinalDirection new_direction)    { cur_latCardinal   = new_direction;    }
    void setLongCardinal(CardinalDirection new_direction)   { cur_longCardinal  = new_direction;    }
    void setVarCardinal(CardinalDirection new_direction)    { cur_varCardinal   = new_direction;    }
    void setFixQuality(int new_fixQuality)                  { cur_fixQuality    = new_fixQuality;   }
    void setDillution(qreal new_dillution)                  { cur_dillution     = new_dillution;    }
    void setNumSatellites(int new_numSatellites)            { cur_numSatellites = new_numSatellites;}
    void setFixType(FixType new_fixType)                    { cur_fixType       = new_fixType;      }
    void setFixMode(FixMode new_fixMode)                    { cur_fixMode       = new_fixMode;      }
    void setFixStatus(FixStatus new_fixStatus)              { cur_fixStatus     = new_fixStatus;    }

    QString cur_device;

    QDateTime cur_datetime;

    qreal cur_latitude;
    qreal cur_longitude;
    qreal cur_altitude;
    qreal cur_heading;
    qreal cur_speed;
    qreal cur_variation;
    qreal cur_dillution;

    CardinalDirection cur_latCardinal;
    CardinalDirection cur_longCardinal;
    CardinalDirection cur_varCardinal;

    int cur_fixQuality;
    int cur_numSatellites;

    int satArray[50][3];
    int activeSats[12];

    FixType cur_fixType;
    FixMode cur_fixMode;
    FixStatus cur_fixStatus;

    QFile* LogFile;
    void parseNMEA(const QByteArray& array);
    bool parseGGA(const char *ggaString = 0);
    bool parseGLL(const char *ggaString = 0);
    bool parseGSA(const char *gsaString = 0);
    bool parseGSV(const char *gsvString = 0);
    bool parseRMC(const char *gsvString = 0);

private:
    virtual void onLinkReady() = 0;
    virtual void onDataAvailable() = 0;
    virtual void onStop() = 0;

    friend class GPSSlotForwarder;
};

#ifndef _MOBILE
class QGPSComDevice : public QGPSDevice
{
    Q_OBJECT

public:
    QGPSComDevice(const QString &device);
    virtual ~QGPSComDevice();

    virtual bool openDevice();
    virtual bool closeDevice();

private:
    virtual void onLinkReady();
    virtual void onDataAvailable();
    virtual void onStop();

    QextSerialPort *port;
    QByteArray Buffer;

    virtual void run();

protected:
    virtual void checkDataAvailable();
};
#endif

class QGPSFileDevice : public QGPSDevice
{
    Q_OBJECT

public:
    QGPSFileDevice(const QString &device);

    virtual bool openDevice();
    virtual bool closeDevice();

private:
    virtual void onLinkReady();
    virtual void onDataAvailable();
    virtual void onStop();

    QFile* theFile;

    virtual void run();
};

#ifndef _MOBILE

#ifdef USE_GPSD_LIB
#include "libgpsmm.h"

class QGPSDDevice;
class QGPSDDevice : public QGPSDevice
{
    Q_OBJECT

public:
    QGPSDDevice(const QString& device);

    virtual bool openDevice();
    virtual bool closeDevice();

protected:
    virtual void run();

private:
    void onLinkReady();
    void onDataAvailable();
    void onStop();

    gpsmm* Server;
    struct gps_data_t* gpsdata;
    QByteArray Buffer;

    bool serverOk = true;

    friend class GPSSlotForwarder;
};
#else /*USE_GPSD_LIB*/
class QTcpSocket;

class QGPSDDevice;
class QGPSDDevice : public QGPSDevice
{
    Q_OBJECT

public:
    QGPSDDevice(const QString& device);

    virtual bool openDevice();
    virtual bool closeDevice();

protected:
    virtual void run();

private:
    void onLinkReady();
    void onDataAvailable();
    void onStop();

    void parse(const QString& s);
    void parseO(const QString& s);
    void parseY(const QString& s);
    QTcpSocket* Server;
    QByteArray Buffer;

    friend class GPSSlotForwarder;
};
#endif /*USE_GPSD_LIB*/

#endif

#if defined Q_OS_SYMBIAN || defined(Q_WS_SIMULATOR)
#include <QGeoPositionInfo>
#include <QGeoPositionInfoSource>
#include <QGeoSatelliteInfo>
#include <QGeoSatelliteInfoSource>

// Use the QtMobility namespace
QTM_USE_NAMESPACE

class QGPSMobileDevice : public QGPSDevice
{
    Q_OBJECT

public:
    QGPSMobileDevice();

    virtual bool openDevice();
    virtual bool closeDevice();

    void satInfo(int index, int &elev, int &azim, int &snr);

protected:
    virtual void run();

private slots:
    void onUpdateTimeout();
    void onPositionUpdated ( const QGeoPositionInfo & update ) ;
//    void setUpdateInterval(int arg);

    void on_satRequestTimeout();
    void on_satellitesInViewUpdated(QList<QGeoSatelliteInfo>);
    void on_satellitesInUseUpdated(QList<QGeoSatelliteInfo>);


private:
    void onLinkReady();
    void onDataAvailable();
    void onStop();

    friend class GPSSlotForwarder;

private:
    QGeoPositionInfoSource* src;
    QGeoSatelliteInfoSource* satsrc;

    QList<QGeoSatelliteInfo> m_List;
    QList<QGeoSatelliteInfo> m_UseList;
    int m_accuracy;
};
#endif

#endif // QGPS_DEVICE_H
