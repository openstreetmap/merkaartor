#ifndef MERKATOR_MOVETRACKPOINTINTERACTION_H_
#define MERKATOR_MOVETRACKPOINTINTERACTION_H_

#include "Interaction/Interaction.h"
#include "Maps/Coord.h"

#include <QList>

class CommandList;

class MoveTrackPointInteraction : public FeatureSnapInteraction
{
	public:
		MoveTrackPointInteraction(MapView* aView);
		~MoveTrackPointInteraction(void);

		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		Coord calculateNewPosition(QMouseEvent* event, MapFeature* aLast, CommandList* theList);
		QList<TrackPoint*> Moving;
		QList<Coord> OriginalPosition;
		Coord StartDragPosition;
};

#endif


