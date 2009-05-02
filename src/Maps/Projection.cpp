#include "Maps/Projection.h"
#include "Maps/TrackPoint.h"

#include <QRect>
#include <QRectF>

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137.0
#define POLARRADIUS      6356752.0
//#define PROJ_RATIO ((double(INT_MAX)/M_PI) / EQUATORIALRADIUS)

#ifndef _MOBILE
#include <geometry/geometries/cartesian2d.hpp>
#include <geometry/geometries/latlong.hpp>

#include <geometry/projections/parameters.hpp>
#include <geometry/projections/factory.hpp>

using namespace geometry;

projection::projection<geometry::point_ll_deg, geometry::point_2d> *theProj;
projection::factory<geometry::point_ll_deg, geometry::point_2d> fac;
projection::parameters par;

QPointF Projection::projProject(const Coord & Map)
{
	point_ll_deg in(longitude<>(intToAng(Map.lon())), latitude<>(intToAng(Map.lat())));
	point_2d out;

	theProj->forward(in, out);

	return QPointF(out.x(), out.y());
}

Coord Projection::projInverse(const QPointF & pProj)
{
	point_2d in(pProj.x(), pProj.y());
	point_ll_deg out;

	theProj->inverse(in, out);

	return Coord(angToInt(out.lat()), angToInt(out.lon()));
}

bool Projection::projIsLatLong()
{
	return false;
}
#endif

Projection::Projection(void)
  : ScaleLat(1000000), ScaleLon(1000000),
  DeltaLat(0), DeltaLon(0), Viewport(WORLD_COORDBOX),
  layermanager(0)
{
#ifndef _MOBILE
	QString theProjString;
	theProjectionType = M_PREFS->getProjectionType();

	theProjString = M_PREFS->getProjection(theProjectionType).projection;
	par = projection::init(std::string(QString("%1 +over").arg(theProjString).toLatin1().data()));
	theProj = fac.create_new(par);
	if (!theProj) {
		par = projection::init(std::string(QString("%1 +over").arg(M_PREFS->getProjection("mercator").projection).toLatin1().data()));
		theProj = fac.create_new(par);
		if (!theProj) {
			qDebug() << "Unable to set projection : " << theProjectionType;
			Q_ASSERT(false);
			exit(1);
		}
	}
#endif
}

// Common routines

void Projection::setLayerManager(LayerManager * lm)
{
	layermanager = lm;
}

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

#ifndef _MOBILE
bool Projection::setProjectionType(ProjectionType aProjectionType)
{
	delete theProj;
	QString theProjString;
	theProjectionType = aProjectionType;

	theProjString = M_PREFS->getProjection(theProjectionType).projection;
	par = projection::init(std::string(QString("%1 +over").arg(theProjString).toLatin1().data()));
	theProj = fac.create_new(par);
	if (!theProj) {
		par = projection::init(std::string(QString("%1 +over").arg(M_PREFS->getProjection("mercator").projection).toLatin1().data()));
		theProj = fac.create_new(par);
		if (!theProj) {
			qDebug() << "Unable to set projection : " << theProjectionType;
			Q_ASSERT(false);
			exit(1);
		}
		return false;
	}
	return true;
}
#endif

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
	if (aNode && aNode->projectionType() == theProjectionType && !aNode->projection().isNull())
		return QPointF(aNode->projection().x() * ScaleLon + DeltaLon,
					   -aNode->projection().y() * ScaleLat + DeltaLat).toPoint();

	QPointF p = projProject(aNode->position());

	aNode->setProjectionType(theProjectionType);
	aNode->setProjection(p);

	return QPointF(p.x() * ScaleLon + DeltaLon, -p.y() * ScaleLat + DeltaLat).toPoint();
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
	if (LAYERMANAGER_OK) {
		layermanager->setView(QPointF(intToAng(Viewport.center().lon()), intToAng(Viewport.center().lat())), false);
	}
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

