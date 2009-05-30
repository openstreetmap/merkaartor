#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "Preferences/MerkaartorPreferences.h"

#include "Maps/Coord.h"
#include "Maps/MapFeature.h"
#include "Maps/MapDocument.h"
#include "Maps/MapLayer.h"

#include <QtCore/QDateTime>
#include <QtXml>

#ifndef _MOBILE
#include <geometry/geometry.hpp>
#include <geometry/geometries/register/register_point.hpp>
#endif

class QProgressDialog;

class TrackPoint : public MapFeature
{
	Q_OBJECT

	public:
		TrackPoint(const Coord& aCoord);
		TrackPoint(const TrackPoint& other);
		virtual ~TrackPoint();

		virtual QString getClass() const {return "TrackPoint";}

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual void drawHover(QPainter& P, const Projection& theProjection, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;
		virtual RenderPriority renderPriority(double aPixelPerM);

		virtual int find(MapFeature* Pt) const;
		virtual void remove(int idx);
		virtual void remove(MapFeature* F);
		virtual int size() const;
		virtual MapFeature* get(int idx);
		virtual const MapFeature* get(int Idx) const;
		virtual bool isNull() const;
		virtual bool isInteresting() const;

		const Coord& position() const;
		void setPosition(const Coord& aCoord);
		const QPointF& projection() const;
		void setProjection(const QPointF& aProjection);
#ifndef _MOBILE
		int projectionRevision() const;
		void setProjectionRevision(int aProjectionRevision);
#endif

		double speed() const;
		void setSpeed(double aSpeed);

		double elevation() const;
		void setElevation(double aElevation);

		virtual void partChanged(MapFeature* F, int ChangeId);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress);
		static TrackPoint* fromXML(MapDocument* d, MapLayer* L, const QDomElement e);
		static TrackPoint* fromGPX(MapDocument* d, MapLayer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static TrackPoint* fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id);

private:
		Coord Position;
		CoordBox BBox;

		double Elevation;
		double Speed;
		QPointF Projected;
#ifndef _MOBILE
		int ProjectionRevision;
#endif
};

Q_DECLARE_METATYPE( TrackPoint * );

#ifndef _MOBILE
// Register this point as being a recognizable point by the GGL
GEOMETRY_REGISTER_POINT_2D_CONST(TrackPoint, qreal, cs::cartesian, projection().x(), projection().y())
#endif


#endif


