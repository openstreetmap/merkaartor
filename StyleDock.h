//
// C++ Interface: StyleList
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef STYLEDOCK_H
#define STYLEDOCK_H

#include "Utils/MDockAncestor.h"

#include "ui_StyleDock.h"

class MainWindow;
class QAction;

class StyleDock : public MDockAncestor
{
Q_OBJECT
public:
    StyleDock(MainWindow* aParent);

    ~StyleDock();

	void setCurrent(QAction * a);
    //void updateList();
	void clearItems();
	void addItem(QAction* a);

public slots:
	void on_StyleList_itemSelectionChanged();
    void on_StyleList_itemDoubleClicked(QListWidgetItem* item);
    void on_StyleList_customContextMenuRequested(const QPoint & pos);

private:
    MainWindow* Main;
    Ui::StyleDockWidget ui;

public:
    void changeEvent(QEvent*);
    void retranslateUi();
};

#endif // STYLEDOCK_H
