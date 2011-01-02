#include "Coord.h"
#include "Utils/LineF.h"

#include <stdio.h>

/*
double angle(Coord & vertex, Coord p1, Coord p2)
{
    p1=p1-vertex;
    p2=p2-vertex;
    return angle(p2)-angle(p1);
}
*/
double angle(Coord p1)
{
    if (p1.length() == 0)
        return 0;
    double adjacent = (double)p1.x() / p1.length();
    if (p1.y() > 0)
        return acos(adjacent);
    return -acos(adjacent);
}

void rotate(Coord & p1,double angle)
{
    Coord p1p(cos(angle)*p1.x()-sin(angle)*p1.y(), sin(angle)*p1.x()+cos(angle)*p1.y());
    p1=p1p;
}

CoordBox::CoordBox(const Coord &C1, const Coord &C2)
{
    setBottomLeft(QPointF(C1.x()<C2.x()?C1.x():C2.x(), C1.y()<C2.y()?C1.y():C2.y()));
    setTopRight(QPointF(C1.x()>C2.x()?C1.x():C2.x(), C1.y()>C2.y()?C1.y():C2.y()));
}

CoordBox CoordBox::zoomed(double f) const
{
    Coord C(center());
    double DLat = latDiff()/2*f;
    double DLon = lonDiff()/2*f;
    return CoordBox(Coord(C.x()-DLon, C.y()-DLat), Coord(C.x()+DLon, C.y()+DLat) );
}

bool CoordBox::toXML(QString elName, QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement(elName);
    Coord(topRight()).toXML("topright", stream);
    Coord(bottomLeft()).toXML("bottomleft", stream);
    stream.writeEndElement();

    return OK;
}

bool CoordBox::toXML(QString elName, QDomElement& xParent) const
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement(elName);
    xParent.appendChild(e);

    Coord(topRight()).toXML("topright", e);
    Coord(bottomLeft()).toXML("bottomleft", e);

    return OK;
}

CoordBox CoordBox::fromXML(QDomElement e)
{
    Coord tr = Coord::fromXML(e.firstChildElement("topright"));
    Coord bl = Coord::fromXML(e.firstChildElement("bottomleft"));

    return CoordBox(Coord(bl.x(), tr.y()), Coord(tr.x(), bl.y()));
}

CoordBox CoordBox::fromXML(QXmlStreamReader& stream)
{
    Coord tr, bl;

    stream.readNext();
    while(!stream.atEnd() && !stream.isEndElement()) {
        if (stream.name() == "topright")
            tr = Coord::fromXML(stream);
        else if (stream.name() == "bottomleft")
            bl = Coord::fromXML(stream);

        stream.readNext();
    }

    return CoordBox(Coord(bl.x(), tr.y()), Coord(tr.x(), bl.y()));
}

#define EQUATORIALRADIUSKM 6378.137
double Coord::distanceFrom(const Coord& other) const
{
    double dlon = other.x() - x();

    const double slat1 = sin(angToRad(y()));
    const double clat1 = cos(angToRad(y()));

    const double slat2 = sin(angToRad(other.y()));
    const double clat2 = cos(angToRad(other.y()));

    const double sdlon = sin(angToRad(dlon));
    const double cdlon = cos(angToRad(dlon));

    const double t1 = clat2 * sdlon;
    const double t2 = clat1 * slat2 - slat1 * clat2 * cdlon;
    const double t3 = slat1 * slat2 + clat1 * clat2 * cdlon;
    const double dist = atan2(sqrt(t1*t1 + t2*t2), t3);

    return dist * EQUATORIALRADIUSKM;
}

bool Coord::toXML(QString elName, QXmlStreamWriter& stream) const
{
    bool OK = true;

    stream.writeStartElement(elName);
    stream.writeAttribute("lon",COORD2STRING(x()));
    stream.writeAttribute("lat", COORD2STRING(y()));
    stream.writeEndElement();

    return OK;
}

bool Coord::toXML(QString elName, QDomElement& xParent) const
{
    bool OK = true;

    QDomElement e = xParent.ownerDocument().createElement(elName);
    xParent.appendChild(e);

    e.setAttribute("lon",COORD2STRING(x()));
    e.setAttribute("lat", COORD2STRING(y()));

    return OK;
}

Coord Coord::fromXML(QDomElement e)
{
    double lat = e.attribute("lat").toDouble();
    double lon = e.attribute("lon").toDouble();

    return Coord(lon, lat);
}

Coord Coord::fromXML(QXmlStreamReader& stream)
{
    double lat = stream.attributes().value("lat").toString().toDouble();
    double lon = stream.attributes().value("lon").toString().toDouble();
    stream.readNext();

    return Coord(lon, lat);
}

void CoordBox::resize(double d)
{
    double dlat = (topRight().y()-bottomLeft().y())*(d-1)/2;
    double dlon = (topRight().x()-bottomLeft().x())*(d-1)/2;
    setBottom(bottom()-dlat);
    setLeft(left()-dlon);
    setTop(top()+dlat);
    setRight(right()+dlon);
}

bool CoordBox::visibleLine(const CoordBox & viewport, Coord & last, Coord & here)
{
    if (viewport.contains(last) && viewport.contains(here))
        return true;

    Coord A, B;
    LineF(last, here).intersectionWith(viewport, &A, &B);
    if (A.isNull() && B.isNull())
        return false;

    if (!A.isNull() && !B.isNull()) {
        last = A;
        here = B;
        return true;
    }

    if (viewport.contains(here))
        last = A;
    else
        here = A;

    return true;
//	return viewport.intersects( CoordBox(last, here) );
}

uint qHash(const Coord &c)
{
    return (uint)(c.y() + 65537 * c.x());
}
