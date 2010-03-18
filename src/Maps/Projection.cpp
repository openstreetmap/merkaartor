#include "Maps/Projection.h"
#include "Node.h"

#include <QRect>
#include <QRectF>

#include <math.h>

#include <ggl/extensions/gis/projections/parameters.hpp>
#include <ggl/extensions/gis/projections/factory.hpp>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
//#define PROJ_RATIO ((double(INT_MAX)/M_PI) / EQUATORIALRADIUS)

using namespace ggl;

// ProjectionPrivate

class ProjectionPrivate
{
public:
    ProjProjection *theWGS84Proj;
    ProjectionType projType;
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
        double x = intToAng(c.lon()) / 180. * 20037508.34;
        double y = log(tan(intToRad(c.lat())) + 1/cos(intToRad(c.lat()))) / M_PI * (20037508.34);

        return QPointF(x, y);
    }

    Coord mercatorInverse(const QPointF& point) const
    {
        int longitude = angToInt(point.x()*180.0/20037508.34);
        int latitude = radToInt(atan(sinh(point.y()/20037508.34*M_PI)));

        return Coord(latitude, longitude);
    }
};

//Projection

Projection::Projection(void)
: theProj(0), p(new ProjectionPrivate)
{
#ifndef _MOBILE
    p->theWGS84Proj = Projection::getProjection("+proj=longlat +ellps=WGS84 +datum=WGS84");
    p->projType = "";
    setProjectionType(M_PREFS->getProjectionType());
#endif
}

Projection::~Projection(void)
{
    delete p;
}

QPointF Projection::project(const Coord & Map) const
{
#ifndef _MOBILE
    if (p->IsMercator)
        return p->mercatorProject(Map);
    else
    if (p->IsLatLong)
        return QPointF(intToAng(Map.lon()), intToAng(Map.lat()));
    else
        return projProject(Map);
#else
    int numberOfTiles, tilesize;
    numberOfTiles = tilesize = 1;
    QPointF coordinate(intToAng(Map.lon()), intToAng(Map.lat()));
    double x = (coordinate.x()+180.) * (numberOfTiles*tilesize) /360.;                // coord to pixel!
    double y = (1.-(log(tan(M_PI/4+angToRad(coordinate.y())/2)) /M_PI)) * (numberOfTiles*tilesize) /2;

    return QPointF(x, y);
#endif
}

QPointF Projection::project(Node* aNode) const
{
#ifndef _MOBILE
    if (aNode && aNode->projectionRevision() == p->ProjectionRevision)
        return aNode->projection();

    QPointF pt;
    if (p->IsMercator)
        pt = p->mercatorProject(aNode->position());
    else
    if (p->IsLatLong)
        pt = QPointF(intToAng(aNode->position().lon()), intToAng(aNode->position().lat()));
    else
        pt = projProject(aNode->position());

    aNode->setProjectionRevision(p->ProjectionRevision);
    aNode->setProjection(pt);

    return pt;
#else
    if (aNode && aNode->projectionRevision() == p->ProjectionRevision)
        return aNode->projection();

    QPointF pt = project(aNode->position());
    aNode->setProjectionRevision(p->ProjectionRevision);
    aNode->setProjection(pt);

    return pt;
#endif
}

Coord Projection::inverse(const QPointF & Screen) const
{
#ifndef _MOBILE
    if (p->IsLatLong)
        return Coord(angToInt(Screen.y()), angToInt(Screen.x()));
    else
    if (p->IsMercator)
        return p->mercatorInverse(QPointF(Screen.x(), Screen.y()));
    else
        return projInverse(QPointF(Screen.x(), Screen.y()));
#else
    int numberOfTiles, tilesize;
    numberOfTiles = tilesize = 1;
    double longitude = (Screen.x() * (360.0/(numberOfTiles*(double)tilesize)) ) -180.0;
    double latitude = radToAng(atan(sinh((1.0-Screen.y() * (2.0/(numberOfTiles*(double)tilesize)) ) *M_PI)));

    return Coord(angToInt(latitude), angToInt(longitude));
#endif
}

#ifndef _MOBILE

