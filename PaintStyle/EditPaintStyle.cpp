#include "PaintStyle/EditPaintStyle.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Road.h"
#include "Map/Way.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <math.h>
#include <utility>

/* FEATUREPAINTSELECTOR */

FeaturePaintSelector::FeaturePaintSelector()
: DrawBackground(false), DrawForeground(false), DrawTouchup(false), ZoomLimit(false), 
  ForegroundFill(false), DrawTrafficDirectionMarks(true)
{
}

static bool localZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < 0.25;
}

static bool regionalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < 0.05;
}

static bool globalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < 0.01;
}

FeaturePaintSelector& FeaturePaintSelector::drawTrafficDirectionMarks()
{
	DrawTrafficDirectionMarks = true;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::limitToZoom(FeaturePaintSelector::ZoomType aType)
{
	ZoomLimit = true;
	switch (aType)
	{
		case GlobalZoom : PixelPerMZoomLimit = 0.01; break;
		case RegionalZoom : PixelPerMZoomLimit = 0.05; break;
		case LocalZoom : PixelPerMZoomLimit = 0.25; break;
		case NoZoomLimit : ZoomLimit = false; break;
	}
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::foregroundFill(const QColor& StrokeColor, const QColor& FillColor, double StrokeWidth)
{
	ForegroundFill = true;
	ForegroundFillStrokeColor = StrokeColor;
	ForegroundFillFillColor = FillColor;
	ForegroundFillStrokeWidth = StrokeWidth;
	return *this;
}


FeaturePaintSelector& FeaturePaintSelector::background(const QColor& Color, double Scale, double Offset)
{
	DrawBackground = true;
	BackgroundColor = Color;
	BackgroundScale = Scale;
	BackgroundOffset = Offset;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::touchupDash(double Dash, double White)
{
	TouchupDashSet = true;
	TouchupDash = Dash;
	TouchupWhite = White;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::touchup(const QColor& Color, double Scale, double Offset)
{
	DrawTouchup = true;
	TouchupColor = Color;
	TouchupScale = Scale;
	TouchupOffset = Offset;
	TouchupDashSet = false;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::foregroundDash(double Dash, double White)
{
	ForegroundDashSet = true;
	ForegroundDash = Dash;
	ForegroundWhite = White;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::foreground(const QColor& Color, double Scale, double Offset)
{
	DrawForeground = true;
	ForegroundColor = Color;
	ForegroundScale = Scale;
	ForegroundOffset = Offset;
	ForegroundDashSet = false;
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::selectOnTag(const QString& Tag, const QString& Value)
{
	OneOfTheseTags.push_back(std::make_pair(Tag,Value));
	return *this;
}

FeaturePaintSelector& FeaturePaintSelector::selectOnTag(const QString& Tag, const QString& Value1, const QString& Value2)
{
	selectOnTag(Tag,Value1);
	selectOnTag(Tag,Value2);
	return *this;
}

bool FeaturePaintSelector::isHit(const Way* W, double PixelPerM) const
{
	if (ZoomLimit && (PixelPerM < PixelPerMZoomLimit))
		return false;
	for (unsigned int i=0; i<OneOfTheseTags.size(); ++i)
		if (cascadedTagValue(W,OneOfTheseTags[i].first,QString()) == OneOfTheseTags[i].second)
			return true;
	return false;
}

bool FeaturePaintSelector::isHit(const Road* R, double PixelPerM) const
{
	if (ZoomLimit && (PixelPerM < PixelPerMZoomLimit))
		return false;
	for (unsigned int i=0; i<OneOfTheseTags.size(); ++i)
		if (R->tagValue(OneOfTheseTags[i].first,QString::null) == OneOfTheseTags[i].second)
			return true;
	return false;
}

void FeaturePaintSelector::drawBackground(Way* W, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawBackground) return;
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf(W)*BackgroundScale+BackgroundOffset;
	if (WW < 0) return;
	QPen thePen(BackgroundColor,WW);
	thePen.setCapStyle(Qt::RoundCap);
	QPainterPath Path;
	QPointF FromF(theProjection.project(W->from()->position()));
	QPointF ToF(theProjection.project(W->to()->position()));
	Path.moveTo(FromF);
	Path.lineTo(ToF);
	thePainter.strokePath(Path,thePen);
}

void FeaturePaintSelector::drawForeground(Way* W, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawForeground) return;
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf(W)*ForegroundScale+ForegroundOffset;
	if (WW < 0) return;
	QPen thePen(ForegroundColor,WW);
	thePen.setCapStyle(Qt::RoundCap);
	if (ForegroundDashSet)
	{
		QVector<qreal> Pattern;
		Pattern << ForegroundDash << ForegroundWhite;
		thePen.setDashPattern(Pattern);
	}
	QPainterPath Path;
	QPointF FromF(theProjection.project(W->from()->position()));
	QPointF ToF(theProjection.project(W->to()->position()));
	Path.moveTo(FromF);
	Path.lineTo(ToF);
	thePainter.strokePath(Path,thePen);
}

void FeaturePaintSelector::drawTouchup(Way* W, QPainter& thePainter, const Projection& theProjection) const
{
	if (DrawTouchup)
	{
		double PixelPerM = theProjection.pixelPerM();
		double WW = PixelPerM*widthOf(W)*TouchupScale+TouchupOffset;
		if (WW > 0)
		{
			QPen thePen(TouchupColor,WW);
			thePen.setCapStyle(Qt::FlatCap);
			if (TouchupDashSet)
			{
				QVector<qreal> Pattern;
				Pattern << TouchupDash << TouchupWhite;
				thePen.setDashPattern(Pattern);
			}
			QPainterPath Path;
			QPointF FromF(theProjection.project(W->from()->position()));
			QPointF ToF(theProjection.project(W->to()->position()));
			Path.moveTo(FromF);
			Path.lineTo(ToF);
			thePainter.strokePath(Path,thePen);
		}
	}
	if (DrawTrafficDirectionMarks)
	{
		double theWidth = theProjection.pixelPerM()*widthOf(W)-4;
		if (theWidth > 8)
			theWidth = 8;
		double DistFromCenter = 2*(theWidth+4);
		if (theWidth > 0)
		{
			QPointF FromF(theProjection.project(W->from()->position()));
			QPointF ToF(theProjection.project(W->to()->position()));
			if (distance(FromF,ToF) > (DistFromCenter*2+4))
			{
				QPointF H(FromF+ToF);
				H *= 0.5;
				double A = angle(FromF-ToF);
				QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
				QPointF V1(theWidth*cos(A+3.141592/6),theWidth*sin(A+3.141592/6));
				QPointF V2(theWidth*cos(A-3.141592/6),theWidth*sin(A-3.141592/6));
				MapFeature::TrafficDirectionType TT = W->trafficDirection();
				if ( (TT == MapFeature::OtherWay) || (TT == MapFeature::BothWays) )
				{
					thePainter.setPen(QColor(0,0,0));
					thePainter.drawLine(H+T,H+T-V1);
					thePainter.drawLine(H+T,H+T-V2);
				}
				if ( (TT == MapFeature::OneWay) || (TT == MapFeature::BothWays) )
				{
					thePainter.setPen(QColor(0,0,0));
					thePainter.drawLine(H-T,H-T+V1);
					thePainter.drawLine(H-T,H-T+V2);
				}
			}
		}
	}
}

void FeaturePaintSelector::drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (R->size() < 3) return;
	if (!ForegroundFill) return;
	QPen thePen(ForegroundFillStrokeColor,ForegroundFillStrokeWidth);
	thePen.setCapStyle(Qt::RoundCap);
	QBrush theBrush(ForegroundFillFillColor);
	QPainterPath Path;
	Path.moveTo(theProjection.project(R->get(0)->from()->position()));
	for (unsigned int i=0; i<R->size(); ++i)
		Path.lineTo(theProjection.project(R->get(i)->to()->position()));
	thePainter.setPen(thePen);
	thePainter.setBrush(theBrush);
	thePainter.drawPath(Path);
}

class EPBackgroundLayer : public CascadingPaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Way* W);
		virtual void draw(Road* R);
	private:
		EditPaintStylePrivate* p;
};

class EPForegroundLayer : public CascadingPaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Way* W);
		virtual void draw(Road* R);
	private:
		EditPaintStylePrivate* p;
};

