#ifndef INTERACTION_CREATEROUNDABOUTINTERACTION_H
#define INTERACTION_CREATEROUNDABOUTINTERACTION_H

#include <ui_CreateRoundaboutDock.h>
#include "Interaction.h"
#include "Coord.h"

class QDockWidget;

class CreateRoundaboutInteraction : public Interaction
{
    Q_OBJECT

    public:
        CreateRoundaboutInteraction(MainWindow* aMain);
        ~CreateRoundaboutInteraction();

        virtual void mousePressEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

    private:
        QDockWidget* theDock;
        Ui::CreateRoundaboutDock DockData;
        Coord Center;
        QPointF LastCursor;
        bool HaveCenter;
};

#endif // INTERACTION\CREATEROUNDABOUTINTERACTION_H
