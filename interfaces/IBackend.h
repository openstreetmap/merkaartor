#ifndef IBACKEND_H
#define IBACKEND_H

#include <QPointF>
#include <QRectF>
#include <QList>

class Feature;
class Node;
class Way;
class Relation;
class TrackSegment;
class IBackend
{
public:
    virtual Node* allocNode(const Node& other) = 0;
    virtual Node* allocNode(const QPointF& aCoord) = 0;

    virtual Way* allocWay() = 0;
    virtual Way* allocWay(const Way& other) = 0;

    virtual Relation* allocRelation() = 0;
    virtual Relation* allocRelation(const Relation& other) = 0;

    virtual TrackSegment* allocSegment() = 0;

    virtual void deallocFeature(Feature*) = 0;

    virtual void sync(Feature* f) = 0;

    virtual void get(const QRectF& hz, QList<Feature*>& theFeatures) = 0;
};

#endif // IBACKEND_H
