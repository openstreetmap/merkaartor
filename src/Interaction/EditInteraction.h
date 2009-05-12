#ifndef MERKAARTOR_INTERACTION_H_
#define MERKAARTOR_INTERACTION_H_

#include "Interaction/Interaction.h"

class EditInteraction :	public FeatureSnapInteraction
{
	Q_OBJECT

	public:
		typedef enum { EditMode, MoveMode, RotateMode, ScaleMode } EditModeEnum;

		EditInteraction(MapView* theView);
		~EditInteraction(void);

		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);

	private:
#ifndef Q_OS_SYMBIAN
		QCursor moveCursor() const;
#endif

	public slots:
		void on_remove_triggered();
		void on_reverse_triggered();

	private:
		bool Dragging;
		Coord StartDrag;
		Coord EndDrag;

		Coord calculateNewPosition(QMouseEvent* event, MapFeature* aLast, CommandList* theList);
		QList<TrackPoint*> Moving;
		QList<Coord> OriginalPosition;
		Coord StartDragPosition;

		EditModeEnum currentMode;
		bool Moved;
};

#endif


