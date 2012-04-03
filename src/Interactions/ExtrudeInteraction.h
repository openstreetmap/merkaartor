#ifndef MERKATOR_INTERACTION_EXTRUDE_H
#define MERKATOR_INTERACTION_EXTRUDE_H

#include "Interaction.h"

class MainWindow;
class Way;

class QDockWidget;

class ExtrudeInteraction : public FeatureSnapInteraction
{
    Q_OBJECT

    public:
        ExtrudeInteraction(MainWindow* aMain);
        ~ExtrudeInteraction();

        virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual void snapMouseDoubleClickEvent(QMouseEvent* , Feature*);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

public:
        void setSnapAngle(qreal angle);
        qreal snapAngle();

        virtual void closeAndFinish();

    private:
        Way* theRoad;
        QPointF LastCursor;
        bool Creating;
        qreal SnapAngle;
        int BestSegment;
        QLineF OrigSegment;
};

#endif // MERKATOR_INTERACTION_EXTRUDE_H
