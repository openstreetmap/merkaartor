#include "PaintStyle.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Utils/LineF.h"
#include "Utils/SvgCache.h"

#include <QtCore/QString>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

#include <math.h>

FeaturePainter::FeaturePainter()
: theSelector(0),
  ZoomLimitSet(false), ZoomUnder(0), ZoomUpper(10e6),
  DrawBackground(false), BackgroundScale(0), BackgroundOffset(3),
  DrawForeground(false), ForegroundScale(0), ForegroundOffset(2),
  ForegroundDashSet(false),
  DrawTouchup(false), TouchupScale(0), TouchupOffset(1),
  TouchupDashSet(false),
  ForegroundFill(false), DrawTrafficDirectionMarks(false),
  DrawIcon(false), IconScale(0), IconOffset(0),
  DrawLabel(false), LabelScale(0), LabelOffset(0),
  DrawLabelBackground(false)
{
}

FeaturePainter::FeaturePainter(const FeaturePainter& f)
: theSelector(0),
  ZoomLimitSet(f.ZoomLimitSet), ZoomUnder(f.ZoomUnder), ZoomUpper(f.ZoomUpper),
  DrawBackground(f.DrawBackground), BackgroundColor(f.BackgroundColor),
  BackgroundScale(f.BackgroundScale), BackgroundOffset(f.BackgroundOffset),
  DrawForeground(f.DrawForeground), ForegroundColor(f.ForegroundColor),
  ForegroundScale(f.ForegroundScale), ForegroundOffset(f.ForegroundOffset),
  ForegroundDashSet(f.ForegroundDashSet), ForegroundDash(f.ForegroundDash), ForegroundWhite(f.ForegroundWhite),
  DrawTouchup(f.DrawTouchup), TouchupColor(f.TouchupColor),
  TouchupScale(f.TouchupScale), TouchupOffset(f.TouchupOffset),
  TouchupDashSet(f.TouchupDashSet),
  TouchupDash(f.TouchupDash), TouchupWhite(f.TouchupWhite),
  ForegroundFill(f.ForegroundFill), ForegroundFillFillColor(f.ForegroundFillFillColor),
  DrawTrafficDirectionMarks(f.DrawTrafficDirectionMarks),
  DrawIcon(f.DrawIcon), IconName(f.IconName), IconScale(f.IconScale), IconOffset(f.IconOffset),
  DrawLabel(f.DrawLabel), LabelTag(f.LabelTag), LabelColor(f.LabelColor), LabelScale(f.LabelScale), LabelOffset(f.LabelOffset),
  DrawLabelBackground(f.DrawLabelBackground), LabelBackgroundColor(f.LabelBackgroundColor), LabelBackgroundTag(f.LabelBackgroundTag),
  LabelFont(f.LabelFont)
{
	if (f.theSelector)
		theSelector = f.theSelector->copy();

}

FeaturePainter& FeaturePainter::operator=(const FeaturePainter& f)
{
	if (&f == this) return *this;
	delete theSelector;
	if (f.theSelector)
		theSelector = f.theSelector->copy();
	else
		theSelector = 0;
	ZoomLimitSet = f.ZoomLimitSet;
	ZoomUnder = f.ZoomUnder;
	ZoomUpper = f.ZoomUpper;
	DrawBackground = f.DrawBackground;
	BackgroundColor = f.BackgroundColor;
	BackgroundScale = f.BackgroundScale;
	BackgroundOffset = f.BackgroundOffset;
	DrawForeground = f.DrawForeground;
	ForegroundColor = f.ForegroundColor;
	ForegroundScale = f.ForegroundScale;
	ForegroundOffset = f.ForegroundOffset;
	ForegroundDashSet = f.ForegroundDashSet ;
	ForegroundDash = f.ForegroundDash;
	ForegroundWhite = f.ForegroundWhite;
	DrawTouchup = f.DrawTouchup;
	TouchupColor = f.TouchupColor;
	TouchupScale = f.TouchupScale;
	TouchupOffset = f.TouchupOffset;
	TouchupDashSet = f.TouchupDashSet;
	TouchupDash = f.TouchupDash;
	TouchupWhite = f.TouchupWhite;
	ForegroundFill = f.ForegroundFill;
	ForegroundFillFillColor = f.ForegroundFillFillColor;
	DrawTrafficDirectionMarks = f.DrawTrafficDirectionMarks;
	DrawIcon = f.DrawIcon;
	IconName = f.IconName;
	IconScale = f.IconScale;
	IconOffset = f.IconOffset;
	DrawLabel = f.DrawLabel;
	LabelColor = f.LabelColor;
	LabelScale = f.LabelScale;
	LabelOffset = f.LabelOffset;
	DrawLabelBackground = f.DrawLabelBackground;
	LabelBackgroundColor = f.LabelBackgroundColor;
	LabelFont = f.LabelFont;
	LabelTag = f.LabelTag;
	LabelBackgroundTag = f.LabelBackgroundTag;
	return *this;
}

