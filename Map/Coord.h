#ifndef MERKATOR_COORD_H_
#define MERKATOR_COORD_H_

#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#include <QRectF>
#include <QtDebug>
#include <QtXml>

inline double angToRad(double a)
{
	return a*M_PI/180;
}

inline double radToAng(double a)
{
	return a*180/M_PI;
}



class Coord
{
	public:
		Coord()
			: Lat(0.0), Lon(0.0) {}
		Coord(const QPointF& P)
			: Lat(P.x()), Lon(P.y()) {}
		Coord(double aLat, double aLon)
			: Lat(aLat), Lon(aLon) {}

		bool isNull() const
		{
			return (Lat == 0.0) && (Lon == 0.0);
		}

		double lat() const
		{
			return Lat;
		}

		double lon() const
		{
			return Lon;
		}

		void setLat(double l)
		{
			Lat = l;
		}

		void setLon(double l)
		{
			Lon = l;
		}

		double length() const
		{
			return sqrt(Lat*Lat+Lon*Lon);
		}

		double distanceFrom(const Coord& other) const;

		bool toXML(QString elName, QDomElement& xParent) const;
		static Coord fromXML(QDomElement e);

		QPointF toQPointF() const
		{
			return QPointF(Lat, Lon);
		}

	private:
		double Lat;
		double Lon;
};

inline Coord operator-(const Coord& A, const Coord& B)
{
	return Coord(A.lat()-B.lat(),A.lon()-B.lon());
}

inline Coord operator-(const Coord& A, const double B)
{
	return Coord(A.lat()-B,A.lon()-B);
}

inline Coord operator+(const Coord& A, const Coord& B)
{
	return Coord(A.lat()+B.lat(),A.lon()+B.lon());
}

inline Coord operator+(const Coord& A, const double B)
{
	return Coord(A.lat()+B,A.lon()+B);
}

inline Coord operator*(const Coord& A, double d)
{
	return Coord(A.lat()*d,A.lon()*d);
}

inline Coord operator/(const Coord& A, double d)
{
	if(d==0)
	{
		qDebug()<<"Error: divide by 0"<<endl;
		return A;
	}
	return Coord(A.lat()/d,A.lon()/d);
}

inline bool operator==(const Coord& A,const Coord& B)
{
	return A.lat()==B.lat() && A.lon()==B.lon();
}

double angle(Coord p1);
void rotate(Coord & p1,double angle);

class CoordBox
{
	public:
		CoordBox() {};
		CoordBox(const Coord& C1, const Coord& C2);

		bool isNull() const
		{
			return (BottomLeft.isNull() && TopRight.isNull());
		}
		void merge(const Coord& C)
		{
			if (C.lat() < BottomLeft.lat())
				BottomLeft.setLat(C.lat());
			if (C.lon() < BottomLeft.lon())
				BottomLeft.setLon(C.lon());
			if (C.lat() > TopRight.lat())
				TopRight.setLat(C.lat());
			if (C.lon() > TopRight.lon())
				TopRight.setLon(C.lon());
		}

		bool contains(const Coord& C) const
		{
			return (BottomLeft.lat() <= C.lat()) && (BottomLeft.lon() <= C.lon()) &&
				(C.lat() < TopRight.lat()) && (C.lon() <= TopRight.lon());
		}

		bool contains(const CoordBox& B) const
		{
			return contains(B.BottomLeft) && contains(B.TopRight);
		}

		bool intersects(const Coord& C) const
		{
			return contains(C);
		}

		bool intersects(const CoordBox& B) const
		{
			if ((B.latDiff() == 0) && (B.lonDiff() == 0)) {
				return contains(B.bottomLeft());
			}
			return qMax(BottomLeft.lon(), B.bottomLeft().lon()) <= qMin(TopRight.lon(), B.topRight().lon())
					&& qMax(BottomLeft.lat(), B.bottomLeft().lat()) <= qMin(TopRight.lat(), B.topRight().lat());

/*			QRectF BRect(B.bottomLeft().lon(), B.topRight().lat(), B.lonDiff(), B.latDiff());
			QRectF myRect(BottomLeft.lon(), TopRight.lat(), lonDiff(), latDiff());

			return myRect.intersects(BRect);*/
		}

		void merge(const CoordBox& B)
		{
			merge(B.BottomLeft);
			merge(B.TopRight);
		}

		Coord center() const
		{
			return Coord( (BottomLeft.lat()+TopRight.lat())/2,(BottomLeft.lon()+TopRight.lon())/2 );
		}

		double lonDiff() const
		{
			return TopRight.lon()-BottomLeft.lon();
		}
		double latDiff() const
		{
			return TopRight.lat()-BottomLeft.lat();
		}
		CoordBox zoomed(double f) const;
		const Coord& bottomLeft() const
		{
			return BottomLeft;
		}

		const Coord& topRight() const
		{
			return TopRight;
		}

		Coord topLeft() const
		{
			return Coord(TopRight.lat(), BottomLeft.lon());
		}

		Coord bottomRight() const
		{
			return Coord(BottomLeft.lat(), TopRight.lon());
		}

		bool disjunctFrom(const CoordBox& B) const
		{
			if (BottomLeft.lat() > B.TopRight.lat()) return true;
			if (BottomLeft.lon() > B.TopRight.lon()) return true;
			if (TopRight.lat() < B.BottomLeft.lat()) return true;
			if (TopRight.lon() < B.BottomLeft.lon()) return true;
			return false;
		}

		QRectF toRectF()
		{
			return QRectF(BottomLeft.lon(), TopRight.lat(), lonDiff(), latDiff());
		}
		void resize(double f);

		static bool visibleLine(const CoordBox & viewport, Coord & last, Coord & here);

		bool toXML(QString elName, QDomElement& xParent) const;
		static CoordBox fromXML(QDomElement e);

	private:
		Coord BottomLeft, TopRight;
};

#endif


