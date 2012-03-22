#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "MerkaartorPreferences.h"

#include "Coord.h"
#include "Feature.h"
#include "Document.h"
#include "Layer.h"

#include <QtCore/QDateTime>
#include <QtXml>

#ifndef _MOBILE
#if QT_VERSION < 0x040700
#include <ggl/ggl.hpp>
#include <ggl/geometries/register/point.hpp>
#endif
#endif

class QProgressDialog;

class Node : public Feature
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

public:
    Node()
        : ProjectionRevision(0)
    {
    }

    Node(const Coord& aCoord);
    Node(const Node& other);
    virtual ~Node();

    quint16 ProjectionRevision;
    QPointF Projected;

    bool IsWaypoint;
    bool IsPOI;

public:
    virtual QString getClass() const {return "Node";}
    virtual char getType() const {return IFeature::Point;}
    virtual void updateMeta();

    virtual const CoordBox& boundingBox(bool update=true) const;
    virtual void draw(QPainter& P, MapRenderer* theRenderer);
    virtual void drawSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawParentsSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawChildrenSpecial(QPainter& P, QPen& Pen, MapView* theView, int depth);

    virtual qreal pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const;
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
    virtual bool isPOI();
    virtual bool isWaypoint();

    /** check if the feature is drawable
         * @return true if to be drawn
         */
    virtual bool isSelectable(qreal PixelPerM, RendererOptions options);

    virtual void partChanged(Feature* F, int ChangeId);

public:
    const QPointF& projected() const;
    const QPointF &projected(const Projection &aProjection);
    void setProjection(const Projection& aProjection);

    Coord position() const;
    void setPosition(const Coord& aCoord);

    bool toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict=false, QString changetsetid="");
    static Node* fromXML(Document* d, Layer* L, QXmlStreamReader& stream);

    bool toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, QString element, bool forExport=false);

    QString toHtml();
};

class TrackNode : public Node
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

protected:
    TrackNode(const Coord& aCoord);
    TrackNode(const Node& other);
    TrackNode(const TrackNode& other);

public:
    qreal speed() const;
    void setSpeed(qreal aSpeed);

    qreal elevation() const;
    void setElevation(qreal aElevation);

#ifdef FRISIUS_BUILD
    const QDateTime& time() const;
    void setTime(const QDateTime& aTime);
    void setTime(uint epoch);
#endif

    virtual bool toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, QString element, bool forExport=false);
    static TrackNode* fromGPX(Document* d, Layer* L, QXmlStreamReader& stream);

    virtual QString toHtml();

private:
#ifdef FRISIUS_BUILD
    uint Time;
#endif
    qreal Elevation;
    qreal Speed;
};

class PhotoNode : public TrackNode
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

protected:
    PhotoNode(const Coord& aCoord);
    PhotoNode(const Node& other);
    PhotoNode(const TrackNode& other);
    virtual ~PhotoNode();

public:
    virtual void draw(QPainter &thePainter, MapRenderer* theRenderer);
#ifdef GEOIMAGE
    virtual void drawHover(QPainter& P, MapView* theView);
#endif
    virtual qreal pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const;

    QPixmap photo() const;
    void setPhoto(QPixmap thePhoto);

protected:
    QPixmap* Photo;
    mutable bool photoLocationBR;
};


Q_DECLARE_METATYPE( Node * );

#ifndef _MOBILE
#if QT_VERSION < 0x040700
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


#endif


