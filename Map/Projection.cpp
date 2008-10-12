#include "Map/Projection.h"
#include "Map/Coord.h"

#include <QtCore/QRect>

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
}

// Common routines

void Projection::setLayerManager(LayerManager * lm)
{
	layermanager = lm;
}

double Projection::pixelPerM() const
{
	double LatAngPerM = 1.0 / EQUATORIALRADIUS;
	return LatAngPerM / M_PI * INT_MAX * ScaleLat;
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

QPoint Projection::project(const Coord & Map) const
{
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		const QPointF c(intToAng(Map.lon()), intToAng(Map.lat()));
		return coordinateToScreen(c);
	}
	return QPoint(int(Map.lon() * ScaleLon + DeltaLon),
				   int(-Map.lat() * ScaleLat + DeltaLat));

}

Coord Projection::inverse(const QPointF & Screen) const
{
	if (LAYERMANAGER_OK && BGPROJ_SELECTED)
	{
		QPointF c(screenToCoordinate(Screen));
		return Coord(angToInt(c.y()),angToInt(c.x()));
	}
	return Coord(int(-(Screen.y() - DeltaLat) / ScaleLat),
				 int((Screen.x() - DeltaLon) / ScaleLon));

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
	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = int(Screen.width() / 2 - PLon);
	DeltaLat = int(Screen.height() - (Screen.height() / 2 - PLat));
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
		if (ScaleLat * d < 1.0 && ScaleLon * d < 1.0) {
			Coord Before = inverse(Around);
			ScaleLon *= d;
			ScaleLat *= d;
			Coord After = inverse(Around);
			DeltaLat = int(Around.y() + Before.lat() * ScaleLat);
			DeltaLon = int(Around.x() - Before.lon() * ScaleLon);
			viewportRecalc(Screen);
			if (LAYERMANAGER_OK) {
				layerManagerSetViewport(Viewport, Screen);
			}
		}

	}
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

