#ifndef INTERACTION_CREATEROUNDABOUTINTERACTION_H
#define INTERACTION_CREATEROUNDABOUTINTERACTION_H

#include <ui_CreateRoundaboutDock.h>
#include "Interaction.h"
#include "Maps/Coord.h"

class QDockWidget;

class CreateRoundaboutInteraction : public Interaction
{
	Q_OBJECT

	public:
		CreateRoundaboutInteraction(MainWindow* Main, MapView* aView);
		~CreateRoundaboutInteraction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		void testIntersections(CommandList* L, Way* Left, int FromIdx, Way* Right, int RightIdx);
		MainWindow* Main;
		QDockWidget* theDock;
		Ui::CreateRoundaboutDock DockData;
		Coord Center;
		QPointF LastCursor;
		bool HaveCenter;
};

#endif // INTERACTION\CREATEROUNDABOUTINTERACTION_H
