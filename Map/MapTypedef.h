#ifndef MERKAARTOR_MAPTYPEDEF_H
#define MERKAARTOR_MAPTYPEDEF_H

#include <QPointer>

class MapFeature;
typedef QPointer<MapFeature> MapFeaturePtr;

class TrackPoint;
typedef QPointer<TrackPoint> TrackPointPtr;

class Road;
typedef QPointer<Road> RoadPtr;

class Relation;
typedef QPointer<Relation> RelationPtr;

class TrackSegment;
typedef QPointer<TrackSegment> TrackSegmentPtr;

#endif // MERKAARTOR_MAPTYPEDEF_H

