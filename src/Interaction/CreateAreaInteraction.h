#ifndef MERKATOR_INTERACTION_CREATEAREAINTERACTION_H
#define MERKATOR_INTERACTION_CREATEAREAINTERACTION_H

#include "Interaction/Interaction.h"

class MainWindow;
class Road;
class Way;

class QDockWidget;

class CreateAreaInteraction : public FeatureSnapInteraction
{
	Q_OBJECT

	public:
		CreateAreaInteraction(MainWindow* Main, MapView* aView);
		~CreateAreaInteraction();

		virtual void snapMousePressEvent(QMouseEvent * event, MapFeature* aLast);
		virtual void snapMouseMoveEvent(QMouseEvent* event, MapFeature* aLast);
		virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
		virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
		virtual QCursor cursor() const;
#endif
		
	private:
		void startNewRoad(QMouseEvent* anEvent, MapFeature* Snap);
		void createNewRoad(CommandList* L);
		void addToRoad(QMouseEvent* anEvent, MapFeature* Snap, CommandList* L);
		void finishRoad(CommandList* L);

		MainWindow* Main;
		QPointF LastCursor;
		Relation* theRelation;
		Road* theRoad;
		Road* LastRoad;
		Coord FirstPoint;
		TrackPoint* FirstNode;
		bool HaveFirst, EndNow;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
