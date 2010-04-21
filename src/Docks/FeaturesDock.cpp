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

#include "Features.h"

#include <QAction>
#include <QTimer>
#include <QMenu>

#define MAX_FEATS 100

FeaturesDock::FeaturesDock(MainWindow* aParent)
    : MDockAncestor(aParent), Main(aParent), curFeatType(Feature::Relations)
{
    setMinimumSize(220,100);
    setObjectName("FeaturesDock");

    ui.setupUi(getWidget());

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
    ui.tabBar->setTabData(t, Feature::Relations);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, Feature::Ways);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, Feature::Nodes);
    t = ui.tabBar->addTab(NULL);
    ui.tabBar->setTabData(t, Feature::All);
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
    Main->on_editRemoveAction_triggered();
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
        CoordBox mini(cb.center()-10, cb.center()+10);
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
    Feature* F;
    QList<Feature*> toResolve;
    for (int i=0; i < ui.FeaturesList->selectedItems().count(); ++i) {
        F = ui.FeaturesList->selectedItems()[i]->data(Qt::UserRole).value<Feature*>();

        if (F->notEverythingDownloaded()) {
            toResolve.push_back(F);
        }
    }
    Main->downloadFeatures(toResolve);
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

void FeaturesDock::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    MDockAncestor::changeEvent(event);
}

void FeaturesDock::tabChanged(int idx)
{
    curFeatType = (Feature::FeatureType)ui.tabBar->tabData(idx).toInt();
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

    if (curFeatType == Feature::Relations || curFeatType == Feature::All)
    {
        if (Relation* L = CAST_RELATION(F)) {
            QListWidgetItem* anItem = new QListWidgetItem(L->description(), ui.FeaturesList);
            anItem->setData(Qt::UserRole, QVariant::fromValue(F));
        }
    }
    if (curFeatType == Feature::Ways || curFeatType == Feature::All)
    {
        if (Way* R = CAST_WAY(F)) {
            QListWidgetItem* anItem = new QListWidgetItem(R->description(), ui.FeaturesList);
            anItem->setData(Qt::UserRole, QVariant::fromValue(F));
        }
    }
    if (curFeatType == Feature::Nodes || curFeatType == Feature::All)
    {
        if (Node* N = CAST_NODE(F)) {
        for (int i=0; i<N->tagSize(); ++i)
            if ((N->tagKey(i) != "created_by") && (N->tagKey(i) != "ele")) {
                QListWidgetItem* anItem = new QListWidgetItem(N->description(), ui.FeaturesList);
                anItem->setData(Qt::UserRole, QVariant::fromValue(F));
                break;
            }
        }
    }
}

void FeaturesDock::clearItems()
{
    for(int i=ui.FeaturesList->count()-1; i>=0; --i) {
        if (!ui.FeaturesList->item(i)->isSelected())
            delete ui.FeaturesList->item(i);
    }
}

void FeaturesDock::updateList()
{
    setUpdatesEnabled(false);
    ui.FeaturesList->blockSignals(true);

    clearItems();

    if (!isVisible() || !Main->document())
        return;

    for (int j=0; j<Main->document()->layerSize(); ++j) {
        if (!Main->document()->getLayer(j)->size()
            || !Main->document()->getLayer(j)->isVisible()
            || Main->document()->getLayer(j)->isIndexingBlocked()
            )
            continue;

        if (dynamic_cast<ImageMapLayer*>(Main->document()->getLayer(j)))
            continue;

        QList < MapFeaturePtr > ret = Main->document()->getLayer(j)->indexFind(theViewport);
        foreach (MapFeaturePtr F, ret) {
            if (ui.cbWithin->isChecked()) {
                if (ggl::within(F->boundingBox(), Main->view()->viewport()))
                    addItem(F);
            } else
                addItem(F);
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
    setWindowTitle(tr("Features"));
    centerAction->setText(tr("Center map"));
    centerZoomAction->setText(tr("Center && Zoom map"));
    downloadAction->setText(tr("Download missing children"));
    addSelectAction->setText(tr("Add to selection"));
    deleteAction->setText(tr("Delete"));
}

void FeaturesDock::retranslateTabBar()
{
    ui.tabBar->setTabText(0, tr("Relations"));
    ui.tabBar->setTabText(1, tr("Roads"));
    ui.tabBar->setTabText(2, tr("POI's"));
    ui.tabBar->setTabText(3, tr("All"));
}


