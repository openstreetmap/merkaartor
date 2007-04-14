#include "Coord.h"

CoordBox::CoordBox(const Coord &C1, const Coord &C2)
: BottomLeft(C1.lat()<C2.lat()?C1.lat():C2.lat() , C1.lon()<C2.lon()?C1.lon():C2.lon()),
TopRight(C1.lat()>C2.lat()?C1.lat():C2.lat() , C1.lon()>C2.lon()?C1.lon():C2.lon())
{
}

CoordBox CoordBox::zoomed(double f) const
{
	Coord C(center());
	double DLat = latDiff()/2*f;
	double DLon = lonDiff()/2*f;
	return CoordBox(Coord(C.lat()-DLat,C.lon()-DLon), Coord(C.lat()+DLat,C.lon()+DLon) );
}


