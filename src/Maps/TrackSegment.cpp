#include "Maps/TrackSegment.h"
#include "Command/DocumentCommands.h"
#include "Command/TrackSegmentCommands.h"
#include "MapView.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QProgressDialog>

#include <algorithm>
#include <QList>

class TrackSegmentPrivate
{
	public:
		TrackSegmentPrivate()
		: Distance(0), MetaUpToDate(false)
		{
		}

		QList<TrackPoint*> Nodes;
		double Distance;

		bool MetaUpToDate;
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
	for (int i=0; i<p->Nodes.size(); ++i)
		p->Nodes[i]->unsetParentFeature(this);
	delete p;
}

void TrackSegment::sortByTime()
{
	for (int i=0; i<p->Nodes.size(); ++i)
	{
		for (int j=i+1; j<p->Nodes.size(); ++j)
		{
			if (p->Nodes[i]->time() > p->Nodes[j]->time())
			{
				QDateTime dt(p->Nodes[i]->time());
				p->Nodes[i]->setTime(p->Nodes[j]->time());
				p->Nodes[j]->setTime(dt);
			}
		}
	}
}

QString TrackSegment::description() const
{
	return "tracksegment";
}

RenderPriority TrackSegment::renderPriority()
{
	RenderPriority apriority(RenderPriority::IsLinear,0.); 
	setRenderPriority(apriority);
	return apriority;
}

void TrackSegment::add(TrackPoint* aPoint)
{
	p->Nodes.push_back(aPoint);
	aPoint->setParentFeature(this);
}

void TrackSegment::add(TrackPoint* Pt, int Idx)
{
	p->Nodes.push_back(Pt);
	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
}

int TrackSegment::find(MapFeature* Pt) const
{
	for (int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == Pt)
			return i;
	return p->Nodes.size();
}

void TrackSegment::remove(int idx)
{
	TrackPoint* Pt = p->Nodes[idx];
	p->Nodes.erase(p->Nodes.begin()+idx);
	Pt->unsetParentFeature(this);
}

void TrackSegment::remove(MapFeature* F)
{
	for (int i=p->Nodes.size(); i; --i)
		if (p->Nodes[i-1] == F)
			remove(i-1);
}

int TrackSegment::size() const
{
	return p->Nodes.size();
}

MapFeature* TrackSegment::get(int i)
{
	return p->Nodes[i];
}

TrackPoint* TrackSegment::getNode(int i)
{
	return p->Nodes[i];
}

const MapFeature* TrackSegment::get(int Idx) const
{
	return p->Nodes[Idx];
}

bool TrackSegment::isNull() const
{
	return (p->Nodes.size() == 0);
}

static void configurePen(QPen & pen, double slope, double speed)
{
	// Encode speed in width of path ...
	double penWidth = 1.0;
	if (speed > 10.0)
		penWidth = qMin(1.0+speed*0.02, 5.0);

	// ... and slope in the color
	int green = 0;
	int red = 0;

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

	QPointF H((FromF+ToF) / 2.0);
	P.drawLine(H-T,H-T+V1);
	P.drawLine(H-T,H-T+V2);
}

