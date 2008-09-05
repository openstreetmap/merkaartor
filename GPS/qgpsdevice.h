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

#include <QObject>
#include <QThread>
#include <QDateTime>
#include <QFile>

class QString;
class QMutex;
class QextSerialPort;
class QFile;

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
        
        float latitude()        { return cur_latitude;      }
        float longitude()       { return cur_longitude;     }
        float altitude()        { return cur_altitude;      }
        float heading()         { return cur_heading;       }
        float speed()           { return cur_speed;         }
        float variation()       { return cur_variation;     }
        float dillution()       { return cur_dillution;     }
        
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
    
        void  updatePosition(float latitude, float longitude, QDateTime time, float altitude, float speed, float heading);
        void  updateStatus();
        
    protected:
    
        virtual void run() = 0;
    
        int     fd;
        bool    stopLoop;
        
        QMutex  *mutex;
        
        // functions to set various properties - private
        
        void setLatitude(float new_latitude)                    { cur_latitude      = new_latitude;     }
        void setLongitude(float new_longitude)                  { cur_longitude     = new_longitude;    }
        void setAltitude(float new_altitude)                    { cur_altitude      = new_altitude;     }
        void setHeading(float new_heading)                      { cur_heading       = new_heading;      }
        void setSpeed(float new_speed)                          { cur_speed         = new_speed;        }
        void setVariation(float new_variation)                  { cur_variation     = new_variation;    }
        void setLatCardinal(CardinalDirection new_direction)    { cur_latCardinal   = new_direction;    }
        void setLongCardinal(CardinalDirection new_direction)   { cur_longCardinal  = new_direction;    }
        void setVarCardinal(CardinalDirection new_direction)    { cur_varCardinal   = new_direction;    }
        void setFixQuality(int new_fixQuality)                  { cur_fixQuality    = new_fixQuality;   }
        void setDillution(float new_dillution)                  { cur_dillution     = new_dillution;    }
        void setNumSatellites(int new_numSatellites)            { cur_numSatellites = new_numSatellites;}
        void setFixType(FixType new_fixType)                    { cur_fixType       = new_fixType;      }
        void setFixMode(FixMode new_fixMode)                    { cur_fixMode       = new_fixMode;      }
        void setFixStatus(FixStatus new_fixStatus)              { cur_fixStatus     = new_fixStatus;    }

        QString cur_device;

		QDateTime cur_datetime;

        float cur_latitude;
        float cur_longitude;
        float cur_altitude;
        float cur_heading;
        float cur_speed;
        float cur_variation;
        float cur_dillution;
        
        CardinalDirection cur_latCardinal;
        CardinalDirection cur_longCardinal;
        CardinalDirection cur_varCardinal;
        
        unsigned int cur_fixQuality;
        unsigned int cur_numSatellites;
        
        unsigned int satArray[50][3];
        unsigned int activeSats[12];
        
        FixType cur_fixType;
        FixMode cur_fixMode;
        FixStatus cur_fixStatus;
        
        bool parseGGA(const char *ggaString = 0);
        bool parseGSA(const char *gsaString = 0);
        bool parseGSV(const char *gsvString = 0);
        bool parseRMC(const char *gsvString = 0);
        
};
        
class QGPSComDevice : public QGPSDevice
{
Q_OBJECT

public:
	QGPSComDevice(const QString &device);
	virtual ~QGPSComDevice();

	virtual bool openDevice();
    virtual bool closeDevice();

private:
	QextSerialPort *port;
	QFile* LogFile;

	virtual void run();
};        

class QGPSFileDevice : public QGPSDevice
{
Q_OBJECT

public:
	QGPSFileDevice(const QString &device);

	virtual bool openDevice();
    virtual bool closeDevice();

private:
	QFile* theFile;

	virtual void run();
};        

class QTcpSocket;

class QGPSDDevice : public QGPSDevice
{
	Q_OBJECT

	public:
		QGPSDDevice(const QString& device);

		virtual bool openDevice();
		virtual bool closeDevice();

	protected:
		virtual void run();

	public slots:
		void onLinkReady();
		void onDataAvailable();
		void onWatch();
	private:
		void parse(const QString& s);
		QTcpSocket* Server;
		QByteArray Buffer;
};

