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

Projection::Projection(void)
    : ProjectionRevision(0)
    , IsMercator(false)
    , IsLatLong(false)
{
#if defined(Q_OS_WIN) && !defined(_MOBILE)
    QString pdir(QDir::toNativeSeparators(qApp->applicationDirPath() + "/" STRINGIFY(SHARE_DIR) "/proj"));
    const char* proj_dir = pdir.toUtf8().constData();
    //    const char* proj_dir = "E:\\cbro\\src\\merkaartor-devel\\binaries\\bin\\share\\proj";
    pj_set_searchpath(1, &proj_dir);
#endif // Q_OS_WIN

#ifndef _MOBILE
    theProj = NULL;
    theWGS84Proj = Projection::getProjection("+proj=longlat +ellps=WGS84 +datum=WGS84");
    setProjectionType(M_PREFS->getProjectionType());
#endif
}

Projection::~Projection(void)
{
#ifndef _MOBILE
    pj_free(theProj);
#endif // _MOBILE
}

QPointF Projection::inverse2Point(const QPointF & Map) const
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

QPointF Projection::project(const QPointF & Map) const
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

QPointF Projection::project(Node* aNode) const
{
    return project(aNode->position());
}

QLineF Projection::project(const QLineF & Map) const
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


Coord Projection::inverse2Coord(const QPointF & projPoint) const
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

QRectF Projection::toProjectedRectF(const QRectF& Viewport, const QRect& screen) const
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

CoordBox Projection::fromProjectedRectF(const QRectF& Viewport) const
{
    Coord tl, br;
    CoordBox bbox;

    tl = inverse2Coord(Viewport.topLeft());
    br = inverse2Coord(Viewport.bottomRight());
    bbox = CoordBox(tl, br);

    return bbox;
}

#ifndef _MOBILE

void Projection::projTransform(ProjProjection srcdefn,
                               ProjProjection dstdefn,
                               long point_count, int point_offset, qreal *x, qreal *y, qreal *z )
{
    pj_transform(srcdefn, dstdefn, point_count, point_offset, x, y, z);
}

void Projection::projTransformFromWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const
{
    pj_transform (theWGS84Proj, theProj, point_count, point_offset, x, y, z);
}

void Projection::projTransformToWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const
{
    pj_transform(theProj, theWGS84Proj, point_count, point_offset, x, y, z);
}

QPointF Projection::projProject(const QPointF & Map) const
{
    qreal x = angToRad(Map.x());
    qreal y = angToRad(Map.y());

    projTransformFromWGS84(1, 0, &x, &y, NULL);

    return QPointF(x, y);
}

Coord Projection::projInverse(const QPointF & pProj) const
{
    qreal x = pProj.x();
    qreal y = pProj.y();

    projTransformToWGS84(1, 0, &x, &y, NULL);

    return Coord(radToAng(x), radToAng(y));
}
#endif // _MOBILE

bool Projection::projIsLatLong() const
{
    return IsLatLong;
}

//bool Projection::projIsMercator()
//{
//    return IsMercator;
//}


#ifndef _MOBILE
ProjProjection Projection::getProjection(QString projString)
{
    ProjProjection theProj = pj_init_plus(QString("%1 +over").arg(projString).toLatin1());
    return theProj;
}
#endif // _MOBILE

bool Projection::setProjectionType(QString aProjectionType)
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
        projProj4 = M_PREFS->getProjection(aProjectionType).projection;
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

QString Projection::getProjectionType() const
{
    return projType;
}

QString Projection::getProjectionProj4() const
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

int Projection::projectionRevision() const
{
    return ProjectionRevision;
}

// Common routines

qreal Projection::latAnglePerM() const
{
    qreal LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    return 1 / LengthOfOneDegreeLat;
}

qreal Projection::lonAnglePerM(qreal Lat) const
{
    qreal LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    qreal LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(Lat));
    return 1 / LengthOfOneDegreeLon;
}

bool Projection::toXML(QXmlStreamWriter& stream)
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

void Projection::fromXML(QXmlStreamReader& stream)
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

QPointF Projection::mercatorProject(const QPointF& c) const
{
    qreal x = c.x() / 180. * EQUATORIALMETERHALFCIRCUMFERENCE;
    qreal y = log(tan(angToRad(c.y())) + 1/cos(angToRad(c.y()))) / M_PI * (EQUATORIALMETERHALFCIRCUMFERENCE);

    return QPointF(x, y);
}

Coord Projection::mercatorInverse(const QPointF& point) const
{
    qreal longitude = point.x()*180.0/EQUATORIALMETERHALFCIRCUMFERENCE;
    qreal latitude = radToAng(atan(sinh(point.y()/EQUATORIALMETERHALFCIRCUMFERENCE*M_PI)));

    return Coord(longitude, latitude);
}
