#include "Map/Projection.h"
#include "Map/Coord.h"

#include <QtCore/QRect>

#include "QMapControl/mapadapter.h"
#include "QMapControl/layermanager.h"

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137
#define POLARRADIUS      6356752
#define PI 3.14159265

Projection::Projection(void): ScaleLat(1000000),
		DeltaLat(0),
		ScaleLon(1000000), DeltaLon(0), Viewport(Coord(0, 0), Coord(0, 0))
{
}

DrawingProjection::DrawingProjection(void)
{
}

DrawingProjection::~DrawingProjection(void)
{
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


void DrawingProjection::viewportRecalc(const QRect & Screen)
{
	Viewport =
		CoordBox(inverse(Screen.bottomLeft()),
				 inverse(Screen.topRight()));
}


void DrawingProjection::setViewport(const CoordBox & Map,
									const QRect & Screen)
{
	Coord Center(Map.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(Center.lat()));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / Map.lonDiff() * .9;
	ScaleLat = ScaleLon / Aspect;
	if ((ScaleLat * Map.latDiff()) > Screen.height()) {
		ScaleLat = Screen.height() / Map.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}
	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
	viewportRecalc(Screen);
}

void DrawingProjection::panScreen(const QPoint & p, const QRect & Screen)
{
	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
}

QPointF DrawingProjection::project(const Coord & Map) const
{
	return QPointF(Map.lon() * ScaleLon + DeltaLon,
				   -Map.lat() * ScaleLat + DeltaLat);
}

Coord DrawingProjection::inverse(const QPointF & Screen) const
{
	return Coord(-(Screen.y() - DeltaLat) / ScaleLat,
				 (Screen.x() - DeltaLon) / ScaleLon);
}

CoordBox Projection::viewport() const
{
	return Viewport;
}

void DrawingProjection::zoom(double d, const QPointF & Around,
							 const QRect & Screen)
{
	Coord Before = inverse(Around);
	ScaleLon *= d;
	ScaleLat *= d;
	Coord After = inverse(Around);
	DeltaLat = Around.y() + Before.lat() * ScaleLat;
	DeltaLon = Around.x() - Before.lon() * ScaleLon;
//      DeltaLon += Screen.width()/2-F.x();
//      DeltaLat += Screen.height()/2-F.y();
	viewportRecalc(Screen);
}


////////////////////////////////////
/////// ImageProjection
////////////////////////////////////

ImageProjection::ImageProjection()
{
}

ImageProjection::~ImageProjection(void)
{
}

void ImageProjection::setLayerManager(LayerManager * lm)
{
	layermanager = lm;
}

void ImageProjection::setViewport(const CoordBox& Map, const QRect& Screen)
{
	screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);

	QList < QPointF > ql;
	Coord cbl(Map.bottomLeft());
	QPointF cblf = QPointF(radToAng(cbl.lon()), radToAng(cbl.lat()));
	ql.append(cblf);
	Coord ctr(Map.topRight());
	QPointF ctrf = QPointF(radToAng(ctr.lon()), radToAng(ctr.lat()));
	ql.append(ctrf);
	layermanager->setZoom(0);
	layermanager->setView(ql);

	viewportRecalc(Screen);

	Coord Center(Viewport.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(Center.lat()));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / Viewport.lonDiff() * .9;
	ScaleLat = ScaleLon / Aspect;
	if ((ScaleLat * Viewport.latDiff()) > Screen.height()) {
		ScaleLat = Screen.height() / Viewport.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}
	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
//    viewRect = new QRect(Screen);
}

void ImageProjection::panScreen(const QPoint& p, const QRect& Screen)
{
	screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);
	layermanager->scrollView(-p);
	viewportRecalc(Screen);
}

QPointF ImageProjection::project(const Coord& Map) const
{
	const QPointF c = QPointF(radToAng(Map.lon()), radToAng(Map.lat()));

	return coordinateToScreen(c);
}

Coord ImageProjection::inverse(const QPointF& Screen) const
{
	return Coord(-(Screen.y() - DeltaLat) / ScaleLat,
				 (Screen.x() - DeltaLon) / ScaleLon);
}

void ImageProjection::viewportRecalc(const QRect& Screen)
{
	QPointF tr = screenToCoordinate(Screen.topRight());
	QPointF bl = screenToCoordinate(Screen.bottomLeft());

	Coord trc = Coord(angToRad(tr.y()), angToRad(tr.x()));
	Coord blc = Coord(angToRad(bl.y()), angToRad(bl.x()));

	Viewport = CoordBox(trc, blc);
}

void ImageProjection::zoom(double d, const QPointF& Around,
						   const QRect& Screen)
{
	screen_middle = QPoint(Screen.width() / 2, Screen.height() / 2);

	QPoint c = QPoint((int) Around.x(), (int) Around.y());
	QPointF v = screenToCoordinate(c);
	layermanager->setView(v);

	if (d < 1)
		layermanager->zoomOut();
	else
		if (d > 1)
			layermanager->zoomIn();

	viewportRecalc(Screen);

	Coord Center(Viewport.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS * PI / 180;
	double LengthOfOneDegreeLon =
		LengthOfOneDegreeLat * fabs(cos(Center.lat()));
	double Aspect = LengthOfOneDegreeLon / LengthOfOneDegreeLat;
	ScaleLon = Screen.width() / Viewport.lonDiff() * .9;
	ScaleLat = ScaleLon / Aspect;
	if ((ScaleLat * Viewport.latDiff()) > Screen.height()) {
		ScaleLat = Screen.height() / Viewport.latDiff();
		ScaleLon = ScaleLat * Aspect;
	}
	double PLon = Center.lon() * ScaleLon;
	double PLat = Center.lat() * ScaleLat;
	DeltaLon = Screen.width() / 2 - PLon;
	DeltaLat = Screen.height() - (Screen.height() / 2 - PLat);
}

QPointF ImageProjection::screenToCoordinate(QPoint click)
{
	// click coordinate to image coordinate
	QPoint displayToImage =
		QPoint(click.x() - screen_middle.x() +
			   layermanager->getMapmiddle_px().x(),
			   click.y() - screen_middle.y() +
			   layermanager->getMapmiddle_px().y());
	// image coordinate to world coordinate
	return layermanager->getLayer()->getMapAdapter()->
		   displayToCoordinate(displayToImage);
}

QPoint ImageProjection::coordinateToScreen(QPointF click) const
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
