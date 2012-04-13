#include "RotateInteraction.h"

#include "MainWindow.h"
#include "MapView.h"
#include "DocumentCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Coord.h"
#include "Document.h"
#include "Projection.h"
#include "Node.h"
#include "LineF.h"
#include "MDiscardableDialog.h"
#include "PropertiesDock.h"

#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QMessageBox>

#include <QList>
#include <QPainter>

RotateInteraction::RotateInteraction(MainWindow* aMain)
    : FeatureSnapInteraction(aMain), StartDragPosition(0,0)
{
    QPixmap pm(":/Icons/rotate.png");
    rotateCursor =  QCursor(pm.scaledToWidth(22));

}

RotateInteraction::~RotateInteraction(void)
{
}

QString RotateInteraction::toHtml()
{
    QString help;
    help = (MainWindow::tr("HOVER to select;LEFT-DRAG to rotate"));

    QStringList helpList = help.split(";");

    QString desc;
    desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Rotate Interaction"));

    QString S =
    "<html><head/><body>"
    "<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
    + desc;
    S += "<hr/>";
    S += "<ul style=\"margin-left: 0px; padding-left: 0px;\">";
    for (int i=0; i<helpList.size(); ++i) {
        S+= "<li>" + helpList[i] + "</li>";
    }
    S += "</ul>";
    S += "</body></html>";

    return S;
}

#ifndef _MOBILE
QCursor RotateInteraction::cursor() const
{
    if (LastSnap || Rotating.size()) {
        return rotateCursor;
    }

    return FeatureSnapInteraction::cursor();
}
#endif


void RotateInteraction::snapMousePressEvent(QMouseEvent * anEvent, Feature* aLast)
{
    QList<Feature*> sel;
    if (view()->isSelectionLocked()) {
        if (theMain->properties()->selection(0))
            sel.append(theMain->properties()->selection(0));
        else
            sel.append(aLast);
    } else {
        sel = theMain->properties()->selection();
        if (!sel.size() && aLast)
            sel.append(aLast);
    }
    Angle = 0.0;
    clearNoSnap();
    Rotating.clear();
    OriginalPosition.clear();

    if (!sel.size())
        return;

    view()->setInteracting(true);

    StartDragPosition = XY_TO_COORD(anEvent->pos());
    OriginNode = NULL;
    NodeOrigin  = false;
    CoordBox selBB = sel[0]->boundingBox();
    for (int j=0; j<sel.size(); j++) {
        selBB.merge(sel[j]->boundingBox());
        if (CHECK_WAY(sel[j])) {
            Way* R = STATIC_CAST_WAY(sel[j]);
            for (int i=0; i<R->size(); ++i)
                if (std::find(Rotating.begin(),Rotating.end(),R->get(i)) == Rotating.end())
                    Rotating.push_back(R->getNode(i));
            addToNoSnap(R);
        } else if (CHECK_NODE(sel[j])) {
            if (!OriginNode && !NodeOrigin) {
                OriginNode = STATIC_CAST_NODE(sel[j]);
                NodeOrigin = true;
            } else {
                NodeOrigin = false;
            }
        }
    }
    if (Rotating.size() > 1) {
        if (NodeOrigin) {
            RotationCenter = COORD_TO_XY(OriginNode->position());
        } else {
            RotationCenter = COORD_TO_XY(selBB.center());
        }
        for (int i=0; i<Rotating.size(); ++i)
        {
            OriginalPosition.push_back(Rotating[i]->position());
            addToNoSnap(Rotating[i]);
        }
    } else
        Rotating.clear();
}

void RotateInteraction::snapMouseReleaseEvent(QMouseEvent * anEvent, Feature* /*Closer*/)
{
    Q_UNUSED(anEvent);

    if (Angle != 0.0 && Rotating.size() && !panning())
    {
        CommandList* theList;
        theList = new CommandList(MainWindow::tr("Rotate Feature").arg(Rotating[0]->id().numId), Rotating[0]);
        for (int i=0; i<Rotating.size(); ++i)
        {
            if (NodeOrigin && Rotating[i] == OriginNode)
                continue;
            Rotating[i]->setPosition(OriginalPosition[i]);
            if (Rotating[i]->layer()->isTrack())
                theList->add(new MoveNodeCommand(Rotating[i],rotatePosition(OriginalPosition[i], Angle), Rotating[i]->layer()));
            else
                theList->add(new MoveNodeCommand(Rotating[i],rotatePosition(OriginalPosition[i], Angle), document()->getDirtyOrOriginLayer(Rotating[i]->layer())));
        }


        document()->addHistory(theList);
        view()->invalidate(true, false);
    }
    Angle = 0.0;
    Rotating.clear();
    OriginalPosition.clear();
    view()->setInteracting(false);
    clearNoSnap();
}

void RotateInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, Feature* /*Closer*/)
{
    if (Rotating.size() && !panning())
    {
        Angle = calculateNewAngle(anEvent);
        for (int i=0; i<Rotating.size(); ++i) {
            if (NodeOrigin && Rotating[i] == OriginNode)
                continue;
            Rotating[i]->setPosition(rotatePosition(OriginalPosition[i], Angle));
        }
        view()->invalidate(true, false);
    }
}

Coord RotateInteraction::rotatePosition(Coord position, qreal angle)
{
    QPointF p = COORD_TO_XY(position);
    QLineF v(RotationCenter, p);
    v.setAngle(v.angle() + angle);

    return XY_TO_COORD(v.p2().toPoint());
}

qreal RotateInteraction::calculateNewAngle(QMouseEvent *event)
{
    QPointF p1 = COORD_TO_XY(StartDragPosition);
    QLineF v1(RotationCenter, p1);
    QLineF v2(RotationCenter, event->pos());

    return v1.angleTo(v2);
}

void RotateInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (!RotationCenter.isNull())
    {
        thePainter.setPen(QPen(QColor(255,0,0),1));
        thePainter.drawEllipse(COORD_TO_XY(RotationCenter), 5, 5);
    }
    FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

