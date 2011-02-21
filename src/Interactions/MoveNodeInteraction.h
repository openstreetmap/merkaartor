#ifndef MERKATOR_MOVENODEINTERACTION_H_
#define MERKATOR_MOVENODEINTERACTION_H_

#include "Interaction.h"
#include "Coord.h"

#include <QList>

class CommandList;

class MoveNodeInteraction : public FeatureSnapInteraction
{
    public:
        MoveNodeInteraction(MapView* aView);
        ~MoveNodeInteraction(void);

        virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

        virtual bool isIdle();

    private:
        Coord calculateNewPosition(QMouseEvent* event, Feature* aLast, CommandList* theList);
        QList<Node*> Moving;
        QList<Coord> OriginalPosition;
        Coord StartDragPosition;
        CommandList* theList;
        bool Virtual;
        bool HasMoved;
};

#endif


