//
// C++ Implementation: FeaturesDock
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "FeaturesDock.h"

#include "MainWindow.h"
#include "MapView.h"
#include "Document.h"
#include "ImageMapLayer.h"
#include "PropertiesDock.h"
#include "Global.h"

#include "Features.h"

#include "SelectionDialog.h"
#include "TagSelector.h"

#include <QAction>
#include <QTimer>
#include <QMenu>

#define MAX_FEATS 100

FeaturesDock::FeaturesDock(MainWindow* aParent)
    : MDockAncestor(aParent),
    Main(aParent),
    curFeatType(IFeature::OsmRelation),
    findMode(false)
{
//    setMinimumSize(220,100);
    setObjectName("FeaturesDock");

    ui.setupUi(getWidget());

#ifdef Q_OS_MAC
    ui.cbWithin->setMinimumWidth(30);
#endif

    ui.cbWithin->setChecked(M_PREFS->getFeaturesWithin());
    ui.FeaturesList->sortItems();

    connect(this, SIGNAL(visibilityChanged(bool)), this, SLOT(on_Viewport_changed()));

    deleteAction = new QAction(NULL, this);
//    deleteAction->setShortcut(QKeySequence::Delete);
    ui.FeaturesList->addAction(deleteAction);
    connect(deleteAction, SIGNAL(triggered()), SLOT(on_FeaturesList_delete()));

    connect(ui.FeaturesList, SIGNAL(itemSelectionChanged()), this, SLOT(on_FeaturesList_itemSelectionChanged()));
    connect(ui.FeaturesList, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_FeaturesList_itemDoubleClicked(QListWidgetItem*)));
    connect(ui.FeaturesList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_FeaturesList_customContextMenuRequested(const QPoint &)));

    connect(ui.cbWithin, SIGNAL(stateChanged(int)), this, SLOT(on_rbWithin_stateChanged(int)));
    connect(ui.btFind, SIGNAL(clicked(bool)), SLOT(on_btFind_clicked(bool)));
    connect(ui.btReset, SIGNAL(clicked(bool)), SLOT(on_btReset_clicked(bool)));

    centerAction = new QAction(NULL, this);
    connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
    centerZoomAction = new QAction(NULL, this);
    connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));
    downloadAction = new QAction(NULL, this);
    connect(downloadAction, SIGNAL(triggered()), this, SLOT(on_downloadAction_triggered()));
    addSelectAction = new QAction(NULL, this);
    connect(addSelectAction, SIGNAL(triggered()), this, SLOT(on_addSelectAction_triggered()));

    int t;
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, IFeature::OsmRelation);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, IFeature::LineString);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, IFeature::Point);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, IFeature::All);
    ui.tabBar->setElideMode(Qt::ElideRight);
    ui.tabBar->setUsesScrollButtons(true);
    retranslateTabBar();

    connect(ui.tabBar, SIGNAL(currentChanged (int)), this, SLOT(tabChanged(int)));
    //connect(ui.tabBar, SIGNAL(customContextMenuRequested (const QPoint&)), this, SLOT(tabContextMenuRequested(const QPoint&)));

    retranslateUi();
}

FeaturesDock::~FeaturesDock()
{
}

void FeaturesDock::on_FeaturesList_itemSelectionChanged()
{
    Highlighted.clear();
    for (int i=0; i<ui.FeaturesList->selectedItems().count(); ++i) {
        QListWidgetItem* item = ui.FeaturesList->selectedItems()[i];
        Feature * F = item->data(Qt::UserRole).value<Feature*>();
        Highlighted.push_back(F);
    }

    Main->view()->update();
}

void FeaturesDock::on_FeaturesList_itemDoubleClicked(QListWidgetItem* item)
{
    Feature * F = item->data(Qt::UserRole).value<Feature*>();
    Main->properties()->setSelection(F);
    Main->view()->update();
}

