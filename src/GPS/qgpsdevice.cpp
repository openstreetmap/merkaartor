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

#include <QThread>
#include <QObject>
#include <QString>
#include <QMutex>
#include <QFile>
#include <QStringList>
#include <QMessageBox>
#include <QTcpSocket>
#include <QTimer>
#include <QHostAddress>

#include "qgpsdevice.h"
#ifndef Q_OS_SYMBIAN
#include "qextserialport.h"
#endif

#include "Preferences/MerkaartorPreferences.h"

/* GPSSLOTFORWARDER */

GPSSlotForwarder::GPSSlotForwarder(QGPSDevice* aTarget)
: Target(aTarget)
{
}

void GPSSlotForwarder::onLinkReady()
{
	Target->onLinkReady();
}

void GPSSlotForwarder::onDataAvailable()
{
	Target->onDataAvailable();
}

void GPSSlotForwarder::onStop()
{
	Target->onStop();
}

void GPSSlotForwarder::checkDataAvailable()
{
	Target->checkDataAvailable();
}

/**
 * QGPSDevice::QGPSDevice()
 *
 * Takes in an optional serialPort string and sets the serialPort property
 * accordingly.
 *
 * @param char serialPort   Serial port to listen to for GPS dat
 */

QGPSDevice::QGPSDevice()
	:LogFile(0)
{
    mutex = new QMutex(QMutex::Recursive);

    setLatitude(0);
    setLongitude(0);
    setAltitude(0);
    setHeading(0);
    setSpeed(0);
    setVariation(0);

	setFixMode(QGPSDevice::FixAuto);
	setFixType(QGPSDevice::FixUnavailable);
	setFixStatus(QGPSDevice::StatusVoid);

    stopLoop = false;

	cur_numSatellites = 0;
    for(int i = 0; i < 50; i ++)
    {
        satArray[i][0] = satArray[i][1] = satArray[i][2] = 0;
    }
}

/**
 * Accessor functions
 */

int QGPSDevice::latDegrees()    { return (int) (fabs(latitude()));													}
int QGPSDevice::latMinutes()
{
	double m = fabs(latitude()) - latDegrees();
	return int(m * 60);
}
int QGPSDevice::latSeconds()
{
	double m = fabs(latitude()) - latDegrees();
	double s = (m * 60) - int(m * 60);
	return int(s * 60);
}
int QGPSDevice::longDegrees()    { return (int) (fabs(longitude()));													}
int QGPSDevice::longMinutes()
{
	double m = fabs(longitude()) - longDegrees();
	return int(m * 60);
}
int QGPSDevice::longSeconds()
{
	double m = fabs(longitude()) - longDegrees();
	double s = (m * 60) - int(m * 60);
	return int(s * 60);
}

bool QGPSDevice::isActiveSat(int prn)
{
	for (int i=0; i<12; i++) {
		if (activeSats[i] == prn)
			return true;
	}
	return false;
}

void QGPSDevice::satInfo(int index, int &elev, int &azim, int &snr)
{
    mutex->lock();

    elev = satArray[index][0];
    azim = satArray[index][1];
    snr  = satArray[index][2];

    mutex->unlock();
}

/**
 * QGPSDevice::run()
 *
 * Begins the thread loop, reading data from the GPS and parsing
 * full strings.
 */

/**
 * QGPSDevice::parseGGA()
 *
 * Parses a GPGGA string that contains fix information, such as
 * latitude, longitude, etc.
 *
 * The format of the GPGGA String is as follows:
 *
 *  $GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47
 *  |||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 *  01234567890123456789012345678901234567890123456789012345678901234
 *  |         |         |         |         |         |         |
 *  0         10        20        30        40        50        60
 *
 *  GPGGA       -   Global Positioning System Fix Data
 *  123519      -   Fix taken at 12:35:19 UTC
 *  4807.038,N  -   Latitude 48 deg 07.038' N
 *  01131.000,E -   Longitude 11 deg 31.000' E
 *  1           -   Fix quality:
 *                      0 = Invalid
 *                      1 = GPS fix (SPS)
 *                      2 = DGPS fix
 *                      3 = PPS fix
 *                      4 = Real time kinematic
 *                      5 = Float RTK
 *                      6 = Estimated (dead reckoning)
 *                      7 = Manual input mode
 *                      8 = Simulation mode
 *  08          -   Number of satellites being tracked
 *  0.9         -   Horizontal dissolution of precision
 *  545.4,M     -   Altitude (meters) above sea level
 *  46.9,M      -   Height of geoid (sea level) above WGS84 ellipsoid
 *  (empty)     -   Seconds since last DGPS update
 *  (empty)     -   DGPS station ID number
 *  *47         -   Checksum, begins with *
 *
 * @param char ggaString    The full NMEA GPGGA string, starting with
 *                          the $ and ending with the checksum
 */

