#include "Projection.h"

#include <QRect>
#include <QRectF>

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
#define EQUATORIALMETERCIRCUMFERENCE  40075016.68
#define EQUATORIALMETERHALFCIRCUMFERENCE  20037508.34
#define EQUATORIALMETERPERDEGREE    222638.981555556

ProjectionBackend::ProjectionBackend(QString initProjection, std::function<QString(QString)> mapProjectionName)
    : ProjectionRevision(0)
    , IsMercator(false)
    , IsLatLong(false)
    , mapProjectionName(mapProjectionName)
{


    projCtx = std::shared_ptr<PJ_CONTEXT>(proj_context_create(), proj_context_destroy);
#if defined(Q_OS_WIN)
    QString pdir(QDir::toNativeSeparators(qApp->applicationDirPath() + "/" STRINGIFY(SHARE_DIR) "/proj"));
    const char* proj_dir = pdir.toUtf8().constData();
    proj_context_set_search_paths(projCtx.get(), 1, &proj_dir);
#endif // Q_OS_WIN
    projTransform = std::shared_ptr<PJ>(nullptr);
    projMutex = std::shared_ptr<QMutex>(new QMutex());
    setProjectionType(initProjection);
}

QPointF ProjectionBackend::project(const QPointF & Map) const
{
    if  (IsMercator)
        return mercatorProject(Map);
    if  (IsLatLong)
        return latlonProject(Map);
    return projProject(Map);
}

QLineF ProjectionBackend::project(const QLineF & Map) const
{
    return QLineF(project(Map.p1()), project(Map.p2()));
}


QPointF ProjectionBackend::inverse(const QPointF & projPoint) const
{
    if (IsLatLong)
        return latlonInverse(projPoint);
    if (IsMercator)
        return mercatorInverse(projPoint);
    return projInverse(projPoint);
}

QRectF ProjectionBackend::toProjectedRectF(const QRectF& Viewport, const QRect& screen) const
{
    QPointF tl, br;
    QRectF pViewport;

    tl = project(Viewport.topLeft());
    br = project(Viewport.bottomRight());
    pViewport = QRectF(tl, br);

    QPointF pCenter(pViewport.center());

    qreal wv, hv;
    //wv = (pViewport.width() / Viewport.londiff()) * ((double)screen.width() / Viewport.londiff());
    //hv = (pViewport.height() / Viewport.latdiff()) * ((double)screen.height() / Viewport.latdiff());

    qreal Aspect = (double)screen.width() / screen.height();
    qreal pAspect = fabs(pViewport.width() / pViewport.height());

    if (pAspect > Aspect) {
        wv = fabs(pViewport.width());
        hv = fabs(pViewport.height() * pAspect / Aspect);
    } else {
        wv = fabs(pViewport.width() * Aspect / pAspect);
        hv = fabs(pViewport.height());
    }

    pViewport = QRectF((pCenter.x() - wv/2), (pCenter.y() + hv/2), wv, -hv);

    return pViewport;
}

CoordBox ProjectionBackend::fromProjectedRectF(const QRectF& Viewport) const
{
    Coord tl, br;
    CoordBox bbox;

    tl = inverse(Viewport.topLeft());
    br = inverse(Viewport.bottomRight());
    bbox = CoordBox(tl, br);

    return bbox;
}

QPointF ProjectionBackend::projProject(const QPointF & Map) const
{
    QMutexLocker locker(projMutex.get());
    auto trans = proj_trans(projTransform.get(), PJ_DIRECTION::PJ_FWD, {{Map.x(), Map.y(), 0}});
    //qDebug() << "Project(fromWSG84, " << getProjectionType() << "): " << Map << " -> " << qSetRealNumberPrecision(20) << x << "," << y;
    return QPointF(trans.xy.x, trans.xy.y);
}

Coord ProjectionBackend::projInverse(const QPointF & pProj) const
{
    QMutexLocker locker(projMutex.get());
    auto trans = proj_trans(projTransform.get(), PJ_DIRECTION::PJ_INV, {{pProj.x(), pProj.y(), 0}});
    return Coord(trans.xy.x, trans.xy.y);
}

bool ProjectionBackend::projIsLatLong() const
{
    return IsLatLong;
}

