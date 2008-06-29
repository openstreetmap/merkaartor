#include "Map/TrackSegment.h"
#include "Command/DocumentCommands.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>

#include <vector>

class TrackSegmentPrivate
{
	public:
		std::vector<TrackPoint*> Points;
};

TrackSegment::TrackSegment(void)
{
	p = new TrackSegmentPrivate;
}

TrackSegment::TrackSegment(const TrackSegment& other)
: MapFeature(other), p(0)
{
}

TrackSegment::~TrackSegment(void)
{
	delete p;
}

void TrackSegment::sortByTime()
{
	for (unsigned int i=0; i<p->Points.size(); ++i)
	{
		for (unsigned int j=i+1; j<p->Points.size(); ++j)
		{
			if (p->Points[i]->time() > p->Points[j]->time())
			{
				QDateTime dt(p->Points[i]->time());
				p->Points[i]->setTime(p->Points[j]->time());
				p->Points[j]->setTime(dt);
			}
		}
	}
}

QString TrackSegment::description() const
{
	return "tracksegment";
}

RenderPriority TrackSegment::renderPriority(double) const
{
	return RenderPriority(RenderPriority::IsLinear,0);
}

void TrackSegment::add(TrackPoint* aPoint)
{
	p->Points.push_back(aPoint);
}

unsigned int TrackSegment::size() const
{
	return p->Points.size();
}

TrackPoint* TrackSegment::get(int i)
{
	return p->Points[i];
}

bool TrackSegment::visibleLine(const CoordBox & viewport, const Coord & last, const Coord & here)
{
	if (viewport.contains(last) || viewport.contains(here))
		return true;

	return viewport.intersects( CoordBox(last, here) );
}

static void configurePen(QPen & pen, double slope, double speed)
{
	// Encode speed in width of path ...
	double penWidth = 1.0;
	if (speed > 10.0)
		penWidth = qMin(1.0+speed*0.02, 5.0);

	// ... and slope in the color
	unsigned int green = 0;
	unsigned int red = 0;

	if (slope > 2.0)
	{
		slope = qMin(slope, 20.0);
		green = 48 + int(slope*79.0 / 20.0);
	}
	else if (slope < -2.0)
	{
		slope = qMax(slope, - 20.0);
		red = 48 + int(-slope*79.0 / 20.0);
	}

	pen.setColor(QColor(128 + red, 128 + green, 128));

	pen.setStyle(Qt::DotLine);
	pen.setWidthF(penWidth);
}

void TrackSegment::drawDirectionMarkers(QPainter &P, QPen &pen, const QPointF & FromF, const QPointF & ToF)
{
	if (::distance(FromF,ToF) <= 30.0)
		return;

	const double DistFromCenter=10.0;
	const double theWidth=5.0;
	const double A = angle(FromF-ToF);

	QPointF T(DistFromCenter*cos(A), DistFromCenter*sin(A));
	QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
	QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));

	pen.setStyle(Qt::SolidLine);
	P.setPen(pen);

	QPointF H((FromF+ToF) / 2);
	P.drawLine(H-T,H-T+V1);
	P.drawLine(H-T,H-T+V2);
}

void TrackSegment::draw(QPainter &P, const Projection& theProjection)
{
	QPen pen;

	for (unsigned int i=1; i<p->Points.size(); ++i)
	{
		const Coord & last = p->Points[i-1]->position();
		const Coord & here = p->Points[i]->position();

		if (visibleLine(theProjection.viewport(), last, here) == false)
			continue;

		QPointF FromF(theProjection.project(last));
		QPointF ToF(theProjection.project(here));

		const double distance = here.distanceFrom(last);
		const double slope = (p->Points[i]->elevation() - p->Points[i-1]->elevation()) / (distance * 10.0);
		const double speed = p->Points[i]->speed();

		configurePen(pen, slope, speed);
		P.setPen(pen);

		P.drawLine(FromF,ToF);
		drawDirectionMarkers(P, pen, FromF, ToF);
	}
}

bool TrackSegment::notEverythingDownloaded() const
{
	return false;
}

void TrackSegment::drawFocus(QPainter &, const Projection &)
{
	// Can't be selection
}

void TrackSegment::drawHover(QPainter &, const Projection &)
{
	// Can't be selection
}

CoordBox TrackSegment::boundingBox() const
{
	if (p->Points.size())
	{
		CoordBox Box(p->Points[0]->position(),p->Points[0]->position());
		for (unsigned int i=1; i<p->Points.size(); ++i)
			Box.merge(p->Points[i]->position());
		return Box;
	}
	return CoordBox(Coord(0,0),Coord(0,0));
}

double TrackSegment::pixelDistance(const QPointF& , double , const Projection&) const
{
	// unable to select that one
	return 1000000;
}

void TrackSegment::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& /*Alternatives*/)
{
	for (unsigned int i=0; i<p->Points.size(); ++i)
	{
		// TODO don't remove whole list, but just the point in the list
		if (p->Points[i] == aFeature)
		{
			// TODO use alternative if available
/*			TrackPoint* Alternative = 0;
			if (Alternatives.size() == 1)
				Alternative = dynamic_cast<TrackPoint*>(Alternatives[0]);
			if (Alternative)
*/
			theList->add(new RemoveFeatureCommand(theDocument,this));
			return;
/*			if (p->Points.size() == 1)
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
				theList->add(new   */
		}
	}
}

void TrackSegment::partChanged(MapFeature*, unsigned int)
{
}

bool TrackSegment::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("trkseg");
	xParent.appendChild(e);

	e.setAttribute("xml:id", xmlId());

	for (unsigned int i=0; i<size(); ++i) {
		get(i)->toTrackXML(e);
	}

	return OK;
}

TrackSegment* TrackSegment::fromXML(MapDocument* d, MapLayer* L, const QDomElement e)
{
	TrackSegment* l = new TrackSegment();

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "trkpt") {
			TrackPoint* N = TrackPoint::fromXML(d, L, c);
			l->add(N);
//			L->add(N);
		}
		c = c.nextSiblingElement();
	}

	return l;
}
