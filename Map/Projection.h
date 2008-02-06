#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "Map/Coord.h"

#include <QtCore/QPointF>

class QRect;
class LayerManager;

class Projection
{
	public:
		Projection(void);
		virtual ~Projection(void) {};

		virtual void setViewport(const CoordBox& Map, const QRect& Screen) = 0;
		virtual void panScreen(const QPoint& p, const QRect& Screen) = 0;
		virtual CoordBox viewport() const;
		virtual QPointF project(const Coord& Map) const = 0;
		virtual double pixelPerM() const;
		virtual double latAnglePerM() const;
		virtual double lonAnglePerM(double Lat) const;
		virtual Coord inverse(const QPointF& Screen) const = 0;
		virtual void zoom(double d, const QPointF& Around, const QRect& Screen) = 0;
	protected:
		double ScaleLat, DeltaLat, ScaleLon, DeltaLon;
		CoordBox Viewport;
};

class DrawingProjection : public Projection
{
	public:
		DrawingProjection(void);
		~DrawingProjection(void);

		void setViewport(const CoordBox& Map, const QRect& Screen);
		void panScreen(const QPoint& p, const QRect& Screen);
		QPointF project(const Coord& Map) const;
		Coord inverse(const QPointF& Screen) const;
		void zoom(double d, const QPointF& Around, const QRect& Screen);
	private:
		void viewportRecalc(const QRect& Screen);
};

class ImageProjection : public Projection
{
	public:
		ImageProjection(void);
		~ImageProjection(void);

		void setViewport(const CoordBox& Map, const QRect& Screen);
		void panScreen(const QPoint& p, const QRect& Screen);
		QPointF project(const Coord& Map) const;
		Coord inverse(const QPointF& Screen) const;
		void zoom(double d, const QPointF& Around, const QRect& Screen);
		void setLayerManager(LayerManager* lm);
	private:
		void viewportRecalc(const QRect& Screen);
		QPointF screenToCoordinate(QPoint click);
		QPoint coordinateToScreen(QPointF click) const;
	private:
		LayerManager* layermanager;
		QPoint screen_middle;
};
#endif


