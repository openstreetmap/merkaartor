#include "ExtrudeInteraction.h"
#include "CreateNodeInteraction.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Painting.h"
#include "Way.h"
#include "Node.h"
#include "LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"
#include "Global.h"

#include <QtGui/QPainter>

ExtrudeInteraction::ExtrudeInteraction(MapView* aView)
    : FeatureSnapInteraction(aView)
    , Creating(false)
    , BestSegment(-1)
{
}

ExtrudeInteraction::~ExtrudeInteraction()
{
}

void ExtrudeInteraction::setSnapAngle(qreal angle)
{
    SnapAngle = angle;
}

qreal ExtrudeInteraction::snapAngle()
{
    return SnapAngle;
}

QString ExtrudeInteraction::toHtml()
{
    QString help;
    //help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create way Interaction"));
    desc += QString("<b>%1</b><br/>").arg(help);

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "</body></html>";

    return S;
}

void ExtrudeInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (Creating) {
        QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
        QPen TP(SomeBrush,qBound(3, int(view()->pixelPerM()*4+2), 10));
        thePainter.setRenderHint(QPainter::Antialiasing);
        thePainter.setPen(TP);

        QLineF l(OrigSegment.p1(), LastCursor);
        qreal a = OrigSegment.angleTo(l);
        qreal largeur = sin(angToRad(a))*l.length();

        QLineF n = OrigSegment.normalVector();
        n.setLength(largeur);

        QPointF pb = n.p2();
        QLineF s2 = QLineF::fromPolar(OrigSegment.length(), OrigSegment.angle());
        s2.translate(pb);

        thePainter.drawLine(s2.p1(), s2.p2());
        thePainter.drawLine(OrigSegment.p1(), s2.p1());
        thePainter.drawLine(OrigSegment.p2(), s2.p2());
    }

    FeatureSnapInteraction::paintEvent(anEvent,thePainter);
}

void ExtrudeInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* lastSnap)
{
    Q_UNUSED(lastSnap)

    LastCursor = ev->posF();
    view()->update();
}

void ExtrudeInteraction::snapMousePressEvent(QMouseEvent* anEvent, Feature* lastSnap)
{
    Q_UNUSED(lastSnap)

    theRoad = CAST_WAY(lastSnap);
    if (!theRoad)
        return;

    if (theRoad->bestSegment() != -1) {
        Creating = true;
        QLineF l = theRoad->getSegment(theRoad->bestSegment());
        BestSegment = theRoad->bestSegment();
        OrigSegment = QLineF(COORD_TO_XY(Coord(l.p1())), COORD_TO_XY(Coord(l.p2())));
        LastCursor = anEvent->pos();
    }
}

void ExtrudeInteraction::snapMouseReleaseEvent(QMouseEvent* anEvent, Feature* lastSnap)
{
    Q_UNUSED(anEvent)
    Q_UNUSED(lastSnap)

    if (Creating) {
        QLineF l(OrigSegment.p1(), anEvent->posF());
        qreal a = OrigSegment.angleTo(l);
        qreal largeur = sin(angToRad(a))*l.length();

        QLineF n = OrigSegment.normalVector();
        n.setLength(largeur);

        QPointF pb = n.p2();
        QLineF s2 = QLineF::fromPolar(OrigSegment.length(), OrigSegment.angle());
        s2.translate(pb);

        CommandList* theList  = new CommandList(MainWindow::tr("Extrude Road %1").arg(theRoad->description()), theRoad);
        if (theRoad->segmentCount() == 1) {
            int pos = 0;
            Node* N = g_backend.allocNode(theRoad->layer(), XY_TO_COORD(s2.p1().toPoint()));
            theList->add(new AddFeatureCommand(theRoad->layer(), N, true));
            theList->add(new WayAddNodeCommand(theRoad, N, ++pos));
            N = g_backend.allocNode(theRoad->layer(), XY_TO_COORD(s2.p2().toPoint()));
            theList->add(new AddFeatureCommand(theRoad->layer(), N, true));
            theList->add(new WayAddNodeCommand(theRoad, N, ++pos));
            theList->add(new WayAddNodeCommand(theRoad, theRoad->getNode(0)));
        } else {
            int pos = BestSegment;
            Node* N = g_backend.allocNode(theRoad->layer(), XY_TO_COORD(s2.p1().toPoint()));
            theList->add(new AddFeatureCommand(theRoad->layer(), N, true));
            theList->add(new WayAddNodeCommand(theRoad, N, ++pos));
            N = g_backend.allocNode(theRoad->layer(), XY_TO_COORD(s2.p2().toPoint()));
            theList->add(new AddFeatureCommand(theRoad->layer(), N, true));
            theList->add(new WayAddNodeCommand(theRoad, N, ++pos));
        }


        document()->addHistory(theList);
        view()->invalidate(true, false);
    }
    Creating = false;
}

void ExtrudeInteraction::snapMouseDoubleClickEvent(QMouseEvent* anEvent, Feature*)
{
    Q_UNUSED(anEvent)
}

#ifndef _MOBILE
QCursor ExtrudeInteraction::cursor() const
{
    return QCursor(Qt::SplitHCursor);
}
#endif

void ExtrudeInteraction::closeAndFinish()
{
}
