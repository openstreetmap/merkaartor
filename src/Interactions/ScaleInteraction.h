#ifndef MERKATOR_SCALEINTERACTION_H_
#define MERKATOR_SCALEINTERACTION_H_

#include "Interaction.h"
#include "Maps/Coord.h"

#include <QList>

class CommandList;

class ScaleInteraction : public FeatureSnapInteraction
{
    public:
        ScaleInteraction(MapView* aView);
        ~ScaleInteraction(void);

        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
        virtual QCursor cursor() const;
#endif

    private:
        Coord scalePosition(Coord position, double radius);
        QList<Node*> Scaling;
        QList<Coord> OriginalPosition;
        Coord StartDragPosition;
        bool NodeOrigin;
        Node* OriginNode;
        QPointF ScaleCenter;
        double Radius;

        QCursor rotateCursor;
};

#endif // MERKATOR_SCALEINTERACTION_H_