void QGPSDevice::parseNMEA(const QByteArray& bufferString)
{
	if (bufferString.length() < 6) return;
	if(bufferString[3] == 'G' && bufferString[4] == 'G' && bufferString[5] == 'A')
	{
		//strcpy(nmeastr_gga, bufferString);
		parseGGA(bufferString.data());
	}
	else if(bufferString[3] == 'G' && bufferString[4] == 'L' && bufferString[5] == 'L')
	{
		//strcpy(nmeastr_gga, bufferString);
		parseGLL(bufferString.data());
	}
	else if(bufferString[3] == 'G' && bufferString[4] == 'S' && bufferString[5] == 'V')
	{
		//strcpy(nmeastr_gsv, bufferString);
		parseGSV(bufferString.data());
	}
	else if(bufferString[3] == 'G' && bufferString[4] == 'S' && bufferString[5] == 'A')
	{
		//strcpy(nmeastr_gsa, bufferString);
		parseGSA(bufferString.data());
	}
	else if(bufferString[3] == 'R' && bufferString[4] == 'M' && bufferString[5] == 'C')
	{
		//strcpy(nmeastr_rmc, bufferString);
		if (parseRMC(bufferString.data()))
			if (fixStatus() == QGPSDevice::StatusActive && (fixType() == QGPSDevice::Fix3D || fixType() == QGPSDevice::FixUnavailable))
				emit updatePosition(latitude(), longitude(), dateTime(), altitude(), speed(), heading());
	}
	emit updateStatus();
}

bool QGPSDevice::parseGGA(const char *ggaString)
{
    mutex->lock();

	QString line(ggaString);
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	double lat = tokens[2].left(2).toDouble();
	double latmin = tokens[2].mid(2).toDouble();
	lat += latmin / 60.0;
	if (tokens[3] != "N")
		lat = -lat;
    //cur_latitude = lat;

	if (!tokens[3].isEmpty())
	{
		if (tokens[3].at(0) == 'N')
			setLatCardinal(CardinalNorth);
		else if (tokens[3].at(0) == 'S')
			setLatCardinal(CardinalSouth);
		else
			setLatCardinal(CardinalNone);
	}


	double lon = tokens[4].left(3).toDouble();
	double lonmin = tokens[4].mid(3).toDouble();
	lon += lonmin / 60.0;
	if (tokens[5] != "E")
		lon = -lon;
    //cur_longitude = lon;

	if (!tokens[5].isEmpty())
	{
		if (tokens[5].at(0) == 'E')
			setLatCardinal(CardinalEast);
		else if (tokens[5].at(0) == 'W')
			setLatCardinal(CardinalWest);
		else
			setLatCardinal(CardinalNone);
	}

	int fix = tokens[6].toInt();
	setFixQuality(fix);

	int numSat = tokens[7].toInt();
	setNumSatellites(numSat);

	float dilut = tokens[8].toFloat();
    setDillution(dilut);

	float altitude = tokens[9].toFloat();
	setAltitude(altitude);

    mutex->unlock();

	return true;
} // parseGGA()

