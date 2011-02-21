#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "IProjection.h"
#include "Coord.h"

#include <QPointF>

#ifndef _MOBILE
#include "MerkaartorPreferences.h"

#include <proj_api.h>
typedef projPJ ProjProjection;

#endif // _MOBILE

class QRect;
class Node;
class ProjectionPrivate;

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
#endif

    private:
        ProjectionPrivate* p;
};


#endif