class EPTouchupLayer : public CascadingPaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Way* W);
		virtual void draw(Road* R);
	private:
		EditPaintStylePrivate* p;
};

class EditPaintStylePrivate
{
	public:
		EditPaintStylePrivate(QPainter& P, const Projection& aProj)
			: thePainter(P), theProjection(aProj)
		{
			First.setP(this);
			Second.setP(this);
			Thirth.setP(this);
			First.setNextLayer(&Second);
			Second.setNextLayer(&Thirth);
			initPainters();
		}

		void initPainters();

		QPainter& thePainter;
		const Projection& theProjection;
		EPBackgroundLayer First;
		EPForegroundLayer Second;
		EPTouchupLayer Thirth;
		std::vector<FeaturePaintSelector> Painters;
};

void EditPaintStylePrivate::initPainters()
{
	FeaturePaintSelector MotorWay;
	MotorWay.background(QColor(0xff,0,0),1,0).foreground(QColor(0xff,0xff,0),0.5,0);
	MotorWay.selectOnTag("highway","motorway","motorway_link");
	MotorWay.drawTrafficDirectionMarks();
	Painters.push_back(MotorWay);

	FeaturePaintSelector Trunk;
	Trunk.foreground(QColor(0xff,0,0),1,0);
	Trunk.selectOnTag("highway","trunk","trunk_link");
	Trunk.drawTrafficDirectionMarks();
	Painters.push_back(Trunk);

	FeaturePaintSelector Primary;
	Primary.foreground(QColor(0,0xff,0),1,0);
	Primary.selectOnTag("highway","primary","primary_link").limitToZoom(FeaturePaintSelector::GlobalZoom);
	Primary.drawTrafficDirectionMarks();
	Painters.push_back(Primary);

	FeaturePaintSelector Secondary;
	Secondary.foreground(QColor(0xff,0xff,0),1,0);
	Secondary.selectOnTag("highway","secondary","secondary_link").limitToZoom(FeaturePaintSelector::RegionalZoom);
	Secondary.drawTrafficDirectionMarks();
	Painters.push_back(Secondary);

	FeaturePaintSelector Tertiary;
	Tertiary.foreground(QColor(0xff,0xff,0x77),1,0);
	Tertiary.selectOnTag("highway","tertiary","tertiary_link").limitToZoom(FeaturePaintSelector::RegionalZoom);
	Tertiary.drawTrafficDirectionMarks();
	Painters.push_back(Tertiary);

	FeaturePaintSelector Cycleway;
	Cycleway.foreground(QColor(0,0,0xff),1,0).foregroundDash(2,2);
	Cycleway.selectOnTag("highway","cycleway").limitToZoom(FeaturePaintSelector::LocalZoom);
	Painters.push_back(Cycleway);

	FeaturePaintSelector Footway;
	Footway.foreground(QColor(0,0,0),1,0).foregroundDash(2,2);
	Footway.selectOnTag("highway","footway","track").limitToZoom(FeaturePaintSelector::LocalZoom);
	Painters.push_back(Footway);

	FeaturePaintSelector Pedestrian;
	Pedestrian.foreground(QColor(0xaa,0xaa,0xaa),1,0);
	Pedestrian.selectOnTag("highway","pedestrian").limitToZoom(FeaturePaintSelector::LocalZoom);
	Painters.push_back(Pedestrian);

	FeaturePaintSelector Residential;
	Residential.background(QColor(0,0,0),1,0).foreground(QColor(0xff,0xff,0xff),1,-2);
	Residential.selectOnTag("highway","residential","unclassified").limitToZoom(FeaturePaintSelector::LocalZoom);
	Residential.drawTrafficDirectionMarks();
	Painters.push_back(Residential);

	FeaturePaintSelector Railway;
	Railway.background(QColor(0,0,0),1,0).foreground(QColor(0xff,0xff,0xff),1,-3).touchup(QColor(0,0,0),1,-3).touchupDash(3,3);
	Railway.selectOnTag("railway","rail").limitToZoom(FeaturePaintSelector::GlobalZoom);
	Painters.push_back(Railway);

	FeaturePaintSelector Park;
	Park.foregroundFill(QColor(0,0x77,0),QColor(0x77,0xff,0x77,0x77),1);
	Park.selectOnTag("leisure","park").limitToZoom(FeaturePaintSelector::GlobalZoom);
	Painters.push_back(Park);

	FeaturePaintSelector Pitch;
	Pitch.foregroundFill(QColor(0x77,0,0),QColor(0xff,0x77,0x77,0x77),1);
	Pitch.selectOnTag("leisure","pitch").limitToZoom(FeaturePaintSelector::GlobalZoom);
	Painters.push_back(Pitch);

	FeaturePaintSelector Water;
	Water.foregroundFill(QColor(0,0,0x77),QColor(0x77,0x77,0xff,0x77),1);
	Water.selectOnTag("natural","water").limitToZoom(FeaturePaintSelector::GlobalZoom);
	Painters.push_back(Water);
}

void EPBackgroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}


void EPBackgroundLayer::draw(Way* W)
{
	if (p->theProjection.viewport().disjunctFrom(W->boundingBox())) return;
	double PixelPerM = p->theProjection.pixelPerM();
	for (unsigned int i=0; i<p->Painters.size(); ++i)
		if (p->Painters[i].isHit(W,PixelPerM))
		{
			p->Painters[i].drawBackground(W,p->thePainter,p->theProjection);
			return;
		}
	if (globalZoom(p->theProjection))
		return;
	QPen thePen(QColor(0,0,0),1);
	if (regionalZoom(p->theProjection))
		thePen = QPen(QColor(0x77,0x77,0x77),1);
	QPainterPath Path;
	QPointF FromF(p->theProjection.project(W->from()->position()));
	QPointF ToF(p->theProjection.project(W->to()->position()));
	Path.moveTo(FromF);
	Path.lineTo(ToF);
	p->thePainter.strokePath(Path,thePen);
}

void EPBackgroundLayer::draw(Road*)
{
	// roads don't need background for now
}

void EPForegroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPForegroundLayer::draw(Road* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	double PixelPerM = p->theProjection.pixelPerM();
	for (unsigned int i=0; i<p->Painters.size(); ++i)
		if (p->Painters[i].isHit(R, PixelPerM))
		{
			p->Painters[i].drawForeground(R,p->thePainter,p->theProjection);
			return;
		}
}

