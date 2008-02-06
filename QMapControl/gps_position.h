#ifndef GPS_POSITION_H
#define GPS_POSITION_H

#include <QString>

//! Represents a coordinate from a GPS receiver
/*!
 * This class is used to represent a coordinate which has been parsed from a NMEA string.
 * This is not fully integrated in the API. An example which uses this data type can be found under Samples.
 * @author Kai Winter
*/
class GPS_Position
{
	public:
		GPS_Position(float time, float longitude, QString longitude_dir, float latitude, QString latitude_dir);
		float time; /*!< time of the string*/
		float longitude; /*!< longitude coordinate*/
		float latitude; /*!< latitude coordinate*/
		
	private:
		QString longitude_dir;
		QString latitude_dir;
		
};

#endif
