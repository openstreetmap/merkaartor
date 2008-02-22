#ifndef MERKATOR_INTERACTION_CREATESINGLEWAYINTERACTION_H
#define MERKATOR_INTERACTION_CREATESINGLEWAYINTERACTION_H

#include "Interaction/Interaction.h"

class MainWindow;
class Road;
class Way;

class QDockWidget;

class CreateSingleWayInteraction : public GenericFeatureSnapInteraction<MapFeature>
{
	Q_OBJECT

	public:
		CreateSingleWayInteraction(MainWindow* Main, MapView* aView, bool aCurved);
		~CreateSingleWayInteraction();

		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);

	private:
		MainWindow* Main;
		QPointF LastCursor;
		Road* theRoad;
		Coord FirstPoint;
		TrackPoint* FirstNode;
		bool HaveFirst;
		bool IsCurved;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