void EPForegroundLayer::draw(Way* W)
{
	if (p->theProjection.viewport().disjunctFrom(W->boundingBox())) return;
	double PixelPerM = p->theProjection.pixelPerM();
	for (unsigned int i=0; i<p->Painters.size(); ++i)
		if (p->Painters[i].isHit(W, PixelPerM))
		{
			p->Painters[i].drawForeground(W,p->thePainter,p->theProjection);
			return;
		}
}



void EPTouchupLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPTouchupLayer::draw(Road*)
{
	// Roads don't need touchup for now
}

void EPTouchupLayer::draw(Way* W)
{
	if (p->theProjection.viewport().disjunctFrom(W->boundingBox())) return;
	double PixelPerM = p->theProjection.pixelPerM();
	for (unsigned int i=0; i<p->Painters.size(); ++i)
		if (p->Painters[i].isHit(W, PixelPerM))
		{
			p->Painters[i].drawTouchup(W,p->thePainter,p->theProjection);
			return;
		}
}


/* EDITPAINTSTYLE */


EditPaintStyle::EditPaintStyle(QPainter& P, const Projection& theProjection)
{
	p = new EditPaintStylePrivate(P,theProjection);
}

EditPaintStyle::~EditPaintStyle(void)
{
	delete p;
}

PaintStyle* EditPaintStyle::firstLayer()
{
	return &p->First;
}

PaintStyle* EditPaintStyle::nextLayer()
{
	return &p->First;
}