#ifndef MERKATOR_COORD_H_
#define MERKATOR_COORD_H_

#include <math.h>
#ifndef M_PI
#define M_PI        3.14159265358979323846
#endif
#include <QRectF>
#include <QtDebug>
#include <QtXml>

#define COORD_MAX 180.0
#define COORD_ENLARGE 0.00015

#define COORD2STRING(c) QString::number(c, 'f', 7)
inline QString Coord2Sexa(double c)
{
    int deg = int(c);
    double min = fabs((c - deg)*60);
    double sec = (min - int(min)) *60;

    return QString("%1° %2' %3\"").arg(deg).arg(int(min)).arg(sec, 0, 'f', 2);
}


inline double angToRad(double a)
{
    return a*M_PI/180.;
}

inline double radToAng(double a)
{
    return a*180/M_PI;
}

#define angToCoord(a) (a)
//inline double  angToCoord(double a)
//{
//    return a;
//}

#define coordToAng(a) (a)
//inline double coordToAng(double a)
//{
//    return a;
//}

inline double coordToRad(double a)
{
    return angToRad(a);
}

inline double radToCoord(double x)
{
    return radToAng(x);
}

class Coord
{
    public:
        Coord()
            : Lat(0), Lon(0) {}
        Coord(const Coord& c)
            : Lat(c.Lat), Lon(c.Lon) {}
        Coord(const QPoint& P)
            : Lat(P.x()), Lon(P.y()) {}
        Coord(const QPointF& P)
            : Lat(P.x()), Lon(P.y()) {}
        Coord(double aLat, double aLon)
            : Lat(aLat), Lon(aLon) {}

        bool isNull() const
        {
            return (Lat == 0) && (Lon == 0);
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
            return sqrt((Lat*Lat+Lon*Lon));
        }

        double distanceFrom(const Coord& other) const;

        bool toXML(QString elName, QXmlStreamWriter& stream) const;
        bool toXML(QString elName, QDomElement& xParent) const;
        static Coord fromXML(QDomElement e);
        static Coord fromXML(QXmlStreamReader& stream);

        QPointF toPointF() const
        {
            return QPointF(Lat, Lon);
        }

        QPointF toQPointF() const
        {
            return QPointF(Lon, Lat);
        }

        static Coord fromQPointF(const QPointF& pt)
        {
            return Coord(pt.y(), pt.x());
        }

    private:
        double Lat;
        double Lon;
};

uint qHash(const Coord &c);


#ifndef _MOBILE
#if QT_VERSION < 0x040700
#include <ggl/ggl.hpp>
#include <ggl/geometries/register/point.hpp>

GEOMETRY_REGISTER_POINT_2D_GET_SET(Coord, double, cs::cartesian, lat, lon, setLat, setLon)

#endif
#endif

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
        CoordBox() {}
        CoordBox(const CoordBox& cb);
        CoordBox(const Coord& C1, const Coord& C2);

        bool isNull() const
        {
            return (BottomLeft.isNull() && TopRight.isNull());
        }
        bool isEmpty() const
        {
            return (!lonDiff() || !latDiff());
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
            return Coord( BottomLeft.lat() + latDiff()/2, BottomLeft.lon() + lonDiff()/2 );
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

        QRectF toRectF() const
        {
            return QRectF(BottomLeft.lon(), BottomLeft.lat(), lonDiff(), latDiff());
        }

        QRectF toQRectF() const
        {
            return QRectF(BottomLeft.lon(), TopRight.lat(), lonDiff(), BottomLeft.lat()-TopRight.lat());
        }

        static CoordBox fromQRectF(const QRectF& r)
        {
            return CoordBox(Coord(r.topLeft().y(), r.topLeft().x()), Coord(r.bottomRight().y(), r.bottomRight().x()));
        }

        void resize(double f);

        static bool visibleLine(const CoordBox & viewport, Coord & last, Coord & here);

        bool toXML(QString elName, QXmlStreamWriter& stream) const;
        bool toXML(QString elName, QDomElement& xParent) const;
        static CoordBox fromXML(QDomElement e);
        static CoordBox fromXML(QXmlStreamReader& stream);

    //private:
        Coord BottomLeft, TopRight;
};

Q_DECLARE_METATYPE( CoordBox );

#ifndef _MOBILE
#if QT_VERSION < 0x040700
#include <ggl/geometries/register/box.hpp>

GEOMETRY_REGISTER_BOX(CoordBox, Coord, BottomLeft, TopRight)
#endif
#endif

#endif


