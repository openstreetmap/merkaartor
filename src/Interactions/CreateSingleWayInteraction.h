#ifndef MERKATOR_INTERACTION_CREATESINGLEWAYINTERACTION_H
#define MERKATOR_INTERACTION_CREATESINGLEWAYINTERACTION_H

#include "Interaction.h"

class MainWindow;
class Way;

class QDockWidget;

class CreateSingleWayInteraction : public FeatureSnapInteraction
{
	Q_OBJECT

	public:
		CreateSingleWayInteraction(MainWindow* Main, MapView* aView, Node * firstNode, bool aCurved);
		~CreateSingleWayInteraction();

		virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
		
	private:
		MainWindow* Main;
		QPointF LastCursor;
		Way* theRoad;
		Coord FirstPoint;
		Node* FirstNode;
		bool HaveFirst;
		bool Prepend;
		bool IsCurved;
		bool Creating;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
