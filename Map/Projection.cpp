#include "Map/Projection.h"
#include "Map/Coord.h"

#include <QtCore/QRect>

#include <math.h>

// from wikipedia
#define EQUATORIALRADIUS 6378137
#define POLARRADIUS      6356752
#define PI 3.14159265

Projection::Projection(void)
: ScaleLat(1000000), DeltaLat(0), ScaleLon(1000000), DeltaLon(0),
  Viewport(Coord(0,0),Coord(0,0))
{
}

Projection::~Projection(void)
{
}

double Projection::pixelPerM() const
{
	double LatAngPerM = 1.0/EQUATORIALRADIUS;
	return LatAngPerM*ScaleLat;
}

double Projection::latAnglePerM() const
{
	double LengthOfOneDegreeLat = EQUATORIALRADIUS*PI/180;
	return 1/LengthOfOneDegreeLat;
}

double Projection::lonAnglePerM(double Lat) const
{
	double LengthOfOneDegreeLat = EQUATORIALRADIUS*PI/180;
	double LengthOfOneDegreeLon = LengthOfOneDegreeLat*fabs(cos(Lat));
	return 1/LengthOfOneDegreeLon;
}


void Projection::viewportRecalc(const QRect& Screen)
{
	Viewport = CoordBox(inverse(Screen.bottomLeft()),inverse(Screen.topRight()));
}


void Projection::setViewport(const CoordBox& Map, const QRect& Screen)
{
	Coord Center(Map.center());
	double LengthOfOneDegreeLat = EQUATORIALRADIUS*PI/180;
	double LengthOfOneDegreeLon = LengthOfOneDegreeLat*fabs(cos(Center.lat()));
	double Aspect = LengthOfOneDegreeLon/LengthOfOneDegreeLat;
	ScaleLon = Screen.width()/Map.lonDiff()*.9;
	ScaleLat = ScaleLon/Aspect;
	if ( (ScaleLat*Map.latDiff()*.9) > Screen.height() )
	{
		ScaleLat = Screen.height()/Map.latDiff()*.9;
		ScaleLon = ScaleLat*Aspect;
	}
	double PLon = Center.lon()*ScaleLon;
	double PLat = Center.lat()*ScaleLat;
	DeltaLon = Screen.width()/2 - PLon;
	DeltaLat = Screen.height()-(Screen.height()/2 - PLat);
	viewportRecalc(Screen);
}

void Projection::panScreen(const QPoint& p, const QRect& Screen)
{
	DeltaLon += p.x();
	DeltaLat += p.y();
	viewportRecalc(Screen);
}

QPointF Projection::project(const Coord& Map) const
{
	return QPointF(Map.lon()*ScaleLon + DeltaLon, - Map.lat()*ScaleLat + DeltaLat);
}

Coord Projection::inverse(const QPointF& Screen) const
{
	return Coord(-(Screen.y()-DeltaLat)/ScaleLat, (Screen.x()-DeltaLon)/ScaleLon );
}

CoordBox Projection::viewport() const
{
	return Viewport;
}

void Projection::zoom(double d, const QRect& Screen)
{
	Coord C = Viewport.center();
	ScaleLon *= d;
	ScaleLat *= d;
	QPointF F(project(C));
	DeltaLon += Screen.width()/2-F.x();
	DeltaLat += Screen.height()/2-F.y();
	viewportRecalc(Screen);
}
