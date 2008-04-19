#include "TrackPoint.h"

#include "Map/Projection.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>

TrackPoint::TrackPoint(const Coord& aCoord)
: Position(aCoord), Time(QDateTime::currentDateTime())
{
}

TrackPoint::TrackPoint(const TrackPoint& other)
: MapFeature(other), Position(other.Position), Time(other.Time)
{
}

TrackPoint::~TrackPoint(void)
{
}

const QDateTime& TrackPoint::time() const
{
	return Time;
}

void TrackPoint::setTime(const QDateTime& time)
{
	Time = time;
	notifyChanges();
}

const Coord& TrackPoint::position() const
{
	return Position;
}

void TrackPoint::setPosition(const Coord& aCoord)
{
	Position = aCoord;
	notifyChanges();
}

bool TrackPoint::notEverythingDownloaded() const
{
	return lastUpdated() == MapFeature::NotYetDownloaded;
}

CoordBox TrackPoint::boundingBox() const
{
	return CoordBox(Position,Position);
}

void TrackPoint::draw(QPainter& /* thePainter */, const Projection& /*theProjection*/ )
{
}

void TrackPoint::drawFocus(QPainter& thePainter, const Projection& theProjection)
{
	thePainter.setPen(MerkaartorPreferences::instance()->getFocusColor());
	QPointF P(theProjection.project(Position));
	QRectF R(P-QPointF(3,3),QSize(6,6));
	thePainter.drawRect(R);
	R.adjust(-7, -7, 7, 7);
	thePainter.drawEllipse(R);
}

void TrackPoint::drawHover(QPainter& thePainter, const Projection& theProjection)
{
	thePainter.setPen(MerkaartorPreferences::instance()->getHoverColor());
	QPointF P(theProjection.project(Position));
	QRectF R(P-QPointF(3,3),QSize(6,6));
	thePainter.drawRect(R);
	R.adjust(-7, -7, 7, 7);
	thePainter.drawEllipse(R);
}

double TrackPoint::pixelDistance(const QPointF& Target, double, const Projection& theProjection) const
{
	return distance(Target,theProjection.project(Position));
}

void TrackPoint::cascadedRemoveIfUsing(MapDocument*, MapFeature*, CommandList*, const std::vector<MapFeature*>&)
{
}

QString TrackPoint::description() const
{
	QString s(tagValue("name",""));
	if (!s.isEmpty())
		return QApplication::translate("TrackMapLayer", "%1 (node %2)").arg(s).arg(id());
	return
		QApplication::translate("TrackMapLayer", "node %1").arg(id());
}

void TrackPoint::partChanged(MapFeature*, unsigned int)
{
}

RenderPriority TrackPoint::renderPriority(double) const
{
	return RenderPriority(RenderPriority::IsSingular,0);
}

QString TrackPoint::toXML(unsigned int lvl)
{
	QString S(lvl*2, ' ');
	S += "<node id=\"%1\" lat=\"%2\" lon=\"%3\">\n";
	S += tagsToXML(lvl+1);
	S += QString(lvl*2, ' ') + "</node>\n";
	return S.arg(stripToOSMId(id())).arg(radToAng(position().lat()),0,'f',8).arg(radToAng(position().lon()),0,'f',8);
}

bool TrackPoint::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("node");
	xParent.appendChild(e);

	e.setAttribute("id", xmlId());
	e.setAttribute("lon",QString::number(radToAng(Position.lon()),'f',8));
	e.setAttribute("lat", QString::number(radToAng(Position.lat()),'f',8));

	tagsToXML(e);

	return OK;
}

bool TrackPoint::toTrackXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("trkpt");
	xParent.appendChild(e);

	e.setAttribute("xml:id", xmlId());
	e.setAttribute("lon",QString::number(radToAng(Position.lon()),'f',8));
	e.setAttribute("lat", QString::number(radToAng(Position.lat()),'f',8));
	e.setAttribute("time", Time.toString("yyyy-MM-ddTHH:mm:ssZ"));

	tagsToXML(e);

	return OK;
}

TrackPoint * TrackPoint::fromXML(MapDocument* d, MapLayer* L, const QDomElement e)
{
	double Lat = e.attribute("lat").toDouble();
	double Lon = e.attribute("lon").toDouble();

	QString id;
	if (e.hasAttribute("id"))
		id = "node_"+e.attribute("id");
	else
		id = "node_"+e.attribute("xml:id");
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(d->getFeature(id));
	if (!Pt) {
		Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
		Pt->setId(id);
		Pt->setLastUpdated(MapFeature::OSMServer);
		L->add(Pt);
	} else {
		Pt->setPosition(Coord(angToRad(Lat), angToRad(Lon)));
		if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
			Pt->setLastUpdated(MapFeature::OSMServer);
	}

	MapFeature::tagsFromXML(d, Pt, e);

	return Pt;
}
