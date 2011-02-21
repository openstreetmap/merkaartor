#ifndef INTERACTION_CREATEPOLYGONINTERACTION_H
#define INTERACTION_CREATEPOLYGONINTERACTION_H

#include "Interaction.h"
#include "Coord.h"

class CreatePolygonInteraction : public Interaction
{
    Q_OBJECT

    public:
        CreatePolygonInteraction(MainWindow* Main, MapView* aView, int sides, const QList< QPair <QString, QString> >& tags);
        ~CreatePolygonInteraction();

        virtual void mousePressEvent(QMouseEvent * event);
        virtual void mouseMoveEvent(QMouseEvent* event);
        virtual void mouseReleaseEvent(QMouseEvent* event);
        virtual void paintEvent(QPaintEvent* anEvent, QPainter& thePainter);
        virtual QString toHtml();
#ifndef _MOBILE
        virtual QCursor cursor() const;
#endif

    private:
        MainWindow* Main;
        Coord Origin;
        QPointF OriginF;
        int Sides;
        QPointF LastCursor;
        bool HaveOrigin;

        qreal bAngle;
        QPointF bScale;
        QList< QPair <QString, QString> > theTags;
};

#endif // INTERACTION\CreatePolygonInteraction_H
