#ifndef MERKATOR_CREATEWAYINTERACTION_H_
#define MERKATOR_CREATEWAYINTERACTION_H_

#include "Interaction/Interaction.h"
#include "Map/Coord.h"

#include <QtCore/QPointF>

class MapView;
class MainWindow;
class TrackPoint;
class Main;

class CreateWayInteraction : public TrackPointSnapInteraction
{
	public:
		CreateWayInteraction(MainWindow* aMain, MapView* aView, bool aBezier);
		~CreateWayInteraction();

		virtual void snapMouseReleaseEvent(QMouseEvent * event, TrackPoint* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, TrackPoint* aLast);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QCursor cursor() const;

	private:
		bool HaveFirstPoint;
		Coord P1, P2;
		TrackPoint* From;
		bool BezierWay;
		MainWindow* Main;
};

#endif