bool QGPSDevice::parseGLL(const char *ggaString)
{
    mutex->lock();

	QString line(ggaString);
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	double lat = tokens[1].left(2).toDouble();
	double latmin = tokens[1].mid(2).toDouble();
	lat += latmin / 60.0;
	if (tokens[2] != "N")
		lat = -lat;
    //cur_latitude = lat;

	if (!tokens[2].isEmpty())
	{
		if (tokens[2].at(0) == 'N')
			setLatCardinal(CardinalNorth);
		else if (tokens[2].at(0) == 'S')
			setLatCardinal(CardinalSouth);
		else
			setLatCardinal(CardinalNone);
	}

	double lon = tokens[3].left(3).toDouble();
	double lonmin = tokens[3].mid(3).toDouble();
	lon += lonmin / 60.0;
	if (tokens[4] != "E")
		lon = -lon;
    //cur_longitude = lon;

	if (!tokens[4].isEmpty())
	{
		if (tokens[4].at(0) == 'E')
			setLatCardinal(CardinalEast);
		else if (tokens[4].at(0) == 'W')
			setLatCardinal(CardinalWest);
		else
			setLatCardinal(CardinalNone);
	}


 	if (tokens[6] == "A")
    {
        setFixStatus(StatusActive);
    }
    else
    {
        setFixStatus(StatusVoid);
    }

    mutex->unlock();

	return true;
} // parseGGA()

/**
 * QGPSDevice::parseGSA()
 *
 * Parses a GPGSA string that contains information about the nature
 * of the fix, such as DOP (dillution of precision) and active satellites
 * based on the viewing mask and almanac data of the reciever.
 *
 * The format of the GPGSA String is as follows:
 *
 *  $GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39
 *  |||||||||||||||||||||||||||||||||||||||||||||||
 *  01234567890123456789012345678901234567890123456
 *  |         |         |         |         |
 *  0         10        20        30        40
 *
 *  GPGSA       -   Information about satellite status
 *  A           -   Fix mode, (A)utomatic or (M)anual
 *  3           -   Fix type:
 *                      1 = Invalid
 *                      2 = 2D
 *                      3 = 3D (4 or more satellites)
 *  04,05,...   -   Satellites used in the solution (up to 12)
 *  2.5         -   DOP (dillution of precision)
 *  1.3         -   Horizontal DOP
 *  2.1         -   Vertical DOP
 *  *39         -   Checksum
 *
 * @param char  The full NMEA GPGSA string, from $ to checksum
 */

bool QGPSDevice::parseGSA(const char *gsaString)
{
    mutex->lock();

	QString line(gsaString);
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	QString autoSelectFix = tokens[1];
    if(autoSelectFix == "A")
        setFixMode(FixAuto);
    else
        setFixMode(FixManual);

	int fix = tokens[2].toInt();
    if(fix == 1)
        setFixType(FixInvalid);
    else if(fix == 2)
        setFixType(Fix2D);
    else
        setFixType(Fix3D);

	for(int index = 0; index < 12; index ++) {
		activeSats[index] = tokens[index+3].toInt();
	}

    mutex->unlock();

	return true;
} // parseGSA()

/**
 * QGPSDevice::parseRMC()
 *
 * Parses an RMC string, which contains the recommended minimum fix
 * data, such as latitude, longitude, altitude, speed, track angle,
 * date, and magnetic variation. Saves us some calculating :)
 *
 * The format of the GPRMC string is as follows:
 *
 *  $GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A
 *  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 *  01234567890123456789012345678901234567890123456789012345678901234567
 *  |         |         |         |         |         |         |
 *  0         10        20        30        40        50        60
 *
 *  GPRMC       -   Recommended minimum fix data
 *  123519      -   Fix taken at 12:35:19 UTC
 *  A           -   Fix status, (A)ctive or (V)oid
 *  4807.038,N  -   Latitude 48 degrees 07.038' N
 *  01131.000,E -   Longitude 11 degrees, 31.000' E
 *  022.4       -   Ground speed in knots
 *  084.4       -   Track angle in degrees (true north)
 *  230394      -   Date: 23 March 1994
 *  003.1,W     -   Magnetic Variation
 *  *6A         -   Checksum
 *
 * @param char  Full RMC string, from $ to checksum
 */

