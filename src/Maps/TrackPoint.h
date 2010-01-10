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
#include <ggl/ggl.hpp>
#include <ggl/geometries/register/point.hpp>
#endif

class TrackPointPrivate;
class QProgressDialog;

class TrackPoint : public MapFeature
{
	Q_OBJECT

	public:
		TrackPoint(const Coord& aCoord);
		TrackPoint(const TrackPoint& other);
		virtual ~TrackPoint();

		virtual QString getClass() const {return "TrackPoint";}
		virtual MapFeature::FeatureType getType() const {return MapFeature::Nodes;}
		virtual void updateMeta();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, MapView* theView);
		virtual void drawFocus(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHover(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHighlight(QPainter& P, MapView* theView, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded();
		virtual QString description() const;
		virtual const RenderPriority& renderPriority();

		virtual int find(MapFeature* Pt) const;
		virtual void remove(int idx);
		virtual void remove(MapFeature* F);
		virtual int size() const;
		virtual MapFeature* get(int idx);
		virtual const MapFeature* get(int Idx) const;
		virtual bool isNull() const;
		virtual bool isInteresting() const;
		virtual bool isPOI() const;
		virtual bool isWaypoint();

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
		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport=false);
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

private:
		TrackPointPrivate* p;

};

Q_DECLARE_METATYPE( TrackPoint * );

#ifndef _MOBILE
// Register this point as being a recognizable point by the GGL
//GEOMETRY_REGISTER_POINT_2D_CONST(TrackPoint, qreal, cs::cartesian, projection().x(), projection().y())

namespace ggl { namespace traits {

template<> struct tag<TrackPointPtr>
{ typedef point_tag type; };

template<> struct coordinate_type<TrackPointPtr>
{ typedef qreal type; };

template<> struct coordinate_system<TrackPointPtr>
{ typedef cs::cartesian type; };

template<> struct dimension<TrackPointPtr>
	: boost::mpl::int_<2> {};

template<>
struct access<TrackPointPtr>
{
	template <std::size_t I>
	static inline qreal get(const TrackPointPtr& p)
	{
		return I == 0 ? p->projection().x() : p->projection().y();
	}

//    template <std::size_t I>
//    static inline void set(TrackPointPtr& p, const qreal& value)
//    {
//        // Or (better) implement an accessor with specializations
//        if (I == 0) p->position().setLon(value);
//        else if (I == 1) p->position().setLat(value);
//    }

};

}} // namespace ggl::traits

#endif


#endif


