#include "Maps/Projection.h"

#include <QRect>
#include <QRectF>

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
#define EQUATORIALMETERCIRCUMFERENCE  40075016.68
#define EQUATORIALMETERHALFCIRCUMFERENCE  20037508.34
#define EQUATORIALMETERPERDEGREE    222638.981555556

#ifndef _MOBILE

#include "Node.h"

#ifndef USE_PROJ
#include <ggl/extensions/gis/projections/parameters.hpp>
#include <ggl/extensions/gis/projections/factory.hpp>
#include "ggl/extensions/gis/projections/impl/pj_transform.hpp"

using namespace ggl;
#endif
#endif


// ProjectionPrivate

class ProjectionPrivate
{
public:
#ifndef _MOBILE
    ProjProjection theWGS84Proj;
#endif
    QString projType;
    QString projProj4;
    QRectF ProjectedViewport;
    int ProjectionRevision;
    bool IsMercator;
    bool IsLatLong;

public:
    ProjectionPrivate()
        : ProjectionRevision(0)
        , IsMercator(false)
        , IsLatLong(false)
    {
    }

    QPointF mercatorProject(const Coord& c) const
    {
        return mercatorProject(QPointF(coordToAng(c.lon()), coordToAng(c.lat())));
    }

    QPointF mercatorProject(const QPointF& c) const
    {
        double x = coordToAng(c.x()) / 180. * EQUATORIALMETERHALFCIRCUMFERENCE;
        double y = log(tan(coordToRad(c.y())) + 1/cos(coordToRad(c.y()))) / M_PI * (EQUATORIALMETERHALFCIRCUMFERENCE);

        return QPointF(x, y);
    }

    Coord mercatorInverse(const QPointF& point) const
    {
        double longitude = angToCoord(point.x()*180.0/EQUATORIALMETERHALFCIRCUMFERENCE);
        double latitude = radToCoord(atan(sinh(point.y()/EQUATORIALMETERHALFCIRCUMFERENCE*M_PI)));

        return Coord(latitude, longitude);
    }

    inline QPointF latlonProject(const Coord& c) const
    {
        return latlonProject(QPointF(coordToAng(c.lon()), coordToAng(c.lat())));
    }

    inline QPointF latlonProject(const QPointF& c) const
    {
        return QPointF(coordToAng(c.x())*EQUATORIALMETERPERDEGREE, coordToAng(c.y())*EQUATORIALMETERPERDEGREE);
    }

    inline Coord latlonInverse(const QPointF& point) const
    {
        return Coord(angToCoord(point.y()/EQUATORIALMETERPERDEGREE), angToCoord(point.x()/EQUATORIALMETERPERDEGREE));
    }
};

//Projection

Projection::Projection(void)
: p(new ProjectionPrivate)
{
#ifdef USE_PROJ
#ifdef Q_OS_WIN
    QString pdir(qApp->applicationDirPath() + "/" STRINGIFY(SHARE_DIR) "/proj");
    const char* proj_dir = QDir::toNativeSeparators(pdir).toUtf8().constData();
//    const char* proj_dir = "E:\\cbro\\src\\merkaartor-devel\\binaries\\bin\\share\\proj";
    pj_set_searchpath(1, &proj_dir);
#endif // Q_OS_WIN
#endif // USE_PROJ

#ifndef _MOBILE
    theProj = NULL;
    p->theWGS84Proj = Projection::getProjection("+proj=longlat +ellps=WGS84 +datum=WGS84");
    setProjectionType(M_PREFS->getProjectionType());
#endif
}

Projection::~Projection(void)
{
#ifndef _MOBILE
#ifdef USE_PROJ
    pj_free(theProj);
#else
    SAFE_DELETE(theProj)
#endif
#endif // _MOBILE
    delete p;
}

QPointF Projection::project(const Coord & Map) const
{
    if (p->IsMercator)
        return p->mercatorProject(Map);
    else
    if (p->IsLatLong)
        return p->latlonProject(Map);
#ifndef _MOBILE
    else
        return projProject(Map);
#endif
}

QPointF Projection::project(const QPointF & Map) const
{
    if (p->IsMercator)
        return p->mercatorProject(Map);
    else
    if (p->IsLatLong)
        return p->latlonProject(Map);
#ifndef _MOBILE
    else
        return projProject(Map);
#endif
}

#ifndef _MOBILE
QPointF Projection::project(Node* aNode) const
{
    if (aNode && aNode->projectionRevision() == p->ProjectionRevision)
        return aNode->projection();

    QPointF pt;
    if (p->IsMercator)
        pt = p->mercatorProject(aNode->position());
    else
    if (p->IsLatLong)
        pt = p->latlonProject(aNode->position());
    else
        pt = projProject(aNode->position());

    aNode->setProjectionRevision(p->ProjectionRevision);
    aNode->setProjection(pt);

    return pt;
}
#endif

