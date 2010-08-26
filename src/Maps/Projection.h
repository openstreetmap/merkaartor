#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "Maps/Coord.h"

#include <QPointF>

#ifndef _MOBILE

#include "Preferences/MerkaartorPreferences.h"

#ifdef USE_PROJ
#include <proj_api.h>
typedef projPJ ProjProjection;
#else
#include <ggl/geometries/cartesian2d.hpp>
#include <ggl/extensions/gis/latlong/latlong.hpp>
#include <ggl/extensions/gis/projections/projection.hpp>

typedef ggl::projection::projection<ggl::point_ll_deg, ggl::point_2d>* ProjProjection;
#endif // USE_PROJ
#endif // _MOBILE

class QRect;
class Node;
class ProjectionPrivate;

class Projection
{
    public:
        Projection(void);
        virtual ~Projection(void);

        double latAnglePerM() const;
        double lonAnglePerM(double Lat) const;
        QPointF project(const Coord& Map) const;
        Coord inverse(const QPointF& Screen) const;

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
        bool toXML(QDomElement xParent);
        void fromXML(const QDomElement e);

    protected:
#ifndef _MOBILE
        ProjProjection theProj;
        QPointF projProject(const Coord& Map) const;
        Coord projInverse(const QPointF& Screen) const;
#endif

    private:
        ProjectionPrivate* p;
};


#endif


