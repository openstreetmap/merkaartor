#ifndef INTERACTION_CREATEPOLYGONINTERACTION_H
#define INTERACTION_CREATEPOLYGONINTERACTION_H

#include "Interaction/Interaction.h"
#include "Maps/Coord.h"

class CreatePolygonInteraction : public Interaction
{
	Q_OBJECT

	public:
		CreatePolygonInteraction(MainWindow* Main, MapView* aView, int sides);
		~CreatePolygonInteraction();

		virtual void mousePressEvent(QMouseEvent * event);
		virtual void mouseMoveEvent(QMouseEvent* event);
		virtual void mouseReleaseEvent(QMouseEvent* event);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		void testIntersections(CommandList* L, Road* Left, int FromIdx, Road* Right, int RightIdx);
		MainWindow* Main;
		QDockWidget* theDock;
		Coord Origin;
		QPointF OriginF;
		int Sides;
		QPointF LastCursor;
		bool HaveOrigin;

		double bAngle;
		QPointF bScale;
};

#endif // INTERACTION\CreatePolygonInteraction_H
