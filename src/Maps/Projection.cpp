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
  : ScaleLat(1000000), ScaleLon(1000000),
  DeltaLat(0), DeltaLon(0), Viewport(WORLD_COORDBOX),
  theProj(0)
{
	p = new ProjectionPrivate;
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

void Projection::projTransformWGS84(long point_count, int point_offset, double *x, double *y, double *z )
{
	projection::impl::pj_transform(p->theWGS84Proj, theProj, point_count, point_offset, x, y, z);
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

QRectF Projection::getProjectedViewport(QRect& screen)
{
	QPointF br, tl;

	double x = intToRad(Viewport.topLeft().lon());
	double y = intToRad(Viewport.topLeft().lat());
	projTransformWGS84(1, 0, &x, &y, NULL);
	if (theProj->params().is_latlong)
		tl = QPointF(radToAng(x), radToAng(y));
	else
		tl = QPointF(x, y);

	x = intToRad(Viewport.bottomRight().lon());
	y = intToRad(Viewport.bottomRight().lat());
	projTransformWGS84(1, 0, &x, &y, NULL);
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

double Projection::pixelPerM() const
{
	return PixelPerM;
}

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

QPoint Projection::project(const Coord & Map) const
{
#ifndef _MOBILE
	QPointF p = projProject(Map);
	return QPointF(p.x() * ScaleLon + DeltaLon, -p.y() * ScaleLat + DeltaLat).toPoint();
#else
	return QPoint(int(Map.lon() * ScaleLon + DeltaLon),
				   int(-Map.lat() * ScaleLat + DeltaLat));
#endif
}

QPoint Projection::project(TrackPoint* aNode) const
{
#ifndef _MOBILE
	if (aNode && aNode->projectionRevision() == p->ProjectionRevision)
		return QPointF(aNode->projection().x() * ScaleLon + DeltaLon,
					   -aNode->projection().y() * ScaleLat + DeltaLat).toPoint();

	QPointF pt = projProject(aNode->position());

	aNode->setProjectionRevision(p->ProjectionRevision);
	aNode->setProjection(pt);

	return QPointF(pt.x() * ScaleLon + DeltaLon, -pt.y() * ScaleLat + DeltaLat).toPoint();
#else
	return project(aNode->position());
#endif
}

Coord Projection::inverse(const QPointF & Screen) const
{
#ifndef _MOBILE
	Coord c = projInverse((QPointF((Screen.x() - DeltaLon ) / ScaleLon, -(Screen.y() - DeltaLat) / ScaleLat)));
	return c;
#else
	return Coord(int(-(Screen.y() - DeltaLat) / ScaleLat),
				 int((Screen.x() - DeltaLon) / ScaleLon));
#endif
}

void Projection::panScreen(const QPoint & p, const QRect & Screen)
{
	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
}

CoordBox Projection::viewport() const
{
	return Viewport;
}

void Projection::viewportRecalc(const QRect & Screen)
{
	Viewport =
		CoordBox(inverse(Screen.bottomLeft()),
			 inverse(Screen.topRight()));
}

void Projection::setViewport(const CoordBox & TargetMap)
{
	Viewport = TargetMap;
}

void Projection::setViewport(const CoordBox & TargetMap,
									const QRect & Screen)
{
#ifndef _MOBILE
	QPointF bl = projProject(TargetMap.bottomLeft());
	QPointF tr = projProject(TargetMap.topRight());
	QRectF pViewport = QRectF(bl, QSizeF(tr.x() - bl.x(), tr.y() - bl.y()));

	Coord Center(TargetMap.center());
	QPointF pCenter(pViewport.center());

	double Aspect = (double)Screen.width() / Screen.height();
	double pAspect = pViewport.width() / pViewport.height();

	double wv, hv;
	if (pAspect > Aspect) {
		wv = pViewport.width();
		hv = pViewport.height() * pAspect / Aspect;
	} else {
		wv = pViewport.width() * Aspect / pAspect;
		hv = pViewport.height();
	}

	ScaleLon = Screen.width() / wv;
	ScaleLat = Screen.height() / hv;

	double PLon = pCenter.x() * ScaleLon;
	double PLat = pCenter.y() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);

	viewportRecalc(Screen);

	// Calculate PixelPerM
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(Center.lat())));
	double degAspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	double so = Screen.width() / (double)Viewport.lonDiff();
	double sa = so / degAspect;

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	PixelPerM = LatAngPerM / M_PI * INT_MAX * sa;
	//
#else
	Viewport = TargetMap;
	Coord Center(Viewport.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(Center.lat())));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / (double)Viewport.lonDiff();
	ScaleLat = ScaleLon / Aspect;
	
	if ((ScaleLat * Viewport.latDiff()) > Screen.height())
	{
		ScaleLat = Screen.height() / (double)Viewport.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	PixelPerM = LatAngPerM / M_PI * INT_MAX * ScaleLat;

	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
	viewportRecalc(Screen);
#endif
}

void Projection::zoom(double d, const QPointF & Around,
							 const QRect & Screen)
{
#ifndef _MOBILE
	if (PixelPerM < 100) {
		Coord Before = inverse(Around);
		QPointF pBefore = projProject(Before);
		ScaleLon *= d;
		ScaleLat *= d;

		DeltaLat = int(Around.y() + pBefore.y() * ScaleLat);
		DeltaLon = int(Around.x() - pBefore.x() * ScaleLon);

		double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
		double LengthOfOneDegreeLon =
			LengthOfOneDegreeLat * fabs(cos(intToRad(Before.lat())));
		double degAspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
		double so = Screen.width() / (double)Viewport.lonDiff();
		double sa = so / degAspect;

		double LatAngPerM = 1.0 / EQUATORIALRADIUS;
		PixelPerM = LatAngPerM / M_PI * INT_MAX * sa;

		viewportRecalc(Screen);
	}
#else
	if (ScaleLat * d < 1.0 && ScaleLon * d < 1.0) {
		Coord Before = inverse(Around);
		ScaleLon *= d;
		ScaleLat *= d;
		DeltaLat = int(Around.y() + Before.lat() * ScaleLat);
		DeltaLon = int(Around.x() - Before.lon() * ScaleLon);

		double LatAngPerM = 1.0 / EQUATORIALRADIUS;
		PixelPerM = LatAngPerM / M_PI * INT_MAX * ScaleLat;

		viewportRecalc(Screen);
	}
#endif
}

void Projection::resize(QSize oldS, QSize newS)
{
	Q_UNUSED(oldS)
#ifndef _MOBILE
	viewportRecalc(QRect(QPoint(0,0), newS));
#else
	Q_UNUSED(newS)
#endif
}

void Projection::setCenter(Coord & Center, const QRect & Screen)
{
	Coord curCenter(Viewport.center());
	QPoint curCenterScreen = project(curCenter);
	QPoint newCenterScreen = project(Center);

	QPoint panDelta = (curCenterScreen - newCenterScreen);
	panScreen(panDelta, Screen);
}

bool Projection::toXML(QDomElement xParent) const
{
	bool OK = true;

	QDomElement e = xParent.namedItem("Projection").toElement();
	if (!e.isNull()) {
		xParent.removeChild(e);
	}
	e = xParent.ownerDocument().createElement("Projection");
	xParent.appendChild(e);

	viewport().toXML("Viewport", e);

	return OK;
}

void Projection::fromXML(QDomElement e, const QRect & Screen)
{
	CoordBox cb = CoordBox::fromXML(e.firstChildElement("Viewport"));
	setViewport(cb, Screen);
}

