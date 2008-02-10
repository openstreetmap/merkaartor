#ifndef MERKAARTOR_PAINTSTYLE_H_
#define MERKAARTOR_PAINTSTYLE_H_

#include <QtCore/QString>
#include <QtGui/QColor>

#include <vector>

class MapFeature;
class Projection;
class Relation;
class Road;
class TrackPoint;
class QPainter;
class QPainterPath;

void buildPathFromRoad(Road *R, const Projection& theProjection, QPainterPath &Path);

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

		bool isFilled() const;
		bool matchesTag(const MapFeature* F) const;
		bool matchesZoom(double PixelPerM) const;
		FeaturePainter& selectOnTag(const QString& Tag, const QString& Value);
		FeaturePainter& selectOnTag(const QString& Tag, const QString& Value1, const QString& Value2);
		FeaturePainter& background(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foreground(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foregroundDash(double Dash, double White);
		FeaturePainter& touchup(const QColor& Color, double Scale, double Offset);
		FeaturePainter& touchupDash(double Dash, double White);
		FeaturePainter& foregroundFill(const QColor& FillColor);
		FeaturePainter& zoomBoundary(double anUnder, double anUpper);
		FeaturePainter& drawTrafficDirectionMarks();
		FeaturePainter& trackPointIcon(const QString& Filename);

		QString userName() const;
		const std::vector<std::pair<QString, QString> >& tagSelectors() const;
		void clearSelectors();
		std::pair<double, double> zoomBoundaries() const;
		LineParameters backgroundBoundary() const;
		LineParameters foregroundBoundary() const;
		void clearForegroundDash();
		LineParameters touchupBoundary() const;
		void clearTouchupDash();
		QColor fillColor() const;
		QString iconName() const;

		QString asXML() const;

		void drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawBackground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(TrackPoint* R, QPainter& thePainter, const Projection& theProjection) const;
	private:
		std::vector<std::pair<QString, QString> > OneOfTheseTags;

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
		QString TrackPointIconName;
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

