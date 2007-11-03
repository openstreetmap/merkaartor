#include "Map/Road.h"

#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Map/Coord.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <algorithm>
#include <vector>

class RoadPrivate
{
	public:
		RoadPrivate()
		: BBox(Coord(0,0),Coord(0,0)), BBoxUpToDate(true)
		{
		}
		std::vector<TrackPoint*> Nodes;
		CoordBox BBox;
		bool BBoxUpToDate;
};

Road::Road(void)
: p(new RoadPrivate)
{
}

Road::Road(const Road& )
: p(0)
{
}

Road::~Road(void)
{
	delete p;
}

void Road::addedToDocument()
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		p->Nodes[i]->setParent(this);
}

void Road::removedFromDocument()
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		p->Nodes[i]->unsetParent(this);
}

void Road::partChanged(MapFeature*)
{
	p->BBoxUpToDate = false;
}

QString Road::description() const
{
	QString s(tagValue("name",""));
	if (!s.isEmpty())
		return QString("%1 (road %2)").arg(s).arg(id());
	return QString("road %1").arg(id());
}

void Road::add(TrackPoint* Pt)
{
	p->Nodes.push_back(Pt);
	Pt->setParent(this);
	p->BBoxUpToDate = false;
}

void Road::add(TrackPoint* Pt, unsigned int Idx)
{
	p->Nodes.push_back(Pt);
	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
	Pt->setParent(this);
	p->BBoxUpToDate = false;
}

unsigned int Road::find(TrackPoint* Pt) const
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == Pt)
			return i;
	return p->Nodes.size();
}

void Road::remove(unsigned int idx)
{
	TrackPoint* Pt = p->Nodes[idx];
	p->Nodes.erase(p->Nodes.begin()+idx);
	Pt->unsetParent(this);
	p->BBoxUpToDate = false;
}

unsigned int Road::size() const
{
	return p->Nodes.size();
}

TrackPoint* Road::get(unsigned int idx)
{
	return p->Nodes[idx];
}

const TrackPoint* Road::get(unsigned int idx) const
{
	return p->Nodes[idx];
}

bool Road::notEverythingDownloaded() const
{
	if (lastUpdated() == MapFeature::NotYetDownloaded)
		return true;
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i]->notEverythingDownloaded())
			return true;
	return false;
}

CoordBox Road::boundingBox() const
{
	if (!p->BBoxUpToDate)
	{
		if (p->Nodes.size())
		{
			p->BBox = CoordBox(p->Nodes[0]->position(),p->Nodes[0]->position());
			for (unsigned int i=1; i<p->Nodes.size(); ++i)
				p->BBox.merge(p->Nodes[i]->position());
		}
		else
			p->BBox = CoordBox(Coord(0,0),Coord(0,0));
		p->BBoxUpToDate = true;
	}
	return p->BBox;
}

void Road::draw(QPainter& thePainter, const Projection& theProjection)
{
}

void Road::drawFocus(QPainter& thePainter, const Projection& theProjection)
{
	// FIXME
	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(QColor(0,0,255));
	thePainter.setPen(TP);
	thePainter.setBrush(QColor(0,0,255));
	for (unsigned int i=1; i<p->Nodes.size(); ++i)
	{
		::draw(thePainter,TP,MapFeature::UnknownDirection,p->Nodes[i-1]->position(),p->Nodes[i]->position(),widthOf(this),theProjection);
	}
}

double Road::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	double Best = 1000000;
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
	{
		double x = distance(Target,theProjection.project(p->Nodes[i]->position()));
		if (x<ClearEndDistance)
			return Best;
	}
	for (unsigned int i=1; i<p->Nodes.size(); ++i)
	{
		LineF F(theProjection.project(p->Nodes[i-1]->position()),theProjection.project(p->Nodes[i]->position()));
		double D = F.capDistance(Target);
		if (D < ClearEndDistance)
			Best = D;
	}
	return Best;
}

void Road::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Proposals)
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == aFeature)
		{
			std::vector<TrackPoint*> Alternatives;
			for (unsigned int j=0; j<Proposals.size(); ++j)
			{
				TrackPoint* Pt = dynamic_cast<TrackPoint*>(Proposals[j]);
				if (Pt)
					Alternatives.push_back(Pt);
			}
			if ( (p->Nodes.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				theList->add(new RoadRemoveTrackPointCommand(this, p->Nodes[i]));
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					theList->add(new RoadAddTrackPointCommand(this, Alternatives[j], i+j));
			}
			return;
		}
}

MapFeature::TrafficDirectionType trafficDirection(const Road* R)
{
	// TODO some duplication with Way trafficDirection
	QString d;
	unsigned int idx=R->findKey("oneway");
	if (idx<R->tagSize())
		d = R->tagValue(idx);
	else
		return MapFeature::UnknownDirection;
	if ( (d == "yes") || (d == "1") ) return MapFeature::OneWay;
	if (d == "no") return MapFeature::BothWays;
	if (d == "-1") return MapFeature::OtherWay;
	return MapFeature::UnknownDirection;
}

#define DEFAULTWIDTH 6
#define LANEWIDTH 4

double widthOf(const Road* R) 
{ 
	QString s(R->tagValue("width",QString())); 
	if (!s.isNull()) 
		return s.toDouble(); 
	QString h = R->tagValue("highway",QString()); 
	if ( (h == "motorway") || (h=="motorway_link") ) 
		return 4*LANEWIDTH; // 3 lanes plus emergency 
	else if ( (h == "trunk") || (h=="trunk_link") ) 
		return 3*LANEWIDTH; // 2 lanes plus emergency 
	else if ( (h == "primary") || (h=="primary_link") ) 
		return 2*LANEWIDTH; // 2 lanes 
	else if (h == "secondary") 
		return 2*LANEWIDTH; // 2 lanes 
	else if (h == "tertiary") 
		return 1.5*LANEWIDTH; // shared middle lane 
	else if (h == "cycleway") 
		return 1.5; 
	return DEFAULTWIDTH; 
} 

unsigned int findSnapPointIndex(const Road* R, Coord& P)
{
	LineF L(R->get(0)->position(),R->get(1)->position());
	unsigned int BestIdx = 1;
	double BestDistance = L.capDistance(P);
	for (unsigned int i = 2; i<R->size(); ++i)
	{
		LineF L(R->get(i-1)->position(),R->get(i)->position());
		double Distance = L.capDistance(P);
		if (Distance < BestDistance)
		{
			BestIdx = i;
			BestDistance = Distance;
		}
	}
	LineF F(R->get(BestIdx-1)->position(),R->get(BestIdx)->position());
	P = F.project(Coord(P));
	return BestIdx;
}

bool isClosed(const Road* R)
{
	return R->size() && (R->get(0) == R->get(R->size()-1));
}

