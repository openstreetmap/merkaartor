#ifndef MERKATOR_COORD_H_
#define MERKATOR_COORD_H_

#include <math.h>

inline double angToRad(double a)
{
	return a*3.141592/180;
}

inline double radToAng(double a)
{
	return a*180/3.141592;
}

class Coord
{
	public:
		Coord(double aLat, double aLon)
			: Lat(aLat), Lon(aLon)
		{
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


class CoordBox
{
	public:
		CoordBox(const Coord& C1, const Coord& C2);

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

		bool disjunctFrom(const CoordBox& B) const
		{
			if (BottomLeft.lat() > B.TopRight.lat()) return true;
			if (BottomLeft.lon() > B.TopRight.lon()) return true;
			if (TopRight.lat() < B.BottomLeft.lat()) return true;
			if (TopRight.lon() < B.BottomLeft.lon()) return true;
			return false;
		}

	private:
		Coord BottomLeft, TopRight;
};

#endif


