//
// C++ Interface: DirtyDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DIRTYDOCK_H
#define DIRTYDOCK_H

#include "MDockAncestor.h"
#include <QTextBrowser>

#include "ui_DirtyDock.h"

#include <QList>

class Feature;

/**
    @author cbro <cbro@semperpax.com>
*/
class DirtyDock : public MDockAncestor
{
Q_OBJECT
public:
    DirtyDock();

    ~DirtyDock();

public slots:
    void updateList();
    void on_ChangesList_itemSelectionChanged();
    void on_ChangesList_itemDoubleClicked(QListWidgetItem* item);
    void on_ChangesList_customContextMenuRequested(const QPoint & pos);
    void on_centerAction_triggered();
    void on_centerZoomAction_triggered();
    void on_pbCleanupHistory_clicked();

private:
    Ui::DirtyDockWidget ui;
    QAction* centerAction;
    QAction* centerZoomAction;

    QList<Feature*> Selection;
public:
    void changeEvent(QEvent*);
    void retranslateUi();
};

#endif
