#ifndef MERKATOR_ZOOMINTERACTION_H_
#define MERKATOR_ZOOMINTERACTION_H_

#include "Interaction.h"

#include <QtCore/QPointF>

class MapView;

class ZoomInteraction : public Interaction
{
    public:
        ZoomInteraction();
        ~ZoomInteraction();

        virtual void mouseReleaseEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

    private:
        bool HaveFirstPoint;
        QPoint P1, P2;
};

#endif


