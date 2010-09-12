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
        ExtrudeInteraction(MapView* aView, Way* aRoad);
        ~ExtrudeInteraction();

        virtual void snapMousePressEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseReleaseEvent(QMouseEvent * event, Feature* aLast);
        virtual void snapMouseMoveEvent(QMouseEvent* event, Feature* aLast);
        virtual void snapMouseDoubleClickEvent(QMouseEvent* , Feature*);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef Q_OS_SYMBIAN
        virtual QCursor cursor() const;
#endif

public:
        void setSnapAngle(double angle);
        double snapAngle();

        virtual void closeAndFinish();

    private:
        Way* theRoad;
        QPointF LastCursor;
        bool Creating;
        double SnapAngle;
        int BestSegment;
        QLineF OrigSegment;
};

#endif // MERKATOR_INTERACTION_EXTRUDE_H
