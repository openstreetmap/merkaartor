#ifndef MERKAARTOR_LINEF_
#define MERKAARTOR_LINEF_

#include "Map/Coord.h"

#include <QtCore/QPointF>

#include <math.h>

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
		return (x>0)?3.141592/2:-3.141592;
	x = asin(x/(length(A)*length(B)));
	if (d<0)
	{
		if (x > 0)
			x = 3.141592 - x;
		else
			x = -3.141592 - x;
	}
	return x;

}

inline double angle(const QPointF& A)
{
	return atan2(A.y(),A.x());
}

class LineF
{
public:
	LineF(const QPointF& aP1, const QPointF& aP2)
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

	double capDistance(const QPointF& P) const
	{
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
	}

	Coord project(const Coord& P)
	{
		if (Valid)
		{
			double SD = A*P.lat()+B*P.lon()+C;
			return Coord(P.lat()-A*SD,P.lon()-B*SD);
		}
		return Coord(P1.x(),P1.y());
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

#endif


