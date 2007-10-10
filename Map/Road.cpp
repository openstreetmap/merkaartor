#include "Map/Road.h"

#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
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
		std::vector<TrackPoint*> Nodes;
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

void Road::add(TrackPoint* Pt)
{
	p->Nodes.push_back(Pt);
}

void Road::add(TrackPoint* Pt, unsigned int Idx)
{
	p->Nodes.push_back(Pt);
	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
}

unsigned int Road::find(TrackPoint* Pt) const
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == Pt)
			return i;
	return p->Nodes.size();
}

void Road::remove(TrackPoint* Pt)
{
	std::vector<TrackPoint*>::iterator i = std::find(p->Nodes.begin(),p->Nodes.end(),Pt);
	if (i != p->Nodes.end())
		p->Nodes.erase(i);
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
	if (p->Nodes.size())
	{
		CoordBox BBox(p->Nodes[0]->boundingBox());
		for (unsigned int i=1; i<p->Nodes.size(); ++i)
			BBox.merge(p->Nodes[i]->boundingBox());
		return BBox;
	}
	return CoordBox(Coord(0,0),Coord(0,0));
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
	if (d == "yes") return MapFeature::OneWay;
	if (d == "no") return MapFeature::BothWays;
	if (d == "-1") return MapFeature::OtherWay;
	return MapFeature::UnknownDirection;
}

#define DEFAULTWIDTH 4

double widthOf(const Road* R) 
{ 
	QString s(R->tagValue("width",QString())); 
	if (!s.isNull()) 
		return s.toDouble(); 
	QString h = R->tagValue("highway",QString()); 
	if ( (h == "motorway") || (h=="motorway_link") ) 
		return 4*4; // 3 lanes plus emergency 
	else if ( (h == "trunk") || (h=="trunk_link") ) 
		return 3*4; // 2 lanes plus emergency 
	else if ( (h == "primary") || (h=="primary_link") ) 
		return 2*4; // 2 lanes 
	else if (h == "secondary") 
		return 2*4; // 2 lanes 
	else if (h == "tertiary") 
		return 1.5*4; // shared middle lane 
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