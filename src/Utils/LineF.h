#ifndef MERKAARTOR_LINEF_
#define MERKAARTOR_LINEF_

#include "Map/Coord.h"

#include <QtCore/QPointF>

#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2		1.57079632679489661923
#endif

inline double distance(const QPointF& A, const QPointF& B)
{
	double dx = A.x()-B.x();
	double dy = A.y()-B.y();
	return sqrt( dx*dx+dy*dy );
}

inline double length(const QPointF& A)
{
	return sqrt(A.x()*A.x()+A.y()*A.y());
}

inline double angle(const QPointF& A, const QPointF& B)
{
	double d = A.x()*B.x()+A.y()*B.y();
	double x = A.x()*B.y()-A.y()*B.x();
	// numerical stability : in extreme cases the argument of asin gets slightly larger than 1
	if (fabs(d) < 0.00001)
		return (x>0)?M_PI_2:-M_PI;
	x = asin(x/(length(A)*length(B)));
	if (d<0)
	{
		if (x > 0)
			x = M_PI - x;
		else
			x = -M_PI - x;
	}
	return x;

}

inline double angle(const QPointF& A)
{
	return atan2(A.y(),A.x());
}

inline QPointF toQt(const Coord& C)
{
	return QPointF(C.lat(),C.lon());
}

inline Coord toCoord(const QPointF& F)
{
	return Coord(int(F.x()),int(F.y()));
}

class LineF
{
public:
	LineF(const QPointF& aP1, const QPointF& aP2)
		: P1(aP1), P2(aP2), Valid(true)
	{
		init();
	}

	LineF(const QPoint& aP1, const QPoint& aP2)
		: P1(aP1), P2(aP2), Valid(true)
	{
		init();
	}

	LineF(const Coord& aP1, const Coord& aP2)
		: P1(aP1.lat(),aP1.lon()), P2(aP2.lat(),aP2.lon()), Valid(true)
	{
		init();
	}

	void init()
	{
		A = P2.y()-P1.y();
		B = -P2.x()+P1.x();
		C = -P1.y()*B-P1.x()*A;
		double F = sqrt(A*A+B*B);
		if (F<0.00000001)
			Valid=false;
		else
		{
			A/=F;
			B/=F;
			C/=F;
		}
	}

	void slide(double d)
	{
		C += d*sqrt(A*A+B*B);
	}

	double distance(const QPointF& P) const
	{
		if (Valid)
			return fabs(A*P.x()+B*P.y()+C);
		else
			return sqrt( (P.x()-P1.x())*(P.x()-P1.x()) + (P.y()-P1.y())*(P.y()-P1.y()) );
	}

	//double capDistance(const QPointF& P) const
	//{
	//	if (Valid)
	//	{
	//		double dx = P2.x()-P1.x();
	//		double dy = P2.y()-P1.y();
	//		double px = P.x()-P1.x();
	//		double py = P.y()-P1.y();
	//		if ( (dx*px+dy*py) < 0)
	//			return ::distance(P,P1);
	//		px = P.x()-P2.x();
	//		py = P.y()-P2.y();
	//		if ( (dx*px+dy*py) > 0)
	//			return ::distance(P,P2);
	//		return fabs(A*P.x()+B*P.y()+C);
	//	}
	//	else
	//		return sqrt( (P.x()-A)*(P.x()-A) + (P.y()-B)*(P.y()-B) );
	//}

	double capDistance(const Coord& cd)
	{
		QPointF P(cd.lat(), cd.lon());
		if (Valid)
		{
			double dx = P2.x()-P1.x();
			double dy = P2.y()-P1.y();
			double px = P.x()-P1.x();
			double py = P.y()-P1.y();
			if ( (dx*px+dy*py) < 0)
				return ::distance(P,P1);
			px = P.x()-P2.x();
			py = P.y()-P2.y();
			if ( (dx*px+dy*py) > 0)
				return ::distance(P,P2);
			return fabs(A*P.x()+B*P.y()+C);
		}
		else
			return sqrt( (P.x()-A)*(P.x()-A) + (P.y()-B)*(P.y()-B) );
		//return capDistance(QPointF(P.lat(),P.lon()));
	}

	Coord project(const Coord& P)
	{
		if (Valid)
		{
			double SD = A*P.lat()+B*P.lon()+C;
			return Coord(int(P.lat()-A*SD),int(P.lon()-B*SD));
		}
		return Coord(int(P1.x()),int(P1.y()));
	}
	QPointF project(const QPointF& P)
	{
		if (Valid)
		{
			double SD = A*P.x()+B*P.y()+C;
			return QPointF(P.x()-A*SD,P.y()-B*SD);
		}
		return P1;
	}
	QPointF project(const QPoint& P)
	{
		return project(QPointF(P));
	}

