#include "gps_position.h"

GPS_Position::GPS_Position(float time, float longitude, QString longitude_dir, float latitude, QString latitude_dir)
	:time(time), longitude(longitude), latitude(latitude), longitude_dir(longitude_dir), latitude_dir(latitude_dir)
{
}
