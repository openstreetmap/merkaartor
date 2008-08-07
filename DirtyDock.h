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

#include <QDockWidget>
#include <QTextBrowser>

#include "ui_DirtyDock.h"

#include <vector>

class MainWindow;
class MapFeature;

/**
	@author cbro <cbro@semperpax.com>
*/
class DirtyDock : public QDockWidget
{
Q_OBJECT
public:
    DirtyDock(MainWindow* aParent);

    ~DirtyDock();

public slots:
	void updateList();
	void on_ChangesList_itemSelectionChanged();
	void on_ChangesList_itemDoubleClicked(QListWidgetItem* item);
	void on_ChangesList_customContextMenuRequested(const QPoint & pos);
	void on_centerAction_triggered();
	void on_centerZoomAction_triggered();

private:
	MainWindow* Main;
	Ui::DirtyDockWidget ui;
	QAction* centerAction;
	QAction* centerZoomAction;
	
	std::vector<MapFeature*> Selection;
};

#endif