	bool intersectsWith(const CoordBox& C) const
	{
		QPointF intersection;
		if ( QLineF(P1, P2).intersect( QLineF( C.topLeft().toPointF(), C.topRight().toPointF() ), &intersection) == QLineF::BoundedIntersection ) return true;
		if ( QLineF(P1, P2).intersect( QLineF( C.topRight().toPointF(), C.bottomRight().toPointF() ), &intersection) == QLineF::BoundedIntersection ) return true;
		if ( QLineF(P1, P2).intersect( QLineF( C.bottomRight().toPointF(), C.bottomLeft().toPointF() ), &intersection) == QLineF::BoundedIntersection ) return true;
		if ( QLineF(P1, P2).intersect( QLineF( C.bottomLeft().toPointF(), C.topLeft().toPointF() ), &intersection) == QLineF::BoundedIntersection ) return true;
		return false;
	}

	void intersectionWith(const CoordBox& C, Coord* C1, Coord* C2) const
	{
		QPointF intersection;
		bool hasC1 = false;

		if ( QLineF(P1, P2).intersect( QLineF( C.topLeft().toPointF(), C.topRight().toPointF() ), &intersection) == QLineF::BoundedIntersection ) {
			*C1 = intersection;
			hasC1 = true;
		}
		if ( QLineF(P1, P2).intersect( QLineF( C.topRight().toPointF(), C.bottomRight().toPointF() ), &intersection) == QLineF::BoundedIntersection ) {
			if (hasC1) {
				*C2 = intersection;
				return;
			} else {
				*C1 = intersection;
				hasC1 = true;
			}
		}
		if ( QLineF(P1, P2).intersect( QLineF( C.bottomRight().toPointF(), C.bottomLeft().toPointF() ), &intersection) == QLineF::BoundedIntersection ) {
			if (hasC1) {
				*C2 = intersection;
				return;
			} else {
				*C1 = intersection;
				hasC1 = true;
			}
		}
		if ( QLineF(P1, P2).intersect( QLineF( C.bottomLeft().toPointF(), C.topLeft().toPointF() ), &intersection) == QLineF::BoundedIntersection ) {
			if (hasC1) {
				*C2 = intersection;
				return;
			} else {
				*C1 = intersection;
				hasC1 = true;
			}
		}
	}

	QPointF intersectionWith(const LineF& L)
	{
		double D = A*L.B - L.A*B;
		if (fabs(D) < 0.00001)
			return P2;
		double x = B*L.C - L.B*C;
		double y = L.A*C - A*L.C;
		return QPointF(x/D,y/D);
	}

	bool segmentContains(const QPointF& X)
	{
		return
			( ((P1.x() <= X.x()) && (X.x() <= P2.x())) ||
			  ((P1.x() >= X.x()) && (X.x() >= P2.x())) ) &&
			( ((P1.y() <= X.y()) && (X.y() <= P2.y())) ||
			  ((P1.y() >= X.y()) && (X.y() >= P2.y())) );
	}


private:
	QPointF P1, P2;
	bool Valid;
	double A,B,C;
};

class BezierF
{
	public:
		BezierF(const QPointF& aA, const QPointF& aB, const QPointF& aC, const QPointF& aD)
			: A(aA), B(aB), C(aC), D(aD)
		{
		}
		BezierF(const QPoint& aA, const QPoint& aB, const QPoint& aC, const QPoint& aD)
			: A(aA), B(aB), C(aC), D(aD)
		{
		}

		BezierF(const Coord& aA, const Coord& aB, const Coord& aC, const Coord& aD)
			: A(toQt(aA)), B(toQt(aB)), C(toQt(aC)), D(toQt(aD))
		{
		}

		double distance(const QPointF& T) const
		{
			double LowestZ = ::distance(A,T);
			for (qreal t=0;t<1.0125; t+=0.025)
			{
				QPointF P = A*(1-t)*(1-t)*(1-t) + 3*B*(1-t)*(1-t)*t + 3*C*(1-t)*t*t + D*t*t*t;
				double z = ::distance(P,T);
				if (z < LowestZ)
					LowestZ = z;
			}
			return LowestZ;
		}

		QPointF project(const QPointF& T) const
		{
			double LowestZ = ::distance(A,T);
			QPointF ClosestP(A);
			for (qreal t=0;t<1.0125; t+=0.025)
			{
				QPointF P = A*(1-t)*(1-t)*(1-t) + 3*B*(1-t)*(1-t)*t + 3*C*(1-t)*t*t + D*t*t*t;
				double z = ::distance(P,T);
				if (z < LowestZ)
				{
					LowestZ = z;
					ClosestP = P;
				}
			}
			return ClosestP;
		}


	private:
		QPointF A,B,C,D;
};

#endif


