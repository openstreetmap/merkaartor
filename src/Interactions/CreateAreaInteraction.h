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
        CreateAreaInteraction();
        ~CreateAreaInteraction();

        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

        virtual void closeAndFinish();

    private:
        void startNewRoad(QMouseEvent* anEvent, Feature* Snap);
        void createNewRoad(CommandList* L);
        void addToRoad(QMouseEvent* anEvent, Feature* Snap, CommandList* L);
        void finishRoad(CommandList* L);

        QPoint LastCursor;
        Relation* theRelation;
        Way* theRoad;
        Way* LastRoad;
        Coord FirstPoint;
        Node* FirstNode;
        bool HaveFirst, EndNow;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
