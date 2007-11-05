#include "TrackPoint.h"

#include "Map/Projection.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>

TrackPoint::TrackPoint(const Coord& aCoord)
: Position(aCoord), Time(QDateTime::currentDateTime())
{
}

TrackPoint::TrackPoint(const TrackPoint& other)
: MapFeature(other), Position(other.Position), Time(other.Time)
{
}

TrackPoint::~TrackPoint(void)
{
}

const QDateTime& TrackPoint::time() const
{
	return Time;
}

void TrackPoint::setTime(const QDateTime& time)
{
	Time = time;
	notifyChanges();
}

const Coord& TrackPoint::position() const
{
	return Position;
}

void TrackPoint::setPosition(const Coord& aCoord)
{
	Position = aCoord;
	notifyChanges();
}

bool TrackPoint::notEverythingDownloaded() const
{
	return lastUpdated() == MapFeature::NotYetDownloaded;
}

CoordBox TrackPoint::boundingBox() const
{
	return CoordBox(Position,Position);
}

void TrackPoint::draw(QPainter& thePainter, const Projection& theProjection)
{
}

void TrackPoint::drawFocus(QPainter& thePainter, const Projection& theProjection)
{
	thePainter.setPen(QColor(0,0,255));
	QPointF P(theProjection.project(Position));
	QRectF R(P-QPointF(3,3),QSize(6,6));
	thePainter.drawRect(R);
}

double TrackPoint::pixelDistance(const QPointF& Target, double, const Projection& theProjection) const
{
	return distance(Target,theProjection.project(Position));
}

void TrackPoint::cascadedRemoveIfUsing(MapDocument*, MapFeature*, CommandList*, const std::vector<MapFeature*>&)
{
}

QString TrackPoint::description() const
{
	QString s(tagValue("name",""));
	if (!s.isEmpty())
		return QString("%1 (node %2)").arg(s).arg(id());
	return
		QString("node %1").arg(id());
}

void TrackPoint::partChanged(MapFeature*, unsigned int)
{
}

RenderPriority TrackPoint::renderPriority(FeaturePainter::ZoomType) const
{
	return RenderPriority(RenderPriority::IsSingular,0);
}




