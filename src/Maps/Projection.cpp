#include "Maps/Projection.h"
#include "Maps/TrackPoint.h"

#include <QRect>
#include <QRectF>

#include <math.h>

#include <geometry/projections/parameters.hpp>
#include <geometry/projections/factory.hpp>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
//#define PROJ_RATIO ((double(INT_MAX)/M_PI) / EQUATORIALRADIUS)

using namespace geometry;

// ProjectionPrivate

class ProjectionPrivate
{
public:
	ProjProjection *theWGS84Proj;
	QRectF ProjectedViewport;
	int ProjectionRevision;

public:
	ProjectionPrivate()
		: ProjectionRevision(0)
	{
	}
};

//Projection

Projection::Projection(void)
: theProj(0), p(new ProjectionPrivate)
{
#ifndef _MOBILE
	p->theWGS84Proj = Projection::getProjection("+proj=longlat +ellps=WGS84 +datum=WGS84");
	setProjectionType(M_PREFS->getProjectionType());
#endif
}

Projection::~Projection(void)
{
	delete p;
}


#ifndef _MOBILE

#include "geometry/projections/impl/pj_transform.hpp"
void Projection::projTransform(ProjProjection *srcdefn, 
						   ProjProjection *dstdefn, 
						   long point_count, int point_offset, double *x, double *y, double *z )
{
	projection::impl::pj_transform(srcdefn, dstdefn, point_count, point_offset, x, y, z);
}

void Projection::projTransformFromWGS84(long point_count, int point_offset, double *x, double *y, double *z )
{
	projection::impl::pj_transform(p->theWGS84Proj, theProj, point_count, point_offset, x, y, z);
}

void Projection::projTransformToWGS84(long point_count, int point_offset, double *x, double *y, double *z )
{
	projection::impl::pj_transform(theProj, p->theWGS84Proj, point_count, point_offset, x, y, z);
}

QPointF Projection::projProject(const Coord & Map) const
{
	point_ll_deg in(longitude<>(intToAng(Map.lon())), latitude<>(intToAng(Map.lat())));
	point_2d out;

	theProj->forward(in, out);

	return QPointF(out.x(), out.y());
}

Coord Projection::projInverse(const QPointF & pProj) const
{
	point_2d in(pProj.x(), pProj.y());
	point_ll_deg out;

	theProj->inverse(in, out);

	return Coord(angToInt(out.lat()), angToInt(out.lon()));
}

bool Projection::projIsLatLong()
{
	return (theProj->params().is_latlong > 0);
}

QRectF Projection::getProjectedViewport(CoordBox& Viewport, QRect& screen)
{
	QPointF br, tl;

	double x = intToRad(Viewport.topLeft().lon());
	double y = intToRad(Viewport.topLeft().lat());
	projTransformFromWGS84(1, 0, &x, &y, NULL);
	if (theProj->params().is_latlong)
		tl = QPointF(radToAng(x), radToAng(y));
	else
		tl = QPointF(x, y);

	x = intToRad(Viewport.bottomRight().lon());
	y = intToRad(Viewport.bottomRight().lat());
	projTransformFromWGS84(1, 0, &x, &y, NULL);
	if (theProj->params().is_latlong)
		br = QPointF(radToAng(x), radToAng(y));
	else
		br = QPointF(x, y);

	QRectF pViewport = QRectF(tl, br);

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
	projection::factory<geometry::point_ll_deg, geometry::point_2d> fac;
	projection::parameters par;

	par = projection::init(std::string(QString("%1 +over").arg(projString).toLatin1().data()));
	projection::projection<geometry::point_ll_deg, geometry::point_2d> *theProj = fac.create_new(par);
	if (!theProj) {
		par = projection::init(std::string(QString("%1 +over").arg(M_PREFS->getProjection("mercator").projection).toLatin1().data()));
		theProj = fac.create_new(par);
		if (!theProj) {
			qDebug() << "Unable to set projection : " << projString;
			return NULL;
		}
	}
	return theProj;
}

bool Projection::setProjectionType(ProjectionType aProjectionType)
{
	delete theProj;
	p->ProjectionRevision++;
	theProj = getProjection(M_PREFS->getProjection(aProjectionType).projection);
	return (theProj != NULL);
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

QPointF Projection::project(const Coord & Map) const
{
#ifndef _MOBILE
	return projProject(Map);
#else
	return QPoint(qRound(Map.lon()), qRound(Map.lat()));
#endif
}

QPointF Projection::project(TrackPoint* aNode) const
{
#ifndef _MOBILE
	if (aNode && aNode->projectionRevision() == p->ProjectionRevision)
		return aNode->projection();

	QPointF pt = projProject(aNode->position());

	aNode->setProjectionRevision(p->ProjectionRevision);
	aNode->setProjection(pt);

	return pt;
#else
	return project(aNode->position());
#endif
}

Coord Projection::inverse(const QPointF & Screen) const
{
#ifndef _MOBILE
	return projInverse(QPointF(Screen.x(), Screen.y()));
#else
	return Coord(qRound(Screen.y()),
				 qRound(Screen.x()));
#endif
}

