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

	private:
		QCursor moveCursor() const;

	public slots:
		void on_remove_triggered();
		void on_reverse_triggered();

	private:
		bool Dragging;
		Coord StartDrag;
		Coord EndDrag;

		Coord calculateNewPosition(QMouseEvent* event, MapFeature* aLast, CommandList* theList);
		std::vector<TrackPoint*> Moving;
		std::vector<Coord> OriginalPosition;
		Coord StartDragPosition;

		bool MoveMode;
};

#endif


