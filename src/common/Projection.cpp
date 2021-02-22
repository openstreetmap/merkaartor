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


#ifndef _MOBILE
    projCtx = proj_context_create();
#if defined(Q_OS_WIN)
    QString pdir(QDir::toNativeSeparators(qApp->applicationDirPath() + "/" STRINGIFY(SHARE_DIR) "/proj"));
    const char* proj_dir = pdir.toUtf8().constData();
    proj_context_set_search_paths(projCtx, 1, &proj_dir);
#endif // Q_OS_WIN
    projTransform = NULL;
    projMutex = new QMutex();
    setProjectionType(initProjection);
#endif
}

ProjectionBackend::~ProjectionBackend(void)
{
    /* TODO: pj_free should be called, but it segfaults if two of the same
     * Projection objects have the same projPJ. A better machinism, perhaps
     * projPJ caching, should be provided.
     *
     * In the meantime, pj_free is not called, which does little harm as it's
     * usually called at exit only.
     * */
#ifndef _MOBILE
    if (projTransform) {
      //proj_destroy(projTransform);
      //delete projMutex;
    }
    //proj_context_destroy(projCtx);
#endif // _MOBILE
}

QPointF ProjectionBackend::inverse2Point(const QPointF & Map) const
{
    if  (IsLatLong)
        return latlonInverse(Map);
    else
        if  (IsMercator)
            return mercatorInverse(Map);
#ifndef _MOBILE
        else
            return projInverse(Map);
#endif
    return QPointF();
}

QPointF ProjectionBackend::project(const QPointF & Map) const
{
    if  (IsMercator)
        return mercatorProject(Map);
    else
        if  (IsLatLong)
            return latlonProject(Map);
#ifndef _MOBILE
        else
            return projProject(Map);
#endif
    return QPointF();
}

QLineF ProjectionBackend::project(const QLineF & Map) const
{
    if  (IsMercator)
        return QLineF (mercatorProject(Map.p1()), mercatorProject(Map.p2()));
    else
        if  (IsLatLong)
            return QLineF (latlonProject(Map.p1()), latlonProject(Map.p2()));
#ifndef _MOBILE
        else
            return QLineF(projProject(Map.p1()), projProject(Map.p2()));
#endif
    return QLineF();
}


Coord ProjectionBackend::inverse2Coord(const QPointF & projPoint) const
{
    if  (IsLatLong)
        return latlonInverse(projPoint);
    else
        if  (IsMercator)
            return mercatorInverse(projPoint);
#ifndef _MOBILE
        else
            return projInverse(projPoint);
#endif
    return Coord();
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

    tl = inverse2Coord(Viewport.topLeft());
    br = inverse2Coord(Viewport.bottomRight());
    bbox = CoordBox(tl, br);

    return bbox;
}

#ifndef _MOBILE

QPointF ProjectionBackend::projProject(const QPointF & Map) const
{
    QMutexLocker locker(projMutex);
    auto trans = proj_trans(projTransform, PJ_DIRECTION::PJ_FWD, {{Map.x(), Map.y(), 0}});
    //qDebug() << "Project(fromWSG84, " << getProjectionType() << "): " << Map << " -> " << qSetRealNumberPrecision(20) << x << "," << y;
    return QPointF(trans.xy.x, trans.xy.y);
}

Coord ProjectionBackend::projInverse(const QPointF & pProj) const
{
    QMutexLocker locker(projMutex);
    auto trans = proj_trans(projTransform, PJ_DIRECTION::PJ_INV, {{pProj.x(), pProj.y(), 0}});
    return Coord(trans.xy.x, trans.xy.y);
}
#endif // _MOBILE

bool ProjectionBackend::projIsLatLong() const
{
    return IsLatLong;
}

#ifndef _MOBILE
PJ* ProjectionBackend::getProjection(QString projString)
{
    QString WGS84("+proj=longlat +ellps=WGS84 +datum=WGS84 +xy_in=deg");
    PJ* proj = proj_create_crs_to_crs(projCtx, WGS84.toLatin1(), projString.toLatin1(), 0);
    if (!proj) {
            qDebug() << "Failed to initialize projection" << WGS84 << "to" << projString << "with error:" << proj_errno_string(proj_errno(nullptr));
    }
    return proj;
}
#endif // _MOBILE

bool ProjectionBackend::setProjectionType(QString aProjectionType)
{
    QMutexLocker locker(projMutex);
    if (aProjectionType == projType)
        return true;

#ifndef _MOBILE
    if (projTransform) {
        proj_destroy(projTransform);
        projTransform = nullptr;
    }
#endif // _MOBILE

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

#ifndef _MOBILE
    try {
        projProj4 = mapProjectionName(aProjectionType);
        projTransform = getProjection(projProj4);
        if (!projTransform) {
            // Fall back to the EPSG:3857 and return false. getProjection already logged the error into qDebug().
            projType = "EPSG:3857";
            IsMercator = true;
            return false;
        }
    } catch (...) {
        return false;
    }
    return (projTransform != NULL || IsLatLong || IsMercator);
#else
    return false;
#endif // _MOBILE
}

QString ProjectionBackend::getProjectionType() const
{
    return projType;
}

QString ProjectionBackend::getProjectionProj4() const
{
  QMutexLocker locker(projMutex);
    if  (IsLatLong)
        return "+init=EPSG:4326";
    else if  (IsMercator)
        return "+init=EPSG:3857";
#ifndef _MOBILE
    else
        return QString(proj_pj_info(projTransform).definition);
#endif
    return QString();
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
