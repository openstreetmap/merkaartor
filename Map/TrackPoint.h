#ifndef MERKATOR_TRACKPOINT_H_
#define MERKATOR_TRACKPOINT_H_

#include "Map/Coord.h"
#include "Map/MapFeature.h"

#include <QtCore/QDateTime>

class TrackPoint : public MapFeature
{
	public:
		TrackPoint(const Coord& aCoord);
		TrackPoint(const TrackPoint& other);
		virtual ~TrackPoint();

		virtual CoordBox boundingBox() const;
		virtual void draw(QPainter& P, const Projection& theProjection);
		virtual void drawFocus(QPainter& P, const Projection& theProjection);
		virtual double pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const;
		virtual void cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Alternatives);
		virtual bool notEverythingDownloaded() const;
		virtual QString description() const;

		const Coord& position() const;
		void setPosition(const Coord& aCoord);
		const QDateTime& time() const;
		void setTime(const QDateTime& aTime);

		virtual void addedToDocument();
		virtual void removedFromDocument();
		virtual void partChanged(MapFeature* F);
	private:
		Coord Position;
		QDateTime Time;
};

#endif