PJ* ProjectionBackend::getProjection(QString projString)
{
    QString WGS84("+proj=longlat +ellps=WGS84 +datum=WGS84 +xy_in=deg");
    PJ* proj = proj_create_crs_to_crs(projCtx.get(), WGS84.toLatin1(), projString.toLatin1(), 0);
    if (!proj) {
            qDebug() << "Failed to initialize projection" << WGS84 << "to" << projString << "with error:" << proj_errno_string(proj_errno(nullptr));
    }
    return proj;
}

bool ProjectionBackend::setProjectionType(QString aProjectionType)
{
    QMutexLocker locker(projMutex.get());
    if (aProjectionType == projType)
        return true;

    projTransform = nullptr;

    ProjectionRevision++;
    projType = aProjectionType;
    projProj4 = QString();
    IsLatLong = false;
    IsMercator = false;

    // Hardcode "Google " projection
    if (
            projType.isEmpty() ||
            projType.toUpper().contains("OSGEO:41001") ||
            projType.toUpper().contains("EPSG:3785") ||
            projType.toUpper().contains("EPSG:900913") ||
            projType.toUpper().contains("EPSG:3857")
            )
    {
        IsMercator = true;
        projType = "EPSG:3857";
        return true;
    }
    // Hardcode "lat/long " projection
    if ( projType.toUpper() == "EPSG:4326" )
    {
        IsLatLong = true;
        projType = "EPSG:4326";
        return true;
    }

    projProj4 = mapProjectionName(aProjectionType);
    projTransform = std::shared_ptr<PJ>(getProjection(projProj4), proj_destroy);
    if (!projTransform) {
        // Fall back to the EPSG:3857 and return false. getProjection already logged the error into qDebug().
        projType = "EPSG:3857";
        IsMercator = true;
        return false;
    }
    return (projTransform != NULL || IsLatLong || IsMercator);
}

QString ProjectionBackend::getProjectionType() const
{
    return projType;
}

QString ProjectionBackend::getProjectionProj4() const
{
  QMutexLocker locker(projMutex.get());
    if (IsLatLong)
        return "+init=EPSG:4326";
    if (IsMercator)
        return "+init=EPSG:3857";
    return QString(proj_pj_info(projTransform.get()).definition);
}

int ProjectionBackend::projectionRevision() const
{
    return ProjectionRevision;
}

// Common routines

qreal ProjectionBackend::latAnglePerM() const
{
    qreal LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    return 1 / LengthOfOneDegreeLat;
}

qreal ProjectionBackend::lonAnglePerM(qreal Lat) const
{
    qreal LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    qreal LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(Lat));
    return 1 / LengthOfOneDegreeLon;
}

bool ProjectionBackend::toXML(QXmlStreamWriter& stream)
{
    bool OK = true;

    stream.writeStartElement("Projection");
    stream.writeAttribute("type", projType);
    if (!IsLatLong && !IsMercator && !projProj4.isEmpty()) {
        stream.writeCharacters(projProj4);
    }
    stream.writeEndElement();


    return OK;
}

void ProjectionBackend::fromXML(QXmlStreamReader& stream)
{
    if (stream.name() == "Projection") {
        QString proj;
        if (stream.attributes().hasAttribute("type"))
            proj = stream.attributes().value("type").toString();
        else
            proj = QCoreApplication::translate("Projection", "Document");
        stream.readNext();
        if (stream.tokenType() == QXmlStreamReader::Characters) {
            setProjectionType(stream.text().toString());
            projType = proj;
            stream.readNext();
        } else
            setProjectionType(proj);
    }
}

QPointF ProjectionBackend::mercatorProject(const QPointF& c) const
{
    qreal x = c.x() / 180. * EQUATORIALMETERHALFCIRCUMFERENCE;
    qreal y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (EQUATORIALMETERHALFCIRCUMFERENCE);

    return QPointF(x, y);
}

Coord ProjectionBackend::mercatorInverse(const QPointF& point) const
{
    qreal longitude = point.x()*180.0/EQUATORIALMETERHALFCIRCUMFERENCE;
    qreal latitude = radToAng(atan(sinh(point.y()/EQUATORIALMETERHALFCIRCUMFERENCE*M_PI)));

    return Coord(longitude, latitude);
}
