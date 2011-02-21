#ifndef MERKAARTOR_MAPTYPEDEF_H
#define MERKAARTOR_MAPTYPEDEF_H

//#include <QPointer>

class Feature;
//typedef QPointer<MapFeature> MapFeaturePtr;
typedef Feature* MapFeaturePtr;

class Node;
//typedef QPointer<TrackPoint> TrackPointPtr;
typedef Node* NodePtr;

class Way;
//typedef QPointer<Road> RoadPtr;
typedef Way* WayPtr;

class Relation;
//typedef QPointer<Relation> RelationPtr;
typedef Relation* RelationPtr;

class TrackSegment;
//typedef QPointer<TrackSegment> TrackSegmentPtr;
typedef TrackSegment* TrackSegmentPtr;

#endif // MERKAARTOR_MAPTYPEDEF_H

