#ifndef MERKAARTOR_MAPTYPEDEF_H
#define MERKAARTOR_MAPTYPEDEF_H

//#include <QPointer>

class MapFeature;
//typedef QPointer<MapFeature> MapFeaturePtr;
typedef MapFeature* MapFeaturePtr;

class TrackPoint;
//typedef QPointer<TrackPoint> TrackPointPtr;
typedef TrackPoint* TrackPointPtr;

class Road;
//typedef QPointer<Road> RoadPtr;
typedef Road* RoadPtr;

class Relation;
//typedef QPointer<Relation> RelationPtr;
typedef Relation* RelationPtr;

class TrackSegment;
//typedef QPointer<TrackSegment> TrackSegmentPtr;
typedef TrackSegment* TrackSegmentPtr;

#endif // MERKAARTOR_MAPTYPEDEF_H