FeaturePainter::~FeaturePainter()
{
	delete theSelector;
}

QString paddedHexa(unsigned int i)
{
	QString r=QString::number(i,16);
	if (r.length() < 2)
		r = "0"+r;
	return r;
}

QString asXML(const QColor& c)
{
	return "#"+paddedHexa(c.red())+paddedHexa(c.green())+paddedHexa(c.blue())+paddedHexa(c.alpha());
}

QString boundaryAsXML(const QString& name, const QColor& c, double Scale, double Offset)
{
	return
		name+"Color=\""+asXML(c)+"\" "+name+"Scale=\""+QString::number(Scale)+"\" "+name+"Offset=\""+QString::number(Offset)+"\"\n";
}

QString iconAsXML(const QString& name, const QString& fn, double Scale, double Offset)
{
	return
		name+"=\""+fn+"\" "+name+"Scale=\""+QString::number(Scale)+"\" "+name+"Offset=\""+QString::number(Offset)+"\"\n";
}

QString FeaturePainter::asXML() const
{
	QString r;
	r += "<painter\n";
	if (ZoomLimitSet)
		r += " zoomUnder=\""+QString::number(ZoomUnder)+"\" zoomUpper=\""+QString::number(ZoomUpper)+"\"\n";
	if (DrawBackground)
		r += " " + boundaryAsXML("background",BackgroundColor, BackgroundScale, BackgroundOffset);
	if (DrawForeground)
		r += " " + boundaryAsXML("foreground",ForegroundColor, ForegroundScale, ForegroundOffset);
	if (ForegroundDashSet && DrawForeground)
		r += " foregroundDashDown=\""+QString::number(ForegroundDash)+"\" foregroundDashUp=\""+QString::number(ForegroundWhite)+"\"\n";
	if (DrawTouchup)
		r += " " + boundaryAsXML("touchup",TouchupColor, TouchupScale, TouchupOffset);
	if (TouchupDashSet && DrawTouchup)
		r += " touchupDashDown=\""+QString::number(TouchupDash)+"\" touchupDashUp=\""+QString::number(TouchupWhite)+"\"\n";
	if (ForegroundFill)
		r += " fillColor=\""+::asXML(ForegroundFillFillColor)+"\"\n";
	if (!IconName.isEmpty() && DrawIcon)
		r += " " + iconAsXML("icon",IconName, IconScale, IconOffset);
	if (DrawTrafficDirectionMarks)
		r += " drawTrafficDirectionMarks=\"yes\"";
	else
		r += " drawTrafficDirectionMarks=\"no\"";
	if (DrawLabel) {
		r += " " + boundaryAsXML("label",LabelColor, LabelScale, LabelOffset);
		r += " labelFont=\"" + LabelFont.toString() + "\"";
		r += " labelTag=\"" + LabelTag + "\"";
	}
	if (DrawLabelBackground) {
		r += " labelBackgroundColor=\""+::asXML(LabelBackgroundColor)+"\"";
		r += " labelBackgroundTag=\""+ LabelBackgroundTag +"\"\n";
	}
	r += ">\n";


	if (theSelector)
		r += "  <selector expr=\""+theSelector->asExpression(false)+"\"/>\n";
	r += "</painter>\n";
	return r;
}


QString FeaturePainter::userName() const
{
	if (theSelector)
		return theSelector->asExpression(false);
	return "Unnamed";
}

std::pair<double, double> FeaturePainter::zoomBoundaries() const
{
	if (ZoomLimitSet)
		return std::make_pair(ZoomUnder,ZoomUpper);
	return std::make_pair(0.0,0.0);
}

QColor FeaturePainter::fillColor() const
{
	if (!ForegroundFill)
		return QColor();
	return ForegroundFillFillColor;
}

bool FeaturePainter::isFilled() const
{
	return ForegroundFill;
}

