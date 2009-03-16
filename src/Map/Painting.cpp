#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QLineF>

#include <utility>

static void buildCubicPath(QPainterPath& Path, const QPointF& P1, const QPointF& P2, const QPointF& P3, const QPointF& P4)
{
	LineF L(P1,P4);
	double D2 = L.distance(P2);
	double D3 = L.distance(P3);
	if ( (D2 < 0.5) && (D3<0.5) )
		Path.lineTo(P4);
	else
	{
		QPointF H = (P2+P3)/2.0;
		QPointF L2 = (P1+P2)/2.0;
		QPointF R3 = (P3+P4)/2.0;
		QPointF L3 = (L2+H)/2.0;
		QPointF R2 = (H+R3)/2.0;
		QPointF L4 = (L3+R2)/2.0;
		buildCubicPath(Path,P1,L2,L3,L4);
		buildCubicPath(Path,L4,R2,R3,P4);

	}
}

//bool QRectInterstects(const QRect& r, const QLine& l, QPoint& a, QPoint& b)
//{
//	QLineF lF = QLineF(l);
//	QPointF pF;
//	bool hasP1 = false;
//	bool hasP2 = false;
//
//	if (QLineF(r.topLeft(), r.bottomLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
//		a = pF.toPoint();
//		hasP1 = true;
//	} 
//	if (QLineF(r.bottomLeft(), r.bottomRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
//		if (hasP1) {
//			b = pF.toPoint();
//			hasP2 = true;
//		} else {
//			a = pF.toPoint();
//			hasP1 = true;
//		}
//	} 
//	if (QLineF(r.bottomRight(), r.topRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
//		if (hasP1) {
//			b = pF.toPoint();
//			hasP2 = true;
//		} else {
//			a = pF.toPoint();
//			hasP1 = true;
//		}
//	} 
//	if (QLineF(r.topRight(), r.topLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
//		if (hasP1) {
//			b = pF.toPoint();
//			hasP2 = true;
//		} else {
//			a = pF.toPoint();
//			hasP1 = true;
//		}
//	}
//
//	if (hasP1 && hasP2) {
//		if (QLineF(a,b).angleTo(lF) > 15.0) {
//			QPoint t = b;
//			b = a;
//			a = t;
//		}
//	}
//	if (hasP1)
//		return true;
//	else
//		return false;
//}
//
//void buildPathFromRoad(Road *R, Projection const &theProjection, QPainterPath &Path, const QRect& clipRect)
//{
//	unsigned int first=0, last=R->size();
//	//if (!theProjection.viewport().contains(R->boundingBox())) {
//	//	for (unsigned int i=0; i<R->size(); ++i) {
//	//		if (theProjection.viewport().contains(R->get(i)->boundingBox())) {
//	//			if (!first)
//	//				first=i;
//	//			last=i;
//	//		}
//	//	}
//	//	if (first) first--;
//	//	last=qMin(last+2, R->size());
//	//}
//
//	bool lastPointVisible = true;
//	QPoint lastPoint = theProjection.project(R->get(first)->position());
//	QPoint p = lastPoint;
//
//	if (!clipRect.contains(p)) {
//		p.setX(qMax(clipRect.left(), p.x()));
//		p.setX(qMin(clipRect.right(), p.x()));
//		p.setY(qMax(clipRect.top(), p.y()));
//		p.setY(qMin(clipRect.bottom(), p.y()));
//		lastPointVisible = false;
//	}
//	Path.moveTo(p);
//	if (R->smoothed().size())
//	{
//		for (unsigned int i=3; i<R->smoothed().size(); i+=3)
//			Path.cubicTo(
//				theProjection.project(R->smoothed()[i-2]),
//				theProjection.project(R->smoothed()[i-1]),
//				theProjection.project(R->smoothed()[i]));
//	}
//	else
//		for (unsigned int j=first+1; j<last; ++j) {
//			p = theProjection.project(R->get(j)->position());
//			if (!clipRect.contains(p)) {
//				if (!lastPointVisible) {
//					QPoint a, b;
//					if (QRectInterstects(clipRect, QLine(lastPoint, p), a, b)) {
//						Path.lineTo(a);
//						lastPoint = p;
//						p = b;
//					} else {
//						lastPoint = p;
//						p.setX(qMax(clipRect.left(), p.x()));
//						p.setX(qMin(clipRect.right(), p.x()));
//						p.setY(qMax(clipRect.top(), p.y()));
//						p.setY(qMin(clipRect.bottom(), p.y()));
//					}
//				} else {
//					QPoint a, b;
//					QRectInterstects(clipRect, QLine(lastPoint, p), a, b);
//					lastPoint = p;
//					p = a;
//				}
//				lastPointVisible = false;
//			} else {
//				if (!lastPointVisible) {
//					QPoint a, b;
//					QRectInterstects(clipRect, QLine(lastPoint, p), a, b);
//					Path.lineTo(a);
//				}
//				lastPoint = p;
//				lastPointVisible = true;
//			}
//			Path.lineTo(p);
//		}
//}
//
void buildPolygonFromRoad(Road *R, Projection const &theProjection, QPolygonF &Polygon)
{
	for (unsigned int i=0; i<R->size(); ++i)
		Polygon.append(theProjection.project(R->getNode(i)));
}

/// draws way with oneway markers
void draw(QPainter& thePainter, QPen& thePen, MapFeature::TrafficDirectionType TT, const QPointF& FromF, const QPointF& ToF, double theWidth, const Projection&)
{
	QPainterPath Path;
	Path.moveTo(FromF);
	Path.lineTo(ToF);
	double DistFromCenter = theWidth*2;
	if (distance(FromF,ToF) > std::max(40.0,DistFromCenter*2+4))
	{
		QPointF H(FromF+ToF);
		H *= 0.5;
		double A = angle(FromF-ToF);
		QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
		QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
		QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));
//		MapFeature::TrafficDirectionType TT = W->trafficDirection();
		if ( (TT == MapFeature::OtherWay) || (TT == MapFeature::BothWays) )
		{
			thePainter.setPen(QColor(0,0,0));
			thePainter.drawLine(H+T,H+T-V1);
			thePainter.drawLine(H+T,H+T-V2);
		}
		if ( (TT == MapFeature::OneWay) || (TT == MapFeature::BothWays) )
		{
			thePainter.setPen(QColor(0,0,0));
			thePainter.drawLine(H-T,H-T+V1);
			thePainter.drawLine(H-T,H-T+V2);
		}
	}
	thePainter.strokePath(Path,thePen);
}

void draw(QPainter& thePainter, QPen& thePen, MapFeature::TrafficDirectionType TT, const Coord& From, const Coord& To, double theWidth, const Projection& theProjection)
{
	QPointF FromF(theProjection.project(From));
	QPointF ToF(theProjection.project(To));
	draw(thePainter,thePen,TT,FromF,ToF,theWidth,theProjection);
}


/* void draw(QPainter& thePainter, QPen& thePen, Way* W, const Projection& theProjection)
{
	QPainterPath Path;
	QPointF FromF(theProjection.project(W->from()));
	QPointF ToF(theProjection.project(W->to()));
	Path.moveTo(FromF);
	Path.lineTo(ToF);
	thePainter.strokePath(Path,thePen);
} */



