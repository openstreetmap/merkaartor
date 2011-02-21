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
        virtual void snapMouseDoubleClickEvent(QMouseEvent* , Feature*);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

public:
        void setSnapAngle(qreal angle);
        void setParallelMode(bool val);

        virtual void closeAndFinish();

    private:
        MainWindow* Main;
        QPoint LastCursor;
        Way* theRoad;
        Coord FirstPoint;
        Node* FirstNode;
        bool HaveFirst;
        bool Prepend;
        bool IsCurved;
        bool Creating;
        qreal SnapAngle;
        bool ParallelMode;
};

#endif // INTERACTION\CREATEDOUBLEWAYINTERACTION_H
