#ifndef MERKATOR_MOVETRACKPOINTINTERACTION_H_
#define MERKATOR_MOVETRACKPOINTINTERACTION_H_

#include "Interaction/Interaction.h"
#include "Map/Coord.h"

class MoveTrackPointInteraction : public TrackPointSnapInteraction
{
	public:
		MoveTrackPointInteraction(MapView* aView);
		~MoveTrackPointInteraction(void);

		virtual void snapMousePressEvent(QMouseEvent * event, TrackPoint* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, TrackPoint* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, TrackPoint* aLast);\

	private:
		TrackPoint* Moving;
		Coord Orig;
};

#endif


