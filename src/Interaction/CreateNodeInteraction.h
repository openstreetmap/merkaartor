#ifndef MERKAARTOR_CREATENODEINTERATION_H_
#define MERKAARTOR_CREATENODEINTERATION_H_

#include "Interaction/Interaction.h"

class MoveTrackPointInteraction;

class CreateNodeInteraction : public FeatureSnapInteraction
{
	public:
		CreateNodeInteraction(MapView* aView);
		~CreateNodeInteraction(void);


        virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
    private:
        MoveTrackPointInteraction* theMoveInteraction;
};

#endif
