#include "PaintStyle.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Road.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <math.h>

FeaturePainter::FeaturePainter()
: DrawBackground(false), DrawForeground(false), DrawTouchup(false), ZoomLimit(NoZoomLimit), 
  ForegroundFill(false), DrawTrafficDirectionMarks(true)
{
}


FeaturePainter& FeaturePainter::drawTrafficDirectionMarks()
{
	DrawTrafficDirectionMarks = true;
	return *this;
}

FeaturePainter& FeaturePainter::limitToZoom(FeaturePainter::ZoomType aType)
{
	ZoomLimit = aType;
	return *this;
}

FeaturePainter& FeaturePainter::foregroundFill(const QColor& FillColor)
{
	ForegroundFill = true;
	ForegroundFillFillColor = FillColor;
	return *this;
}


FeaturePainter& FeaturePainter::background(const QColor& Color, double Scale, double Offset)
{
	DrawBackground = true;
	BackgroundColor = Color;
	BackgroundScale = Scale;
	BackgroundOffset = Offset;
	return *this;
}

FeaturePainter& FeaturePainter::touchupDash(double Dash, double White)
{
	TouchupDashSet = true;
	TouchupDash = Dash;
	TouchupWhite = White;
	return *this;
}

FeaturePainter& FeaturePainter::touchup(const QColor& Color, double Scale, double Offset)
{
	DrawTouchup = true;
	TouchupColor = Color;
	TouchupScale = Scale;
	TouchupOffset = Offset;
	TouchupDashSet = false;
	return *this;
}

FeaturePainter& FeaturePainter::foregroundDash(double Dash, double White)
{
	ForegroundDashSet = true;
	ForegroundDash = Dash;
	ForegroundWhite = White;
	return *this;
}

FeaturePainter& FeaturePainter::foreground(const QColor& Color, double Scale, double Offset)
{
	DrawForeground = true;
	ForegroundColor = Color;
	ForegroundScale = Scale;
	ForegroundOffset = Offset;
	ForegroundDashSet = false;
	return *this;
}

FeaturePainter& FeaturePainter::trackPointIcon(const QString& Filename)
{
	TrackPointIconName = Filename;
	return *this;
}

FeaturePainter& FeaturePainter::selectOnTag(const QString& Tag, const QString& Value)
{
	OneOfTheseTags.push_back(std::make_pair(Tag,Value));
	return *this;
}

FeaturePainter& FeaturePainter::selectOnTag(const QString& Tag, const QString& Value1, const QString& Value2)
{
	selectOnTag(Tag,Value1);
	selectOnTag(Tag,Value2);
	return *this;
}

bool FeaturePainter::isHit(const MapFeature* F, ZoomType Zoom) const
{
	if (ZoomLimit > Zoom)
		return false;
	for (unsigned int i=0; i<OneOfTheseTags.size(); ++i)
		if (F->tagValue(OneOfTheseTags[i].first,QString::null) == OneOfTheseTags[i].second)
			return true;
	return false;
}

void buildPathFromRoad(Road *R, Projection const &theProjection, QPainterPath &Path)
{
	Path.moveTo(theProjection.project(R->get(0)->position()));
	for (unsigned int i=1; i<R->size(); ++i)
		Path.lineTo(theProjection.project(R->get(i)->position()));
}

void FeaturePainter::drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawBackground) return;
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf(R)*BackgroundScale+BackgroundOffset;
	if (WW < 0) return;
	QPen thePen(BackgroundColor,WW);
	thePen.setCapStyle(Qt::RoundCap);
	QPainterPath Path;
	buildPathFromRoad(R, theProjection, Path);
	thePainter.strokePath(Path,thePen);
}

void FeaturePainter::drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawForeground && !ForegroundFill) return;
	if (DrawForeground)
	{
		double PixelPerM = theProjection.pixelPerM();
		double WW = PixelPerM*widthOf(R)*ForegroundScale+ForegroundOffset;
		if (WW < 0) return;
		QPen thePen(ForegroundColor,WW);
		thePen.setCapStyle(Qt::RoundCap);
		if (ForegroundDashSet)
		{
			QVector<qreal> Pattern;
			Pattern << ForegroundDash << ForegroundWhite;
			thePen.setDashPattern(Pattern);
		}
		thePainter.setPen(thePen);
	}
	else
		thePainter.setPen(QPen(Qt::NoPen));
	if (ForegroundFill && (R->size() > 2))
	{
		QBrush theBrush(ForegroundFillFillColor);
		thePainter.setBrush(theBrush);
	}
	else
		thePainter.setBrush(QBrush(Qt::NoBrush));
	QPainterPath Path;
	buildPathFromRoad(R, theProjection, Path);
	thePainter.drawPath(Path);
}

void FeaturePainter::drawTouchup(TrackPoint* Pt, QPainter& thePainter, const Projection& theProjection) const
{
	if (TrackPointIconName != "")
	{
		QPixmap pm(TrackPointIconName);
		QPointF C(theProjection.project(Pt->position()));
		thePainter.drawPixmap( C.x()-pm.width()/2,C.y()-pm.height()-5 , pm);
	}
}

void FeaturePainter::drawTouchup(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (DrawTouchup)
	{
		double PixelPerM = theProjection.pixelPerM();
		double WW = PixelPerM*widthOf(R)*TouchupScale+TouchupOffset;
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
			buildPathFromRoad(R, theProjection, Path);
			thePainter.strokePath(Path,thePen);
		}
	}
	if (DrawTrafficDirectionMarks)
	{
		double theWidth = theProjection.pixelPerM()*widthOf(R)-4;
		if (theWidth > 8)
			theWidth = 8;
		double DistFromCenter = 2*(theWidth+4);
		if (theWidth > 0)
		{
			for (unsigned int i=1; i<R->size(); ++i)
			{
				QPointF FromF(theProjection.project(R->get(i-1)->position()));
				QPointF ToF(theProjection.project(R->get(i)->position()));
				if (distance(FromF,ToF) > (DistFromCenter*2+4))
				{
					QPointF H(FromF+ToF);
					H *= 0.5;
					double A = angle(FromF-ToF);
					QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
					QPointF V1(theWidth*cos(A+3.141592/6),theWidth*sin(A+3.141592/6));
					QPointF V2(theWidth*cos(A-3.141592/6),theWidth*sin(A-3.141592/6));
					MapFeature::TrafficDirectionType TT = trafficDirection(R);
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
}

PaintStyleLayer::~PaintStyleLayer()
{
}

void PaintStyle::add(PaintStyleLayer *aLayer)
{
	Layers.push_back(aLayer);
}

unsigned int PaintStyle::size() const
{
	return Layers.size();
}

PaintStyleLayer* PaintStyle::get(unsigned int i)
{
	return Layers[i];
}

