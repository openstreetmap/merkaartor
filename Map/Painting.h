#ifndef MERKAARTOR_PAINTING_H_
#define MERKAARTOR_PAINTING_H_

#include "Map/MapFeature.h"

class Coord;
class Projection;
class Road;
class Way;

class QPainter;
class QPainterPath;
class QPen;

void buildPathFromRoad(Road *R, Projection const &theProjection, QPainterPath &Path);
void buildPathFromRelation(Relation *R, Projection const &theProjection, QPainterPath &Path);

/// draws way with oneway markers
void draw(QPainter& thePainter, QPen& thePen, MapFeature::TrafficDirectionType Dir, const QPointF& FromF, const QPointF& ToF, double theWidth, const Projection& theProjection);
void draw(QPainter& thePainter, QPen& thePen, MapFeature::TrafficDirectionType Dir, const Coord& From, const Coord& To, double theWidth, const Projection& theProjection);
/// draws way with oneway markers
void draw(QPainter& thePainter, QPen& thePen, Way* W, double theWidth, const Projection& theProjection);
/// draw way without oneway markers (as in focus)
void draw(QPainter& thePainter, QPen& thePen, Way* W, const Projection& theProjection);

#endif


