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

bool CoordBox::toXML(QString elName, QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(elName);
	xParent.appendChild(e);

	TopRight.toXML("topright", e);
	BottomLeft.toXML("bottomleft", e);

	return OK;
}

CoordBox CoordBox::fromXML(QDomElement e)
{
	Coord tr = Coord::fromXML(e.firstChildElement("topright"));
	Coord bl = Coord::fromXML(e.firstChildElement("bottomleft"));

	return CoordBox(tr, bl);
}

bool Coord::toXML(QString elName, QDomElement& xParent) const
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(elName);
	xParent.appendChild(e);

	e.setAttribute("lon",QString::number(radToAng(Lon),'f',8));
	e.setAttribute("lat", QString::number(radToAng(Lat),'f',8));

	return OK;
}

Coord Coord::fromXML(QDomElement e)
{
	double lat = angToRad(e.attribute("lat").toDouble());
	double lon = angToRad(e.attribute("lon").toDouble());

	return Coord(lat, lon);
}

