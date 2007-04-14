#ifndef MERKAARTOR_CREATENODEINTERATION_H_
#define MERKAARTOR_CREATENODEINTERATION_H_

#include "Interaction/Interaction.h"

class CreateNodeInteraction : public WaySnapInteraction
{
	public:
		CreateNodeInteraction(MapView* aView);
		~CreateNodeInteraction(void);

		virtual void snapMouseReleaseEvent(QMouseEvent * event, Way* aWay);
		virtual QCursor cursor() const;
};

#endif