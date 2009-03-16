#ifndef INTERACTION_CREATEROUNDABOUTINTERACTION_H
#define INTERACTION_CREATEROUNDABOUTINTERACTION_H

#include <ui_CreateRoundaboutDock.h>
#include "Interaction/Interaction.h"
#include "Map/Coord.h"

class QDockWidget;

class CreateRoundaboutInteraction : public Interaction
{
	Q_OBJECT

	public:
		CreateRoundaboutInteraction(MainWindow* Main, MapView* aView);
		~CreateRoundaboutInteraction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QCursor cursor() const;
		
	private:
		void testIntersections(CommandList* L, Road* Left, unsigned int FromIdx, Road* Right, unsigned int RightIdx);
		MainWindow* Main;
		QDockWidget* theDock;
		Ui::CreateRoundaboutDock DockData;
		Coord Center;
		QPointF LastCursor;
		bool HaveCenter;
};

#endif // INTERACTION\CREATEROUNDABOUTINTERACTION_H
