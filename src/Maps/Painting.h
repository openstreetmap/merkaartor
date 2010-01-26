#ifndef MERKAARTOR_PAINTING_H_
#define MERKAARTOR_PAINTING_H_

#include "Feature.h"

class Coord;
class Projection;
class Way;
class Way;

class QPainter;
class QPainterPath;
class QPolygonF;
class QPen;

//void buildPathFromRoad(Road *R, Projection const &theProjection, QPainterPath &Path, const QRect& clipRect);
void buildPolygonFromRoad(Way *R, Projection const &theProjection, QPolygonF &Polygon);

/// draws way with oneway markers
void draw(QPainter& thePainter, QPen& thePen, Feature::TrafficDirectionType Dir, const QPointF& FromF, const QPointF& ToF, double theWidth, const Projection& theProjection);
void draw(QPainter& thePainter, QPen& thePen, Feature::TrafficDirectionType Dir, const Coord& From, const Coord& To, double theWidth, const Projection& theProjection);
/// draws way with oneway markers
void draw(QPainter& thePainter, QPen& thePen, Way* W, double theWidth, const Projection& theProjection);
/// draw way without oneway markers (as in focus)
void draw(QPainter& thePainter, QPen& thePen, Way* W, const Projection& theProjection);

#endif


