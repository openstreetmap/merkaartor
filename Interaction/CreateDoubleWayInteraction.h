#ifndef MERKATOR_INTERACTION_CREATEDOUBLEWAYINTERACTION_H
#define MERKATOR_INTERACTION_CREATEDOUBLEWAYINTERACTION_H

#include "GeneratedFiles/ui_CreateDoubleWayDock.h"
#include "Interaction/Interaction.h"

class MainWindow;
class Road;
class Way;

class QDockWidget;

class CreateDoubleWayInteraction : public Interaction
{
	Q_OBJECT

	public:
		CreateDoubleWayInteraction(MainWindow* Main, MapView* aView);
		~CreateDoubleWayInteraction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);

	private:
		MainWindow* Main;
		QDockWidget* theDock;
		Ui::CreateDoubleWayDock DockData;
		QPointF LastCursor;
		Road* R1;
		Road* R2;
		Coord FirstPoint;
		double FirstDistance;
		bool HaveFirst;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
