#ifndef MERKAARTOR_PAINTSTYLE_H_
#define MERKAARTOR_PAINTSTYLE_H_

#include <QtGui/QColor>

#include <vector>

class Projection;
class Road;
class TrackPoint;
class QPainter;
class QPainterPath;
class QString;

void buildPathFromRoad(Road *R, Projection const &theProjection, QPainterPath &Path);

class FeaturePainter
{
	public:
		typedef enum { GlobalZoom, RegionalZoom, LocalZoom, NoZoomLimit } ZoomType;

		FeaturePainter();

		bool isHit(const Road* R, double PixelPerM) const;
		bool isHit(const TrackPoint* Pt, double PixelPerM) const;
		FeaturePainter& selectOnTag(const QString& Tag, const QString& Value);
		FeaturePainter& selectOnTag(const QString& Tag, const QString& Value1, const QString& Value2);
		FeaturePainter& background(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foreground(const QColor& Color, double Scale, double Offset);
		FeaturePainter& foregroundDash(double Dash, double White);
		FeaturePainter& touchup(const QColor& Color, double Scale, double Offset);
		FeaturePainter& touchupDash(double Dash, double White);
		FeaturePainter& foregroundFill(const QColor& FillColor);
		FeaturePainter& limitToZoom(ZoomType aType);
		FeaturePainter& drawTrafficDirectionMarks();
		FeaturePainter& trackPointIcon(const QString& Filename);

		void drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(TrackPoint* R, QPainter& thePainter, const Projection& theProjection) const;
	private:
		std::vector<std::pair<QString, QString> > OneOfTheseTags;

		bool ZoomLimit;
		double PixelPerMZoomLimit;
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
		virtual void draw(Road* R) = 0;
		virtual void draw(TrackPoint* Pt) = 0;
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