FeaturePainter& FeaturePainter::drawTrafficDirectionMarks()
{
	DrawTrafficDirectionMarks = true;
	return *this;
}

FeaturePainter& FeaturePainter::zoomBoundary(double anUnder, double anUpper)
{
	ZoomLimitSet = true;
	ZoomUnder = anUnder;
	ZoomUpper = anUpper;
	return *this;
}

FeaturePainter& FeaturePainter::fillActive(bool b)
{
	ForegroundFill = b;
	if (ForegroundFill && !ForegroundFillFillColor.isValid())
		ForegroundFillFillColor.setRgb(0,0,0);
	return *this;
}

FeaturePainter& FeaturePainter::foregroundFill(const QColor& FillColor)
{
	ForegroundFill = true;
	ForegroundFillFillColor = FillColor;
	return *this;
}

FeaturePainter& FeaturePainter::backgroundActive(bool b)
{
	DrawBackground = b;
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

LineParameters FeaturePainter::backgroundBoundary() const
{
	LineParameters P;
	P.Draw = DrawBackground;
	P.Color = BackgroundColor;
	P.Proportional = BackgroundScale;
	P.Fixed = BackgroundOffset;
	P.Dashed = false;
	P.DashOn = P.DashOff = 0;
	return P;
}

FeaturePainter& FeaturePainter::touchupActive(bool b)
{
	DrawTouchup = b;
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

FeaturePainter& FeaturePainter::foregroundActive(bool b)
{
	DrawForeground = b;
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

void FeaturePainter::clearForegroundDash()
{
	ForegroundDashSet = false;
}

FeaturePainter& FeaturePainter::labelActive(bool b)
{
	DrawLabel = b;
	return *this;
}

FeaturePainter& FeaturePainter::labelTag(const QString& val)
{
	LabelTag = val;
	return *this;
}

FeaturePainter& FeaturePainter::labelBackgroundTag(const QString& val)
{
	LabelBackgroundTag = val;
	return *this;
}

FeaturePainter& FeaturePainter::label(const QColor& Color, double Scale, double Offset)
{
	DrawLabel = true;
	LabelColor = Color;
	LabelScale = Scale;
	LabelOffset = Offset;
	return *this;
}

FeaturePainter& FeaturePainter::labelBackgroundActive(bool b)
{
	DrawLabelBackground = b;
	if (DrawLabelBackground && !LabelBackgroundColor.isValid())
		LabelBackgroundColor.setRgb(0,0,0);
	return *this;
}

FeaturePainter& FeaturePainter::labelBackground(const QColor& bgColor)
{
	DrawLabelBackground = true;
	LabelBackgroundColor = bgColor;
	return *this;
}

FeaturePainter& FeaturePainter::setLabelFont(const QString& descFont)
{
	LabelFont.fromString(descFont);
	return *this;
}

QColor FeaturePainter::labelBackgroundColor() const
{
	if (!DrawLabelBackground)
		return QColor();
	return LabelBackgroundColor;
}

QFont FeaturePainter::getLabelFont() const
{
	return LabelFont;
}

QString FeaturePainter::getLabelTag() const
{
	return LabelTag;
}

QString FeaturePainter::getLabelBackgroundTag() const
{
	return LabelBackgroundTag;
}

LineParameters FeaturePainter::foregroundBoundary() const
{
	LineParameters P;
	P.Draw = DrawForeground;
	P.Color = ForegroundColor;
	P.Proportional = ForegroundScale;
	P.Fixed = ForegroundOffset;
	P.Dashed = ForegroundDashSet;
	P.DashOn = ForegroundDash;
	P.DashOff = ForegroundWhite;
	if (!P.Dashed)
		P.DashOff = P.DashOn = 0;
	return P;
}

void FeaturePainter::clearTouchupDash()
{
	TouchupDashSet = false;
}

LineParameters FeaturePainter::touchupBoundary() const
{
	LineParameters P;
	P.Draw = DrawTouchup;
	P.Color = TouchupColor;
	P.Proportional = TouchupScale;
	P.Fixed = TouchupOffset;
	P.Dashed = TouchupDashSet;
	P.DashOn = TouchupDash;
	P.DashOff = TouchupWhite;
	if (!P.Dashed)
		P.DashOff = P.DashOn = 0;
	return P;
}

LineParameters FeaturePainter::labelBoundary() const
{
	LineParameters P;
	P.Draw = DrawLabel;
	P.Color = LabelColor;
	P.Proportional = LabelScale;
	P.Fixed = LabelOffset;
	P.Dashed = false;
	if (!P.Dashed)
		P.DashOff = P.DashOn = 0.0;
	return P;
}

FeaturePainter& FeaturePainter::setIcon(const QString& Name, double Scale, double Offset)
{
	DrawIcon = true;
	IconName = Name;
	IconScale = Scale;
	IconOffset = Offset;
	return *this;
}

IconParameters FeaturePainter::icon() const
{
	IconParameters P;
	P.Draw = DrawIcon;
	P.Name = IconName;
	P.Proportional = IconScale;
	P.Fixed = IconOffset;
	return P;
}

FeaturePainter& FeaturePainter::iconActive(bool b)
{
	DrawIcon = b;
	return *this;
}

void FeaturePainter::setSelector(const QString& anExpression)
{
	delete theSelector;
	theSelector = TagSelector::parse(anExpression);
}

void FeaturePainter::setSelector(TagSelector* aSel)
{
	delete theSelector;
	theSelector = aSel;
}

TagSelectorMatchResult FeaturePainter::matchesTag(const MapFeature* F) const
{
	TagSelectorMatchResult res;

	if (!theSelector) return TagSelect_NoMatch;
	// Special casing for multipolygon roads
	if (const Road* R = dynamic_cast<const Road*>(F))
	{
		// TODO create a isPartOfMultiPolygon(R) function for this
		for (unsigned int i=0; i<R->sizeParents(); ++i)
		{
			if (const Relation* Parent = dynamic_cast<const Relation*>(R->getParent(i)))
				if (Parent->tagValue("type","") == "multipolygon")
					return TagSelect_NoMatch;
		}
	}
	if ((res = theSelector->matches(F)))
		return res;
	// Special casing for multipolygon relations
	if (const Relation* R = dynamic_cast<const Relation*>(F))
	{
		for (unsigned int i=0; i<R->size(); ++i)
			if ((res = theSelector->matches(R->get(i))))
				return res;
	}
	return TagSelect_NoMatch;
}

bool FeaturePainter::matchesZoom(double PixelPerM) const
{
	if (ZoomLimitSet)
		return (ZoomUnder <= PixelPerM) && (PixelPerM <= ZoomUpper);
	return true;
}
/*
void buildPathFromRoad(Road *R, const Projection &theProjection, QPainterPath &Path)
{
	Path.moveTo(theProjection.project(R->get(0)->position()));
	for (unsigned int i=1; i<R->size(); ++i)
		Path.lineTo(theProjection.project(R->get(i)->position()));
}

void buildPathFromRelation(Relation *R, const Projection &theProjection, QPainterPath &Path)
{
	for (unsigned int i=0; i<R->size(); ++i)
		if (Road* M = dynamic_cast<Road*>(R->get(i)))
			buildPathFromRoad(M, theProjection, Path);
}

*/
void FeaturePainter::drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawBackground) return;
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf(R)*BackgroundScale+BackgroundOffset;
	if (WW < 0) return;
	QPen thePen(BackgroundColor,WW);
	thePen.setCapStyle(Qt::RoundCap);
	thePainter.strokePath(R->getPath(),thePen);
}

void FeaturePainter::drawBackground(Relation* R, QPainter& thePainter, const Projection& /* theProjection */) const
{
	if (!DrawBackground) return;
//	double PixelPerM = theProjection.pixelPerM();
//	double WW = PixelPerM*widthOf(R)*BackgroundScale+BackgroundOffset;
	double WW = BackgroundOffset;
	if (WW < 0) return;
	QPen thePen(BackgroundColor,WW);
	thePen.setCapStyle(Qt::RoundCap);
	thePainter.strokePath(R->getPath(),thePen);
}

void FeaturePainter::drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawForeground && !ForegroundFill) return;

	double WW = 0.0;
	if (DrawForeground)
	{
		double PixelPerM = theProjection.pixelPerM();
		WW = PixelPerM*widthOf(R)*ForegroundScale+ForegroundOffset;
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
	thePainter.drawPath(R->getPath());
}

