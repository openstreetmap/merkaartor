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

/**
 * This is a helper class that composes the proj search path. It is shared between this module and GDAL code.
 *
 * Note: qApp must be called after QApplication is instanciated - this must not be static, as that would call that earlier.
 */
struct ProjDirs {
  private:
    /* FIXME: Proj >= 7.0 supports relative path lookup natively, we can remove
     * the relative path here once that gets into msys2. */
    QString projDirRelative = QDir::toNativeSeparators(qApp->applicationDirPath() + "/../share/proj");
    /* The following are hardcoded paths, as if search paths are set, proj no longer searches the default. Can be safely removed with proj7.*/
    QString projDirMingwStd = QDir::toNativeSeparators("c:/msys64/mingw64/share/proj");       // Standard msys2 path
    QString projDirMingwTravis = QDir::toNativeSeparators("c:/tools/msys64/mingw64/share/proj"); // Travis-CI msys2 installation
    QString projDirMingwGithub = QDir::toNativeSeparators("D:/a/_temp/msys64/mingw64/share/proj"); // Github Actions msys2 installation
  public:
    const char* const dirs[5] = {
        projDirRelative.toUtf8().constData(),
        projDirMingwStd.toUtf8().constData(),
        projDirMingwTravis.toUtf8().constData(),
        projDirMingwGithub.toUtf8().constData(),
	NULL
    };
    static const int count = 4;
};


#endif


