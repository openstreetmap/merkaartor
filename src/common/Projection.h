#ifndef MERKATOR_PROJECTION_H_
#define MERKATOR_PROJECTION_H_

#include "IProjection.h"
#include "Coord.h"

#include "MerkaartorPreferences.h"


#include <QPointF>
#include <proj.h>
#include <functional>
#include <memory>

class QRect;
class Node;

class ProjectionBackend : public IProjection
{
public:
    ProjectionBackend(QString initProjection, std::function<QString(QString)> mapProjectionName);

    qreal latAnglePerM() const;
    qreal lonAnglePerM(qreal Lat) const;
    QLineF project(const QLineF & Map) const;
    QPointF project(const QPointF& Map) const;
    QPointF inverse(const QPointF& Map) const;

    bool setProjectionType(QString aProjectionType);
    QString getProjectionType() const;
    bool projIsLatLong() const;

    QRectF toProjectedRectF(const QRectF& Viewport, const QRect& screen) const;
    CoordBox fromProjectedRectF(const QRectF& Viewport) const;

    int projectionRevision() const;
    QString getProjectionProj4() const;

    bool toXML(QXmlStreamWriter& stream);
    void fromXML(QXmlStreamReader& stream);

protected:
    PJ* getProjection(QString projString);
    QPointF projProject(const QPointF& Map) const;
    Coord projInverse(const QPointF& Screen) const;

    QString projType;
    QString projProj4;
    QRectF ProjectedViewport;
    int ProjectionRevision;
    bool IsMercator;
    bool IsLatLong;
    std::function<QString(QString)> mapProjectionName;
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
private:
    // Note: keep the order of projCtx and projTransform, as projTransform depends on projCtx.
    std::shared_ptr<PJ_CONTEXT> projCtx;
    std::shared_ptr<PJ> projTransform;
    std::shared_ptr<QMutex> projMutex;
    // TODO: projTransform is not thread-safe by itself, so we need to protect
    // it by a mutex.  In theory, each thread could have it's own projection
    // object, but currently the object is copied around.  Until this changes,
    // the mutex stays here.
};

/**
 * Proxy class to inject M_PREFS externally and allow unit testing of ProjectionBackend itself.
 */
class Projection : public ProjectionBackend {
  public:
    static QString mapProjectionName(QString projName) {
        return M_PREFS->getProjection(projName).projection;
    }

    Projection(void) :
      ProjectionBackend(M_PREFS->getProjectionType(), mapProjectionName)
    {
    }
};


#endif


