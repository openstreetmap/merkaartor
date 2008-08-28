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
#include "DirtyDock.h"
#include "PropertiesDock.h"
#include "MainWindow.h"
#include "MapView.h"
#include "Preferences/MerkaartorPreferences.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "Map/MapFeature.h"
#include "Map/DownloadOSM.h"
#include "Command/Command.h"
#include "Sync/DirtyList.h"

#include <QMessageBox>

DirtyDock::DirtyDock(MainWindow* aParent)
	: MDockAncestor(aParent), Main(aParent)
{
	setMinimumSize(220,100);
	setWindowTitle(tr("Undo"));
	setObjectName("dirtyDock");

	ui.setupUi(getWidget());

	centerAction = new QAction(tr("Center map"), this);
	connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
	centerZoomAction = new QAction(tr("Center && Zoom map"), this);
	connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));

	connect(ui.ChangesList, SIGNAL(itemSelectionChanged()), this, SLOT(on_ChangesList_itemSelectionChanged()));
	connect(ui.ChangesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_ChangesList_itemDoubleClicked(QListWidgetItem*)));
	connect(ui.ChangesList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_ChangesList_customContextMenuRequested(const QPoint &)));
}


DirtyDock::~DirtyDock()
{
}


void DirtyDock::updateList()
{
	int dirtyObjects = Main->document()->getDirtyLayer()->size();

	switch (dirtyObjects)
	{
		case 0:
			ui.label->setText(tr("There is <b>no</b> object in the dirty layer"));
			break;
		case 1:
			ui.label->setText(tr("There is <b>one</b> object in the dirty layer"));
			break;

		default:
			ui.label->setText(tr("There are <b>%1</b> objects in the dirty layer").arg(dirtyObjects));
			break;
	}

	ui.ChangesList->clear();

	Main->document()->history().buildUndoList(ui.ChangesList);
}

void DirtyDock::on_ChangesList_itemSelectionChanged()
{
	MapFeature* F;

	if (ui.ChangesList->selectedItems().count() != 0) {

		if (ui.ChangesList->selectedItems().count() == 1) {
			F = Main->document()->getFeature(ui.ChangesList->selectedItems()[0]->data(Qt::UserRole).toString());
			if (F)
				Main->properties()->setSelection(F);
		} else {
			Selection.clear();
			for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
				F = Main->document()->getFeature(ui.ChangesList->selectedItems()[i]->data(Qt::UserRole).toString());
				if (F)
					Selection.push_back(F);
			}
			Main->properties()->setMultiSelection(Selection);
		}

	}
	Main->view()->update();
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
	MapFeature* F;
	CoordBox cb;

	Main->setUpdatesEnabled(false);
	for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
		F = Main->document()->getFeature(ui.ChangesList->selectedItems()[0]->data(Qt::UserRole).toString());
		if (F) {
			if (cb.isNull())
				cb = F->boundingBox();
			else
				cb.merge(F->boundingBox());
		}
	}
	if (!cb.isNull()) {
		Coord c = cb.center();
		Main->view()->projection().setCenter(c, Main->view()->rect());
		Main->invalidateView();
	}
	Main->setUpdatesEnabled(true);
}

void DirtyDock::on_centerZoomAction_triggered()
{
	MapFeature* F;
	CoordBox cb;

	Main->setUpdatesEnabled(false);
	for (int i=0; i < ui.ChangesList->selectedItems().count(); ++i) {
		F = Main->document()->getFeature(ui.ChangesList->selectedItems()[0]->data(Qt::UserRole).toString());
		if (F) {
			if (cb.isNull())
				cb = F->boundingBox();
			else
				cb.merge(F->boundingBox());
		}
	}
	if (!cb.isNull()) {
		CoordBox min(cb.center()-10, cb.center()+10);
		cb.merge(min);
		cb = cb.zoomed(1.1);
		Main->view()->projection().setViewport(cb, Main->view()->rect());
		Main->invalidateView();
	}
	Main->setUpdatesEnabled(true);
}
