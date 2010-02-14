#ifndef MERKATOR_INTERACTION_CREATEAREAINTERACTION_H
#define MERKATOR_INTERACTION_CREATEAREAINTERACTION_H

#include "Interaction.h"

class MainWindow;
class Way;
class Way;

class QDockWidget;

class CreateAreaInteraction : public FeatureSnapInteraction
{
	Q_OBJECT

	public:
		CreateAreaInteraction(MainWindow* Main, MapView* aView);
		~CreateAreaInteraction();

		virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif

	private:
		void startNewRoad(QMouseEvent* anEvent, Feature* Snap);
		void createNewRoad(CommandList* L);
		void addToRoad(QMouseEvent* anEvent, Feature* Snap, CommandList* L);
		void finishRoad(CommandList* L);

		MainWindow* Main;
		QPointF LastCursor;
		Relation* theRelation;
		Way* theRoad;
		Way* LastRoad;
		Coord FirstPoint;
		Node* FirstNode;
		bool HaveFirst, EndNow;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
