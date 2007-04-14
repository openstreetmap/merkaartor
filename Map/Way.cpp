#include "Map/Way.h"
#include "Command/DocumentCommands.h"
#include "Command/WayCommands.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <algorithm>
#include <math.h>

#define DEFAULTWIDTH 4

Way::Way(TrackPoint* aFrom, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* aTo)
: From(aFrom), To(aTo), ControlFrom(aC1), ControlTo(aC2)
{
}

Way::Way(TrackPoint* aFrom, TrackPoint* aTo)
: From(aFrom), To(aTo), ControlFrom(0), ControlTo(0)
{
}

Way::~Way(void)
{
}

void Way::addAsPartOf(Road* R)
{
	PartOf.push_back(R);
}

bool Way::notEverythingDownloaded() const
{
	return (lastUpdated() == MapFeature::NotYetDownloaded) ||
		From->notEverythingDownloaded() ||
		To->notEverythingDownloaded();
}

void Way::removeAsPartOf(Road* R)
{
	std::vector<Road*>::iterator i = std::find(PartOf.begin(),PartOf.end(),R);
	if (i!=PartOf.end())
		PartOf.erase(i);
}

bool Way::isPartOf(Road* R)
{
	return std::find(PartOf.begin(),PartOf.end(),R) != PartOf.end();
}

void Way::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* F, CommandList* theList, const std::vector<MapFeature*>& Alternatives)
{
	if ( (To == F) || (From == F) || (ControlFrom ==F) || (ControlTo == F) )
	{
		TrackPoint* Alternative = 0;
		if (Alternatives.size() == 1)
			Alternative = dynamic_cast<TrackPoint*>(Alternatives[0]);
		if (Alternative)
		{
			TrackPoint* NewTo = (To==F)?Alternative:To;
			TrackPoint* NewFrom = (From==F)?Alternative:From;
			TrackPoint* NewControlTo = (ControlTo==F)?Alternative:ControlTo;
			TrackPoint* NewControlFrom = (ControlFrom==F)?Alternative:ControlFrom;
			theList->add(new WaySetFromToCommand(this,NewFrom, NewControlFrom, NewControlTo, NewTo));
		}
		else
			theList->add(new RemoveFeatureCommand(theDocument,this));
	}
	Road* R = dynamic_cast<Road*>(F);
	if (R) removeAsPartOf(R);
}

double Way::width() const
{
	unsigned int idx=findKey("width");
	if (idx<tagSize())
		return tagValue(idx).toDouble();
	for (unsigned int i=0; i<PartOf.size(); ++i)
	{
		idx = PartOf[i]->findKey("width");
		if (idx < PartOf[i]->tagSize())
			return PartOf[i]->tagValue(idx).toDouble();
	}
	return DEFAULTWIDTH;
}

MapFeature::TrafficDirectionType Way::trafficDirection() const
{
	QString d;
	unsigned int idx=findKey("oneway");
	if (idx<tagSize())
		d = tagValue(idx);
	for (unsigned int i=0; (i<PartOf.size()) && d.isEmpty(); ++i)
	{
		idx = PartOf[i]->findKey("oneway");
		if (idx < PartOf[i]->tagSize())
			d = PartOf[i]->tagValue(idx);
	}
	if (d == "yes") return OneWay;
	if (d == "true") return OneWay;
	if (d == "false") return BothWays;
	if (d == "no") return BothWays;
	if (d == "-1") return OtherWay;
	return UnknownDirection;
}


void Way::setWidth(double w)
{
	if (fabs(w-DEFAULTWIDTH) < 0.01)
		clearTag("width");
	else
		setTag("width",QString::number(w));
}

CoordBox Way::boundingBox() const
{
	return CoordBox(From->position(),To->position());
}

void Way::setFromTo(TrackPoint* aFrom, TrackPoint* aTo)
{
	From = aFrom;
	To = aTo;
	ControlFrom = ControlTo = 0;
}

void Way::setFromTo(TrackPoint* aFrom, TrackPoint* aC1, TrackPoint* aC2, TrackPoint* aTo)
{
	From = aFrom;
	To = aTo;
	ControlFrom = aC1;
	ControlTo = aC2;
}

TrackPoint* Way::from()
{
	return From;
}

TrackPoint* Way::to()
{
	return To;
}

TrackPoint* Way::controlFrom()
{
	return ControlFrom;
}

TrackPoint* Way::controlTo()
{
	return ControlTo;
}


const TrackPoint* Way::from() const
{
	return From;
}

const TrackPoint* Way::to() const
{
	return To;
}


static double pixelDistance(const QPointF& Target, const QPointF& P1, const QPointF& P2, const QPointF& P3, const QPointF& P4)
{
	LineF L(P1,P4);
	double D2 = L.distance(P2);
	double D3 = L.distance(P3);
	if ( (D2 < 0.5) && (D3<0.5) )
		return L.distance(Target);
	else
	{
		QPointF H = (P2+P3)/2;
		QPointF L2 = (P1+P2)/2;
		QPointF R3 = (P3+P4)/2;
		QPointF L3 = (L2+H)/2;
		QPointF R2 = (H+R3)/2;
		QPointF L4 = (L3+R2)/2;
		double A = pixelDistance(Target,P1,L2,L3,L4);
		double B = pixelDistance(Target,L4,R2,R3,P4);
		return A<B?A:B;
	}
}


void Way::draw(QPainter& P, const Projection& theProjection)
{
	if (theProjection.viewport().disjunctFrom(boundingBox())) return;
	double WW = theProjection.pixelPerM()*width();
	if (WW<1)
		WW = 1;
	QPen TP;
	if (lastUpdated() == MapFeature::OSMServerConflict)
		TP = QPen(QBrush(QColor(0xff,0,0)),WW);
	else
		TP = QPen(QBrush(QColor(0xff,0x88,0x22,128)),WW);
	::draw(P,TP,this,WW,theProjection);
}

void Way::drawFocus(QPainter& P, const Projection& theProjection)
{
	double W = theProjection.pixelPerM()*width()/2+1;
	QPen TP(QBrush(QColor(0x00,0x00,0xff,128)),W);
	QPainterPath Path;
	::draw(P,TP,this,theProjection);
}


double Way::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	QPointF F(theProjection.project(From->position()));
	QPointF T(theProjection.project(To->position()));
	if (distance(Target,F) < ClearEndDistance)
		return ClearEndDistance;
	if (distance(Target,T) < ClearEndDistance)
		return ClearEndDistance;
	if (ControlFrom && ControlTo)
	{
		QPointF CF(theProjection.project(ControlFrom->position()));
		QPointF CT(theProjection.project(ControlTo->position()));
		if (distance(Target,CF) < ClearEndDistance)
			return ClearEndDistance;
		if (distance(Target,CT) < ClearEndDistance)
			return ClearEndDistance;
		return ::pixelDistance(Target,F,CF,CT,T);
	}
	else
	{
		LineF L(F,T);
		return L.capDistance(Target);
	}
}
