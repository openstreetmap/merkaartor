#ifndef MERKAARTOR_CREATENODEINTERATION_H_
#define MERKAARTOR_CREATENODEINTERATION_H_

#include "Interaction/Interaction.h"

class CreateNodeInteraction : public RoadSnapInteraction
{
	public:
		CreateNodeInteraction(MapView* aView);
		~CreateNodeInteraction(void);

		virtual void snapMouseReleaseEvent(QMouseEvent * event, Road* aRoad);
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
};

#endif
