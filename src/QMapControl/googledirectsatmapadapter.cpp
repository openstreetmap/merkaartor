#include "googlesatmapadapter.h"

#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

GoogleSatMapAdapter::GoogleSatMapAdapter()
	: MapAdapter("kh.google.com", "/kh?v=8&t=trtqtt", 256, 0, 19)
{
	name = "googlesat";

	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}

GoogleSatMapAdapter::~GoogleSatMapAdapter()
{
}

QString GoogleSatMapAdapter::getHost() const
{
	//int random = qrand() % 4;
	//return QString("kh%1.google.com").arg(random);
	return QString("kh.google.com");
}

QPoint GoogleSatMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
// 	double x = ((coordinate.x()+180)*(tilesize*numberOfTiles/360));
// 	double y = (((coordinate.y()*-1)+90)*(tilesize*numberOfTiles/180));

	double x = (coordinate.x()+180.) * (numberOfTiles*tilesize)/360.;		// coord to pixel!
//	double y = -1*(coordinate.y()-90) * (numberOfTiles*tilesize)/180.;	// coord to pixel!
	double y = (getMercatorYCoord(coordinate.y())-M_PI) * -1 * (numberOfTiles*tilesize)/(2*M_PI);	// coord to pixel!
	return QPoint(int(x), int(y));
}

QPointF GoogleSatMapAdapter::displayToCoordinate(const QPoint& point) const
{
// 	double lon = ((point.x()/tilesize*numberOfTiles)*360)-180;
// 	double lat = (((point.y()/tilesize*numberOfTiles)*180)-90)*-1;

	double lon = (point.x()*(360./(numberOfTiles*tilesize)))-180.;
//	double lat = -(point.y()*(180./(numberOfTiles*tilesize)))+90;
	double lat = getMercatorLatitude(point.y()*-1*(2*M_PI/(numberOfTiles*tilesize)) + M_PI);
	lat = lat *180./M_PI;
	return QPointF(lon, lat);
}

double GoogleSatMapAdapter::getMercatorLatitude(double YCoord) const
{
	// http://welcome.warnercnr.colostate.edu/class_info/nr502/lg4/projection_mathematics/converting.html
	if (YCoord > M_PI) return 9999.;
	if (YCoord < -M_PI) return -9999.;

	double t = atan(exp(YCoord));
	double res = (2.*(t))-(M_PI/2.);
	return res;
}

double GoogleSatMapAdapter::getMercatorYCoord(double lati) const
{
	double lat = lati;

            // conversion degre=>radians
	double phi = M_PI * lat / 180;

	double res;
            //double temp = Math.Tan(Math.PI / 4 - phi / 2);
            //res = Math.Log(temp);
	res = 0.5 * log((1 + sin(phi)) / (1 - sin(phi)));

	return res;
}

void GoogleSatMapAdapter::zoom_in()
{
	current_zoom+=1;
	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}

void GoogleSatMapAdapter::zoom_out()
{
	current_zoom-=1;
	numberOfTiles = pow(2, current_zoom+0.0);
	coord_per_x_tile = 360. / numberOfTiles;
	coord_per_y_tile = 180. / numberOfTiles;
}

bool GoogleSatMapAdapter::isValid(int x, int y, int z) const
{
 	if ((x>=0 && x < numberOfTiles) && (y>=0 && y < numberOfTiles) && z>=0)
	{
		return true;
	}
 	return false;
}
QString GoogleSatMapAdapter::getQuery(int i, int j, int z) const
{
	return getQ(-180+i*coord_per_x_tile,
 					90-(j+1)*coord_per_y_tile, z);
}

QString GoogleSatMapAdapter::getQ(double longitude, double latitude, int zoom) const
{
	double xmin=-180;
	double xmax=180;
	double ymin=-90;
	double ymax=90;

	double xmoy=0;
	double ymoy=0;
	QString location="t";

	//Google uses a latitude divided by 2;
	double halflat = latitude;

	for (int i = 0; i < zoom; i++)
	{
		xmoy = (xmax + xmin) / 2;
		ymoy = (ymax + ymin) / 2;
		if (halflat >= ymoy) //upper part (q or r)
		{
			ymin = ymoy;
			if (longitude < xmoy)
			{ /*q*/
				location+= "q";
				xmax = xmoy;
			}
			else
			{/*r*/
				location+= "r";
				xmin = xmoy;
			}
		}
		else //lower part (t or s)
		{
			ymax = ymoy;
			if (longitude < xmoy)
			{ /*t*/

				location+= "t";
				xmax = xmoy;
			}
			else
			{/*s*/
				location+= "s";
				xmin = xmoy;
			}
		}
	}
	return QString("/kh?v=3&t=%1").arg(location);
}


