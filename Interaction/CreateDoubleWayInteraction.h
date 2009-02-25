#ifndef MERKATOR_INTERACTION_CREATEDOUBLEWAYINTERACTION_H
#define MERKATOR_INTERACTION_CREATEDOUBLEWAYINTERACTION_H

#include <ui_CreateDoubleWayDock.h>
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
		virtual QCursor cursor() const;
		
	private:
		MainWindow* Main;
		QDockWidget* theDock;
		Ui::CreateDoubleWayDock DockData;
		QPoint LastCursor;
		Road* R1;
		Road* R2;
		Coord FirstPoint;
		double FirstDistance;
		bool HaveFirst;
		QHash<int, Coord> PreviousPoints;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