// Routines without layermanager

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
		wv = TargetMap.lonDiff();
		hv = TargetMap.latDiff() * pAspect / Aspect;
	} else {
		wv = TargetMap.lonDiff() * Aspect / pAspect;
		hv = TargetMap.latDiff();
	}

	Viewport = CoordBox(
            Coord(int(Center.lat() - hv/2), int(Center.lon() - wv/2)),
            Coord(int(Center.lat() + hv/2), int(Center.lon() + wv/2))
		);

	bl = projProject(Viewport.bottomLeft());
	tr = projProject(Viewport.topRight());
	pViewport = QRectF(bl, QSizeF(tr.x() - bl.x(), tr.y() - bl.y()));

	ScaleLon = Screen.width() / pViewport.width();
	ScaleLat = Screen.height() / pViewport.height();

	double PLon = pCenter.x() * ScaleLon;
	double PLat = pCenter.y() * ScaleLat;
	DeltaLon = int(Screen.width() / 2 - PLon);
	DeltaLat = int(Screen.height() - (Screen.height() / 2 - PLat));

	double LengthOfOneDegreeLat = EQUATORIALRADIUS * M_PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(intToRad(Center.lat())));
	double degAspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	double so = Screen.width() / (double)Viewport.lonDiff();
	double sa = so / degAspect;

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	PixelPerM = LatAngPerM / M_PI * INT_MAX * sa;
	
	viewportRecalc(Screen);
	if (LAYERMANAGER_OK)
		layerManagerSetViewport(Viewport, Screen);
#else
	if (LAYERMANAGER_OK)
		layerManagerSetViewport(TargetMap, Screen);
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
	DeltaLon = int(Screen.width() / 2 - PLon);
	DeltaLat = int(Screen.height() - (Screen.height() / 2 - PLat));
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
		if (LAYERMANAGER_OK) {
			layerManagerSetViewport(Viewport, Screen);
		}
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
		if (LAYERMANAGER_OK) {
			layerManagerSetViewport(Viewport, Screen);
		}
	}
#endif
}

void Projection::resize(QSize oldS, QSize newS)
{
	Q_UNUSED(oldS)
#ifndef _MOBILE
	viewportRecalc(QRect(QPoint(0,0), newS));
	if (LAYERMANAGER_OK) {
		layerManagerSetViewport(Viewport, QRect(QPoint(0,0), newS));
	}
#else
	Q_UNUSED(newS)
#endif
}


// Routines with layermanager

void Projection::layerManagerViewportRecalc(const QRect & Screen)
{
//	layermanager->setSize(Screen.size());

	QPointF tr = screenToCoordinate(Screen.topRight());
	QPointF bl = screenToCoordinate(Screen.bottomLeft());

	Coord trc = Coord(angToInt(tr.y()), angToInt(tr.x()));
	Coord blc = Coord(angToInt(bl.y()), angToInt(bl.x()));

	Viewport = CoordBox(trc, blc);
	ScaleLat = Screen.height()/(double)Viewport.latDiff();
	ScaleLon = Screen.width()/(double)Viewport.lonDiff();

	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	PixelPerM = LatAngPerM / M_PI * INT_MAX * ScaleLat;
}

void Projection::layerManagerSetViewport(const CoordBox & TargetMap, const QRect& Screen)
{
//	layermanager->setSize(Screen.size());

	screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);
//	QPoint screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);

	QList < QPointF > ql;
	Coord cbl(TargetMap.bottomLeft());
	QPointF cblf = QPointF(intToAng(cbl.lon()), intToAng(cbl.lat()));
	ql.append(cblf);
	Coord ctr(TargetMap.topRight());
	QPointF ctrf = QPointF(intToAng(ctr.lon()), intToAng(ctr.lat()));
	ql.append(ctrf);
	layermanager->setView(ql);
}

QPointF Projection::screenToCoordinate(QPointF click) const
{
	// click coordinate to image coordinate
	QPoint displayToImage = click.toPoint() - screen_middle + layermanager->getMapmiddle_px();

	// image coordinate to world coordinate
	return layermanager->getLayer()->getMapAdapter()->
		   displayToCoordinate(displayToImage);
}

QPoint Projection::coordinateToScreen(QPointF click) const
{
	QPoint p =
		layermanager->getLayer()->getMapAdapter()->
		coordinateToDisplay(click);
	QPoint r =
		QPoint(-layermanager->getMapmiddle_px().x() + p.x() +
			   screen_middle.x(),
			   -layermanager->getMapmiddle_px().y() + p.y() +
			   screen_middle.y());
	return r;
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

