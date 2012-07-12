#include "Global.h"
#include "SelectInteraction.h"

#include "MainWindow.h"
#ifndef _MOBILE
#include "ui_MainWindow.h"
#endif
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "MoveNodeInteraction.h"
#include "Document.h"
#include "Features.h"
#include "FeatureManipulations.h"
#include "Projection.h"
#include "LineF.h"
#include "MDiscardableDialog.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QMessageBox>

#include <QList>

#define PROPERTIES(x) {if (PROPERTIES_DOCK) PROPERTIES_DOCK->x;}

SelectInteraction::SelectInteraction()
: FeatureSnapInteraction(), Dragging(false), StartDrag(0,0), EndDrag(0,0)
{
    defaultCursor = QCursor(Qt::ArrowCursor);

    if (!M_PREFS->getSeparateMoveMode()) {
        setDontSelectVirtual(false);
    }
}

SelectInteraction::~SelectInteraction(void)
{
}

QString SelectInteraction::toHtml()
{
    QString help;
    if (!M_PREFS->getMouseSingleButton())
        help = (MainWindow::tr("LEFT-CLICK to select;RIGHT-CLICK to pan;CTRL-LEFT-CLICK to toggle selection;SHIFT-LEFT-CLICK to add to selection;LEFT-DRAG for area selection;CTRL-RIGHT-DRAG for zoom;DOUBLE-CLICK to create a node;DOUBLE-CLICK on a node to start a way;"));
    else
        help = (MainWindow::tr("CLICK to select/move;CTRL-CLICK to toggle selection;SHIFT-CLICK to add to selection;SHIFT-DRAG for area selection;CTRL-DRAG for zoom;DOUBLE-CLICK to create a node;DOUBLE-CLICK on a node to start a way;"));

    QStringList helpList = help.split(";");

    QString desc;
    desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Select Interaction"));

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

void SelectInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
    if (Dragging)
    {
        thePainter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
        thePainter.drawRect(QRectF(COORD_TO_XY(StartDrag),COORD_TO_XY(EndDrag)));
    }
    FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

static bool modifiersForAdd(Qt::KeyboardModifiers modifiers)
{
    return modifiers.testFlag(Qt::ShiftModifier);
}

static bool modifiersForToggle(Qt::KeyboardModifiers modifiers)
{
    return modifiers.testFlag(Qt::ControlModifier);
}

static bool modifiersForDrag(Qt::KeyboardModifiers modifiers)
{
    if (M_PREFS->getMouseSingleButton())
        return modifiers.testFlag(Qt::ShiftModifier);
    else
        return true;
}

#if 0
static bool modifiersForSegmentSelect(Qt::KeyboardModifiers modifiers)
{
    return modifiers.testFlag(Qt::AltModifier);
}
#endif

static bool modifiersForGreedyAdd(Qt::KeyboardModifiers modifiers)
{
    // whether drag select should include intersected as well as contained features
    if (M_PREFS->getMouseSingleButton())
        return modifiers.testFlag(Qt::ShiftModifier) && modifiers.testFlag(Qt::AltModifier);
    else
        return modifiers.testFlag(Qt::ShiftModifier);
}



void SelectInteraction::snapMousePressEvent(QMouseEvent * ev, Feature* aLast)
{
    Qt::KeyboardModifiers modifiers = ev->modifiers();
    if (!view()->isSelectionLocked()) {
        if (modifiers) {
            if (modifiersForToggle(modifiers) && aLast)
                emit(toggleSelection(aLast));

            if (modifiersForAdd(modifiers) && aLast)
                emit(addSelection(aLast));

            if (g_Merk_Segment_Mode && aLast) {
                emit(setSelection(aLast));
            }
        } else {
            StackSnap = SnapList;
//				if (aLast)
//					emit(setSelection(aLast));
        }
        if (!aLast && modifiersForDrag(modifiers))
        {
            EndDrag = StartDrag = XY_TO_COORD(ev->pos());
            Dragging = true;
        }
        PROPERTIES(checkMenuStatus());
        view()->update();
    }
}

bool SelectInteraction::isIdle() const
{
    if (Dragging && !(StartDrag == EndDrag))
        return false;

    if (panning())
        return false;

    return true;
}

void SelectInteraction::snapMouseReleaseEvent(QMouseEvent * ev , Feature* aLast)
{
    Qt::KeyboardModifiers modifiers = ev->modifiers();
    if (ev->button() != Qt::LeftButton)
        return;

    if (Dragging)
    {
        QList<Feature*> List;
        EndDrag = XY_TO_COORD(ev->pos());
        CoordBox DragBox(StartDrag, EndDrag);
        for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it) {
            if (it.get()->isReadonly())
                continue;

            if (modifiersForGreedyAdd(modifiers))
            {
                if (!DragBox.intersects(it.get()->boundingBox()))
                    continue;
                if (DragBox.contains(it.get()->boundingBox()))
                    List.push_back(it.get());
                else {
                    Coord A, B;
                    if (Way* R = dynamic_cast<Way*>(it.get())) {
                        for (int j=1; j<R->size(); ++j) {
                            A = R->getNode(j-1)->position();
                            B = R->getNode(j)->position();
                            if (CoordBox::visibleLine(DragBox, A, B)) {
                                List.push_back(R);
                                break;
                            }
                        }
                    } else
                    if (Relation* r = dynamic_cast<Relation*>(it.get())) {
                        for (int k=0; k<r->size(); ++k) {
                            if (Way* R = dynamic_cast<Way*>(r->get(k))) {
                                for (int j=1; j<R->size(); ++j) {
                                    A = R->getNode(j-1)->position();
                                    B = R->getNode(j)->position();
                                    if (CoordBox::visibleLine(DragBox, A, B)) {
                                        List.push_back(r);
                                        break;
                                    }
                                }
                            }
                        }
                    }
                }
            } else {
                if (DragBox.contains(it.get()->boundingBox()))
                    List.push_back(it.get());
            }
        }
        if (!List.isEmpty() || (!modifiersForAdd(modifiers) && !modifiersForToggle(modifiers)))
            emit(setMultiSelection(List));
        PROPERTIES(checkMenuStatus());
        Dragging = false;
        view()->update();
    } else {
        if (!panning() && !modifiers) {
            emit(setSelection(aLast));
            PROPERTIES(checkMenuStatus());
            view()->update();
        }
    }
}

void SelectInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, Feature* aLast)
{
    Q_UNUSED(anEvent)
    Q_UNUSED(aLast)

    if (Dragging)
    {
        EndDrag = XY_TO_COORD(anEvent->pos());
        view()->update();
    }
}

void SelectInteraction::snapMouseDoubleClickEvent(QMouseEvent* anEvent, Feature* aLast)
{
    Q_UNUSED(anEvent)
    Q_UNUSED(aLast)

//    Qt::KeyboardModifiers modifiers = anEvent->modifiers();
//    if (!panning() && !modifiers) {
//        if (aLast) {
//            QList<Feature*> theFeatures;
//            theFeatures << aLast;
//            for (int i=0; i<aLast->size(); ++i)
//                theFeatures << aLast->get(i);
//            emit(setSelection(theFeatures));
//            PROPERTIES(checkMenuStatus());
//            view()->update();
//        } else {
//            Node* N = g_backend.allocNode(XY_TO_COORD(anEvent->pos()));
//            CommandList* theList  = new CommandList(MainWindow::tr("Create point %1").arg(N->id()), N);
//            theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(),N,true));
//            document()->addHistory(theList);
//            main()->properties()->setSelection(N);
//            view()->invalidate(true, true, false);
//       }
//    }
}


#ifndef _MOBILE
QCursor SelectInteraction::cursor() const
{
    return QCursor(Qt::PointingHandCursor);
}
#endif
