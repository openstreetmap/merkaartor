#include "CreateAreaInteraction.h"
#include "DocumentCommands.h"
#include "RelationCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Painting.h"
#include "Relation.h"
#include "Way.h"
#include "Node.h"
#include "LineF.h"
#include "MainWindow.h"
#include "PropertiesDock.h"
#include "MDiscardableDialog.h"
#include "Global.h"

#include <QtGui/QDockWidget>
#include <QtGui/QMessageBox>
#include <QtGui/QPainter>

CreateAreaInteraction::CreateAreaInteraction(MainWindow* aMain)
    : FeatureSnapInteraction(aMain),
      theRelation(0), theRoad(0), LastRoad(0), FirstPoint(0,0),
      FirstNode(0), HaveFirst(false), EndNow(false)
{
}

CreateAreaInteraction::~CreateAreaInteraction()
{
}

QString CreateAreaInteraction::toHtml()
{
    QString help;
    //help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create Area Interaction"));
    desc += QString("<b>%1</b><br/>").arg(help);

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "</body></html>";

    return S;
}

void CreateAreaInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (theRoad && (!theRoad->layer() || theRoad->isDeleted())) { // The road was begon and then undoed. Restarting....
        HaveFirst = false;
        theRoad = NULL;
        view()->setInteracting(false);
    }

    if (HaveFirst)
    {
        QPointF PreviousPoint;
        if (theRoad && theRoad->size())
            PreviousPoint = COORD_TO_XY(CAST_NODE(theRoad->get(theRoad->size()-1))->position());
        else
            PreviousPoint = COORD_TO_XY(FirstPoint);
        QBrush SomeBrush(QColor(0xff,0x77,0x11,128));
        QPen TP(SomeBrush,view()->pixelPerM()*4+2);
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

void CreateAreaInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* aFeature)
{
    if (Node* Pt = dynamic_cast<Node*>(aFeature))
        LastCursor = COORD_TO_XY(Pt);
    else if (Way* R = dynamic_cast<Way*>(aFeature))
    {
        Coord P(XY_TO_COORD(ev->pos()));
        findSnapPointIndex(R, P);
        LastCursor = COORD_TO_XY(P);
    }
    else
        LastCursor = ev->pos();
    view()->update();
}

void CreateAreaInteraction::startNewRoad(QMouseEvent* anEvent, Feature* aFeature)
{
    if (Node* Pt = CAST_NODE(aFeature))
        FirstNode = Pt;
    else if (Way* aRoad = CAST_WAY(aFeature))
    {
        Coord P(XY_TO_COORD(anEvent->pos()));
        CommandList* theList  = new CommandList(MainWindow::tr("Create Area %1").arg(aRoad->description()), aRoad);
        int SnapIdx = findSnapPointIndex(aRoad, P);
        Node* N = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), P);
        theList->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),N,true));
        theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx));
        document()->addHistory(theList);
        view()->invalidate(true, true, false);
        FirstNode = N;
    }
    view()->setInteracting(true);
}

void CreateAreaInteraction::createNewRoad(CommandList* L)
{
    Node* From = 0;
    theRoad = g_backend.allocWay(theMain->document()->getDirtyOrOriginLayer());
    if (FirstNode)
    {
        From = FirstNode;
        FirstNode = 0;
        if (!From->isDirty() && !From->hasOSMId() && From->isUploadable())
            L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),From,true));
    }
    else
    {
        From = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), FirstPoint);
        L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),From,true));
    }
    L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),theRoad,true));
    if (M_PREFS->getAutoSourceTag()) {
        QStringList sl = theMain->document()->getCurrentSourceTags();
        if (sl.size())
            theRoad->setTag("source", sl.join(";"));
    }
    L->add(new WayAddNodeCommand(theRoad,From));
    L->setDescription(MainWindow::tr("Area: Create Road %1").arg(theRoad->description()));
    L->setFeature(theRoad);
}

