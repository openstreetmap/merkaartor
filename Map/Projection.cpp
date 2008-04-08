#include "Preferences/MerkaartorPreferences.h"
#include "Map/Projection.h"
#include "Map/Coord.h"

#include <QtCore/QRect>

#include "QMapControl/mapadapter.h"
#include "QMapControl/layermanager.h"

#include <math.h>

#define LAYERMANAGER_OK (layermanager && layermanager->getLayers().size() > 0)
#define BGPROJ_SELECTED (MerkaartorPreferences::instance()->getProjectionType() == Proj_Background)

// from wikipedia
#define EQUATORIALRADIUS 6378137
#define POLARRADIUS      6356752
#define PI 3.14159265

Projection::Projection(void)
: ScaleLat(1000000), DeltaLat(0),
  ScaleLon(1000000), DeltaLon(0), Viewport(Coord(0, 0), Coord(0, 0)),
  layermanager(0)
{
}

// Common routines

void Projection::setLayerManager(LayerManager * lm)
{
	layermanager = lm;
}

double Projection::pixelPerM() const
{
	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	return LatAngPerM * ScaleLat;
}

double Projection::latAnglePerM() const
{
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	return 1 / LengthOfOneDegreeLat;
}

double Projection::lonAnglePerM(double Lat) const
{
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	double LengthOfOneDegreeLon = LengthOfOneDegreeLat * fabs(cos(Lat));
	return 1 / LengthOfOneDegreeLon;
}

QPointF Projection::project(const Coord & Map) const
{
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		const QPointF c(radToAng(Map.lon()), radToAng(Map.lat()));
		return coordinateToScreen(c);
	}
	return QPointF(Map.lon() * ScaleLon + DeltaLon,
				   -Map.lat() * ScaleLat + DeltaLat);

}

Coord Projection::inverse(const QPointF & Screen) const
{
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		QPointF c(screenToCoordinate(Screen));
		return Coord(angToRad(c.y()),angToRad(c.x()));
	}
	return Coord(-(Screen.y() - DeltaLat) / ScaleLat,
				 (Screen.x() - DeltaLon) / ScaleLon);

}

void Projection::panScreen(const QPoint & p, const QRect & Screen)
{
	if (LAYERMANAGER_OK && BGPROJ_SELECTED) {
		layermanager->scrollView(-p);
		layerManagerViewportRecalc(Screen);
		return;
	}

	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
	if (LAYERMANAGER_OK) {
		layerManagerSetViewport(Viewport, Screen);
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
	if (LAYERMANAGER_OK)
		layerManagerSetViewport(TargetMap, Screen);
	if (LAYERMANAGER_OK && BGPROJ_SELECTED) {
		layerManagerViewportRecalc(Screen);
		return;
	}

	Viewport = TargetMap;
	Coord Center(Viewport.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(Center.lat()));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / Viewport.lonDiff() * .9;
	ScaleLat = ScaleLon / Aspect;
	if ((ScaleLat * Viewport.latDiff()) > Screen.height())
	{
		ScaleLat = Screen.height() / Viewport.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}
	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
	viewportRecalc(Screen);
}

void Projection::zoom(double d, const QPointF & Around,
							 const QRect & Screen)
{
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
		Coord Before = inverse(Around);
		ScaleLon *= d;
		ScaleLat *= d;
		Coord After = inverse(Around);
		DeltaLat = Around.y() + Before.lat() * ScaleLat;
		DeltaLon = Around.x() - Before.lon() * ScaleLon;
		viewportRecalc(Screen);
		if (LAYERMANAGER_OK) {
			layerManagerSetViewport(Viewport, Screen);
		}

	}
}


// Routines with layermanager

void Projection::layerManagerViewportRecalc(const QRect & Screen)
{
	QPointF tr = screenToCoordinate(Screen.topRight());
	QPointF bl = screenToCoordinate(Screen.bottomLeft());

	Coord trc = Coord(angToRad(tr.y()), angToRad(tr.x()));
	Coord blc = Coord(angToRad(bl.y()), angToRad(bl.x()));

	Viewport = CoordBox(trc, blc);
	ScaleLat = Screen.height()/Viewport.latDiff();
	ScaleLon = Screen.width()/Viewport.lonDiff();
}

void Projection::layerManagerSetViewport(const CoordBox & TargetMap, const QRect& Screen)
{
	screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);
	QPoint screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);

	QList < QPointF > ql;
	Coord cbl(TargetMap.bottomLeft());
	QPointF cblf = QPointF(radToAng(cbl.lon()), radToAng(cbl.lat()));
	ql.append(cblf);
	Coord ctr(TargetMap.topRight());
	QPointF ctrf = QPointF(radToAng(ctr.lon()), radToAng(ctr.lat()));
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
	QPointF curCenterScreen = project(curCenter);
	QPointF newCenterScreen = project(Center);

	QPoint panDelta = (curCenterScreen - newCenterScreen).toPoint();
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