bool QGPSDevice::parseRMC(const char *rmcString)
{
	mutex->lock();

	// Fix time

	QString line(rmcString);
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	QString strDate = tokens[9] + tokens[1];
	cur_datetime = QDateTime::fromString(strDate, "ddMMyyHHmmss.zzz");

	if (cur_datetime.date().year() < 1970)
		cur_datetime = cur_datetime.addYears(100);

	// Fix status

	if (tokens[2] == "A")
	{
		setFixStatus(StatusActive);
	}
	else
	{
		setFixStatus(StatusVoid);
	}

	// Latitude

	double lat = tokens[3].left(2).toDouble();
	double latmin = tokens[3].mid(2).toDouble();
	lat += latmin / 60.0;
	if (tokens[4] != "N")
		lat = -lat;
	cur_latitude = lat;

	if (!tokens[4].isEmpty())
	{
		if (tokens[4].at(0) == 'N')
			setLatCardinal(CardinalNorth);
		else if (tokens[4].at(0) == 'S')
			setLatCardinal(CardinalSouth);
		else
			setLatCardinal(CardinalNone);
	}

	double lon = tokens[5].left(3).toDouble();
	double lonmin = tokens[5].mid(3).toDouble();
	lon += lonmin / 60.0;
	if (tokens[6] != "E")
		lon = -lon;
	cur_longitude = lon;

	if (!tokens[6].isEmpty())
	{
		if (tokens[6].at(0) == 'E')
			setLatCardinal(CardinalEast);
		else if (tokens[6].at(0) == 'W')
			setLatCardinal(CardinalWest);
		else
			setLatCardinal(CardinalNone);
	}

	// Ground speed in km/h

	double speed = QString::number(tokens[7].toDouble() * 1.852, 'f', 1).toDouble();
	setSpeed(speed);

	// Heading

	double heading = tokens[8].toDouble();
	setHeading(heading);

	// Magnetic variation

	double magvar = tokens[10].toDouble();
	setVariation(magvar);

	if (!tokens[11].isEmpty())
	{
		if (tokens[11].at(0) == 'E')
			setVarCardinal(CardinalEast);
		else if (tokens[11].at(0) == 'W')
			setVarCardinal(CardinalWest);
		else
			setVarCardinal(CardinalNone);
	}

	mutex->unlock();

	return true;
} // parseRMC()

/**
 * QGPSDevice::parseGSV()
 *
 * Parses a GPGSV string, which contains satellite position and signal
 * strenght information. parseGSV() fills the satArray array with the
 * PRNs, elevations, azimuths, and SNRs of the visible satellites. This
 * array is based on the position of the satellite in the strings, not
 * the order of the PRNs! (see README for info)
 *
 * The format of the GPGSV string is as follows:
 *
 *  $GPGSV,2,1,08,01,40,083,46,02,17,308,41,12,07,344,39,14,22,228,45*75
 *  ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||||
 *  01234567890123456789012345678901234567890123456789012345678901234567
 *  |         |         |         |         |         |         |
 *  0         10        20        30        40        50        60
 *
 *  GPGSV       -   Satellite status
 *  2           -   Number of GPGSV sentences for full data
 *  1           -   Current sentence (1 of 2, etc)
 *  08          -   Number of satellites in view
 *
 *  01          -   Satellite PRN
 *  40          -   Elevation, degrees
 *  083         -   Azimuth, degrees
 *  46          -   SNR (signal to noise ratio)
 *      (for up to four satellites per sentence)
 *  *75         -   Checksum
 */

bool QGPSDevice::parseGSV(const char *gsvString)
{
    mutex->lock();

    int totalSentences;
    int currentSentence;
    int totalSatellites;
	int prn, elev, azim, snr;

	QString line(gsvString);
	if (line.count('$') > 1)
		return false;

	QStringList tokens = line.split(",");

	totalSentences = tokens[1].toInt();
	currentSentence = tokens[2].toInt();
	totalSatellites = tokens[3].toInt();

	for(int i = 0; (i < 4) && ((i*4)+4+3 < tokens.size()); i ++) {
		prn = tokens[(i*4)+4].toInt();
		elev = tokens[(i*4)+4+1].toInt();
		azim = tokens[(i*4)+4+2].toInt();
		if (tokens[(i*4)+4+3].contains('*')) {
			QStringList tok2 = tokens[(i*4)+4+3].split("*");
			snr = tok2[0].toInt();
		} else
			snr = tokens[(i*4)+4+3].toInt();
		satArray[prn][0] = elev;
		satArray[prn][1] = azim;
		satArray[prn][2] = snr;
	}

    mutex->unlock();

	return true;
}

