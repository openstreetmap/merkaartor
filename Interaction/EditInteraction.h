#ifndef MERKAARTOR_INTERACTION_H_
#define MERKAARTOR_INTERACTION_H_

#include "Interaction/Interaction.h"

class EditInteraction :	public FeatureSnapInteraction
{
	Q_OBJECT

	public:
		EditInteraction(MapView* theView);
		~EditInteraction(void);

		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);

	public slots:
		void on_remove_triggered();
		void on_move_triggered();
		void on_add_triggered();
		void on_reverse_triggered();
};

#endif


