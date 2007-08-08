#include "Map/Road.h"

#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <algorithm>
#include <vector>

class RoadPrivate
{
	public:
		std::vector<Way*> Ways;
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
	std::vector<Way*> Parts(p->Ways);
	p->Ways.clear();
	for (unsigned int i=0; i<Parts.size(); ++i)
		Parts[i]->removeAsPartOf(this);
	delete p;
}

void Road::add(Way* W)
{
	if (std::find(p->Ways.begin(),p->Ways.end(),W) == p->Ways.end())
	{
		p->Ways.push_back(W);
		W->addAsPartOf(this);
	}
}

void Road::add(Way* W, unsigned int Idx)
{
	if (std::find(p->Ways.begin(),p->Ways.end(),W) == p->Ways.end())
	{
		p->Ways.push_back(W);
		std::rotate(p->Ways.begin()+Idx,p->Ways.end()-1,p->Ways.end());
		W->addAsPartOf(this);
	}
}

unsigned int Road::find(Way* W) const
{
	for (unsigned int i=0; i<p->Ways.size(); ++i)
		if (p->Ways[i] == W)
			return i;
	return p->Ways.size();
}

void Road::erase(Way* W)
{
	std::vector<Way*>::iterator i = std::find(p->Ways.begin(),p->Ways.end(),W);
	if (i != p->Ways.end())
	{
		p->Ways.erase(i);
		W->removeAsPartOf(this);
	}
}

unsigned int Road::size() const
{
	return p->Ways.size();
}

Way* Road::get(unsigned int idx)
{
	return p->Ways[idx];
}

const Way* Road::get(unsigned int idx) const
{
	return p->Ways[idx];
}

bool Road::notEverythingDownloaded() const
{
	if (lastUpdated() == MapFeature::NotYetDownloaded)
		return true;
	for (unsigned int i=0; i<p->Ways.size(); ++i)
		if (p->Ways[i]->notEverythingDownloaded())
			return true;
	return false;
}

CoordBox Road::boundingBox() const
{
	if (p->Ways.size())
	{
		CoordBox BBox(p->Ways[0]->boundingBox());
		for (unsigned int i=1; i<p->Ways.size(); ++i)
			BBox.merge(p->Ways[i]->boundingBox());
		return BBox;
	}
	return CoordBox(Coord(0,0),Coord(0,0));
}

static Coord half(Way* W)
{
	if (W->controlFrom() && W->controlTo())
	{
		double H = (W->controlFrom()->position().lat()+W->controlTo()->position().lat())/2;
		double L2 = (W->from()->position().lat()+W->controlFrom()->position().lat())/2;
		double R3 = (W->controlTo()->position().lat()+W->to()->position().lat())/2;
		double L3 = (L2+H)/2;
		double R2 = (H+R3)/2;
		double Lat = (L3+R2)/2;
		H = (W->controlFrom()->position().lon()+W->controlTo()->position().lon())/2;
		L2 = (W->from()->position().lon()+W->controlFrom()->position().lon())/2;
		R3 = (W->controlTo()->position().lon()+W->to()->position().lon())/2;
		L3 = (L2+H)/2;
		R2 = (H+R3)/2;
		double Lon = (L3+R2)/2;
		return Coord(Lat,Lon);
	}
	double Lat = 0.5*(W->from()->position().lat()+W->to()->position().lat());
	double Lon = 0.5*(W->from()->position().lon()+W->to()->position().lon());
	return Coord(Lat,Lon);
}

void Road::draw(QPainter& thePainter, const Projection& theProjection)
{
/*	::drawPossibleArea(thePainter,this,theProjection);
	thePainter.setBrush(QColor(0x22,0xff,0x22,128));
	thePainter.setPen(QColor(255,255,255,128));
	for (unsigned int i=0; i<p->Ways.size(); ++i)
	{
		Way* W = p->Ways[i];

		QPointF P(theProjection.project(half(W)));
		double Rad = theProjection.pixelPerM()*widthOf(W);
		if (Rad>2)
			thePainter.drawEllipse(P.x()-Rad/2,P.y()-Rad/2,Rad,Rad);
		if (Rad>2)
		{
			QPen TP;
			if (lastUpdated() == MapFeature::OSMServerConflict)
				TP = QPen(QBrush(QColor(0xff,0,0)),Rad/4);
			else
				TP = QPen(QBrush(QColor(0x22,0xff,0x22,128)),Rad/4);
			::draw(thePainter, TP, W, theProjection);
		}
	} */
}

void Road::drawFocus(QPainter& thePainter, const Projection& theProjection)
{
	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(QColor(0,0,255));
	thePainter.setPen(TP);
	thePainter.setBrush(QColor(0,0,255));
	for (unsigned int i=0; i<p->Ways.size(); ++i)
	{
		Way* W = p->Ways[i];
		QPointF F(theProjection.project(W->from()->position()));
		QPointF P(theProjection.project(half(W)));
		::draw(thePainter,TP,W,theProjection);
		if (distance(F,P)>30)
		{
			thePainter.setBrush(Qt::NoBrush);
			thePainter.setPen(QColor(0,0,0));
			thePainter.fillRect(QRect(P.x()-6,P.y()-6,12,12),QColor(255,255,255));
			thePainter.drawText(QRect(P.x()-6,P.y()-6,12,12),Qt::AlignCenter,QString::number(i+1));
			thePainter.setPen(TP);
			thePainter.drawEllipse(P.x()-7,P.y()-7,14,14);
			thePainter.setBrush(QColor(0,0,255));
		}
		else
		{
			double Rad = theProjection.pixelPerM()*widthOf(W);
			thePainter.drawEllipse(P.x()-Rad/2,P.y()-Rad/2,Rad,Rad);
		}
	}
}

double Road::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	double Best = 1000000;
	for (unsigned int i=0; i<p->Ways.size(); ++i)
	{
		double D = p->Ways[i]->pixelDistance(Target,ClearEndDistance,theProjection);
		if (D < ClearEndDistance)
			Best = D;
	}
	// always prefer us over ways
	if (Best<ClearEndDistance)
		Best*=0.99;
	return Best;
}

void Road::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Proposals)
{
	for (unsigned int i=0; i<p->Ways.size(); ++i)
		if (p->Ways[i] == aFeature)
		{
			std::vector<Way*> Alternatives;
			for (unsigned int j=0; j<Proposals.size(); ++j)
			{
				Way* W = dynamic_cast<Way*>(Proposals[j]);
				if (W)
					Alternatives.push_back(W);
			}
			if ( (p->Ways.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				theList->add(new RoadRemoveWayCommand(this, p->Ways[i]));
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					theList->add(new RoadAddWayCommand(this, Alternatives[j], i+j));
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