/**
 * QGPSDevice::startDevice()
 *
 * Calls start() to begin thread execution
 */

void QGPSDevice::startDevice()
{
    mutex->lock();
    stopLoop = false;
    mutex->unlock();

    //printf("We're starting...\n");

    start();
}

/**
 * QGPSDevice::stopDevice()
 *
 * Stops execution of run() and ends thread execution
 * This function will be called outside this thread
 */

void QGPSDevice::stopDevice()
{
    // this is through a queued connection
    emit doStopDevice();
}

#ifndef Q_OS_SYMBIAN
/*** QGPSComDevice  ***/

QGPSComDevice::QGPSComDevice(const QString &device)
	: QGPSDevice()
{
#ifdef Q_OS_WIN
	if (!device.isNull() && !device.startsWith("\\\\.\\"))
		setDevice("\\\\.\\" + device);
	else
#endif
	if(!device.isNull())
	{
		setDevice(device);
	}
}

QGPSComDevice::~QGPSComDevice()
{
	if (LogFile) {
		if (LogFile->isOpen())
			LogFile->close();
		delete LogFile;
	}
}

/**
 * QGPSComDevice::openDevice()
 *
 * Opens the serial port and sets the parameters for data transfer: parity,
 * stop bits, blocking, etc.
 */

bool QGPSComDevice::openDevice()
{
	port = new QextSerialPort(device());
	port->setBaudRate(BAUD4800);
	port->setFlowControl(FLOW_OFF);
	port->setParity(PAR_NONE);
	port->setDataBits(DATA_8);
	port->setStopBits(STOP_2);

	if (port->open(QIODevice::ReadOnly)) {
		if (M_PREFS->getGpsSaveLog()) {
			QString fn = "log-" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".nmea";
			fn.replace(':', '-');
			LogFile = new QFile(M_PREFS->getGpsLogDir() + "/"+fn);
			if (!LogFile->open(QIODevice::WriteOnly)) {
				QMessageBox::critical(NULL, tr("GPS log error"),
					tr("Unable to create GPS log file: %1.").arg(M_PREFS->getGpsLogDir() + "/"+fn), QMessageBox::Ok);
				delete LogFile;
				LogFile = NULL;
			}
		}
		return true;
	}
	return false;
}

/**
 * QGPSComDevice::closeDevice()
 *
 * Closes the serial port
 */

bool QGPSComDevice::closeDevice()
{
	port->close();
	if (LogFile && LogFile->isOpen()) {
		LogFile->close();
		delete LogFile;
	}
	LogFile = NULL;

	return true;
}

void QGPSComDevice::onLinkReady()
{
}

void QGPSComDevice::onStop()
{
	quit();
}


void QGPSComDevice::run()
{
	GPSSlotForwarder Forward(this);

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), &Forward, SLOT(checkDataAvailable()));
	timer->start(150);

//	connect(port,SIGNAL(readyRead()),&Forward,SLOT(onDataAvailable()));
	connect(this,SIGNAL(doStopDevice()),&Forward,SLOT(onStop()));
	exec();
	closeDevice();
}

void QGPSComDevice::checkDataAvailable() {
	if (port->bytesAvailable() > 0)
		onDataAvailable();
}