QLineF Projection::project(const QLineF & Map) const
{
    if (p->IsMercator)
        return QLineF(p->mercatorProject(Map.p1()), p->mercatorProject(Map.p2()));
    else
    if (p->IsLatLong)
        return QLineF(p->latlonProject(Map.p1()), p->latlonProject(Map.p2()));
#ifndef _MOBILE
    else
        return QLineF(projProject(Map.p1()), projProject(Map.p2()));
#endif
}


Coord Projection::inverse(const QPointF & Screen) const
{
    if (p->IsLatLong)
        return p->latlonInverse(Screen);
    else
    if (p->IsMercator)
        return p->mercatorInverse(Screen);
#ifndef _MOBILE
    else
        return projInverse(Screen);
#endif
}

#ifndef _MOBILE

void Projection::projTransform(ProjProjection srcdefn,
                           ProjProjection dstdefn,
                           long point_count, int point_offset, double *x, double *y, double *z )
{
#ifdef USE_PROJ
    pj_transform(srcdefn, dstdefn, point_count, point_offset, x, y, z);
#else
    ggl::projection::detail::pj_transform(srcdefn, dstdefn, point_count, point_offset, x, y, z);
#endif
}

void Projection::projTransformFromWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const
{
#ifdef USE_PROJ
    pj_transform(p->theWGS84Proj, theProj, point_count, point_offset, x, y, z);
#else
    ggl::projection::detail::pj_transform(p->theWGS84Proj, theProj, point_count, point_offset, x, y, z);
#endif
}

void Projection::projTransformToWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const
{
#ifdef USE_PROJ
    pj_transform(theProj, p->theWGS84Proj, point_count, point_offset, x, y, z);
#else
    ggl::projection::detail::pj_transform(theProj, p->theWGS84Proj, point_count, point_offset, x, y, z);
#endif
}

QPointF Projection::projProject(const Coord & Map) const
{
    return projProject(QPointF(coordToAng(Map.lon()), coordToAng(Map.lat())));
}

QPointF Projection::projProject(const QPointF & Map) const
{
#ifdef USE_PROJ
//    projUV in;

//    in.u = angToRad(Map.x());
//    in.v = angToRad(Map.y());

//    projUV out = pj_fwd(in, theProj);

//    return QPointF(out.u, out.v);
    double x = angToRad(Map.x());
    double y = angToRad(Map.y());

    projTransformFromWGS84(1, 0, &x, &y, NULL);

    return QPointF(x, y);
#else
    try {
        point_ll_deg in(longitude<>(Map.x()), latitude<>(Map.y()));
        point_2d out;

        theProj->forward(in, out);

        return QPointF(out.x(), out.y());
    } catch (...) {
        return QPointF(0., 0.);
    }
#endif
}

Coord Projection::projInverse(const QPointF & pProj) const
{
#ifdef USE_PROJ
//    projUV in;
//    in.u = pProj.x();
//    in.v = pProj.y();

//    projUV out = pj_inv(in, theProj);

//    return Coord(radToCoord(out.v), radToCoord(out.u));

    double x = pProj.x();
    double y = pProj.y();

    projTransformToWGS84(1, 0, &x, &y, NULL);

    return Coord(radToCoord(y), radToCoord(x));
#else
    try {
        point_2d in(pProj.x(), pProj.y());
        point_ll_deg out;

        theProj->inverse(in, out);

        return Coord(angToCoord(out.lat()), angToCoord(out.lon()));
    } catch (...) {
        return Coord(0, 0);
    }
#endif
}