void TrackSegment::draw(QPainter &P, MapView* theView)
{
	QPen pen;

	if (!M_PREFS->getTrackSegmentsVisible())
		return;

	for (int i=1; i<p->Nodes.size(); ++i)
	{
		Coord last = p->Nodes[i-1]->position();
		Coord here = p->Nodes[i]->position();

		if (CoordBox::visibleLine(theView->viewport(), last, here) == false)
			continue;

		QPointF FromF(theView->toView(last));
		QPointF ToF(theView->toView(here));

		const double distance = here.distanceFrom(last);
		const double slope = (p->Nodes[i]->elevation() - p->Nodes[i-1]->elevation()) / (distance * 10.0);
		const double speed = p->Nodes[i]->speed();

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

void TrackSegment::drawFocus(QPainter &, MapView*, bool)
{
	// Can't be selection
}

void TrackSegment::drawHover(QPainter &, MapView*, bool)
{
	// Can't be selection
}

CoordBox TrackSegment::boundingBox() const
{
	if (p->Nodes.size())
	{
		CoordBox Box(p->Nodes[0]->position(),p->Nodes[0]->position());
		for (int i=1; i<p->Nodes.size(); ++i)
			Box.merge(p->Nodes[i]->position());
		return Box;
	}
	return CoordBox(Coord(0,0),Coord(0,0));
}

double TrackSegment::pixelDistance(const QPointF& , double , const Projection&, const QTransform& ) const
{
	// unable to select that one
	return 1000000;
}

void TrackSegment::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Proposals)
{
	for (int i=0; i<p->Nodes.size();) {
		if (p->Nodes[i] == aFeature)
		{
			QList<TrackPoint*> Alternatives;
			for (int j=0; j<Proposals.size(); ++j)
			{
				TrackPoint* Pt = dynamic_cast<TrackPoint*>(Proposals[j]);
				if (Pt)
					Alternatives.push_back(Pt);
			}
			if ( (p->Nodes.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				for (int j=0; j<Alternatives.size(); ++j)
				{
					if (i < p->Nodes.size())
					{
						if (p->Nodes[i+j] != Alternatives[j])
						{
							if ((i+j) == 0)
								theList->add(new TrackSegmentAddTrackPointCommand(this, Alternatives[j], i+j,Alternatives[j]->layer()));
							else if (p->Nodes[i+j-1] != Alternatives[j])
								theList->add(new TrackSegmentAddTrackPointCommand(this, Alternatives[j], i+j,Alternatives[j]->layer()));
						}
					}
				}
				theList->add(new TrackSegmentRemoveTrackPointCommand(this, (TrackPoint*)aFeature,aFeature->layer()));
			}
		}
		++i;
	}
}

void TrackSegment::partChanged(MapFeature*, int)
{
}

void TrackSegment::updateMeta() const
{
	p->Distance = 0;

	if (p->Nodes.size() == 0)
	{
		p->MetaUpToDate = true;
		return;
	}

	for (unsigned int i=0; (i+1)<p->Nodes.size(); ++i)
	{
		if (p->Nodes[i] && p->Nodes[i+1]) {
			const Coord & here = p->Nodes[i]->position();
			const Coord & next = p->Nodes[i+1]->position();

			p->Distance += next.distanceFrom(here);
		}
	}


	p->MetaUpToDate = true;
}

double TrackSegment::distance() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->Distance;
}

int TrackSegment::duration() const
{
	return p->Nodes[0]->time().secsTo(p->Nodes[p->Nodes.size() - 1]->time());
}


bool TrackSegment::toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("trkseg");
	xParent.appendChild(e);

	if (!forExport)
		e.setAttribute("xml:id", xmlId());

	for (int i=0; i<size(); ++i) {
		dynamic_cast <TrackPoint*> (get(i))->toGPX(e, progress, forExport);
	}

	return OK;
}

bool TrackSegment::toXML(QDomElement xParent, QProgressDialog & progress)
{
	return toGPX(xParent, progress, false);
}

TrackSegment* TrackSegment::fromGPX(MapDocument* d, MapLayer* L, const QDomElement e, QProgressDialog & progress)
{
	TrackSegment* l = new TrackSegment();

	if (e.hasAttribute("xml:id"))
		l->setId(e.attribute("xml:id"));

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "trkpt") {
			TrackPoint* N = TrackPoint::fromGPX(d, L, c);
			l->add(N);
			progress.setValue(progress.value()+1);
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}

	return l;
}

TrackSegment* TrackSegment::fromXML(MapDocument* d, MapLayer* L, const QDomElement e, QProgressDialog & progress)
{
	return TrackSegment::fromGPX(d, L, e, progress);
}