void FeaturePainter::drawForeground(Relation* R, QPainter& thePainter, const Projection& /* theProjection */) const
{
	if (!DrawForeground && !ForegroundFill) return;

	double WW = 0.0;
	if (DrawForeground)
	{
//		double PixelPerM = theProjection.pixelPerM();
//		double WW = PixelPerM*widthOf(R)*ForegroundScale+ForegroundOffset;
		WW = ForegroundOffset;
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
	if (ForegroundFill)
	{
		QBrush theBrush(ForegroundFillFillColor);
		thePainter.setBrush(theBrush);
	}
	else
		thePainter.setBrush(QBrush(Qt::NoBrush));
	thePainter.drawPath(R->getPath());
}


void FeaturePainter::drawTouchup(TrackPoint* Pt, QPainter& thePainter, const Projection& theProjection) const
{
	if (DrawIcon && (IconName != "") )
	{
		double PixelPerM = theProjection.pixelPerM();
		double WW = PixelPerM*IconScale+IconOffset;

        QPixmap pm = getPixmapFromFile(IconName,WW);
        QPoint C(theProjection.project(Pt->position()));
        thePainter.fillRect(QRect(C-QPoint(2,2),QSize(4,4)),QColor(0,0,0,128));
        thePainter.drawPixmap( int(C.x()-pm.width()/2), int(C.y()-pm.height()/2) , pm);
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
			thePainter.strokePath(R->getPath(),thePen);
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
					QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
					QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));
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

#define LABEL_PATH_DISTANCE 3
#define LABEL_STRAIGHT_DISTANCE 50
#define BG_SPACING 6
#define BG_PEN_SZ 2

void FeaturePainter::drawLabel(TrackPoint* Pt, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawLabel)
		return;

	QString str = Pt->tagValue(getLabelTag(), "");
	QString strBg = Pt->tagValue(getLabelBackgroundTag(), "");
	if (str.isEmpty() && strBg.isEmpty())
		return;

	LineParameters lp = labelBoundary();
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*lp.Proportional+lp.Fixed;
	if (WW < 10) return;

	QFont font = getLabelFont();
	font.setPixelSize(int(WW));
    QFontMetrics metrics(font);

	int modX = 0;
	int modY = 0;
	QPainterPath textPath;
	QPainterPath bgPath;
	QPoint C(theProjection.project(Pt->position()));

	if (!str.isEmpty()) {
		modX = - (metrics.width(str)/2);
		if (DrawIcon && (IconName != "") )
		{
			QPixmap pm(IconName);
			modY = - pm.height();
			if (DrawLabelBackground)
				modY -= BG_SPACING;
		}

		thePainter.save();

		textPath.addText(modX, modY, font, str);
		thePainter.translate(C);

		thePainter.setPen(QPen(Qt::white, WW/5));
		thePainter.drawPath(textPath);
	}
	if (DrawLabelBackground && !strBg.isEmpty()) {
		modX = - (metrics.width(strBg)/2);
		if (DrawIcon && (IconName != "") )
		{
			QPixmap pm(IconName);
			modY = - pm.height();
			if (DrawLabelBackground)
				modY -= BG_SPACING;
		}

		thePainter.save();
		textPath.addText(modX, modY, font, strBg);
		thePainter.translate(C);

		bgPath.addRect(textPath.boundingRect().adjusted(-BG_SPACING, -BG_SPACING, BG_SPACING, BG_SPACING));
		thePainter.setPen(QPen(LabelColor, BG_PEN_SZ));
		thePainter.setBrush(LabelBackgroundColor);
		thePainter.drawPath(bgPath);
	}
	thePainter.setPen(Qt::NoPen);
	thePainter.setBrush(LabelColor);
	thePainter.drawPath(textPath);

	thePainter.restore();

	if (DrawLabelBackground && !strBg.isEmpty()) {
		QRegion rg = thePainter.clipRegion();
		rg -= textPath.boundingRect().toRect().translated(C);
		thePainter.setClipRegion(rg);
	}
}


