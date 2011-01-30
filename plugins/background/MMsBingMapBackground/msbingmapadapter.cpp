#include <QtPlugin>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTimer>

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
    : MapAdapter(QString("ecn.t3.tiles.virtualearth.net"), QString("/tiles/a%1.jpeg?g=587&mkt=en-gb&n=z"), QString("EPSG:3857"), 0, 20)
    , theImageManager(0)
    , theSource("Bing")
    , isLoaded(false)
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
    if (!isLoaded) {
        int random = qrand() % 6;
        return host.arg(random);
    } else
        return host;
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
    return theSource;
}

void MsBingMapAdapter::setSourceTag (const QString& value)
{
    theSource = value;
}

QString MsBingMapAdapter::getLicenseUrl() const
{
    return QString();
}

Q_EXPORT_PLUGIN2(MMsBingMapBackgroundPlugin, MsBingMapAdapterFactory)

QString MsBingMapAdapter::getAttributionsHtml(const QRectF &bbox, const QRect &screen)
{
    QStringList providers;
    int zoom = qRound(log(360. / bbox.width()) / log(2));   // log2 not available on FreeBSD
    qDebug() << "Bing Zoom: " << zoom;
    foreach (BingProvider prov, theProviders) {
        if (prov.bbox.intersects(bbox))
            if (zoom >= prov.zoomMin && zoom <= prov.zoomMax)
                providers << prov.name;
    }

    return QString("<div style=\"color:silver; font-size:9px\">%1</div>").arg(providers.join(" "));
}

QString MsBingMapAdapter::getLogoHtml()
{
    return QString("<center><a href=\"http://www.bing.com/maps/\"><img src=\":/images/bing_logo.png\"/></a><br/><a href=\"http://opengeodata.org/microsoft-imagery-details\" style=\"color:silver; font-size:9px\">%1</a></center>").arg(tr("Terms of Use"));
}

int MsBingMapAdapter::getMinZoom(const QRectF &bbox) const
{
    return min_zoom;
}

int MsBingMapAdapter::getMaxZoom(const QRectF &bbox) const
{
    return max_zoom;
}

int MsBingMapAdapter::getAdaptedMinZoom(const QRectF &bbox) const
{
    return 0;
}

int MsBingMapAdapter::getAdaptedMaxZoom(const QRectF &bbox) const
{
    return max_zoom > min_zoom ? max_zoom - min_zoom : min_zoom - max_zoom;
}

int MsBingMapAdapter::getZoom() const
{
    return current_zoom;
}

int MsBingMapAdapter::getAdaptedZoom() const
{
    return max_zoom < min_zoom ? min_zoom - current_zoom : current_zoom - min_zoom;
}

IImageManager* MsBingMapAdapter::getImageManager()
{
    return theImageManager;
}

void MsBingMapAdapter::setImageManager(IImageManager* anImageManager)
{
    theImageManager = anImageManager;

    QNetworkAccessManager* manager = theImageManager->getNetworkManager();
    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(on_adapterDataFinished(QNetworkReply*)));

    manager->get(QNetworkRequest(QUrl("http://dev.virtualearth.net/REST/v1/Imagery/Metadata/Aerial/0,0?zl=1&mapVersion=v1&key=AlRQe0E4ha3yKkz2MuNI-G1AIk-CIym4zTeqaTgKVWz_LBsnQuPksHrHCOT0381M&include=ImageryProviders&output=xml")));
}

void MsBingMapAdapter::on_adapterDataFinished(QNetworkReply* reply)
{
//    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (reply->error() != QNetworkReply::NoError)
        if (reply->error() != QNetworkReply::OperationCanceledError)
            return;

    QDomDocument theDoc;
    theDoc.setContent(reply->readAll());

    QDomNodeList hostEl = theDoc.elementsByTagName("ImageUrl");
    if (hostEl.size()) {
        QUrl u(hostEl.at(0).toElement().text());
        host = u.host();
    }

    QString curProvider;
    QDomNodeList providers = theDoc.elementsByTagName("ImageryProvider");
    for (int i=0; i<providers.size(); ++i) {
        QDomNode nd = providers.at(i);
        QDomElement provider = nd.firstChildElement("Attribution");
        if (!provider.isNull())
            curProvider = provider.text();
        QDomNodeList coverages = nd.toElement().elementsByTagName("CoverageArea");
        for (int j=0; j<coverages.size(); ++j) {
            QDomNode cover = coverages.at(j);
            BingProvider prov;
            prov.name = curProvider;
            prov.zoomMin = cover.firstChildElement("ZoomMin").text().toInt();
            prov.zoomMax = cover.firstChildElement("ZoomMax").text().toInt();
            QDomElement bbox = cover.firstChildElement("BoundingBox");
            prov.bbox.setBottom(bbox.firstChildElement("SouthLatitude").text().toDouble());
            prov.bbox.setLeft(bbox.firstChildElement("WestLongitude").text().toDouble());
            prov.bbox.setTop(bbox.firstChildElement("NorthLatitude").text().toDouble());
            prov.bbox.setRight(bbox.firstChildElement("EastLongitude").text().toDouble());

            theProviders << prov;
        }
    }
    isLoaded = true;
}
