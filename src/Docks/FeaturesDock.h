//
// C++ Interface: FeaturesDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef FEATURESDOCK_H
#define FEATURESDOCK_H

#include "MDockAncestor.h"
#include "Maps/Coord.h"
#include "Maps/MapFeature.h"
#include "Maps/MapTypedef.h"

#include "ui_FeaturesDock.h"

class MainWindow;
class QAction;

class FeaturesDock : public MDockAncestor
{
Q_OBJECT
public:
	FeaturesDock(MainWindow* aParent);

	~FeaturesDock();

	void updateList();

	MapFeature* highlighted(int idx);
	QList<MapFeature*> highlighted();
	int highlightedSize() const;

public slots:
	void on_FeaturesList_itemSelectionChanged();
	void on_FeaturesList_itemDoubleClicked(QListWidgetItem* item);
	void on_FeaturesList_customContextMenuRequested(const QPoint & pos);

	void on_rbWithin_stateChanged ( int state );

	void on_centerAction_triggered();
	void on_centerZoomAction_triggered();
	void on_downloadAction_triggered();
	void on_addSelectAction_triggered();

	void on_Viewport_changed();

	void tabChanged(int idx);

private:
	QList<MapFeature*> Highlighted;

    MainWindow* Main;
	Ui::FeaturesDockWidget ui;
	QAction* centerAction;
	QAction* centerZoomAction;
	QAction* downloadAction;
	QAction* addSelectAction;

	CoordBox theViewport;
	MapFeature::FeatureType curFeatType;

	void clearItems();
	void addItem(MapFeaturePtr F);

public:
    void changeEvent(QEvent*);
    void retranslateUi();
	void retranslateTabBar();
};

#endif // FEATURESDOCK_H
