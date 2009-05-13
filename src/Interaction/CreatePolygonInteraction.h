#ifndef INTERACTION_CREATEPOLYGONINTERACTION_H
#define INTERACTION_CREATEPOLYGONINTERACTION_H

#include "Interaction/Interaction.h"
#include "Maps/Coord.h"

class CreatePolygonInteraction : public Interaction
{
	Q_OBJECT

	public:
		CreatePolygonInteraction(MainWindow* Main, MapView* aView);
		~CreatePolygonInteraction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
		
	private:
		void testIntersections(CommandList* L, Road* Left, int FromIdx, Road* Right, int RightIdx);
		MainWindow* Main;
		QDockWidget* theDock;
		Coord Center;
        int Sides;
		QPointF LastCursor;
		bool HaveCenter;
};

#endif // INTERACTION\CreatePolygonInteraction_H
