#ifndef MERKAARTOR_INTERACTION_H_
#define MERKAARTOR_INTERACTION_H_

#include "Interaction/Interaction.h"

class MoveTrackPointInteraction;

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
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	public slots:
		void on_remove_triggered();
		void on_reverse_triggered();

	private:
		bool Dragging;
		Coord StartDrag;
		Coord EndDrag;

		EditModeEnum currentMode;

		MoveTrackPointInteraction* theMoveInteraction;
		QCursor moveCursor;
		QCursor defaultCursor;
};

#endif


