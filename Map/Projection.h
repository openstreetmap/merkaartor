#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "Preferences/MerkaartorPreferences.h"
#include "Map/Coord.h"

#ifdef USE_PROJ
#include <proj_api.h>
#endif

#include <QPointF>

#define LAYERMANAGER_OK (layermanager && layermanager->getLayer())
#define BGPROJ_SELECTED (theProjectionType == Proj_Background)

class QRect;
class LayerManager;
class TrackPoint;

class Projection
{
	public:
		Projection(void);
		virtual ~Projection(void) {};

		void setViewport(const CoordBox& Map, const QRect& Screen);
		void panScreen(const QPoint& p, const QRect& Screen);
		CoordBox viewport() const;
		double pixelPerM() const;
		double latAnglePerM() const;
		double lonAnglePerM(double Lat) const;
#ifdef USE_PROJ
		QPoint projProject(const Coord& Map) const;
		Coord projInverse(const QPointF& Screen) const;
#endif
		QPoint project(const Coord& Map) const;
		QPoint project(TrackPoint* aNode) const;
		Coord inverse(const QPointF& Screen) const;
		void zoom(double d, const QPointF& Around, const QRect& Screen);
		void setCenter(Coord& Center, const QRect& Screen);
		void resize(QSize oldS, QSize newS);

		void setLayerManager(LayerManager* lm);

		virtual bool toXML(QDomElement xParent) const;
		void fromXML(QDomElement e, const QRect & Screen);

		void setProjectionType(ProjectionType aProjectionType);

	protected:
		double ScaleLat, ScaleLon;
		int DeltaLat, DeltaLon;
		double PixelPerM;
		CoordBox Viewport;
		QPoint screen_middle;
		LayerManager* layermanager;
		ProjectionType theProjectionType;
#ifdef USE_PROJ
		projPJ theProj;
#endif

	private:
		void viewportRecalc(const QRect& Screen);
		void layerManagerSetViewport(const CoordBox& Map, const QRect& Screen);
		void layerManagerViewportRecalc(const QRect& Screen);
		QPointF screenToCoordinate(QPointF click) const;
		QPoint coordinateToScreen(QPointF click) const;
};

#endif


