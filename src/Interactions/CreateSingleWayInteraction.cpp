#include "CreateSingleWayInteraction.h"
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

#include <QDockWidget>
#include <QPainter>

CreateSingleWayInteraction::CreateSingleWayInteraction(MainWindow* aMain, Node *firstNode, bool aCurved)
    : FeatureSnapInteraction(aMain), theRoad(0), FirstPoint(0,0),
     FirstNode(firstNode), HaveFirst(false), Prepend(false), IsCurved(aCurved), Creating(false)
     , SnapAngle(0)
     , ParallelMode(false)
{
    if (firstNode)
    {
        FirstPoint = firstNode->position();
        LastCursor = COORD_TO_XY(FirstPoint);
        HaveFirst = true;
        view()->setInteracting(true);
        if ((theRoad = Way::GetSingleParentRoad(firstNode))) {
            if (theRoad->isExtrimity(firstNode)) {
                Prepend = (theRoad->get(0) == firstNode) ? true : false;
            } else
                theRoad = NULL;

        }
    }
}

CreateSingleWayInteraction::~CreateSingleWayInteraction()
{
}

void CreateSingleWayInteraction::setSnapAngle(qreal angle)
{
    SnapAngle = angle;
}

void CreateSingleWayInteraction::setParallelMode(bool val)
{
    ParallelMode = val;
}

QString CreateSingleWayInteraction::toHtml()
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

void CreateSingleWayInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (theRoad && (!theRoad->layer() || theRoad->isDeleted())) { // The road was begon and then undoed. Restarting....
        HaveFirst = false;
        theRoad = NULL;
        Creating = false;
        view()->setInteracting(false);
    }

    if (HaveFirst)
    {
        QPointF PreviousPoint;
        if (theRoad && theRoad->size() && !Prepend)
            PreviousPoint = COORD_TO_XY(CAST_NODE(theRoad->get(theRoad->size()-1))->position());
        else
            PreviousPoint = COORD_TO_XY(FirstPoint);
        QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
        QPen TP(SomeBrush,qBound(3, int(view()->pixelPerM()*4+2), 10));
        ::draw(thePainter,TP,Feature::UnknownDirection, PreviousPoint,LastCursor ,4 ,view()->projection());

        Coord NewPoint = XY_TO_COORD(LastCursor);
        const qreal distance = FirstPoint.distanceFrom(NewPoint);

        QString distanceTag;
        if (distance < 1.0)
            distanceTag = QString("%1 m").arg(int(distance * 1000));
        else
            distanceTag = QString("%1 km").arg(distance, 0, 'f', 3);

        thePainter.drawText(LastCursor + QPointF(10,-10), distanceTag);
    }
    FeatureSnapInteraction::paintEvent(anEvent,thePainter);
}

void CreateSingleWayInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* lastSnap)
{
    if (Node* Pt = dynamic_cast<Node*>(lastSnap))
        LastCursor = COORD_TO_XY(Pt);
    else if (Way* R = dynamic_cast<Way*>(lastSnap))
    {
        Coord P(XY_TO_COORD(ev->pos()));
        findSnapPointIndex(R, P);
        LastCursor = COORD_TO_XY(P);
    }
    else if (theRoad && theRoad->size() > 1 && SnapAngle)
    {
        QLineF l1(COORD_TO_XY(theRoad->getNode(theRoad->size()-1)), COORD_TO_XY(theRoad->getNode(theRoad->size()-2)));
        QLineF l2(COORD_TO_XY(theRoad->getNode(theRoad->size()-1)), ev->pos());
        qreal a = l1.angleTo(l2);
        a = qRound(a/SnapAngle) * SnapAngle;
        l2.setAngle(l1.angle() + a);
        LastCursor = l2.p2().toPoint();
    }
    else if (HaveFirst && ParallelMode)
    {
#define CLEAR_DISTANCE 200
        QPointF PreviousPoint;
        if (theRoad && theRoad->size() && !Prepend)
            PreviousPoint = COORD_TO_XY(CAST_NODE(theRoad->get(theRoad->size()-1))->position());
        else
            PreviousPoint = COORD_TO_XY(FirstPoint);

        CoordBox HotZone(XY_TO_COORD(ev->pos()-QPoint(CLEAR_DISTANCE, CLEAR_DISTANCE)),XY_TO_COORD(ev->pos()+QPoint(CLEAR_DISTANCE, CLEAR_DISTANCE)));
        qreal BestDistanceNW = 9999, AngleNW = 0;
        qreal BestDistanceNE = 9999, AngleNE = 0;
        qreal* BestDistance = &BestDistanceNW;
        qreal* BestAngle = &BestDistanceNE;
        qreal curAngle = 666;

        Way* R;
        for (int j=0; j<document()->layerSize(); ++j) {
            QList < Feature* > ret = g_backend.indexFind(document()->getLayer(j), HotZone);
            foreach(Feature* F, ret) {
                if (!(R = CAST_WAY(F)))
                    continue;

                if (R->isHidden())
                    continue;
                if (R->notEverythingDownloaded())
                    continue;

                for (int i=0; i<R->size()-1; ++i)
                {
                    LineF F(COORD_TO_XY(R->getNode(i)),COORD_TO_XY(R->getNode(i+1)));
                    qreal D = F.capDistance(ev->pos());
                    if (D < CLEAR_DISTANCE) {
                        QLineF l(COORD_TO_XY(R->getNode(i)), COORD_TO_XY(R->getNode(i+1)));
                        qreal a = l.angle();
                        if ((a >= 0 && a < 90) || (a < -270 && a >= -360)) {
                            BestDistance = &BestDistanceNE;
                            BestAngle = &AngleNE;
                            curAngle = a;
                        } else if ((a >= 90 && a < 180) || (a < -180 && a >= -270)) {
                            BestDistance = &BestDistanceNW;
                            BestAngle = &AngleNW;
                            curAngle = a;
                        } else if ((a >= 180 && a < 270) || (a < -90 && a >= -180)) {
                            BestDistance = &BestDistanceNE;
                            BestAngle = &AngleNE;
                            curAngle = a - 180;
                        } else if ((a >= 270 && a < 360) || (a < 0 && a >= -90)) {
                            BestDistance = &BestDistanceNW;
                            BestAngle = &AngleNW;
                            curAngle = a - 180;
                        }

                        if (D < *BestDistance) {
                            *BestDistance = D;
                            *BestAngle = curAngle;
                        }
                    }
                }

                qDebug() << BestDistanceNE << BestDistanceNW << AngleNE << AngleNW;
            }
        }

        /* Check if for some reason not a single angle was found. */
        Q_ASSERT(curAngle >= -360 && curAngle <= 360);

        QLineF l(PreviousPoint, ev->pos());
        qreal a = l.angle();
        if ((a >= 0 && a < 90) || (a < -270 && a >= -360)) {
            if (BestDistanceNE < 9999)
                a = AngleNE;
        } else if ((a >= 90 && a < 180) || (a < -180 && a >= -270)) {
            if (BestDistanceNW < 9999)
                a = AngleNW;
        } else if ((a >= 180 && a < 270) || (a < -90 && a >= -180)) {
            if (BestDistanceNE < 9999)
                a = AngleNE - 180;
        } else if ((a >= 270 && a < 360) || (a < 0 && a >= -90)) {
            if (BestDistanceNW < 9999)
                a = AngleNW - 180;
        }
        l.setAngle(a);
        LastCursor = l.p2().toPoint();
    } else
        LastCursor = ev->pos();
    view()->update();
}

void CreateSingleWayInteraction::snapMousePressEvent(QMouseEvent* /* anEvent */, Feature* lastSnap)
{
    Q_UNUSED(lastSnap)
    Creating = true;
    view()->setInteracting(true);
}

