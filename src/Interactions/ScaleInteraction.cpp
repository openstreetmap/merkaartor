#include "ScaleInteraction.h"

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

ScaleInteraction::ScaleInteraction(MapView* aView)
    : FeatureSnapInteraction(aView), StartDragPosition(0,0)
  {
    QPixmap pm(":/Icons/rotate.png");
    rotateCursor =  QCursor(pm.scaledToWidth(22));

}

ScaleInteraction::~ScaleInteraction(void)
{
}

QString ScaleInteraction::toHtml()
{
    QString help;
    help = (MainWindow::tr("HOVER to select;LEFT-DRAG to scale"));

    QStringList helpList = help.split(";");

    QString desc;
    desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Scale Interaction"));

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
QCursor ScaleInteraction::cursor() const
{
    if (LastSnap || Scaling.size()) {
        return QCursor(Qt::SizeBDiagCursor);
    }

    return FeatureSnapInteraction::cursor();
}
#endif


void ScaleInteraction::snapMousePressEvent(QMouseEvent * anEvent, Feature* aLast)
{
    QList<Feature*> sel;
    if (view()->isSelectionLocked()) {
        if (view()->properties()->selection(0))
            sel.append(view()->properties()->selection(0));
        else
            sel.append(aLast);
    } else {
        sel = view()->properties()->selection();
        if (!sel.size() && aLast)
            sel.append(aLast);
    }
    Radius = 1.0;
    clearNoSnap();
    Scaling.clear();
    OriginalPosition.clear();

    if (!sel.size())
        return;

    StartDragPosition = XY_TO_COORD(anEvent->pos());
    OriginNode = NULL;
    NodeOrigin  = false;
    CoordBox selBB = sel[0]->boundingBox();
    for (int j=0; j<sel.size(); j++) {
        selBB.merge(sel[j]->boundingBox());
        if (CHECK_WAY(sel[j])) {
            Way* R = STATIC_CAST_WAY(sel[j]);
            for (int i=0; i<R->size(); ++i)
                if (std::find(Scaling.begin(),Scaling.end(),R->get(i)) == Scaling.end())
                    Scaling.push_back(R->getNode(i));
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
    if (Scaling.size() > 1) {
        if (NodeOrigin) {
            ScaleCenter = COORD_TO_XY(OriginNode->position());
        } else {
            ScaleCenter = COORD_TO_XY(selBB.center());
        }
        for (int i=0; i<Scaling.size(); ++i)
        {
            OriginalPosition.push_back(Scaling[i]->position());
            addToNoSnap(Scaling[i]);
        }
    } else
        Scaling.clear();
}

void ScaleInteraction::snapMouseReleaseEvent(QMouseEvent * anEvent, Feature* /*Closer*/)
{
    Q_UNUSED(anEvent);

    if (Radius != 1.0 && Scaling.size() && !panning())
    {
        CommandList* theList;
        theList = new CommandList(MainWindow::tr("Scale Feature").arg(Scaling[0]->id().numId), Scaling[0]);
        for (int i=0; i<Scaling.size(); ++i)
        {
            if (NodeOrigin && Scaling[i] == OriginNode)
                continue;
            Scaling[i]->setPosition(OriginalPosition[i]);
            if (Scaling[i]->layer()->isTrack())
                theList->add(new MoveNodeCommand(Scaling[i],scalePosition(OriginalPosition[i], Radius), Scaling[i]->layer()));
            else
                theList->add(new MoveNodeCommand(Scaling[i],scalePosition(OriginalPosition[i], Radius), document()->getDirtyOrOriginLayer(Scaling[i]->layer())));
        }


        document()->addHistory(theList);
        view()->invalidate(true, false);
    }
    Radius = 1.0;
    Scaling.clear();
    OriginalPosition.clear();
    clearNoSnap();
}

void ScaleInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, Feature* /*Closer*/)
{
    if (Scaling.size() && !panning())
    {
        Radius = distance(ScaleCenter,anEvent->pos()) / distance(ScaleCenter, COORD_TO_XY(StartDragPosition));
        for (int i=0; i<Scaling.size(); ++i) {
            if (NodeOrigin && Scaling[i] == OriginNode)
                continue;
            Scaling[i]->setPosition(scalePosition(OriginalPosition[i], Radius));
        }
        view()->invalidate(true, false);
    }
}

Coord ScaleInteraction::scalePosition(Coord position, qreal radius)
{
    QPointF p = COORD_TO_XY(position);
    QLineF v(ScaleCenter, p);
    v.setLength(v.length() * radius);

    return XY_TO_COORD(v.p2().toPoint());
}

void ScaleInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (!ScaleCenter.isNull())
    {
        thePainter.setPen(QPen(QColor(255,0,0),1));
        thePainter.drawEllipse(COORD_TO_XY(ScaleCenter), 5, 5);
    }
    FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

