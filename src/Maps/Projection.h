#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "IProjection.h"
#include "Maps/Coord.h"

#include <QPointF>

#ifndef _MOBILE
#include "Preferences/MerkaartorPreferences.h"

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

        double latAnglePerM() const;
        double lonAnglePerM(double Lat) const;
        QLineF project(const QLineF & Map) const;
        QPointF project(const QPointF& Map) const;
        QPointF project(const Coord& Map) const;
        Coord inverse2Coord(const QPointF& Screen) const;
        QPointF inverse2Point(const QPointF& Map) const;

        bool setProjectionType(QString aProjectionType);
        QString getProjectionType() const;
        bool projIsLatLong() const;

#ifndef _MOBILE
        QPointF project(Node* aNode) const;
        static ProjProjection getProjection(QString projString);
        QString getProjectionProj4() const;

        static void projTransform(ProjProjection srcdefn,
                           ProjProjection dstdefn,
                           long point_count, int point_offset, double *x, double *y, double *z );
        void projTransformToWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const;
        void projTransformFromWGS84(long point_count, int point_offset, double *x, double *y, double *z ) const;
        QRectF getProjectedViewport(const CoordBox& Viewport, const QRect& screen) const;

        int projectionRevision() const;
#endif
        bool toXML(QXmlStreamWriter& stream);
        void fromXML(const QDomElement e);

    protected:
#ifndef _MOBILE
        ProjProjection theProj;
        QPointF projProject(const Coord& Map) const;
        QPointF projProject(const QPointF& Map) const;
        Coord projInverse(const QPointF& Screen) const;
#endif

    private:
        ProjectionPrivate* p;
};


#endif


