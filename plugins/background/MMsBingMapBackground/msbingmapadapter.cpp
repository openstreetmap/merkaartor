#include <QtPlugin>

#include "msbingmapadapter.h"

#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif

static const QUuid theUid ( "{2a888701-1a93-4040-9b34-1e5339f67f43}");
static const QString theName("Bing Maps");

QUuid MsBingMapAdapterFactory::getId() const
{
    return theUid;
}

QString	MsBingMapAdapterFactory::getName() const
{
    return theName;
}

/**************/

MsBingMapAdapter::MsBingMapAdapter()
    : MapAdapter(QString("ecn.t%1.tiles.virtualearth.net"), QString("/tiles/a%1.jpeg?g=587&mkt=en-gb&n=z"), QString("EPSG:3857"), 0, 19)
{
}

MsBingMapAdapter::~MsBingMapAdapter()
{
}

QUuid MsBingMapAdapter::getId() const
{
    return QUuid(theUid);
}

QString	MsBingMapAdapter::getName() const
{
    return theName;
}


IMapAdapter::Type MsBingMapAdapter::getType() const
{
    return IMapAdapter::NetworkBackground;
}

QString MsBingMapAdapter::getHost() const
{
    int random = qrand() % 6;
    return host.arg(random);
}

QPoint MsBingMapAdapter::coordinateToDisplay(const QPointF& coordinate) const
{
    double x = (coordinate.x()+180.) * (getTilesWE(current_zoom)*getTileSizeW())/360.;		// coord to pixel!
    double y = (getMercatorYCoord(coordinate.y())-M_PI) * -1 * (getTilesNS(current_zoom)*getTileSizeH())/(2*M_PI);	// coord to pixel!
    return QPoint(int(x), int(y));
}

QPointF MsBingMapAdapter::displayToCoordinate(const QPoint& point) const
{
    double lon = (point.x()*(360./(getTilesWE(current_zoom)*getTileSizeW())))-180.;
    double lat = getMercatorLatitude(point.y()*-1*(2*M_PI/(getTilesWE(current_zoom)*getTileSizeW())) + M_PI);
    lat = lat *180./M_PI;
    return QPointF(lon, lat);
}

double MsBingMapAdapter::getMercatorLatitude(double YCoord) const
{
    // http://welcome.warnercnr.colostate.edu/class_info/nr502/lg4/projection_mathematics/converting.html
    if (YCoord > M_PI) return 9999.;
    if (YCoord < -M_PI) return -9999.;

    double t = atan(exp(YCoord));
    double res = (2.*(t))-(M_PI/2.);
    return res;
}

double MsBingMapAdapter::getMercatorYCoord(double lati) const
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

void MsBingMapAdapter::zoom_in()
{
    if (min_zoom > max_zoom)
        current_zoom = current_zoom > max_zoom ? current_zoom-1 : max_zoom;
    else if (min_zoom < max_zoom)
        current_zoom = current_zoom < max_zoom ? current_zoom+1 : max_zoom;
}

void MsBingMapAdapter::zoom_out()
{
    if (min_zoom > max_zoom)
        current_zoom = current_zoom < min_zoom ? current_zoom+1 : min_zoom;
    else if (min_zoom < max_zoom)
        current_zoom = current_zoom > min_zoom ? current_zoom-1 : min_zoom;
}

bool MsBingMapAdapter::isValid(int x, int y, int z) const
{
    if ((x>=0 && x < getTilesWE(current_zoom)) && (y>=0 && y < getTilesNS(current_zoom)) && z>=0)
    {
        return true;
    }
    return false;
}

QRectF	MsBingMapAdapter::getBoundingbox() const
{
    return QRectF(QPointF(-20037508.34, -20037508.34), QPointF(20037508.34, 20037508.34));
}

int MsBingMapAdapter::getTilesWE(int zoomlevel) const
{
    return int(pow(2, zoomlevel+1.0));
}

int MsBingMapAdapter::getTilesNS(int zoomlevel) const
{
    return int(pow(2, zoomlevel+1.0));
}

QString MsBingMapAdapter::getQuery(int i, int j, int z) const
{
    return getQ(-180+i*(360./getTilesWE(current_zoom)),
                    90-(j+1)*(180./getTilesNS(current_zoom)), z+1);
}

QString MsBingMapAdapter::getQ(double longitude, double latitude, int zoom) const
{
    double xmin=-180;
    double xmax=180;
    double ymin=-90;
    double ymax=90;

    double xmoy=0;
    double ymoy=0;
    QString location="";

    for (int i = 0; i < zoom; i++)
    {
        xmoy = (xmax + xmin) / 2;
        ymoy = (ymax + ymin) / 2;
        if (latitude >= ymoy) //upper part (q or r)
        {
            ymin = ymoy;
            if (longitude < xmoy)
            { /*q*/
                location+= "0";
                xmax = xmoy;
            }
            else
            {/*r*/
                location+= "1";
                xmin = xmoy;
            }
        }
        else //lower part (t or s)
        {
            ymax = ymoy;
            if (longitude < xmoy)
            { /*t*/

                location+= "2";
                xmax = xmoy;
            }
            else
            {/*s*/
                location+= "3";
                xmin = xmoy;
            }
        }
    }
    return serverPath.arg(location);
}

int MsBingMapAdapter::getTileSizeW() const
{
    return 256;
}

int MsBingMapAdapter::getTileSizeH() const
{
    return 256;
}

QString MsBingMapAdapter::getSourceTag() const
{
    return QString();
}

QString MsBingMapAdapter::getLicenseUrl() const
{
    return QString();
}

Q_EXPORT_PLUGIN2(MMsBingMapBackgroundPlugin, MsBingMapAdapterFactory)
