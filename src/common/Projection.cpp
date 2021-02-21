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

#include "Node.h"

ProjectionBackend::ProjectionBackend(QString initProjection, std::function<QString(QString)> mapProjectionName)
    : ProjectionRevision(0)
    , IsMercator(false)
    , IsLatLong(false)
    , mapProjectionName(mapProjectionName)
{
#if defined(Q_OS_WIN) && !defined(_MOBILE)
    QString pdir(QDir::toNativeSeparators(qApp->applicationDirPath() + "/" STRINGIFY(SHARE_DIR) "/proj"));
    const char* proj_dir = pdir.toUtf8().constData();
    //    const char* proj_dir = "E:\\cbro\\src\\merkaartor-devel\\binaries\\bin\\share\\proj";
    pj_set_searchpath(1, &proj_dir);
#endif // Q_OS_WIN


#ifndef _MOBILE
    theProj = NULL;
    theWGS84Proj = ProjectionBackend::getProjection("+proj=longlat +ellps=WGS84 +datum=WGS84");
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
    if (theProj) {
        //pj_free(theProj);
    }
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

void ProjectionBackend::projTransform(ProjProjection srcdefn,
                               ProjProjection dstdefn,
                               long point_count, int point_offset, qreal *x, qreal *y, qreal *z )
{
    pj_transform(srcdefn, dstdefn, point_count, point_offset, (double *)x, (double *)y, (double *)z);
}

void ProjectionBackend::projTransformFromWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const
{
    pj_transform (theWGS84Proj, theProj, point_count, point_offset, (double *)x, (double *)y, (double *)z);
}

void ProjectionBackend::projTransformToWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const
{
    pj_transform(theProj, theWGS84Proj, point_count, point_offset, (double *)x, (double *)y, (double *)z);
}

QPointF ProjectionBackend::projProject(const QPointF & Map) const
{
    qreal x = angToRad(Map.x());
    qreal y = angToRad(Map.y());

    projTransformFromWGS84(1, 0, &x, &y, NULL);

    return QPointF(x, y);
}

Coord ProjectionBackend::projInverse(const QPointF & pProj) const
{
    qreal x = pProj.x();
    qreal y = pProj.y();

    projTransformToWGS84(1, 0, &x, &y, NULL);

    return Coord(radToAng(x), radToAng(y));
}
#endif // _MOBILE

bool ProjectionBackend::projIsLatLong() const
{
    return IsLatLong;
}

//bool ProjectionBackend::projIsMercator()
//{
//    return IsMercator;
//}


#ifndef _MOBILE
ProjProjection ProjectionBackend::getProjection(QString projString)
{
    ProjProjection theProj = pj_init_plus(QString("%1 +over").arg(projString).toLatin1());
    return theProj;
}
#endif // _MOBILE

bool ProjectionBackend::setProjectionType(QString aProjectionType)
{
    if (aProjectionType == projType)
        return true;

#ifndef _MOBILE
    if (theProj) {
        pj_free(theProj);
        theProj = NULL;
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
    if (
            projType.toUpper().contains("EPSG:4326")
            )
    {
        IsLatLong = true;
        projType = "EPSG:4326";
        return true;
    }

#ifndef _MOBILE
    try {
        projProj4 = mapProjectionName(aProjectionType);
        theProj = getProjection(projProj4);
        if (!theProj) {
            projType = "EPSG:3857";
            IsMercator = true;
            return false;
        }
        //        else {
        //            if (pj_is_latlong(theProj))
        //                projType = "EPSG:4326";
        //                IsLatLong = true;
        //        }
    } catch (...) {
        return false;
    }
    return (theProj != NULL || IsLatLong || IsMercator);
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
    if  (IsLatLong)
        return "+init=EPSG:4326";
    else if  (IsMercator)
        return "+init=EPSG:3857";
#ifndef _MOBILE
    else
        return QString(pj_get_def(theProj, 0));
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