void FeaturesDock::on_FeaturesList_customContextMenuRequested(const QPoint & pos)
{
    QListWidgetItem *it = ui.FeaturesList->itemAt(pos);
    if (!it)
        return;

    QMenu menu(ui.FeaturesList);
    menu.addAction(addSelectAction);
    menu.addAction(deleteAction);
    menu.addSeparator();
    menu.addAction(centerAction);
    menu.addAction(centerZoomAction);
    menu.addSeparator();

    downloadAction->setEnabled(false);
    Feature* F;
    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();
        if (F->notEverythingDownloaded()) {
            downloadAction->setEnabled(true);
            break;
        }
    }
    menu.addAction(downloadAction);
    menu.exec(ui.FeaturesList->mapToGlobal(pos));
}

void FeaturesDock::on_FeaturesList_delete()
{
    if (!ui.FeaturesList->selectedItems().count())
        return;

    Feature* F;
    Main->view()->blockSignals(true);

    Highlighted.clear();
    Main->properties()->setSelection(0);
    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();
        if (F) {
            Main->properties()->addSelection(F);
        }
    }

    Main->view()->blockSignals(false);
#ifndef _MOBILE
    Main->on_editRemoveAction_triggered();
#endif
}

void FeaturesDock::on_rbWithin_stateChanged ( int state )
{
    M_PREFS->setFeaturesWithin((state == Qt::Checked));

    updateList();
}

void FeaturesDock::on_centerAction_triggered()
{
    Feature* F;
    CoordBox cb;

    Main->view()->blockSignals(true);

    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();
        if (F) {
            if (cb.isNull())
                cb = F->boundingBox();
            else
                cb.merge(F->boundingBox());
        }
    }
    if (!cb.isNull()) {
        Coord c = cb.center();
        Main->view()->setCenter(c, Main->view()->rect());
        Main->invalidateView();
    }
    Main->view()->blockSignals(false);

    QTimer::singleShot(10, this, SLOT(on_Viewport_changed()));
}

void FeaturesDock::on_centerZoomAction_triggered()
{
    Feature* F;
    CoordBox cb;

    Main->view()->blockSignals(true);

    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();
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
        Main->view()->setViewport(cb, Main->view()->rect());
        Main->invalidateView();
    }
    Main->view()->blockSignals(false);

    QTimer::singleShot(10, this, SLOT(on_Viewport_changed()));
}

void FeaturesDock::on_downloadAction_triggered()
{
#ifndef _MOBILE
    Feature* F;
    QList<Feature*> toResolve;
    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();

        if (F->notEverythingDownloaded()) {
            toResolve.push_back(F);
        }
    }
    Main->downloadFeatures(toResolve);
#endif
}

void FeaturesDock::on_addSelectAction_triggered()
{
    Feature* F;
    Main->view()->blockSignals(true);

    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();
        if (F) {
            Main->properties()->addSelection(F);
        }
    }

    Main->view()->blockSignals(false);
}

void FeaturesDock::on_btFind_clicked(bool)
{
    SelectionDialog* dlg = new SelectionDialog(Main);
    if (!dlg->exec())
        return;

    TagSelector* tsel = TagSelector::parse(dlg->edTagQuery->text());
    if (!tsel)
        return;

    Found.clear();
    int added = 0;
    for (VisibleFeatureIterator i(Main->document()); !i.isEnd() && (!dlg->sbMaxResult->value() || added < dlg->sbMaxResult->value()); ++i) {
        if (tsel->matches(i.get(), Main->view()->pixelPerM())) {
            Found << i.get();
        }
    }

    findMode = true;
    ui.tabBar->blockSignals(true);
    ui.tabBar->setCurrentIndex(3);
    ui.tabBar->blockSignals(false);
    tabChanged(3);
}

void FeaturesDock::on_btReset_clicked(bool)
{
    findMode = false;
    updateList();
}

void FeaturesDock::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    MDockAncestor::changeEvent(event);
}

void FeaturesDock::tabChanged(int idx)
{
    curFeatType = (IFeature::FeatureType)ui.tabBar->tabData(idx).toInt();
    ui.FeaturesList->clear();
    Highlighted.clear();

    updateList();
}

void FeaturesDock::on_Viewport_changed()
{
    theViewport = Main->view()->viewport();

    updateList();
}