void CreateSingleWayInteraction::snapMouseReleaseEvent(QMouseEvent* anEvent, Feature* lastSnap)
{
    if (M_PREFS->getMouseSingleButton() && anEvent->button() == Qt::RightButton) {  // Abort
        HaveFirst = false;
        theRoad = NULL;
        Creating = false;
        view()->setInteracting(false);
    } else
    if ( Creating && !panning() )
    {
        Node* Pt = dynamic_cast<Node*>(lastSnap);
        if (!HaveFirst)
        {
            HaveFirst = true;
            if (Pt)
                FirstNode = Pt;
            else if (Way* aRoad = dynamic_cast<Way*>(lastSnap))
            {
                Coord P(XY_TO_COORD(LastCursor));
                int SnapIdx = findSnapPointIndex(aRoad, P);
                Node* N = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), P);
                CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
                theList->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),N,true));
                theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,theMain->document()->getDirtyOrOriginLayer(aRoad)));
                document()->addHistory(theList);
                view()->invalidate(true, true, false);
                FirstNode = N;
            }
        }
        else
        {
            CommandList* L  = new CommandList();
            if (!theRoad)
            {
                Node* From = 0;
                theRoad = g_backend.allocWay(theMain->document()->getDirtyOrOriginLayer());
                L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),theRoad,true));
                if (FirstNode) {
                    if (FirstNode->isVirtual()) {
                        Way* aRoad = CAST_WAY(FirstNode->getParent(0));
                        int SnapIdx = aRoad->findVirtual(FirstNode)+1;
                        From = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(aRoad->layer()), *FirstNode);
                        From->setVirtual(false);
                        From->setPosition(FirstNode->position());
                        L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(aRoad->layer()),From,true));
                        L->add(new WayAddNodeCommand(aRoad,From,SnapIdx,main()->document()->getDirtyOrOriginLayer(aRoad->layer())));
                    } else {
                        From = FirstNode;
                        if (!From->isDirty() && !From->hasOSMId() && From->isUploadable())
                            L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),From,true));
                    }
                }
                else
                {
                    From = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), FirstPoint);
                    L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),From,true));
                }
                if (M_PREFS->getAutoSourceTag()) {
                    QStringList sl = theMain->document()->getCurrentSourceTags();
                    if (sl.size())
                        theRoad->setTag("source", sl.join(";"));
                }
                if (IsCurved)
                    theRoad->setTag("smooth","yes");
                L->add(new WayAddNodeCommand(theRoad,From));
                L->setDescription(MainWindow::tr("Create Road: %1").arg(theRoad->description()));
                L->setFeature(theRoad);
            }
            Node* To = 0;
            if (Pt) {
                To = Pt;
                if (!To->isDirty() && !To->hasOSMId() && To->isUploadable()) {
                    L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),To,true));
                    L->setDescription(MainWindow::tr("Create Node: %1").arg(To->description()));
                }
            }
            else if (Way* aRoad = dynamic_cast<Way*>(lastSnap))
            {
                Coord P(XY_TO_COORD(LastCursor));
                int SnapIdx = findSnapPointIndex(aRoad, P);
                Node* N = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), P);
                CommandList* theList  = new CommandList(MainWindow::tr("Create Node %1 in Road %2").arg(N->description()).arg(aRoad->description()), N);
                theList->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),N,true));
                theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,theMain->document()->getDirtyOrOriginLayer(aRoad)));
                document()->addHistory(theList);
                view()->invalidate(true, true, false);
                To = N;
            }
            if (!To)
            {
                To = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), XY_TO_COORD(LastCursor));
                L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),To,true));
                L->setDescription(MainWindow::tr("Create Node %1 in Road %2").arg(To->description()).arg(theRoad->description()));
                L->setFeature(To);
            }
            L->setDescription(MainWindow::tr("Add Node %1 to Road %2").arg(To->description()).arg(theRoad->description()));
            if (Prepend)
                L->add(new WayAddNodeCommand(theRoad,To,(int)0,theMain->document()->getDirtyOrOriginLayer(theRoad)));
            else
                L->add(new WayAddNodeCommand(theRoad,To,theMain->document()->getDirtyOrOriginLayer(theRoad)));
            document()->addHistory(L);
            view()->invalidate(true, true, false);
            theMain->properties()->setSelection(theRoad);
        }
        FirstPoint = XY_TO_COORD(LastCursor);
    }
    Creating = false;
    LastCursor = anEvent->pos();
}

void CreateSingleWayInteraction::snapMouseDoubleClickEvent(QMouseEvent* anEvent, Feature*)
{
    HaveFirst = false;
    theRoad = NULL;
    Creating = false;
    view()->setInteracting(false);

    if ((lastSnap() && lastSnap()->getType() & IFeature::LineString) || !lastSnap())
        CreateNodeInteraction::createNode(XY_TO_COORD(anEvent->pos()), lastSnap());
}

#ifndef _MOBILE
QCursor CreateSingleWayInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif

void CreateSingleWayInteraction::closeAndFinish()
{
    if (!theRoad || theRoad->size() < 3)
        return;

    Node* N = theRoad->getNode(0);
    CommandList* theList  = new CommandList(MainWindow::tr("Close Road %1").arg(theRoad->description()), theRoad);
    theList->add(new WayAddNodeCommand(theRoad,N,theMain->document()->getDirtyOrOriginLayer(theRoad)));
    document()->addHistory(theList);
    view()->invalidate(true, true, false);
    theMain->properties()->setSelection(theRoad);

    HaveFirst = false;
    theRoad = NULL;
    Creating = false;
    view()->setInteracting(false);
}