void QGPSComDevice::onDataAvailable()
{
	QByteArray ba(port->readAll());
	// filter out unwanted characters
	for (int i=ba.count(); i; --i)
		if(ba[i-1] == '\0' || 
			(!isalnum((quint8)ba[i-1]) && 
			 !isspace((quint8)ba[i-1]) && 
			 !ispunct((quint8)ba[i-1])))
		{
			ba.remove(i-1,1);
		}
	if (LogFile)
		LogFile->write(ba);
	Buffer.append(ba);
	if (Buffer.length() > 4096)
		// safety valve
		Buffer.remove(0,Buffer.length()-4096);
	while (Buffer.count())
	{
		// look for begin of sentence marker
		int i = Buffer.indexOf('$');
		if (i<0)
		{
			Buffer.clear();
			return;
		}
		Buffer.remove(0,i);
		// look for end of sentence marker
		for (i=0; i<Buffer.count(); ++i)
			if ( (Buffer[i] == (char)(0x0a)) || (Buffer[i] == (char)(0x0d)) )
				break;
		if (i == Buffer.count())
			return;
		parseNMEA(Buffer.mid(0,i-2));
		Buffer.remove(0,i);
	}
}
#endif

/*** QGPSFileDevice  ***/

QGPSFileDevice::QGPSFileDevice(const QString &device)
	: QGPSDevice()
{
	if(!device.isNull())
	{
		setDevice(device);
	}
}

/**
 * QGPSFileDevice::openDevice()
 *
 * Opens the serial port and sets the parameters for data transfer: parity,
 * stop bits, blocking, etc.
 */

bool QGPSFileDevice::openDevice()
{
	theFile = new QFile(device());

	if (!theFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
		delete theFile;
		theFile = NULL;
        return false;
	}

	return true;
}

void QGPSFileDevice::onLinkReady()
{
}

void QGPSFileDevice::onStop()
{
	quit();
}

/**
 * QGPSFileDevice::closeDevice()
 *
 * Closes the serial port
 */

bool QGPSFileDevice::closeDevice()
{
	if (theFile)
		theFile->close();

	return true;
}

void QGPSFileDevice::run()
{
	GPSSlotForwarder Forward(this);
	QTimer* t = new QTimer;
	connect(t,SIGNAL(timeout()),&Forward,SLOT(onDataAvailable()));
	connect(this,SIGNAL(doStopDevice()),&Forward,SLOT(onStop()));
	t->start(100);
	exec();
	closeDevice();
}

void QGPSFileDevice::onDataAvailable()
{

    int  index = 0;
    char bufferChar;
    char bufferString[100];

	while (theFile->read(&bufferChar, 1) && bufferChar != '$') {}
	if(bufferChar == '$')
	{
		index = 0;
		bufferString[index] = bufferChar;

		do
		{
			theFile->read(&bufferChar, 1);
			if(bufferChar != '\0' && (isalnum(bufferChar) || isspace(bufferChar) || ispunct(bufferChar)))
			{
				index ++;
				bufferString[index] = bufferChar;
			}
		} while(bufferChar != 0x0a && bufferChar != 0x0d);

		bufferString[index + 1] = '\0';

		mutex->lock();

		if(bufferString[3] == 'G' && bufferString[4] == 'G' && bufferString[5] == 'A')
		{
			//strcpy(nmeastr_gga, bufferString);
			parseGGA(bufferString);
		}
		else if(bufferString[3] == 'G' && bufferString[4] == 'L' && bufferString[5] == 'L')
		{
			//strcpy(nmeastr_gga, bufferString);
			parseGLL(bufferString);
		}
		else if(bufferString[3] == 'G' && bufferString[4] == 'S' && bufferString[5] == 'V')
		{
			//strcpy(nmeastr_gsv, bufferString);
			parseGSV(bufferString);
		}
		else if(bufferString[3] == 'G' && bufferString[4] == 'S' && bufferString[5] == 'A')
		{
			//strcpy(nmeastr_gsa, bufferString);
			parseGSA(bufferString);
		}
		else if(bufferString[3] == 'R' && bufferString[4] == 'M' && bufferString[5] == 'C')
		{
			//strcpy(nmeastr_rmc, bufferString);
			if (parseRMC(bufferString))
				if (fixStatus() == QGPSDevice::StatusActive && (fixType() == QGPSDevice::Fix3D || fixType() == QGPSDevice::FixUnavailable))
					emit updatePosition(latitude(), longitude(), dateTime(), altitude(), speed(), heading());
		}

		mutex->unlock();

		emit updateStatus();
	}
}

