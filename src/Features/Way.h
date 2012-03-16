#ifndef MERKAARTOR_ROAD_H_
#define MERKAARTOR_ROAD_H_

#include <QList>

#include "Document.h"
#include "Feature.h"
#include "Layer.h"

#ifndef _MOBILE
#if QT_VERSION < 0x040700
#include <ggl/ggl.hpp>
#endif
#endif

class WayPrivate;
class Node;
class QProgressDialog;

class Way : public Feature
{
    friend class MemoryBackend;
    friend class SpatialiteBackend;

protected:
    Way(void);
    Way(const Way& other);
    virtual ~Way();

public:
    virtual QString getClass() const {return "Way";}
    virtual char getType() const;
    virtual void updateMeta();

    virtual const CoordBox& boundingBox(bool update=true) const;
    virtual void draw(QPainter& P, MapView* theView);
    virtual void drawSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawParentsSpecial(QPainter& P, QPen& Pen, MapView* theView);
    virtual void drawChildrenSpecial(QPainter& P, QPen& Pen, MapView* theView, int depth);

    virtual qreal pixelDistance(const QPointF& Target, qreal ClearEndDistance, const QList<Feature*>& NoSnap, MapView* theView) const;
    Node* pixelDistanceNode(const QPointF& Target, qreal ClearEndDistance, MapView* theView, const QList<Feature*>& NoSnap, bool NoSelectVirtuals) const;
    virtual void cascadedRemoveIfUsing(Document* theDocument, Feature* aFeature, CommandList* theList, const QList<Feature*>& Alternatives);
    virtual bool notEverythingDownloaded();
    virtual QString description() const;

    virtual void add(Node* Pt);
    virtual void add(Node* Pt, int Idx);
    virtual void remove(int Idx);
    virtual void remove(Feature* F);
    virtual int size() const;
    virtual int find(Feature* Pt) const;
    virtual int findVirtual(Feature* Pt) const;
    virtual Feature* get(int idx);
    virtual const Feature* get(int Idx) const;
    virtual bool isNull() const;
    virtual void setDeleted(bool delState);

    Node* getNode(int idx);
    const Node* getNode(int idx) const;
    const QList<NodePtr>& getNodes() const;
    const QList<NodePtr>& getVirtuals() const;

    int segmentCount();
    QLineF getSegment(int i);
    int bestSegment();

    const RenderPriority& renderPriority();

    bool isNodeAtEnd(Node* node);

    virtual void partChanged(Feature* F, int ChangeId);
    virtual void setLayer(Layer* aLayer);

    qreal area();
    bool isClosed() const;
    qreal distance();
    qreal widthOf();

    virtual bool deleteChildren(Document* theDocument, CommandList* theList);

    const QPainterPath& getPath() const;
    void addPathHole(const QPainterPath &pth);
    void rebuildPath(const Projection &theProjection);
    void buildPath(Projection const &theProjection);

    virtual bool toGPX(QXmlStreamWriter& stream, QProgressDialog * progress, bool forExport=false);
    virtual bool toXML(QXmlStreamWriter& stream, QProgressDialog * progress, bool strict=false, QString changetsetid="");
    static Way* fromXML(Document* d, Layer* L, QXmlStreamReader& stream);

    virtual QString toHtml();

    bool isExtrimity(Node* node);
    static Way * GetSingleParentRoad(Feature * mapFeature);
    static Way * GetSingleParentRoadInner(Feature * mapFeature);

    static int createJunction(Document* theDocument, CommandList* theList, Way* R1, Way* R2, bool doIt);

protected:
    WayPrivate* p;
};

Q_DECLARE_METATYPE( Way * );

Feature::TrafficDirectionType trafficDirection(const Way* R);
int findSnapPointIndex(const Way* R, Coord& P);

#endif


