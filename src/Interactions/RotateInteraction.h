#ifndef MERKATOR_ROTATEINTERACTION_H_
#define MERKATOR_ROTATEINTINTERACTION_H_

#include "Interaction.h"
#include "Maps/Coord.h"

#include <QList>

class CommandList;

class RotateInteraction : public FeatureSnapInteraction
{
	public:
		RotateInteraction(MapView* aView);
		~RotateInteraction(void);

		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		Coord rotatePosition(Coord position, double angle, double radius);
		double calculateNewAngle(QMouseEvent* event);
		QList<Node*> Rotating;
		QList<Coord> OriginalPosition;
		Coord StartDragPosition;
		QPointF RotationCenter;
		double Angle;
		double Radius;

		QCursor rotateCursor;
};

#endif