void FeaturesDock::addItem(MapFeaturePtr F)
{
    if (ui.FeaturesList->count() > MAX_FEATS)
        return;

    if (Highlighted.contains(F))
        return;

    if (curFeatType == IFeature::OsmRelation || curFeatType == Feature::All)
    {
        if (Relation* L = CAST_RELATION(F)) {
            QListWidgetItem* anItem = new QListWidgetItem(L->description(), ui.FeaturesList);
            anItem->setData(Qt::UserRole, QVariant::fromValue(F));
        }
    }
    if (curFeatType == IFeature::LineString || curFeatType == IFeature::Polygon || curFeatType == Feature::All)
    {
        if (Way* R = CAST_WAY(F)) {
            QListWidgetItem* anItem = new QListWidgetItem(R->description(), ui.FeaturesList);
            anItem->setData(Qt::UserRole, QVariant::fromValue(F));
        }
    }
    if (curFeatType == IFeature::Point || curFeatType == Feature::All)
    {
        if (Node* N = CAST_NODE(F)) {
            QListWidgetItem* anItem = new QListWidgetItem(N->description(), ui.FeaturesList);
            anItem->setData(Qt::UserRole, QVariant::fromValue(F));
        }
    }
}

void FeaturesDock::invalidate()
{
    ui.FeaturesList->clear();
    Highlighted.clear();
    Found.clear();
}

void FeaturesDock::clearItems()
{
    for(int i=ui.FeaturesList->count()-1; i>=0; --i) {
        QListWidgetItem* item = ui.FeaturesList->item(i);
        if (item->isSelected()) {
            Feature * F = item->data(Qt::UserRole).value<Feature*>();
            if (F->isDeleted()) {
                item->setSelected(false);
                Highlighted.removeOne(F);
            }
        }
        if (!item->isSelected())
            delete item;
    }
}

void FeaturesDock::updateList()
{
    setUpdatesEnabled(false);
    ui.FeaturesList->blockSignals(true);

    clearItems();

    if (!isVisible() || !Main->document())
        return;

    if (findMode) {
        foreach (MapFeaturePtr F, Found) {
            if (ui.cbWithin->isChecked()) {
                if (Main->view()->viewport().contains(F->boundingBox()))
                    addItem(F);
            } else
                addItem(F);
        }
    } else {
        for (int j=0; j<Main->document()->layerSize(); ++j) {
            if (!Main->document()->getLayer(j)->size())
                continue;
            QList < Feature* > ret = g_backend.indexFind(Main->document()->getLayer(j), theViewport);
            foreach (Feature* F, ret) {
                if (F->isHidden())
                    continue;

                if (ui.cbWithin->isChecked()) {
                    if (Main->view()->viewport().contains(F->boundingBox()))
                        addItem(F);
                } else
                    addItem(F);
            }
        }
    }
    ui.FeaturesList->sortItems();

    ui.FeaturesList->blockSignals(false);
    setUpdatesEnabled(true);
}

int FeaturesDock::highlightedSize() const
{
    if (!isVisible())
        return 0;
    return Highlighted.size();
}

Feature* FeaturesDock::highlighted(int idx)
{
    if (idx < Highlighted.size())
        return Highlighted[idx];
    return 0;
}

QList<Feature*> FeaturesDock::highlighted()
{
    return Highlighted;
}

void FeaturesDock::retranslateUi()
{
    ui.retranslateUi(getWidget());

    setWindowTitle(tr("Features"));
    centerAction->setText(tr("Center map"));
    centerZoomAction->setText(tr("Center && Zoom map"));
    downloadAction->setText(tr("Download missing children"));
    addSelectAction->setText(tr("Add to selection"));
    deleteAction->setText(tr("Delete"));

    retranslateTabBar();
}

void FeaturesDock::retranslateTabBar()
{
    ui.tabBar->setTabText(0, tr("Relations"));
    ui.tabBar->setTabText(1, tr("Roads"));
    ui.tabBar->setTabText(2, tr("POI's"));
    ui.tabBar->setTabText(3, tr("All"));
}


