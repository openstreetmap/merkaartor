#include "Global.h"
#include "CreateNodeInteraction.h"

#include "PropertiesDock.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "Projection.h"
#include "Node.h"
#include "LineF.h"
#include "MoveNodeInteraction.h"
#include "Global.h"

#include <QList>

CreateNodeInteraction::CreateNodeInteraction()
    : FeatureSnapInteraction()
    , theMoveInteraction(0)
{
}

CreateNodeInteraction::~CreateNodeInteraction(void)
{
    SAFE_DELETE(theMoveInteraction);
}

QString CreateNodeInteraction::toHtml()
{
    QString help;
    //help = (MainWindow::tr("LEFT-CLICK to select; LEFT-DRAG to move"));

    QString desc;
    desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Create node Interaction"));
    desc += QString("<b>%1</b><br/>").arg(help);

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "</body></html>";

    return S;
}

void CreateNodeInteraction::snapMousePressEvent(QMouseEvent * ev, Feature* aFeat)
{
    if (CAST_NODE(aFeat)) {
        return theMoveInteraction->snapMousePressEvent(ev, aFeat);
    } else {
        SAFE_DELETE(theMoveInteraction);
#ifndef _MOBILE
        CUR_VIEW->setCursor(cursor());
#endif
    }

}

void CreateNodeInteraction::snapMouseMoveEvent(QMouseEvent* ev, Feature* aFeat)
{
    if (CAST_NODE(aFeat)) {
        if (!theMoveInteraction) {
            theMoveInteraction = new MoveNodeInteraction();
        }
#ifndef _MOBILE
        CUR_VIEW->setCursor(theMoveInteraction->cursor());
    } else
        CUR_VIEW->setCursor(cursor());
#else
    }
#endif

    if (theMoveInteraction)
        return theMoveInteraction->snapMouseMoveEvent(ev, aFeat);
}

void CreateNodeInteraction::snapMouseReleaseEvent(QMouseEvent * ev, Feature* aFeat)
{
    if (theMoveInteraction) {
        theMoveInteraction->snapMouseReleaseEvent(ev, aFeat);
        return;
    }

    Way* aRoad = dynamic_cast<Way*>(aFeat);
    if (!aFeat || aRoad) {
        SAFE_DELETE(theMoveInteraction);
        Coord P(XY_TO_COORD(ev->pos()));

        createNode(P, aFeat);

        theMoveInteraction = new MoveNodeInteraction();
#ifndef _MOBILE
        CUR_VIEW->setCursor(theMoveInteraction->cursor());
#endif
    }
}

#ifndef _MOBILE
QCursor CreateNodeInteraction::cursor() const
{
    return QCursor(Qt::CrossCursor);
}
#endif

void CreateNodeInteraction::createNode(Coord P, Feature* aFeat)
{
    Node* N;
    CommandList* theList;
    Way* aRoad = dynamic_cast<Way*>(aFeat);
    if (aRoad)
    {
        PROPERTIES_DOCK->setSelection(0);
        theList  = new CommandList(MainWindow::tr("Create node in Road: %1").arg(aRoad->id().numId), aRoad);
        int SnapIdx = findSnapPointIndex(aRoad, P);
        N = g_backend.allocNode(CUR_DOCUMENT->getDirtyOrOriginLayer(aRoad->layer()), P);
        theList->add(new AddFeatureCommand(CUR_DOCUMENT->getDirtyOrOriginLayer(aRoad->layer()),N,true));
        theList->add(new WayAddNodeCommand(aRoad,N,SnapIdx,CUR_DOCUMENT->getDirtyOrOriginLayer(aRoad->layer())));
    }
    else
    {
        N = g_backend.allocNode(CUR_DOCUMENT->getDirtyOrOriginLayer(), P);
        theList  = new CommandList(MainWindow::tr("Create POI %1").arg(N->id().numId), N);
        theList->add(new AddFeatureCommand(CUR_DOCUMENT->getDirtyOrOriginLayer(),N,true));
        if (M_PREFS->getAutoSourceTag()) {
            QStringList sl = CUR_DOCUMENT->getCurrentSourceTags();
            if (sl.size())
                N->setTag("source", sl.join(";"));
        }
        N->updateMeta();
    }
    CUR_DOCUMENT->addHistory(theList);
    PROPERTIES_DOCK->setSelection(N);
    CUR_VIEW->invalidate(true, true, false);
}
