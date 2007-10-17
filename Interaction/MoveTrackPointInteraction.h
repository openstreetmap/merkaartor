#ifndef MERKATOR_MOVETRACKPOINTINTERACTION_H_
#define MERKATOR_MOVETRACKPOINTINTERACTION_H_

#include "Interaction/Interaction.h"
#include "Map/Coord.h"

#include <vector>

class MoveTrackPointInteraction : public FeatureSnapInteraction
{
	public:
		MoveTrackPointInteraction(MapView* aView);
		~MoveTrackPointInteraction(void);

		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
		virtual QCursor cursor() const;

	private:
		Coord calculateNewPosition(QMouseEvent* event, MapFeature* aLast);
		std::vector<TrackPoint*> Moving;
		std::vector<Coord> OriginalPosition;
		Coord StartDragPosition;
};

#endif


