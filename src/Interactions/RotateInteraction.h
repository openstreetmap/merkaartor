#ifndef MERKATOR_ROTATEINTERACTION_H_
#define MERKATOR_ROTATEINTINTERACTION_H_

#include "Interaction.h"
#include "Coord.h"

#include <QList>

class CommandList;

class RotateInteraction : public FeatureSnapInteraction
{
    public:
        RotateInteraction(MainWindow* aMain);
        ~RotateInteraction(void);

        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

    private:
        Coord rotatePosition(Coord position, qreal angle);
        qreal calculateNewAngle(QMouseEvent* event);
        QList<Node*> Rotating;
        QList<Coord> OriginalPosition;
        Coord StartDragPosition;
        bool NodeOrigin;
        Node* OriginNode;
        QPointF RotationCenter;
        qreal Angle;

        QCursor rotateCursor;
};

#endif


