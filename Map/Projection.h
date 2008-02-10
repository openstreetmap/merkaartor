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

		void setViewport(const CoordBox& Map, const QRect& Screen);
		void panScreen(const QPoint& p, const QRect& Screen);
		CoordBox viewport() const;
		QPointF project(const Coord& Map) const;
		double pixelPerM() const;
		double latAnglePerM() const;
		double lonAnglePerM(double Lat) const;
		Coord inverse(const QPointF& Screen) const;
		void zoom(double d, const QPointF& Around, const QRect& Screen);

		void setLayerManager(LayerManager* lm);
	protected:
		double ScaleLat, DeltaLat, ScaleLon, DeltaLon;
		CoordBox Viewport;
		QPoint screen_middle;
		LayerManager* layermanager;
	private:
		void viewportRecalc(const QRect& Screen);
		void layerManagerSetViewport(const CoordBox& Map, const QRect& Screen);
		QPointF screenToCoordinate(QPointF click) const;
		QPoint coordinateToScreen(QPointF click) const;
};

#endif


