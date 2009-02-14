#include "Map/Projection.h"
#include "Map/TrackPoint.h"

#include <QRect>
#include <QRectF>

#include "QMapControl/mapadapter.h"
#include "QMapControl/layermanager.h"

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137
#define POLARRADIUS      6356752

Projection::Projection(void)
  : ScaleLat(1000000), ScaleLon(1000000),
  DeltaLat(0), DeltaLon(0), Viewport(Coord(-1000, -1000), Coord(1000, 1000)),
  layermanager(0)
{
	theProjectionType = MerkaartorPreferences::instance()->getProjectionType();
#ifdef USE_PROJ
	theProj = pj_init_plus(
		QString("+proj=%1 +a=%2 +b=%3 %4").arg("merc").arg(double(INT_MAX)/M_PI).arg(double(INT_MAX)/M_PI)
		.arg("+lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +no_defs")
		.toLatin1()
	);
	theProjectionType = "Mercator";
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

void Projection::setProjectionType(ProjectionType aProjectionType)
{
	theProjectionType = aProjectionType;
}

#ifdef USE_PROJ
QPoint Projection::projProject(const Coord & Map) const
{
	projUV in;

	in.u = intToRad(Map.lon());
	in.v = intToRad(Map.lat());

	projUV out = pj_fwd(in, theProj);

    return QPoint((int)out.u, int(out.v));
}

Coord Projection::projInverse(const QPointF & pProj) const
{
	projUV in;
	in.u = pProj.x();
	in.v = pProj.y();

	projUV out = pj_inv(in, theProj);

	return Coord(radToInt(out.v), radToInt(out.u));
}

#endif


QPoint Projection::project(const Coord & Map) const
{
#ifdef USE_PROJ
	projUV in;

	in.u = intToRad(Map.lon());
	in.v = intToRad(Map.lat());

	//qDebug() << "prj: " << intToAng(Map.lon()) << " : " << intToAng(Map.lat());
	projUV out = pj_fwd(in, theProj);
	//qDebug() << "prj: "<< out.u << " : " << out.v;

	QPoint p(int(out.u * ScaleLon + DeltaLon),
				   int(-out.v * ScaleLat + DeltaLat));

	return p;
#else
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		const QPointF c(intToAng(Map.lon()), intToAng(Map.lat()));
		return coordinateToScreen(c);
	}
	return QPoint(int(Map.lon() * ScaleLon + DeltaLon),
				   int(-Map.lat() * ScaleLat + DeltaLat));
#endif
}

QPoint Projection::project(TrackPoint* aNode) const
{
#ifdef USE_PROJ
	if (aNode && aNode->projectionType() == theProjectionType && !aNode->projection().isNull())
		return QPoint(int(aNode->projection().x() * ScaleLon + DeltaLon),
					   int(-aNode->projection().y() * ScaleLat + DeltaLat));

	projUV in;

	in.u = intToRad(aNode->position().lon());
	in.v = intToRad(aNode->position().lat());

	//qDebug() << "prj: " << intToAng(Map.lon()) << " : " << intToAng(Map.lat());
	projUV out = pj_fwd(in, theProj);
	//qDebug() << "prj: "<< out.u << " : " << out.v;

	aNode->setProjectionType(theProjectionType);
	aNode->setProjection(QPoint(int(out.u), int(out.v)));

	QPoint p(int(out.u * ScaleLon + DeltaLon),
				   int(-out.v * ScaleLat + DeltaLat));
	return p;
#else
	return project(aNode->position());
#endif
}

Coord Projection::inverse(const QPointF & Screen) const
{
#ifdef USE_PROJ
	projUV in;

	in.u = (Screen.x() - DeltaLon ) / ScaleLon;
	in.v = -(Screen.y() - DeltaLat) / ScaleLat;

	//qDebug() << "inv: "<< in.u << " : " << in.v;
	projUV out = pj_inv(in, theProj);
	//qDebug() << "inv: "<< out.u * RAD_TO_DEG << " : " << out.v * RAD_TO_DEG;

	return Coord(radToInt(out.v), radToInt(out.u));
#else
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		QPointF c(screenToCoordinate(Screen));
		return Coord(angToInt(c.y()),angToInt(c.x()));
	}
	return Coord(int(-(Screen.y() - DeltaLat) / ScaleLat),
				 int((Screen.x() - DeltaLon) / ScaleLon));
#endif
}

void Projection::panScreen(const QPoint & p, const QRect & Screen)
{
#ifdef USE_PROJ
	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
	if (LAYERMANAGER_OK) {
		layermanager->setView(QPointF(intToAng(Viewport.center().lon()), intToAng(Viewport.center().lat())), false);
	}
#else
	if (LAYERMANAGER_OK && BGPROJ_SELECTED) {
		layermanager->scrollView(-p);
		layerManagerViewportRecalc(Screen);
		return;
	}

	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
	if (LAYERMANAGER_OK) {
		layermanager->setView(QPointF(intToAng(Viewport.center().lon()), intToAng(Viewport.center().lat())), false);
	}
#endif
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
#ifndef USE_PROJ
	if (LAYERMANAGER_OK)
		layerManagerSetViewport(TargetMap, Screen);
	if (LAYERMANAGER_OK && BGPROJ_SELECTED) {
		layerManagerViewportRecalc(Screen);
		return;
	}
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
#else
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
#endif
}

void Projection::zoom(double d, const QPointF & Around,
							 const QRect & Screen)
{
#ifndef USE_PROJ
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);

		QPoint c = Around.toPoint();
		QPointF v_before = screenToCoordinate(c);

		if (d < 1)
			layermanager->zoomOut();
		else
			if (d > 1)
				layermanager->zoomIn();
		layerManagerViewportRecalc(Screen);
		QPoint v_after = coordinateToScreen(v_before);
		panScreen(c-v_after, Screen);
	}
	else
	{
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
	}
#else
	if (ScaleLat * d < 1.0 && ScaleLon * d < 1.0) {
		Coord Before = inverse(Around);
		QPoint pBefore = projProject(Before);
		ScaleLon *= d;
		ScaleLat *= d;

		DeltaLat = int(Around.y() + pBefore.y() * ScaleLat);
		DeltaLon = int(Around.x() - pBefore.x() * ScaleLon);

		//double nx1 = Around.x() / ScaleLon;
		//double ny1 = Around.y() / ScaleLat;

		//ScaleLon *= d;
		//ScaleLat *= d;

		//double nx2 = Around.x() / ScaleLon;
		//double ny2 = Around.y() / ScaleLat;

		//DeltaLon -= (nx1 - nx2) * ScaleLon;
		//DeltaLat += (ny1 - ny2) * ScaleLat;

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
#endif
}

void Projection::resize(QSize oldS, QSize newS)
{
	Q_UNUSED(oldS)
#ifdef USE_PROJ
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

