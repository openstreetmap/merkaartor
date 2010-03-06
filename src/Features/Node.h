#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "Preferences/MerkaartorPreferences.h"

#include "Maps/Coord.h"
#include "Feature.h"
#include "Document.h"
#include "Layer.h"

#include <QtCore/QDateTime>
#include <QtXml>

#ifndef _MOBILE
#include <ggl/ggl.hpp>
#include <ggl/geometries/register/point.hpp>
#endif

class NodePrivate;
class QProgressDialog;

class Node : public Feature
{
	Q_OBJECT

	public:
		Node(const Coord& aCoord);
		Node(const Node& other);
		virtual ~Node();

		virtual QString getClass() const {return "TrackPoint";}
		virtual Feature::FeatureType getType() const {return Feature::Nodes;}
		virtual void updateMeta();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, MapView* theView);
		virtual void drawFocus(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHover(QPainter& P, MapView* theView, bool solid=true);
		virtual void drawHighlight(QPainter& P, MapView* theView, bool solid=true);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, bool selectNodes, const Projection& theProjection, const QTransform& theTransform) const;
		virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives);
		virtual bool notEverythingDownloaded();
		virtual QString description() const;

		virtual int find(Feature* Pt) const;
		virtual void remove(int idx);
		virtual void remove(Feature* F);
		virtual int size() const;
		virtual Feature* get(int idx);
		virtual const Feature* get(int Idx) const;
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

		virtual void partChanged(Feature* F, int ChangeId);

		virtual QString toXML(int lvl=0, QProgressDialog * progress=NULL);
		virtual bool toXML(QDomElement xParent, QProgressDialog & progress);
		virtual bool toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport=false);
		static Node* fromXML(Document* d, Layer* L, const QDomElement e);
		static Node* fromGPX(Document* d, Layer* L, const QDomElement e);

		virtual QString toHtml();

		virtual void toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex);
		static Node* fromBinary(Document* d, OsbLayer* L, QDataStream& ds, qint8 c, qint64 id);

	private:
		Coord Position;

		double Elevation;
		double Speed;
		QPointF Projected;

	private:
		NodePrivate* p;

};

Q_DECLARE_METATYPE( Node * );

#ifndef _MOBILE
// Register this point as being a recognizable point by the GGL
//GEOMETRY_REGISTER_POINT_2D_CONST(TrackPoint, qreal, cs::cartesian, projection().x(), projection().y())

namespace ggl { namespace traits {

template<> struct tag<NodePtr>
{ typedef point_tag type; };

template<> struct coordinate_type<NodePtr>
{ typedef qreal type; };

template<> struct coordinate_system<NodePtr>
{ typedef cs::cartesian type; };

template<> struct dimension<NodePtr>
	: boost::mpl::int_<2> {};

template<>
struct access<NodePtr>
{
	template <std::size_t I>
	static inline qreal get(const NodePtr& p)
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


