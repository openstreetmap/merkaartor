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
class QDomElement;

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

class IconParameters
{
	public:
		bool Draw;
		QString Name;
		double Proportional;
		double Fixed;
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
		FeaturePainter& background(QColor Color, double Scale, double Offset);
		FeaturePainter& foregroundActive(bool b);
		FeaturePainter& foreground(QColor Color, double Scale, double Offset);
		FeaturePainter& foregroundDash(double Dash, double White);
		FeaturePainter& touchupActive(bool b);
		FeaturePainter& touchup(QColor Color, double Scale, double Offset);
		FeaturePainter& touchupDash(double Dash, double White);
		FeaturePainter& foregroundFill(QColor FillColor);
		FeaturePainter& zoomBoundary(double anUnder, double anUpper);
		FeaturePainter& drawTrafficDirectionMarks();
		FeaturePainter& trackPointIcon(const QString& Filename);
		FeaturePainter& fillActive(bool b);
		FeaturePainter& iconActive(bool b);
		FeaturePainter& setIcon(const QString& Name, double Scale, double Offset);
		FeaturePainter& labelActive(bool b);
		FeaturePainter& labelTag(const QString& val);
		FeaturePainter& label(QColor Color, double Scale, double Offset);
		FeaturePainter& setLabelFont(const QString& descFont);
		FeaturePainter& labelBackgroundActive(bool b);
		FeaturePainter& labelBackground(QColor bgColor);
		FeaturePainter& labelBackgroundTag(const QString& val);
		FeaturePainter& labelHalo(bool b);
		FeaturePainter& labelArea(bool b);

		QString userName() const;
		std::pair<double, double> zoomBoundaries() const;
		LineParameters backgroundBoundary() const;
		LineParameters foregroundBoundary() const;
		LineParameters labelBoundary() const;
		IconParameters icon() const;
		void clearForegroundDash();
		LineParameters touchupBoundary() const;
		void clearTouchupDash();
		QColor fillColor() const;
		QColor labelBackgroundColor() const;
		QFont getLabelFont() const;
		QString getLabelTag() const;
		QString getLabelBackgroundTag() const;
		bool getLabelHalo() const;
		bool getLabelArea() const;

		QString toXML(QString filename) const;
		static FeaturePainter fromXML(const QDomElement& e, QString filename);

		void drawBackground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawBackground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawForeground(Relation* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawTouchup(TrackPoint* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawLabel(Road* R, QPainter& thePainter, const Projection& theProjection) const;
		void drawPointLabel(QPoint C, QString str, QString strBG, QPainter& thePainter, const Projection& theProjection) const;
		void drawLabel(TrackPoint* Pt, QPainter& thePainter, const Projection& theProjection) const;

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
		QString IconName;
		double IconScale;
		double IconOffset;
		bool DrawLabel;
		QString LabelTag;
		QColor LabelColor;
		double LabelScale;
		double LabelOffset;
		bool DrawLabelBackground;
		QColor LabelBackgroundColor;
		QString LabelBackgroundTag;
		QFont LabelFont;
		bool LabelHalo;
		bool LabelArea;
};

class GlobalPainter
{
	public:
		GlobalPainter();
		GlobalPainter(const GlobalPainter& f);
		GlobalPainter& operator=(const GlobalPainter& F);
		~GlobalPainter();

		GlobalPainter& backgroundActive(bool b);
		GlobalPainter& background(QColor Color);

		bool getDrawBackground() const;
		QColor getBackgroundColor() const;

		QString toXML() const;
		static GlobalPainter fromXML(const QDomElement& e);

	public:
		bool DrawBackground;
		QColor BackgroundColor;
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
	protected:
		QVector<PaintStyleLayer*> Layers;
};

#endif

