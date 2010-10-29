#include "EditInteraction.h"
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "MoveNodeInteraction.h"
#include "Document.h"
#include "Features.h"
#include "Maps/FeatureManipulations.h"
#include "Maps/Projection.h"
#include "Utils/LineF.h"
#include "Utils/MDiscardableDialog.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QMessageBox>

#include <QList>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0)
{
    defaultCursor = QCursor(Qt::ArrowCursor);

    connect(main(),SIGNAL(remove_triggered()),this,SLOT(on_remove_triggered()));
    connect(main(),SIGNAL(reverse_triggered()), this,SLOT(on_reverse_triggered()));
    view()->properties()->checkMenuStatus();

    if (!M_PREFS->getSeparateMoveMode()) {
        setDontSelectVirtual(false);
    }
}

EditInteraction::~EditInteraction(void)
{
    if(main())
    {
        main()->ui->editRemoveAction->setEnabled(false);
        main()->ui->editReverseAction->setEnabled(false);
    }
}

QString EditInteraction::toHtml()
{
    QString help;
    if (!M_PREFS->getMouseSingleButton())
        help = (MainWindow::tr("LEFT-CLICK to select;RIGHT-CLICK to pan;CTRL-LEFT-CLICK to toggle selection;SHIFT-LEFT-CLICK to add to selection;LEFT-DRAG for area selection;CTRL-RIGHT-DRAG for zoom;DOUBLE-CLICK to create a node;DOUBLE-CLICK on a node to start a way;"));
    else
        help = (MainWindow::tr("CLICK to select/move;CTRL-CLICK to toggle selection;SHIFT-CLICK to add to selection;SHIFT-DRAG for area selection;CTRL-DRAG for zoom;DOUBLE-CLICK to create a node;DOUBLE-CLICK on a node to start a way;"));

    QStringList helpList = help.split(";");

    QString desc;
    desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Edit Interaction"));

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

void EditInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
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

static bool modifiersForSegmentSelect(Qt::KeyboardModifiers modifiers)
{
    return modifiers.testFlag(Qt::AltModifier);
}

static bool modifiersForGreedyAdd(Qt::KeyboardModifiers modifiers)
{
    // whether drag select should include intersected as well as contained features
    if (M_PREFS->getMouseSingleButton())
        return modifiers.testFlag(Qt::ShiftModifier) && modifiers.testFlag(Qt::AltModifier);
    else
        return modifiers.testFlag(Qt::ShiftModifier);
}



void EditInteraction::snapMousePressEvent(QMouseEvent * ev, Feature* aLast)
{
    Qt::KeyboardModifiers modifiers = ev->modifiers();
    if (!view()->isSelectionLocked()) {
        if (modifiers) {
            if (modifiersForToggle(modifiers) && aLast)
                view()->properties()->toggleSelection(aLast);

            if (modifiersForAdd(modifiers) && aLast)
                view()->properties()->addSelection(aLast);

            if (g_Merk_Segment_Mode && aLast) {
                view()->properties()->setSelection(aLast);
            }
        } else {
            StackSnap = SnapList;
//				if (aLast)
//					view()->properties()->setSelection(aLast);
        }
        if (!aLast && modifiersForDrag(modifiers))
        {
            EndDrag = StartDrag = XY_TO_COORD(ev->pos());
            Dragging = true;
        }
        view()->properties()->checkMenuStatus();
        view()->update();
    }
}

bool EditInteraction::isIdle() const
{
    if (Dragging && !(StartDrag == EndDrag))
        return false;

    if (panning())
        return false;

    return true;
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * ev , Feature* aLast)
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
            view()->properties()->setSelection(List);
        view()->properties()->checkMenuStatus();
        Dragging = false;
        view()->update();
    } else {
        if (!panning() && !modifiers) {
            view()->properties()->setSelection(aLast);
            view()->properties()->checkMenuStatus();
            view()->update();
        }
    }
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, Feature* aLast)
{
    Q_UNUSED(anEvent)
    Q_UNUSED(aLast)

    if (Dragging)
    {
        EndDrag = XY_TO_COORD(anEvent->pos());
        view()->update();
    }
}

void EditInteraction::snapMouseDoubleClickEvent(QMouseEvent* anEvent, Feature* aLast)
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
//            view()->properties()->setSelection(theFeatures);
//            view()->properties()->checkMenuStatus();
//            view()->update();
//        } else {
//            Node* N = new Node(XY_TO_COORD(anEvent->pos()));
//            CommandList* theList  = new CommandList(MainWindow::tr("Create point %1").arg(N->id()), N);
//            theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(),N,true));
//            document()->addHistory(theList);
//            main()->properties()->setSelection(N);
//            view()->invalidate(true, false);
//       }
//    }
}

void EditInteraction::on_remove_triggered()
{
    if (view()->properties()->size() == 0) return;

    QList<Feature*> Sel;
    for (int i=0; i<view()->properties()->size(); ++i) {
        if (!document()->isDownloadedSafe(view()->properties()->selection(i)->boundingBox()) && view()->properties()->selection(i)->hasOSMId())
            continue;
        else
            Sel.push_back(view()->properties()->selection(i));
    }
    if (Sel.size() == 0) {
        QMessageBox::critical(NULL, tr("Cannot delete"), tr("Cannot delete the selection because it is outside the downloaded area."));
        return;
    } else if (Sel.size() != view()->properties()->size()) {
        if (!QMessageBox::warning(NULL, tr("Cannot delete everything"),
                             tr("The complete selection cannot be deleted because part of it is outside the downloaded area.\n"
                                "Delete what can be?"),
                             QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes))
            return;
    }

    CommandList* theList;
    if (Sel.size() == 1)
        theList  = new CommandList(MainWindow::tr("Remove feature %1").arg(Sel[0]->id().numId), Sel[0]);
    else
        theList  = new CommandList(MainWindow::tr("Remove features"), NULL);

    bool deleteChildrenOKDefined = false;
    bool deleteChildrenOK = false;
    for (int i=0; i<Sel.size(); ++i) {
        if (document()->exists(Sel[i])) {
            QList<Feature*> Alternatives;

            if (Sel[i]->size() && !deleteChildrenOKDefined) {
                MDiscardableMessage dlg(NULL,
                    MainWindow::tr("Delete Children."),
                    MainWindow::tr("Do you want to delete the children nodes also?\n"
                                   "Note that OSM nodes outside the downloaded area will be kept."));
                deleteChildrenOKDefined = true;
                deleteChildrenOK = (dlg.check() == QDialog::Accepted);
            }
            if (deleteChildrenOK)
                Sel[i]->deleteChildren(document(), theList);
            theList->add(new RemoveFeatureCommand(document(), Sel[i], Alternatives));
        }
    }

    if (theList->size()) {
        document()->addHistory(theList);
        view()->properties()->setSelection(0);
        view()->properties()->checkMenuStatus();
    }
    else
        delete theList;
    view()->invalidate(true, false);
}

void EditInteraction::on_reverse_triggered()
{
    Feature* Selection = view()->properties()->selection(0);
    if (Way* R = dynamic_cast<Way*>(Selection))
    {
        CommandList* theList  = new CommandList(MainWindow::tr("Reverse Road %1").arg(R->id().numId), R);
        reversePoints(document(),theList,R);
        document()->addHistory(theList);
    }
    view()->invalidate(true, false);
}

#ifndef Q_OS_SYMBIAN
QCursor EditInteraction::cursor() const
{
    if (LastSnap)
        return defaultCursor;

    return FeatureSnapInteraction::cursor();
}
#endif