#include "ggl/extensions/gis/projections/impl/pj_transform.hpp"
void Projection::projTransform(ProjProjection *srcdefn,
                           ProjProjection *dstdefn,
                           long point_count, int point_offset, double *x, double *y, double *z )
{
    ggl::projection::detail::pj_transform(srcdefn, dstdefn, point_count, point_offset, x, y, z);
}

void Projection::projTransformFromWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const
{
    ggl::projection::detail::pj_transform(p->theWGS84Proj, theProj, point_count, point_offset, x, y, z);
}

void Projection::projTransformToWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const
{
    ggl::projection::detail::pj_transform(theProj, p->theWGS84Proj, point_count, point_offset, x, y, z);
}

QPointF Projection::projProject(const Coord & Map) const
{
    try {
        point_ll_deg in(longitude<>(intToAng(Map.lon())), latitude<>(intToAng(Map.lat())));
        point_2d out;

        theProj->forward(in, out);

        return QPointF(out.x(), out.y());
    } catch (...) {
        return QPointF(0., 0.);
    }
}

Coord Projection::projInverse(const QPointF & pProj) const
{
    try {
        point_2d in(pProj.x(), pProj.y());
        point_ll_deg out;

        theProj->inverse(in, out);

        return Coord(angToInt(out.lat()), angToInt(out.lon()));
    } catch (...) {
        return Coord(0, 0);
    }
}

bool Projection::projIsLatLong() const
{
    return p->IsLatLong;
}

//bool Projection::projIsMercator()
//{
//    return p->IsMercator;
//}

QRectF Projection::getProjectedViewport(const CoordBox& Viewport, const QRect& screen) const
{
    QPointF bl, tr;

    double x, y;
    if (p->IsLatLong)
        tr = QPointF(intToAng(Viewport.topRight().lon()), intToAng(Viewport.topRight().lat()));
    else if (p->IsMercator)
        tr = project(Viewport.topRight());
    else {
        x = intToRad(Viewport.topRight().lon());
        y = intToRad(Viewport.topRight().lat());
        projTransformFromWGS84(1, 0, &x, &y, NULL);
        tr = QPointF(x, y);
    }

    if (p->IsLatLong)
        bl = QPointF(intToAng(Viewport.bottomLeft().lon()), intToAng(Viewport.bottomLeft().lat()));
    else if (p->IsMercator)
        bl = project(Viewport.bottomLeft());
    else {
        x = intToRad(Viewport.bottomLeft().lon());
        y = intToRad(Viewport.bottomLeft().lat());
        projTransformFromWGS84(1, 0, &x, &y, NULL);
        bl = QPointF(x, y);
    }

    QRectF pViewport = QRectF(bl.x(), tr.y(), tr.x() - bl.x(), bl.y() - tr.y());

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

#endif

#ifndef _MOBILE

ProjProjection * Projection::getProjection(QString projString)
{
//    qDebug() << "setProjection: " << projString;

    ggl::projection::factory<ggl::point_ll_deg, ggl::point_2d> fac;
    ggl::projection::parameters par;
    ggl::projection::projection<ggl::point_ll_deg, ggl::point_2d> *theProj;

    try {
        par = ggl::projection::init(std::string(QString("%1 +over").arg(projString).toLatin1().data()));
        theProj = fac.create_new(par);
    } catch (...) {
        par = ggl::projection::init(std::string(QString("%1 +over").arg(M_PREFS->getProjection("mercator").projection).toLatin1().data()));
        theProj = fac.create_new(par);
    }
    return theProj;
}

bool Projection::setProjectionType(ProjectionType aProjectionType)
{
    if (aProjectionType == p->projType)
        return true;

    SAFE_DELETE(theProj)
    p->ProjectionRevision++;
    p->projType = aProjectionType;
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
        return true;
    }
    // Hardcode "lat/long " projection
    if (
            p->projType.toUpper().contains("EPSG:4326")
            )
    {
        p->IsLatLong = true;
        return true;
    }

    try {
        theProj = getProjection(M_PREFS->getProjection(aProjectionType).projection);
        if (!theProj)
            p->IsMercator = true;
    } catch (...) {
        return false;
    }
    return (theProj != NULL || p->IsLatLong || p->IsMercator);
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

int Projection::projectionRevision() const
{
    return p->ProjectionRevision;
}