QRectF Projection::getProjectedViewport(const CoordBox& Viewport, const QRect& screen) const
{
    QPointF bl, tr;
    QRectF pViewport;
    double x, y;

    if (p->IsLatLong)
        pViewport = Viewport.toQRectF();
    else {
        if (p->IsMercator)
            tr = project(Viewport.topRight());
        else {
            x = coordToRad(Viewport.topRight().lon());
            y = coordToRad(Viewport.topRight().lat());
            projTransformFromWGS84(1, 0, &x, &y, NULL);
            tr = QPointF(x, y);
        }

        if (p->IsMercator)
            bl = project(Viewport.bottomLeft());
        else {
            x = coordToRad(Viewport.bottomLeft().lon());
            y = coordToRad(Viewport.bottomLeft().lat());
            projTransformFromWGS84(1, 0, &x, &y, NULL);
            bl = QPointF(x, y);
        }

        pViewport = QRectF(bl.x(), tr.y(), tr.x() - bl.x(), bl.y() - tr.y());
    }

    QPointF pCenter(pViewport.center());

    double wv, hv;
    //wv = (pViewport.width() / Viewport.londiff()) * ((double)screen.width() / Viewport.londiff());
    //hv = (pViewport.height() / Viewport.latdiff()) * ((double)screen.height() / Viewport.latdiff());

    double Aspect = (double)screen.width() / screen.height();
    double pAspect = fabs(pViewport.width() / pViewport.height());

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
#endif // _MOBILE

bool Projection::projIsLatLong() const
{
    return p->IsLatLong;
}

//bool Projection::projIsMercator()
//{
//    return p->IsMercator;
//}


#ifndef _MOBILE
ProjProjection Projection::getProjection(QString projString)
{
#ifdef USE_PROJ
    ProjProjection theProj = pj_init_plus(QString("%1 +over").arg(projString).toLatin1());
#else
    ggl::projection::factory<ggl::point_ll_deg, ggl::point_2d> fac;
    ggl::projection::parameters par;
    ProjProjection theProj;

    try {
        par = ggl::projection::init(std::string(QString("%1 +over").arg(projString).toLatin1().data()));
        theProj = fac.create_new(par);
    } catch (...) {
    }
#endif
    return theProj;
}
#endif // _MOBILE

bool Projection::setProjectionType(QString aProjectionType)
{
    if (aProjectionType == p->projType)
        return true;

#ifndef _MOBILE
#ifdef USE_PROJ
    if (theProj) {
        pj_free(theProj);
        theProj = NULL;
    }
#else
    SAFE_DELETE(theProj)
#endif
#endif // _MOBILE

    p->ProjectionRevision++;
    p->projType = aProjectionType;
    p->projProj4 = QString();
    p->IsLatLong = false;
    p->IsMercator = false;

    // Hardcode "Google " projection
    if (
            p->projType.toUpper().contains("OSGEO:41001") ||
            p->projType.toUpper().contains("EPSG:3785") ||
            p->projType.toUpper().contains("EPSG:900913") ||
            p->projType.toUpper().contains("EPSG:3857")
            )
    {
        p->IsMercator = true;
        p->projType = "EPSG:3857";
        return true;
    }
    // Hardcode "lat/long " projection
    if (
            p->projType.toUpper().contains("EPSG:4326")
            )
    {
        p->IsLatLong = true;
        p->projType = "EPSG:4326";
        return true;
    }

#ifndef _MOBILE
    try {
        p->projProj4 = M_PREFS->getProjection(aProjectionType).projection;
        theProj = getProjection(p->projProj4);
        if (!theProj) {
            p->IsMercator = true;
            return false;
        } else
#ifdef USE_PROJ
            if (pj_is_latlong(theProj))
#else
            if (theProj->params().is_latlong)
#endif
                p->IsLatLong = true;
    } catch (...) {
        return false;
    }
    return (theProj != NULL || p->IsLatLong || p->IsMercator);
#else
    return false;
#endif // _MOBILE
}

#ifndef _MOBILE
QString Projection::getProjectionType() const
{
    return p->projType;
}

QString Projection::getProjectionProj4() const
{
    return p->projProj4;
}

int Projection::projectionRevision() const
{
    return p->ProjectionRevision;
}
#endif

// Common routines

double Projection::latAnglePerM() const
{
    double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    return 1 / LengthOfOneDegreeLat;
}

double Projection::lonAnglePerM(double Lat) const
{
    double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
    double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(Lat));
    return 1 / LengthOfOneDegreeLon;
}

bool Projection::toXML(QDomElement xParent)
{
    bool OK = true;

    QDomElement e = xParent.namedItem("Projection").toElement();
    if (!e.isNull()) {
        xParent.removeChild(e);
    }
    e = xParent.ownerDocument().createElement("Projection");
    xParent.appendChild(e);

    e.setAttribute("type", p->projType);
    if (!p->IsLatLong && !p->IsMercator && !p->projProj4.isEmpty()) {
        e.appendChild(xParent.ownerDocument().createTextNode(p->projProj4));
    }


    return OK;
}

void Projection::fromXML(const QDomElement e)
{
    if (e.tagName() == "Projection") {
        if (e.hasChildNodes()) {
            setProjectionType(e.firstChild().toText().nodeValue());
            if (e.hasAttribute("type"))
                p->projType = e.attribute("type");
            else
                p->projType = QCoreApplication::translate("Projection", "Document");
        } else
            setProjectionType(e.attribute("type"));
    }
}
