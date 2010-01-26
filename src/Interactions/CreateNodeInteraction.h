#ifndef MERKAARTOR_CREATENODEINTERATION_H_
#define MERKAARTOR_CREATENODEINTERATION_H_

#include "Interaction.h"

class MoveNodeInteraction;

class CreateNodeInteraction : public FeatureSnapInteraction
{
	public:
		CreateNodeInteraction(MapView* aView);
		~CreateNodeInteraction(void);


		virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
    private:
		MoveNodeInteraction* theMoveInteraction;
};

#endif
