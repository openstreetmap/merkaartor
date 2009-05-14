#ifndef MERKATOR_ROTATEINTERACTION_H_
#define MERKATOR_ROTATEINTINTERACTION_H_

#include "Interaction/Interaction.h"
#include "Maps/Coord.h"

#include <QList>

class CommandList;

class RotateInteraction : public FeatureSnapInteraction
{
	public:
		RotateInteraction(MapView* aView);
		~RotateInteraction(void);

		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		Coord rotatePosition(Coord position, double angle, double radius);
		double calculateNewAngle(QMouseEvent* event);
		QList<TrackPoint*> Rotating;
		QList<Coord> OriginalPosition;
		Coord StartDragPosition;
		QPointF RotationCenter;
		double Angle;
		double Radius;
};

#endif


