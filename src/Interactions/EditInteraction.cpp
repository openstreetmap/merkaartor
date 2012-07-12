#include "Global.h"
#include "EditInteraction.h"

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

EditInteraction::EditInteraction()
: SelectInteraction()
{
    connect(CUR_MAINWINDOW,SIGNAL(remove_triggered()),this,SLOT(on_remove_triggered()));
    connect(CUR_MAINWINDOW,SIGNAL(reverse_triggered()), this,SLOT(on_reverse_triggered()));
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

void EditInteraction::on_remove_triggered()
{
    if (PROPERTIES_DOCK->selectionSize() == 0) return;

    QList<Feature*> Sel;
    for (int i=0; i<PROPERTIES_DOCK->selectionSize(); ++i) {
        if (!document()->isDownloadedSafe(PROPERTIES_DOCK->selection(i)->boundingBox()) && PROPERTIES_DOCK->selection(i)->hasOSMId())
            continue;
        else
            Sel.push_back(PROPERTIES_DOCK->selection(i));
    }
    if (Sel.size() == 0) {
        QMessageBox::critical(NULL, tr("Cannot delete"), tr("Cannot delete the selection because it is outside the downloaded area."));
        return;
    } else if (Sel.size() != PROPERTIES_DOCK->selectionSize()) {
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
        PROPERTIES_DOCK->setSelection(0);
        PROPERTIES_DOCK->checkMenuStatus();
    }
    else
        delete theList;
    view()->invalidate(true, true, false);
}

void EditInteraction::on_reverse_triggered()
{
    QList<Feature*> selection = PROPERTIES_DOCK->selection();
    QString desc = selection.size() == 1 ? tr("Reverse way %1").arg(selection[0]->id().numId) : tr("Reverse %1 ways");
    CommandList* theList  = new CommandList(MainWindow::tr("Reverse %1 ways").arg(selection.size()), NULL);
    foreach (Feature* f, selection)
        if (Way* R = dynamic_cast<Way*>(f))
            reversePoints(document(), theList, R);
    if (theList->empty()) {
        delete theList;
    } else {
        document()->addHistory(theList);
        view()->invalidate(true, true, false);
    }
}

#ifndef _MOBILE
QCursor EditInteraction::cursor() const
{
    if (LastSnap)
        return defaultCursor;

    return FeatureSnapInteraction::cursor();
}
#endif
