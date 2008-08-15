#ifndef MERKAARTOR_PAINTSTYLE_H_
#define MERKAARTOR_PAINTSTYLE_H_

#include <QtCore/QString>
#include <QtGui/QColor>
#include <QFont>

#include <vector>

#include "PaintStyle/TagSelector.h"

class MapFeature;
class Projection;
class Relation;
class Road;
class TagSelector;
class TrackPoint;
class QPainter;
class QPainterPath;
class QFont;

class LineParameters
{
	public:
		bool Draw;
		bool Dashed;
		QColor Color;
		double Proportional;
		double Fixed;
		double DashOn;
		double DashOff;
};

class FeaturePainter
{
	public:
		typedef enum {NoZoomLimit, GlobalZoom, RegionalZoom, LocalZoom } ZoomType;

		FeaturePainter();
		FeaturePainter(const FeaturePainter& f);
		FeaturePainter& operator=(const FeaturePainter& F);
		~FeaturePainter();

		void setSelector(const QString& aName);
		void setSelector(TagSelector* aSelector);
		bool isFilled() const;
		TagSelectorMatchResult matchesTag(const MapFeature* F) const;
		bool matchesZoom(double PixelPerM) const;
		FeaturePainter& backgroundActive(bool b);
		FeaturePainter& background(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foregroundActive(bool b);
		FeaturePainter& foreground(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foregroundDash(double Dash, double White);
		FeaturePainter& touchupActive(bool b);
		FeaturePainter& touchup(const QColor& Color, double Scale, double Offset);
		FeaturePainter& touchupDash(double Dash, double White);
		FeaturePainter& foregroundFill(const QColor& FillColor);
		FeaturePainter& zoomBoundary(double anUnder, double anUpper);
		FeaturePainter& drawTrafficDirectionMarks();
		FeaturePainter& trackPointIcon(const QString& Filename);
		FeaturePainter& fillActive(bool b);
		FeaturePainter& iconActive(bool b);
		FeaturePainter& labelActive(bool b);
		FeaturePainter& label(const QColor& Color, double Scale, double Offset);
		FeaturePainter& setLabelFont(const QString& descFont);
		FeaturePainter& labelBackgroundActive(bool b);
		FeaturePainter& labelBackground(const QColor& bgColor);

		QString userName() const;
		std::pair<double, double> zoomBoundaries() const;
		LineParameters backgroundBoundary() const;
		LineParameters foregroundBoundary() const;
		LineParameters labelBoundary() const;
		void clearForegroundDash();
		LineParameters touchupBoundary() const;
		void clearTouchupDash();
		QColor fillColor() const;
		QColor labelBackgroundColor() const;
		QFont getLabelFont() const;
		QString iconName() const;
		bool isIconActive() const;

		QString asXML() const;

		void drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawBackground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(TrackPoint* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawLabel(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawLabel(TrackPoint* R, QPainter& thePainter, const Projection& theProjection) const;
	public:

		TagSelector* theSelector;
		bool ZoomLimitSet;
		double ZoomUnder, ZoomUpper;
		bool DrawBackground;
		QColor BackgroundColor;
		double BackgroundScale;
		double BackgroundOffset;
		bool DrawForeground;
		QColor ForegroundColor;
		double ForegroundScale;
		double ForegroundOffset;
		bool ForegroundDashSet;
		double ForegroundDash, ForegroundWhite;
		bool DrawTouchup;
		QColor TouchupColor;
		double TouchupScale;
		double TouchupOffset;
		bool TouchupDashSet;
		double TouchupDash, TouchupWhite;
		bool ForegroundFill;
		QColor ForegroundFillFillColor;
		bool DrawTrafficDirectionMarks;
		bool DrawIcon;
		QString TrackPointIconName;
		bool DrawLabel;
		QColor LabelColor;
		double LabelScale;
		double LabelOffset;
		bool DrawLabelBackground;
		QColor LabelBackgroundColor;
		QFont LabelFont;
};

class PaintStyleLayer
{
	public:
		virtual ~PaintStyleLayer() = 0;
		virtual void draw(Road* R) = 0;
		virtual void draw(TrackPoint* Pt) = 0;
		virtual void draw(Relation* R) = 0;
};

class PaintStyle
{
	public:
		void add(PaintStyleLayer* aLayer);
		unsigned int size() const;
		PaintStyleLayer* get(unsigned int i);
	private:
		std::vector<PaintStyleLayer*> Layers;
};

#endif