#ifndef Q_OS_SYMBIAN
/* GPSSDEVICE */

QGPSDDevice::QGPSDDevice(const QString& device)
{
	setDevice(device);
}

bool QGPSDDevice::openDevice()
{
	if (M_PREFS->getGpsSaveLog()) {
		QString fn = "log-" + QDateTime::currentDateTime().toString(Qt::ISODate) + ".nmea";
		fn.replace(':', '-');
		LogFile = new QFile(M_PREFS->getGpsLogDir() + "/"+fn);
		if (!LogFile->open(QIODevice::WriteOnly)) {
			QMessageBox::critical(NULL, tr("GPS log error"),
				tr("Unable to create GPS log file: %1.").arg(M_PREFS->getGpsLogDir() + "/"+fn), QMessageBox::Ok);
			delete LogFile;
			LogFile = NULL;
		}
	}
	return true;
}

bool QGPSDDevice::closeDevice()
{
	return true;
}

// this function will be called within this thread
void QGPSDDevice::onStop()
{
	quit();
}

void QGPSDDevice::run()
{
	GPSSlotForwarder Forward(this);
	QTcpSocket Link;
	Server = &Link;
	Link.connectToHost(M_PREFS->getGpsdHost(),M_PREFS->getGpsdPort());
	connect(Server,SIGNAL(connected()),&Forward,SLOT(onLinkReady()));
	connect(Server,SIGNAL(readyRead()),&Forward,SLOT(onDataAvailable()));
	connect(this,SIGNAL(doStopDevice()),&Forward,SLOT(onStop()));
	exec();
}

void QGPSDDevice::onDataAvailable()
{
#if 0
	QByteArray ba(Server->readAll());
	Buffer.append(ba);
	if (Buffer.length() > 4096)
		// safety valve
		Buffer.remove(0,Buffer.length()-4096);
	int i = Buffer.indexOf("GPSD");
	if (i < 0)
		return;
	int j = Buffer.indexOf(10,i);
	if (j < 0)
		return;
	parse(QString::fromAscii(Buffer.data()+(i+5),(j-i-6)));
	Buffer.remove(0,j+1);
#else
	QByteArray ba(Server->readAll());
	// filter out unwanted characters
	for (int i=ba.count(); i; --i)
		if(ba[i-1] == '\0' ||
			(!isalnum((quint8)ba[i-1]) &&
			 !isspace((quint8)ba[i-1]) &&
			 !ispunct((quint8)ba[i-1])))
		{
			ba.remove(i-1,1);
		}
	if (LogFile)
		LogFile->write(ba);
	Buffer.append(ba);
	if (Buffer.length() > 4096)
		// safety valve
		Buffer.remove(0,Buffer.length()-4096);
	while (Buffer.count())
	{
		// look for begin of sentence marker
		int i = Buffer.indexOf('$');
		if (i<0)
		{
			Buffer.clear();
			return;
		}
		Buffer.remove(0,i);
		// look for end of sentence marker
		for (i=0; i<Buffer.count(); ++i)
			if ( (Buffer[i] == (char)(0x0a)) || (Buffer[i] == (char)(0x0d)) )
				break;
		if (i == Buffer.count())
			return;
		parseNMEA(Buffer.mid(0,i-2));
		Buffer.remove(0,i);
	}
#endif
}

void QGPSDDevice::parse(const QString& s)
{
	qDebug() << "parsing " << s.toUtf8().data() << "*";
	QStringList Args(s.split(',',QString::SkipEmptyParts));
	for (int i=0; i<Args.count(); ++i)
	{
		QString Left(Args[i].left(2));
		if (Left == "O=")
			parseO(Args[i].right(Args[i].length()-2));
		if (Left == "Y=")
			parseY(Args[i].right(Args[i].length()-2));
	}
}

