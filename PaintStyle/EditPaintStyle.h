#ifndef MERKAARTOR_EDITPAINTSTYLE_H_
#define MERKAARTOR_EDITPAINTSTYLE_H_

#include "PaintStyle.h"

#include <QtGui/QColor>

#include <utility>
#include <vector>

class EditPaintStylePrivate;
class MapFeature;
class Projection;
class Road;
class QPainter;
class QString;

class FeaturePaintSelector
{
	public:
		typedef enum { GlobalZoom, RegionalZoom, LocalZoom, NoZoomLimit } ZoomType;

		FeaturePaintSelector();

		bool isHit(const Road* R, double PixelPerM) const;
		bool isHit(const TrackPoint* Pt, double PixelPerM) const;
		FeaturePaintSelector& selectOnTag(const QString& Tag, const QString& Value);
		FeaturePaintSelector& selectOnTag(const QString& Tag, const QString& Value1, const QString& Value2);
		FeaturePaintSelector& background(const QColor& Color, double Scale, double Offset);
		FeaturePaintSelector& foreground(const QColor& Color, double Scale, double Offset);
		FeaturePaintSelector& foregroundDash(double Dash, double White);
		FeaturePaintSelector& touchup(const QColor& Color, double Scale, double Offset);
		FeaturePaintSelector& touchupDash(double Dash, double White);
		FeaturePaintSelector& foregroundFill(const QColor& FillColor);
		FeaturePaintSelector& limitToZoom(ZoomType aType);
		FeaturePaintSelector& drawTrafficDirectionMarks();
		FeaturePaintSelector& trackPointIcon(const QString& Filename);

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

class EditPaintStyle : public EmptyPaintStyle
{
	public:
		EditPaintStyle(QPainter& P, const Projection& theProjection);
		virtual ~EditPaintStyle();
		
		virtual PaintStyle* firstLayer();
		virtual PaintStyle* nextLayer();

	private:
		EditPaintStylePrivate* p;
};

#endif