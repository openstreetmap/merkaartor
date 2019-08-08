#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "IProjection.h"
#include "Coord.h"

#include <QPointF>

#ifndef _MOBILE
#include "MerkaartorPreferences.h"

#ifndef ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#define ACCEPT_USE_OF_DEPRECATED_PROJ_API_H
#endif
#include <proj_api.h>
typedef projPJ ProjProjection;

#endif // _MOBILE

class QRect;
class Node;

class Projection : public IProjection
{
public:
    Projection(void);
    virtual ~Projection(void);

    qreal latAnglePerM() const;
    qreal lonAnglePerM(qreal Lat) const;
    QLineF project(const QLineF & Map) const;
    QPointF project(const QPointF& Map) const;
    Coord inverse2Coord(const QPointF& Screen) const;
    QPointF inverse2Point(const QPointF& Map) const;

    bool setProjectionType(QString aProjectionType);
    QString getProjectionType() const;
    bool projIsLatLong() const;

    QPointF project(Node* aNode) const;
    QRectF toProjectedRectF(const QRectF& Viewport, const QRect& screen) const;
    CoordBox fromProjectedRectF(const QRectF& Viewport) const;

    int projectionRevision() const;
    QString getProjectionProj4() const;

#ifndef _MOBILE

    static ProjProjection getProjection(QString projString);
    static void projTransform(ProjProjection srcdefn,
                              ProjProjection dstdefn,
                              long point_count, int point_offset, qreal *x, qreal *y, qreal *z );
    void projTransformToWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const;
    void projTransformFromWGS84(long point_count, int point_offset, qreal *x, qreal *y, qreal *z ) const;

#endif
    bool toXML(QXmlStreamWriter& stream);
    void fromXML(QXmlStreamReader& stream);

protected:
#ifndef _MOBILE
    ProjProjection theProj;
    QPointF projProject(const QPointF& Map) const;
    Coord projInverse(const QPointF& Screen) const;

    ProjProjection theWGS84Proj;
#endif

    QString projType;
    QString projProj4;
    QRectF ProjectedViewport;
    int ProjectionRevision;
    bool IsMercator;
    bool IsLatLong;

protected:
    QPointF mercatorProject(const QPointF& c) const;
    Coord mercatorInverse(const QPointF& point) const;

    inline QPointF latlonProject(const QPointF& c) const
    {
        return QPointF(c.x()/**EQUATORIALMETERPERDEGREE*/, c.y()/**EQUATORIALMETERPERDEGREE*/);
    }

    inline Coord latlonInverse(const QPointF& point) const
    {
        return Coord(point.x()/*/EQUATORIALMETERPERDEGREE*/, point.y()/*/EQUATORIALMETERPERDEGREE*/);
    }

};


#endif


