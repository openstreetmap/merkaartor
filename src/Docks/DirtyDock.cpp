//
// C++ Implementation: DirtyDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include "Global.h"

#include "DirtyDock.h"
#include "PropertiesDock.h"
#include "MapView.h"
#include "MerkaartorPreferences.h"
#include "Document.h"
#include "Layer.h"
#include "Feature.h"
#include "Command.h"
#include "DirtyList.h"

#include <QMenu>

DirtyDock::DirtyDock()
    : MDockAncestor()
{
    setMinimumSize(220,100);
    setObjectName("dirtyDock");

    ui.setupUi(getWidget());

    centerAction = new QAction(NULL, this);
    connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
    centerZoomAction = new QAction(NULL, this);
    connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));

    connect(ui.ChangesList, SIGNAL(itemSelectionChanged()), this, SLOT(on_ChangesList_itemSelectionChanged()));
    connect(ui.ChangesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_ChangesList_itemDoubleClicked(QListWidgetItem*)));
    connect(ui.ChangesList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_ChangesList_customContextMenuRequested(const QPoint &)));

    if (M_PREFS->getAutoHistoryCleanup())
        ui.pbCleanupHistory->setVisible(false);
    else
        connect(ui.pbCleanupHistory, SIGNAL(clicked()), this, SLOT(on_pbCleanupHistory_clicked()));

    retranslateUi();
}


DirtyDock::~DirtyDock()
{
}


void DirtyDock::updateList()
{
    ui.ChangesList->clear();

    if (!CUR_DOCUMENT)
        return;

    int dirtyObjects = CUR_DOCUMENT->getDirtySize();
    switch (dirtyObjects)
    {
        case 0:
            ui.label->setText(tr("There is <b>no</b> dirty object"));
            break;
        case 1:
            ui.label->setText(tr("There is <b>one</b> dirty object"));
            break;

        default:
            ui.label->setText(tr("There are <b>%n</b> dirty objects", "", dirtyObjects));
            break;
    }

    CUR_DOCUMENT->history().buildUndoList(ui.ChangesList);

    if (!M_PREFS->getAutoHistoryCleanup()) {
        if (!dirtyObjects)
            ui.pbCleanupHistory->setEnabled(true);
        else
            ui.pbCleanupHistory->setEnabled(false);
    }
}

void DirtyDock::on_ChangesList_itemSelectionChanged()
{
    Feature* F;

    if (ui.ChangesList->selectedItems().count() != 0) {

        if (ui.ChangesList->selectedItems().count() == 1) {
            F = CUR_DOCUMENT->getFeature(ui.ChangesList->selectedItems()[0]->data(Qt::UserRole).value<IFeature::FId>());
            if (F)
                PROPERTIES_DOCK->setSelection(F);
        } else {
            Selection.clear();
            for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
                F = CUR_DOCUMENT->getFeature(ui.ChangesList->selectedItems()[i]->data(Qt::UserRole).value<IFeature::FId>());
                if (F)
                    Selection.push_back(F);
            }
            PROPERTIES_DOCK->setMultiSelection(Selection);
        }

    }
    CUR_VIEW->update();
}

void DirtyDock::on_ChangesList_itemDoubleClicked(QListWidgetItem* /* item */)
{
    on_centerAction_triggered();
}

void DirtyDock::on_ChangesList_customContextMenuRequested(const QPoint & pos)
{
    QListWidgetItem *it = ui.ChangesList->itemAt(pos);
    if (it) {
        QMenu menu(ui.ChangesList);
        menu.addAction(centerAction);
        menu.addAction(centerZoomAction);
        menu.exec(ui.ChangesList->mapToGlobal(pos));
    }
}

void DirtyDock::on_centerAction_triggered()
{
    Feature* F;
    CoordBox cb;

    CUR_MAINWINDOW->setUpdatesEnabled(false);
    for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
        F = CUR_DOCUMENT->getFeature(ui.ChangesList->selectedItems()[i]->data(Qt::UserRole).value<IFeature::FId>());
        if (F) {
            if (cb.isNull())
                cb = F->boundingBox();
            else
                cb.merge(F->boundingBox());
        }
    }
    if (!cb.isNull()) {
        Coord c = cb.center();
        CUR_VIEW->setCenter(c, CUR_VIEW->rect());
        CUR_MAINWINDOW->invalidateView();
    }
    CUR_MAINWINDOW->setUpdatesEnabled(true);
}

void DirtyDock::on_centerZoomAction_triggered()
{
    Feature* F;
    CoordBox cb;

    CUR_MAINWINDOW->setUpdatesEnabled(false);
    for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
        F = CUR_DOCUMENT->getFeature(ui.ChangesList->selectedItems()[i]->data(Qt::UserRole).value<IFeature::FId>());
        if (F) {
            if (cb.isNull())
                cb = F->boundingBox();
            else
                cb.merge(F->boundingBox());
        }
    }
    if (!cb.isNull()) {
        CoordBox mini(cb.center()-COORD_ENLARGE, cb.center()+COORD_ENLARGE);
        cb.merge(mini);
        cb = cb.zoomed(1.1);
        CUR_VIEW->setViewport(cb, CUR_VIEW->rect());
        CUR_MAINWINDOW->invalidateView();
    }
    CUR_MAINWINDOW->setUpdatesEnabled(true);
}

void DirtyDock::on_pbCleanupHistory_clicked()
{
    CUR_DOCUMENT->history().cleanup();
    CUR_DOCUMENT->history().updateActions();
    updateList();
}

void DirtyDock::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    MDockAncestor::changeEvent(event);
}

void DirtyDock::retranslateUi()
{
    ui.retranslateUi(getWidget());

    setWindowTitle(tr("Undo"));
    centerAction->setText(tr("Center map"));
    centerZoomAction->setText(tr("Center && Zoom map"));
    updateList();
}