void CreateAreaInteraction::finishRoad(CommandList* L)
{
    if (theRelation)
        L->add(new RelationAddFeatureCommand(theRelation,"inner",theRoad));
    else if (LastRoad)
    {
        theRelation = g_backend.allocRelation(theMain->document()->getDirtyOrOriginLayer());
        L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),theRelation,true));
        if (M_PREFS->getAutoSourceTag()) {
            QStringList sl = theMain->document()->getCurrentSourceTags();
            if (sl.size())
                theRelation->setTag("source", sl.join(";"));
        }
        theRelation->setTag("type","multipolygon");
        theRelation->add("outer",LastRoad);
        theRelation->add("inner",theRoad);
        LastRoad = 0;
    }
    HaveFirst = false;
    LastRoad = theRoad;
    theRoad = 0;
    view()->setInteracting(false);

    MDiscardableMessage dlg(NULL,
        MainWindow::tr("Add a hole."),
        MainWindow::tr("Do you want to add a(nother) hole to this area?"));
    if (dlg.check() == QDialog::Rejected) {
        EndNow = true;
    }
    L->setDescription(MainWindow::tr("Area: Finish Road %1").arg(LastRoad->description()));
    L->setFeature(LastRoad);
}

void CreateAreaInteraction::addToRoad(QMouseEvent* anEvent, Feature* Snap, CommandList* L)
{
    Node* Pt = dynamic_cast<Node*>(Snap);
    Node* To = 0;
    if (Pt)
        To = Pt;
    else if (Way* aRoad = dynamic_cast<Way*>(Snap))
    {
        Coord P(XY_TO_COORD(anEvent->pos()));
        int SnapIdx = findSnapPointIndex(aRoad, P);
        Node* N = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), P);
        CommandList* theList  = new CommandList(MainWindow::tr("Area: Add node %1 to Road %2").arg(N->description()).arg(theRoad->description()), N);
        theList->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),N,true));
        theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx));
        document()->addHistory(theList);
        view()->invalidate(true, true, false);
        To = N;
    }
    if (!To)
    {
        To = g_backend.allocNode(theMain->document()->getDirtyOrOriginLayer(), XY_TO_COORD(anEvent->pos()));
        L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),To,true));
        L->setDescription(MainWindow::tr("Area: Add node %1 to Road %2").arg(To->description()).arg(theRoad->description()));
        L->setFeature(To);
    } else {
        if (!To->isDirty() && !To->hasOSMId() && To->isUploadable())
            L->add(new AddFeatureCommand(theMain->document()->getDirtyOrOriginLayer(),To,true));
    }
    L->add(new WayAddNodeCommand(theRoad,To));
    if (To == theRoad->get(0))
        finishRoad(L);
}

void CreateAreaInteraction::snapMouseReleaseEvent(QMouseEvent* anEvent, Feature* aFeature)
{
    if (M_PREFS->getMouseSingleButton() && anEvent->button() == Qt::RightButton) {
        LastRoad = NULL;
        theRelation = NULL;
        HaveFirst = false;
        EndNow = false;
        view()->setInteracting(false);
    } else
    if ( !panning() ) {
        if (!HaveFirst)
        {
            HaveFirst = true;
            startNewRoad(anEvent, aFeature);
        }
        else
        {
            CommandList* L  = new CommandList();
            if (!theRoad)
                createNewRoad(L);
            addToRoad(anEvent, aFeature, L);
            document()->addHistory(L);
            view()->invalidate(true, true, false);
            if (theRelation)
                theMain->properties()->setSelection(theRelation);
            else
                theMain->properties()->setSelection(theRoad);
        }
        FirstPoint = XY_TO_COORD(anEvent->pos());

        if (EndNow) {
            if (theRelation)
                theMain->properties()->setSelection(theRelation);
            else
                theMain->properties()->setSelection(LastRoad);
            LastRoad = NULL;
            theRelation = NULL;
            HaveFirst = false;
            EndNow = false;
            view()->setInteracting(false);
        }
    }
}

#ifndef _MOBILE
QCursor CreateAreaInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif

void CreateAreaInteraction::closeAndFinish()
{
    view()->setInteracting(false);

    if (!theRoad || theRoad->size() < 3)
        return;

    Node* N = theRoad->getNode(0);
    CommandList* theList  = new CommandList(MainWindow::tr("Close Area %1").arg(theRoad->description()), theRoad);
    addToRoad(NULL, N, theList);
    document()->addHistory(theList);
    view()->invalidate(true, true, false);
    if (theRelation)
        theMain->properties()->setSelection(theRelation);
    else
        theMain->properties()->setSelection(theRoad);
}
