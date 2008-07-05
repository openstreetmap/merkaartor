#include "TrackPoint.h"

#include "Map/Projection.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>

TrackPoint::TrackPoint(const Coord& aCoord)
: Position(aCoord), Elevation(0.0), Speed(0.0)
{
}

TrackPoint::TrackPoint(const TrackPoint& other)
: MapFeature(other), Position(other.Position), Elevation(other.Elevation), Speed(other.Speed)
{
	setTime(other.time());
}

TrackPoint::~TrackPoint(void)
{
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

double TrackPoint::speed() const
{
	return Speed;
}

void TrackPoint::setSpeed(double aSpeed)
{
	Speed = aSpeed;
}

double TrackPoint::elevation() const
{
	return Elevation;
}

void TrackPoint::setElevation(double aElevation)
{
	Elevation = aElevation;
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
		return QString("%1 (%2)").arg(s).arg(id());
	return
		QString("%1").arg(id());
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
	e.setAttribute("timestamp", time().toUTC().toString(Qt::ISODate));
	e.setAttribute("user", user());

	tagsToXML(e);

	return OK;
}

bool TrackPoint::toGPX(QDomElement xParent)
{
	bool OK = true;

	QString s = tagValue("_waypoint_","");
	QDomElement e;
	if (!s.isEmpty()) 
		e = xParent.ownerDocument().createElement("wpt");
	else
		e = xParent.ownerDocument().createElement("trkpt");
	xParent.appendChild(e);

	e.setAttribute("xml:id", xmlId());
	e.setAttribute("lon",QString::number(radToAng(Position.lon()),'f',8));
	e.setAttribute("lat", QString::number(radToAng(Position.lat()),'f',8));

	QDomElement c = xParent.ownerDocument().createElement("time");
	e.appendChild(c);
	QDomText v = c.ownerDocument().createTextNode(time().toString(Qt::ISODate)+"Z");
	c.appendChild(v);

	s = tagValue("name","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("name");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}
	if (elevation()) {
		QDomElement c = xParent.ownerDocument().createElement("ele");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(QString::number(elevation(),'f',6));
		c.appendChild(v);
	}
	s = tagValue("_comment_","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("cmt");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}
	s = tagValue("_description_","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("desc");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}

	return OK;
}

TrackPoint * TrackPoint::fromXML(MapDocument* d, MapLayer* L, const QDomElement e)
{
	double Lat = e.attribute("lat").toDouble();
	double Lon = e.attribute("lon").toDouble();

	QDateTime time;
	time = QDateTime::fromString(e.attribute("timestamp"), Qt::ISODate);
	QString user = e.attribute("user");

	QString id = (e.hasAttribute("id") ? e.attribute("id") : e.attribute("xml:id"));
	if (!id.startsWith('{'))
		id = "node_" + id;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(d->getFeature(id));
	if (!Pt) {
		Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
		Pt->setId(id);
		Pt->setTime(time);
		Pt->setUser(user);
		Pt->setLastUpdated(MapFeature::OSMServer);
		L->add(Pt);
	} else {
		if (Pt->layer() != L) {
			Pt->layer()->remove(Pt);
			L->add(Pt);
		}
		Pt->setPosition(Coord(angToRad(Lat), angToRad(Lon)));
		Pt->setTime(time);
		Pt->setUser(user);
		if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
			Pt->setLastUpdated(MapFeature::OSMServer);
	}

	MapFeature::tagsFromXML(d, Pt, e);

	return Pt;
}

TrackPoint * TrackPoint::fromGPX(MapDocument* d, MapLayer* L, const QDomElement e)
{
	double Lat = e.attribute("lat").toDouble();
	double Lon = e.attribute("lon").toDouble();

	QString id = (e.hasAttribute("id") ? e.attribute("id") : e.attribute("xml:id"));
	if (!id.startsWith('{'))
		id = "node_" + id;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(d->getFeature(id));
	if (!Pt) {
		Pt = new TrackPoint(Coord(angToRad(Lat),angToRad(Lon)));
		Pt->setId(id);
		Pt->setLastUpdated(MapFeature::OSMServer);
		L->add(Pt);
	} else {
		if (Pt->layer() != L) {
			Pt->layer()->remove(Pt);
			L->add(Pt);
		}
		Pt->setPosition(Coord(angToRad(Lat), angToRad(Lon)));
		if (Pt->lastUpdated() == MapFeature::NotYetDownloaded)
			Pt->setLastUpdated(MapFeature::OSMServer);
	}

	if (e.tagName() == "wpt")
		Pt->setTag("_waypoint_", "yes");

	QDateTime time;
	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "time") {
			QString dtm = c.text();
			dtm.truncate(19);
			Pt->setTime(QDateTime::fromString(dtm, Qt::ISODate));
		} else
		if (c.tagName() == "ele") {
			Pt->setElevation(c.text().toFloat());
		} else
		if (c.tagName() == "cmt") {
			Pt->setTag("_comment_", c.text());
		} else
		if (c.tagName() == "desc") {
			Pt->setTag("_description_", c.text());
		}

		c = c.nextSiblingElement();
	}

	return Pt;
}

QString TrackPoint::toHtml()
{
	QString D;
	unsigned int i;

	D += "<i>"+QApplication::translate("MapFeature", "timestamp")+": </i>" + time().toString(Qt::ISODate) + "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "coord")+": </i>" + QString::number(radToAng(position().lat()), 'f', 4) + " / " + QString::number(radToAng(position().lon()), 'f', 4) + "<br/>";
	if (elevation())
		D += "<i>"+QApplication::translate("MapFeature", "elevation")+": </i>" + QString::number(elevation(), 'f', 4) + "<br/>";
	if (speed())
		D += "<i>"+QApplication::translate("MapFeature", "speed")+": </i>" + QString::number(speed(), 'f', 4) + "<br/>";

	if ((i = findKey("_waypoint_")) < tagSize()) {
		D += "<p><b>"+QApplication::translate("MapFeature", "Waypoint")+"</b><br/>";
		
		if ((i = findKey("_description_")) < tagSize())
			D += "<i>"+QApplication::translate("MapFeature", "description")+": </i>" + tagValue(i) + "<br/>";
		
		if ((i = findKey("_comment_")) < tagSize())
			D += "<i>"+QApplication::translate("MapFeature", "comment")+": </i>" + tagValue(i) + "<br/>";
	}

	return MapFeature::toMainHtml(QApplication::translate("MapFeature", "Node"), "node").arg(D);
}

void TrackPoint::toBinary(QDataStream& ds)
{
	ds << (qint8)'N' << idToLong() << (qint32)(INT_MAX * (Position.lon() / M_PI)) << (qint32)(INT_MAX * (Position.lat() / M_PI_2));
	tagsToBinary(ds);
}

TrackPoint* TrackPoint::fromBinary(MapDocument* d, MapLayer* /* L */, QDataStream& ds)
{
	qint8	c;
	qint64	id;
	qint32	lon;
	qint32	lat;

	ds >> c; if (c != 'N') return NULL;
	ds >> id;
	ds >> lon;
	ds >> lat;

	Coord cd( ((double)lat / INT_MAX * M_PI_2), ((double)lon / INT_MAX * M_PI) );
	TrackPoint* N = new TrackPoint(cd);
	if (id < 1)
		N->setId(QString::number(id));
	else
		N->setId("node_"+QString::number(id));
	MapFeature::tagsFromBinary(d, N, ds);

	return N;
}