void FeaturePainter::drawLabel(Road* R, QPainter& thePainter, const Projection& theProjection) const
{
	if (!DrawLabel)
		return;

	QString str = R->tagValue(getLabelTag(), "");
	QString strBg = R->tagValue(getLabelBackgroundTag(), "");
	if (str.isEmpty() && strBg.isEmpty())
		return;

	LineParameters lp = labelBoundary();
	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf(R)*lp.Proportional+lp.Fixed;
	if (WW < 10) return;
	double WWR = qMax(PixelPerM*widthOf(R)*BackgroundScale+BackgroundOffset, PixelPerM*widthOf(R)*ForegroundScale+ForegroundOffset);

    QFont font = getLabelFont();

	if (!str.isEmpty()) {
		QRegion rg = thePainter.clipRegion();
		font.setPixelSize(int(WW));
		QFontMetrics metrics(font);

		for (int i=int(WW)-1; i> 0; --i) {
			font.setPixelSize(i);
			metrics = QFontMetrics(font);
			if ((metrics.width(str) < R->getPath().length() - 5) && (metrics.height() < WWR))
				break;
		}
		if (font.pixelSize() >= 5) {
			thePainter.setPen(LabelColor);
			thePainter.setFont(font);
			int repeat = int((R->getPath().length() / (metrics.width(str) * LABEL_PATH_DISTANCE)) - 0.5);
			int numSegment = repeat+1;
			qreal lenSegment = R->getPath().length() / numSegment;
			qreal startSegment = 0;
			do {
				QRegion rg = thePainter.clipRegion();

				qreal curLen = startSegment + ((lenSegment - metrics.width(str)) / 2);
				int modIncrement = 1;
				qreal modAngle = 0;
				int modY = 0;
				if (cos(angToRad(R->getPath().angleAtPercent((startSegment+(lenSegment/2))/R->getPath().length()))) < 0) {
					modIncrement = -1;
					modAngle = 180.0;
					curLen += metrics.width(str);
				}
				for (int i = 0; i < str.length(); ++i) {
					qreal t = R->getPath().percentAtLength(curLen);
					QPointF pt = R->getPath().pointAtPercent(t);
					qreal angle = R->getPath().angleAtPercent(t);
					QString txt;
					txt.append(str[i]);
					thePainter.save();
					thePainter.translate(pt);
					//qDebug()<<"txt = "<<txt<<", angle = "<<angle<<curLen<<t;
					thePainter.rotate(-angle+modAngle);
					//p->thePainter.drawText(QRect(0, modY, metrics.width(txt), WW), Qt::AlignVCenter, txt);
					modY = (metrics.ascent()/2)-2;
					thePainter.drawText(0, modY, txt);
					thePainter.restore();

					//rg -= QRect(0, modY, metrics.width(txt), metrics.height()).translated(pt.toPoint());

					qreal incremenet = metrics.width(txt);
					curLen += (incremenet * modIncrement);
				}
				startSegment += lenSegment;
			} while (--repeat >= 0);

			thePainter.setClipRegion(rg);
		}
	}
	if (DrawLabelBackground && !strBg.isEmpty()) {
		QRegion rg = thePainter.clipRegion();
		font.setPixelSize(int(WW));
		QFontMetrics metrics(font);

		int repeat = int((R->getPath().length() / (metrics.width(strBg) * LABEL_STRAIGHT_DISTANCE)) - 0.5);
		int numSegment = repeat+1;
		qreal lenSegment = R->getPath().length() / numSegment;
		qreal startSegment = 0;
		do {

			int modX = 0;
			int modY = 0;

			qreal curLen = startSegment + (lenSegment / 2);
			qreal t = R->getPath().percentAtLength(curLen);
			QPointF pt = R->getPath().pointAtPercent(t);

			modX = - (metrics.width(strBg)/2);
			//modX = WW;
			modY = (metrics.ascent()/2);

			QPainterPath textPath, bgPath;
			textPath.addText(modX, modY, font, strBg);
			bgPath.addRect(textPath.boundingRect().adjusted(-BG_SPACING, -BG_SPACING, BG_SPACING, BG_SPACING));

			bool rgContains = false;
			for (int i=0; i<rg.rects().size(); i++) {
				if (rg.rects()[i].contains(bgPath.boundingRect().toRect().translated(pt.toPoint()))) {
					rgContains = true;
					break;
				}
			}
			if (rgContains) {
				thePainter.save();
				thePainter.translate(pt);

				thePainter.setPen(QPen(LabelColor, BG_PEN_SZ));
				thePainter.setBrush(LabelBackgroundColor);
				thePainter.drawPath(bgPath);

				thePainter.setPen(Qt::NoPen);
				thePainter.setBrush(LabelColor);
				thePainter.drawPath(textPath);

				thePainter.restore();

				rg -= bgPath.boundingRect().toRect().translated(pt.toPoint());
			}

			startSegment += lenSegment;
		} while (--repeat >= 0);

		thePainter.setClipRegion(rg);
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