void QGPSDDevice::parseY(const QString& s)
{
	for(int i = 0; i < 50; i ++)
		satArray[i][0] = satArray[i][1] = satArray[i][2] = 0;
	QStringList Sats(s.split(':',QString::SkipEmptyParts));
	for (int i=1; i<Sats.size(); ++i)
	{
		QStringList Items(Sats[i].split(' ',QString::SkipEmptyParts));
		if (Items.count() < 5)
			continue;
		int id = Items[0].toInt();
		if ( (id >= 0) && (id<50) )
		{
			satArray[id][0] = int(Items[1].toDouble());
			satArray[id][1] = int(Items[2].toDouble());
			satArray[id][2] = int(Items[3].toDouble());
		}
	}
	setNumSatellites(Sats.size());
	emit updateStatus();
}

void QGPSDDevice::parseO(const QString& s)
{
	if (s.isEmpty()) return;
	setFixType(FixInvalid);
	if (s[0] == '?') return;
	QStringList Args(s.split(' ',QString::SkipEmptyParts));
	if (Args.count() < 5) return;
	setFixType(Fix3D);
	setFixStatus(StatusActive);
	setLatitude(Args[3].toDouble());
	setLongitude(Args[4].toDouble());
	double Alt = 0;
	if (Args.count() > 5)
		Alt = Args[5].toDouble();
	double Speed = 0;
	if (Args.count() > 9)
		Speed = Args[9].toDouble();
	double Heading = 0;
	if (Args.count() > 7)
		Heading = Args[7].toDouble();
	emit updatePosition(Args[3].toDouble(),
		Args[4].toDouble(),
		QDateTime::currentDateTime(),
		Alt, Speed, Heading);
	setHeading(Heading);
	setAltitude(Alt);
	setSpeed(Speed);
	emit updateStatus();

}

void QGPSDDevice::onLinkReady()
{
	if (!Server) return;
	Server->write("w+");
	Server->write("r+");
	Server->write("j=1");
}
#endif

#ifdef Q_OS_SYMBIAN
/* GPSS60DEVICE */

#include "xqlocation.h"

QGPSS60Device::QGPSS60Device()
{
}

bool QGPSS60Device::openDevice()
{
	return true;
}

bool QGPSS60Device::closeDevice()
{
	return true;
}

// this function will be called within this thread
void QGPSS60Device::onStop()
{
	quit();
}

void QGPSS60Device::run()
{
	GPSSlotForwarder Forward(this);
	connect(this,SIGNAL(doStopDevice()),&Forward,SLOT(onStop()));

	XQLocation location;
	if (location.open() != XQLocation::NoError) {
		emit(doStopDevice());
		return;
	}
	location.startUpdates(1000);

	connect(&location, SIGNAL(locationChanged(double,double,double,float)), this, SLOT(onLocationChanged(double,double,double,float)));
	connect(&location, SIGNAL(statusChanged(XQLocation::DeviceStatus)), this, SLOT(onStatusChanged(XQLocation::DeviceStatus)));
	connect(&location, SIGNAL(dataQualityChanged(XQLocation::DataQuality)), this, SLOT(onDataQualityChanged(XQLocation::DataQuality)));
	connect(&location, SIGNAL(numberOfSatellitesInViewChanged(int)), this, SLOT(setNumSatellites(int)));

	exec();
	
	location.stopUpdates();
}

void QGPSS60Device::onLocationChanged(double latitude, double longitude, double altitude, float speed)
{
	setLatitude(latitude);
	setLongitude(longitude);
	setAltitude(altitude);
	setSpeed(speed);
	
	emit updatePosition(latitude, longitude, QDateTime::currentDateTime(),
			altitude, speed, cur_heading);
}

void QGPSS60Device::onStatusChanged(XQLocation::DeviceStatus)
{
	
}

void QGPSS60Device::onDataQualityChanged(XQLocation::DataQuality qual)
{
	switch (qual) {
	case XQLocation::DataQualityUnknown:
		setFixType(FixUnavailable);
		break;
	case XQLocation::DataQualityLoss:
		setFixType(FixInvalid);
		break;
	case XQLocation::DataQualityPartial:
		setFixType(Fix2D);
		break;
	case XQLocation::DataQualityNormal:
		setFixType(Fix3D);
	}
}
void QGPSS60Device::onLinkReady()
{
}

void QGPSS60Device::onDataAvailable()
{
}


#endif
