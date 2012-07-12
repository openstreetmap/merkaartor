#include "PropertiesDock.h"

#include "Global.h"

#include "InfoDock.h"
#ifndef _MOBILE
#include "ui_MainWindow.h"
#endif
#include "MapView.h"
#include "Interaction.h"
#include "TagModel.h"
#include "EditCompleterDelegate.h"
#include "ShortcutOverrideFilter.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "NodeCommands.h"
#include "RelationCommands.h"
#include "Coord.h"
#include "Document.h"
#include "Features.h"
#include "FeatureManipulations.h"
#include "TagTemplate.h"

#ifdef GEOIMAGE
#include "GeoImageDock.h"
#endif

#include <QtCore/QTimer>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QTableView>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

#include <algorithm>

PropertiesDock::PropertiesDock()
: MDockAncestor(), CurrentUi(0),
    theTemplates(0), CurrentTagView(0), NowShowing(NoUiShowing)
{
    setMinimumSize(220,100);
    switchToNoUi();
    setObjectName("propertiesDock");
    theModel = new TagModel();
    delegate = new EditCompleterDelegate(this);

    // Set up the shortcut event filter for the tableviews
    // This allows them to react to keys already bound to
    // application wide shortcuts
    shortcutFilter = new ShortcutOverrideFilter();
    shortcutFilter->addOverride(Qt::Key_Up);
    shortcutFilter->addOverride(Qt::Key_Down);
    shortcutFilter->addOverride(Qt::Key_Left);
    shortcutFilter->addOverride(Qt::Key_Right);
    shortcutFilter->addOverride(Qt::Key_F2);
    shortcutFilter->addOverride(Qt::Key_Delete);

    centerAction = new QAction(NULL, this);
    connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
    centerZoomAction = new QAction(NULL, this);
    connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));

    loadTemplates();

    retranslateUi();
}

PropertiesDock::~PropertiesDock(void)
{
    delete theModel;
    delete theTemplates;
    delete shortcutFilter;
}

static bool isChildOfSingleRoadInner(Feature *mapFeature)
{
    return Way::GetSingleParentRoadInner(mapFeature) != NULL;
}

static bool isChildOfArea(Feature *mapFeature)
{
    Way* R =  Way::GetSingleParentRoadInner(mapFeature);
    if (R)
        return (R->area() > 0.0);
    return false;
}

static bool isChildOfSingleRoad(Feature *mapFeature)
{
    return Way::GetSingleParentRoad(mapFeature) != NULL;
}

static bool isChildOfSingleRelation(Feature *mapFeature)
{
    int parentRelations = 0;
    for (int i=0; i<mapFeature->sizeParents(); i++)
    {
        Feature * parent = CAST_FEATURE(mapFeature->getParent(i));
        if (!parent || parent->isDeleted()) continue;

        bool isParentRelation = dynamic_cast<Relation*>(parent) != 0;
        if (isParentRelation)
            parentRelations++;
            if (parentRelations > 1)
                return false;
    }

    return (parentRelations == 1);
}

static bool isChildOfRelation(Feature *mapFeature)
{
    for (int i=0; i<mapFeature->sizeParents(); i++)
    {
        Relation * rel = CAST_RELATION(mapFeature->getParent(i));
        if (rel && !rel->isDeleted())
            return true;
    }

    return false;
}

void PropertiesDock::checkMenuStatus()
{
    bool IsPoint = false;
    bool IsRoad = false;
    bool IsRelation = false;
    bool IsParentRoad = false;
    bool IsParentRoadInner = false;
    bool IsParentRelation = false;
    bool IsParentArea = false;
    bool IsOpenStreetBug = false;
    int NumRoads = 0;
    int NumCommitableFeature = 0;
    int NumPoints = 0;
    int NumRelation = 0;
    int NumRelationChild = 0;
    int NumAreas = 0;
    int NumParents = 0;
    int NumChildren = 0;
    int NumIncomplete = 0;
    if (Selection.size() == 1)
    {
        IsPoint = CAST_NODE(Selection[0]) != 0;
        IsRoad = CAST_WAY(Selection[0]) != 0;
        IsRelation = CAST_RELATION(Selection[0]) != 0;
        IsParentRoad = IsPoint && isChildOfSingleRoad(Selection[0]);
        IsParentRoadInner = IsPoint && isChildOfSingleRoadInner(Selection[0]);
        IsParentRelation = isChildOfSingleRelation(Selection[0]);
        IsParentArea = isChildOfArea(Selection[0]);
        IsOpenStreetBug = isChildOfArea(Selection[0]);
        IsOpenStreetBug = IsPoint && (Selection[0]->id().type & IFeature::Special);
    }
    for (int i=0; i<Selection.size(); ++i)
    {
        if (Selection[i]->sizeParents())
            ++NumParents;
        if (Selection[i]->size())
            ++NumChildren;
        if (Selection[i]->notEverythingDownloaded())
            ++NumIncomplete;

        if (CAST_NODE(Selection[i]))
            ++NumPoints;
        if (Way* R = dynamic_cast<Way*>(Selection[i]))
        {
            if (R->area() > 0.0)
            {
                ++NumAreas;
            }
            else
            {
                ++NumRoads;
            }
        }
        if (CAST_RELATION(Selection[i]))
            ++NumRelation;
        if (isChildOfRelation(Selection[i]))
            ++NumRelationChild;

        if (!Selection[i]->isUploadable() && !Selection[i]->isSpecial())
            ++NumCommitableFeature;
    }
#ifndef _MOBILE
    CUR_MAINWINDOW->ui->createRelationAction->setEnabled(Selection.size());
    CUR_MAINWINDOW->ui->editRemoveAction->setEnabled(Selection.size());
    CUR_MAINWINDOW->ui->editMoveAction->setEnabled(true);
    CUR_MAINWINDOW->ui->editReverseAction->setEnabled(IsRoad || NumAreas + NumRoads > 0);
    CUR_MAINWINDOW->ui->roadAddStreetNumbersAction->setEnabled(NumRoads >= 1);
    CUR_MAINWINDOW->ui->roadJoinAction->setEnabled(NumRoads >= 1 && canJoinRoads(this));
    CUR_MAINWINDOW->ui->roadCreateJunctionAction->setEnabled(NumRoads > 1 && canCreateJunction(this));
    CUR_MAINWINDOW->ui->roadSplitAction->setEnabled((IsParentRoadInner && !IsParentArea) || (NumRoads && NumPoints) || (NumAreas && NumPoints > 1));
    CUR_MAINWINDOW->ui->roadBreakAction->setEnabled(IsParentRoadInner || ((NumRoads == 1 || NumAreas == 1) && NumPoints) || (NumRoads > 1 && canBreakRoads(this)));
    CUR_MAINWINDOW->ui->roadSimplifyAction->setEnabled(IsRoad || NumRoads > 0 || NumAreas > 0);
    CUR_MAINWINDOW->ui->roadSubdivideAction->setEnabled((NumRoads + NumAreas) == 1 && (!NumPoints || NumPoints == 2) && canSubdivideRoad(this));
    CUR_MAINWINDOW->ui->roadAxisAlignAction->setEnabled((NumRoads + NumAreas) > 0 || (NumRelation > 0 && canAxisAlignRoads(this)));
    CUR_MAINWINDOW->ui->areaJoinAction->setEnabled(NumAreas > 1);
    CUR_MAINWINDOW->ui->areaSplitAction->setEnabled(NumAreas == 1 && NumPoints == 2 && canSplitArea(this));
    CUR_MAINWINDOW->ui->areaTerraceAction->setEnabled(NumAreas == 1 && NumRoads == 0 && canTerraceArea(this));
    CUR_MAINWINDOW->ui->featureSelectChildrenAction->setEnabled(NumChildren);
    CUR_MAINWINDOW->ui->featureSelectParentsAction->setEnabled(NumParents);
    CUR_MAINWINDOW->ui->featureDownloadMissingChildrenAction->setEnabled(NumIncomplete);
    CUR_MAINWINDOW->ui->featureDeleteAction->setEnabled((IsPoint || IsRoad || IsRelation) && !Selection[0]->isDirty());
    CUR_MAINWINDOW->ui->featureCommitAction->setEnabled(NumCommitableFeature);
    CUR_MAINWINDOW->ui->nodeMergeAction->setEnabled(NumPoints > 1);
    CUR_MAINWINDOW->ui->nodeAlignAction->setEnabled(NumPoints > 2);
    CUR_MAINWINDOW->ui->nodeSpreadAction->setEnabled(NumPoints > 2);
    CUR_MAINWINDOW->ui->nodeDetachAction->setEnabled(NumPoints && canDetachNodes(this));
    CUR_MAINWINDOW->ui->relationAddMemberAction->setEnabled(NumRelation && Selection.size() > 1);
    CUR_MAINWINDOW->ui->relationRemoveMemberAction->setEnabled((NumRelation && Selection.size() > 1 && NumRelationChild) || IsParentRelation);
    CUR_MAINWINDOW->ui->relationAddToMultipolygonAction->setEnabled((NumAreas > 1) || (NumAreas >0 && NumRelation == 1));
    CUR_MAINWINDOW->ui->menuOpenStreetBugs->setEnabled(IsOpenStreetBug);

    CUR_MAINWINDOW->ui->editCopyAction->setEnabled(Selection.size());
    CUR_MAINWINDOW->clipboardChanged();
#endif
}

int PropertiesDock::selectionSize() const
{
    return Selection.size();
}

Feature* PropertiesDock::selection(int idx)
{
    if (idx < Selection.size())
        return Selection[idx];
    return 0;
}

QList<Feature*> PropertiesDock::selection()
{
    return Selection;
}

void PropertiesDock::setSelection(Feature*aFeature)
{
    cleanUpUi();
    Selection.clear();
    if (aFeature)
        Selection.push_back(aFeature);
    FullSelection = Selection;
    switchUi();
    fillMultiUiSelectionBox();
}

void PropertiesDock::setMultiSelection(const QList<Feature*>& aFeatureList)
{
    cleanUpUi();
    Selection.clear();
    for (int i=0; i<aFeatureList.size(); ++i)
        Selection.push_back(aFeatureList[i]);
    FullSelection = Selection;
    switchUi();
    fillMultiUiSelectionBox();

//    cleanUpUi();
//    Selection.clear();
//    for (int i=0; i<aFeatureList.size(); ++i)
//        Selection.push_back(aFeatureList[i]);
//    FullSelection = Selection;
//    switchToMultiUi();
//    // to prevent slots to change the values also
//    QList<Feature*> Current = Selection;
//    Selection.clear();
//    MultiUi.TagView->setModel(theModel);
//    MultiUi.TagView->setItemDelegate(delegate);
//    INFO_DOCK->setHtml("");
//    #ifdef GEOIMAGE
//    CUR_MAINWINDOW->geoImage()->setImage((Node *)NULL);
//    #endif
//    CurrentTagView = MultiUi.TagView;
//    theModel->setFeature(Current);
//    Selection = Current;
//    fillMultiUiSelectionBox();
}

void PropertiesDock::toggleSelection(Feature* S)
{
    cleanUpUi();
    QList<Feature*>::iterator i = std::find(Selection.begin(),Selection.end(),S);
    if (i == Selection.end())
        Selection.push_back(S);
    else
        Selection.erase(i);
    FullSelection = Selection;
    switchUi();
    fillMultiUiSelectionBox();
}

void PropertiesDock::addSelection(Feature* S)
{
    cleanUpUi();
    QList<Feature*>::iterator i = std::find(Selection.begin(),Selection.end(),S);
    if (i == Selection.end())
        Selection.push_back(S);
    FullSelection = Selection;
    switchUi();
    fillMultiUiSelectionBox();
}

void PropertiesDock::setHighlighted(const QList<Feature*>& aFeatureList)
{
    cleanUpUi();
    Highlighted.clear();
    for (int i=0; i<aFeatureList.size(); ++i)
        Highlighted.push_back(aFeatureList[i]);
}

void PropertiesDock::adjustSelection()
{
    QList<Feature*> aSelection;
    int cnt = Selection.size();

    for (int i=0; i<FullSelection.size(); ++i) {
        if (CUR_DOCUMENT->exists(FullSelection[i]) && FullSelection[i] && !FullSelection[i]->isDeleted()) {
            aSelection.push_back(FullSelection[i]);
        } else {
            QList<Feature*>::iterator it = std::find(Selection.begin(),Selection.end(),FullSelection[i]);
            if (it != Selection.end())
                Selection.erase(it);
        }
    }

    FullSelection = aSelection;
    if (Selection.size() != cnt)
        switchUi();
}

bool PropertiesDock::isSelected(Feature *aFeature)
{
    QList<Feature*>::iterator i = std::find(Selection.begin(),Selection.end(),aFeature);
    if (i == Selection.end())
        return false;
    else
        return true;
}

void PropertiesDock::fillMultiUiSelectionBox()
{
    if (NowShowing == MultiShowing)
    {
        // to prevent on_SelectionList_itemSelectionChanged to kick in
        NowShowing = NoUiShowing;
        CUR_MAINWINDOW->setUpdatesEnabled(false);
        MultiUi.SelectionList->clear();
        for (int i=0; i<FullSelection.size(); ++i)
        {
            QListWidgetItem* it = new QListWidgetItem(FullSelection[i]->description(),MultiUi.SelectionList);
            it->setData(Qt::UserRole,QVariant(i));
            it->setSelected(true);
        }
        MultiUi.lbStatus->setText(tr("%1/%1 selected item(s)").arg(FullSelection.size()));
        CUR_MAINWINDOW->setUpdatesEnabled(true);
        NowShowing = MultiShowing;
    }
}

void PropertiesDock::on_SelectionList_itemSelectionChanged()
{
    if (NowShowing == MultiShowing)
    {
        Highlighted.clear();
        for (int i=0; i<FullSelection.size(); ++i)
            if (MultiUi.SelectionList->item(i)->isSelected())
                Highlighted.push_back(FullSelection[i]);
        if (Highlighted.size() == 1) {
            INFO_DOCK->setHtml(Highlighted[0]->toHtml());

            #ifdef GEOIMAGE
            Node *Pt;
            if ((Pt = dynamic_cast<Node*>(Highlighted[0]))) CUR_MAINWINDOW->geoImage()->setImage(Pt);
            #endif
        }
        MultiUi.lbStatus->setText(tr("%1/%2 selected item(s)").arg(Highlighted.size()).arg(FullSelection.size()));
        CUR_VIEW->update();
    }
}

void PropertiesDock::on_SelectionList_itemDoubleClicked(QListWidgetItem* item)
{
    int i=item->data(Qt::UserRole).toUInt();
    PendingSelectionChange = i;
    // changing directly from this method would delete the current Ui from
    // which this slot is called
    QTimer::singleShot(0,this,SLOT(executePendingSelectionChange()));
}

void PropertiesDock::executePendingSelectionChange()
{
    if (PendingSelectionChange < FullSelection.size())
        setSelection(FullSelection[PendingSelectionChange]);
}

void PropertiesDock::cleanUpUi()
{
//    if (NowShowing == RelationUiShowing)
//    {
//        RelationUi.MembersView->setModel(0);
//        Relation* R = dynamic_cast<Relation*>(FullSelection[0]);
//        R->releaseMemberModel();
//    }
}

void PropertiesDock::switchUi()
{
    if (CurrentTagView)
        M_PREFS->setTagListFirstColumnWidth(qMax(CurrentTagView->columnWidth(0), 20));

    if (FullSelection.size() == 0)
        switchToNoUi();
    else if (FullSelection.size() == 1)
    {
        if (FullSelection[0]->isVirtual())
            switchToNoUi();
        else if (CAST_NODE(FullSelection[0]))
            switchToNodeUi();
        else
            switchToDefaultUi();
    }
    else
        switchToMultiUi();
    resetValues();
}

void PropertiesDock::switchToMultiUi()
{
    if (NowShowing == MultiShowing) return;
    NowShowing = MultiShowing;
    QWidget* NewUi = new QWidget(this);
    MultiUi.setupUi(NewUi);
    MultiUi.TagView->verticalHeader()->hide();
    MultiUi.SelectionList->setContextMenuPolicy(Qt::CustomContextMenu);
    MultiUi.lbStatus->setText(tr("Selected items"));
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    connect(MultiUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
    connect(MultiUi.SelectionList,SIGNAL(itemSelectionChanged()),this,SLOT(on_SelectionList_itemSelectionChanged()));
    connect(MultiUi.SelectionList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(on_SelectionList_itemDoubleClicked(QListWidgetItem*)));
    connect(MultiUi.SelectionList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_SelectionList_customContextMenuRequested(const QPoint &)));
    setWindowTitle(tr("Properties - Multiple elements"));
}

void PropertiesDock::switchToNodeUi()
{
    NowShowing = TrackPointUiShowing;
    QWidget* NewUi = new QWidget(this);
    TrackPointUi.setupUi(NewUi);
    TrackPointUi.TagView->verticalHeader()->hide();
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    connect(TrackPointUi.Longitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLon_editingFinished()));
    connect(TrackPointUi.Latitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLat_editingFinished()));
    connect(TrackPointUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
    connect(TrackPointUi.SourceTagButton,SIGNAL(clicked()),this, SLOT(on_SourceTagButton_clicked()));
    setWindowTitle(tr("Properties - Node"));
}

void PropertiesDock::switchToDefaultUi()
{
    NowShowing = DefaultUiShowing;
    QWidget* NewUi = new QWidget(this);
    DefaultUi.setupUi(NewUi);
    DefaultUi.TagView->verticalHeader()->hide();
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    connect(DefaultUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
    connect(DefaultUi.SourceTagButton,SIGNAL(clicked()),this, SLOT(on_SourceTagButton_clicked()));
    setWindowTitle(tr("Properties - Default"));
}

void PropertiesDock::switchToNoUi()
{
    if (NowShowing == NoUiShowing) return;
    NowShowing = NoUiShowing;
    QWidget* NewUi = new QWidget(this);
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    setWindowTitle(tr("Properties"));
}

void PropertiesDock::resetValues()
{
    Highlighted.clear();

    // Tables that might need column sizing
    CurrentTagView = NULL;

    // to prevent slots to change the values also
    QList<Feature*> Current = Selection;
    Selection.clear();
    if (FullSelection.size() == 1)
    {
        INFO_DOCK->setHtml(FullSelection[0]->toHtml());

        Node* Pt = dynamic_cast<Node*>(FullSelection[0]);
        Feature* F = FullSelection[0];

        if ((Pt) && (NowShowing == TrackPointUiShowing))
        {
            TrackPointUi.Id->setText(QString::number(Pt->id().numId));
            TrackPointUi.Latitude->setText(COORD2STRING(Pt->position().y()));
            TrackPointUi.Longitude->setText(COORD2STRING(Pt->position().x()));
            TrackPointUi.TagView->setModel(theModel);
            TrackPointUi.TagView->setItemDelegate(delegate);
            TrackPointUi.RemoveTagButton->setEnabled(false);
            TrackPointUi.SourceTagButton->setEnabled(!CUR_DOCUMENT->getCurrentSourceTags().isEmpty());

            QWidget* w;
            for (int i=0; i<TrackPointUi.variableLayout->count(); ++i) {
                w = TrackPointUi.variableLayout->itemAt(i)->widget();
                if (w) {
                    w->hide();
                    w->deleteLater();
                }
            }
            if (theTemplates) {
                w = theTemplates->getWidget(Pt, CUR_VIEW);
                w->installEventFilter(shortcutFilter);
                TrackPointUi.variableLayout->addWidget(w);
            }

            CurrentTagView = TrackPointUi.TagView;

            #ifdef GEOIMAGE
            CUR_MAINWINDOW->geoImage()->setImage(Pt);
            #endif
        }
        else if ((F) && (NowShowing == DefaultUiShowing))
        {
            DefaultUi.Id->setText(QString::number(F->id().numId));
            //DefaultUi.Name->setText(F->tagValue("name",""));
            DefaultUi.TagView->setModel(theModel);
            DefaultUi.TagView->setItemDelegate(delegate);
            DefaultUi.RemoveTagButton->setEnabled(false);
            DefaultUi.SourceTagButton->setEnabled(!CUR_DOCUMENT->getCurrentSourceTags().isEmpty());

            QWidget* w;
            for (int i=0; i<DefaultUi.variableLayout->count(); ++i) {
                w = DefaultUi.variableLayout->itemAt(i)->widget();
                if (w) {
                    w->hide();
                    w->deleteLater();
                }
            }
            if (theTemplates) {
                w = theTemplates->getWidget(F, CUR_VIEW);
                w->installEventFilter(shortcutFilter);
                DefaultUi.variableLayout->addWidget(w);
            }

            CurrentTagView = DefaultUi.TagView;
        }

        if (theTemplates)
            theTemplates->apply(FullSelection[0]);
    }
    else if ((FullSelection.size() > 1)  && (NowShowing == MultiShowing))
    {
        INFO_DOCK->setHtml("");
        #ifdef GEOIMAGE
        CUR_MAINWINDOW->geoImage()->setImage((Node *)NULL);
        #endif
        MultiUi.TagView->setModel(theModel);
        MultiUi.TagView->setItemDelegate(delegate);
        MultiUi.RemoveTagButton->setEnabled(false);
        CurrentTagView = MultiUi.TagView;
    }
    theModel->setFeature(Current);
    Selection = Current;
    checkMenuStatus();

    /* If we have standard TableViews in the current UI, set it so that the */
    /* first column is the width of the default text (Edit this to add...)  */
    /* And the rest of the space is assigned to the second column           */
    if (CurrentTagView) {
        if (M_PREFS->getTagListFirstColumnWidth() > 20 && M_PREFS->getTagListFirstColumnWidth() < CurrentTagView->width())
            CurrentTagView->setColumnWidth(
                0, M_PREFS->getTagListFirstColumnWidth()
            );
        else
            CurrentTagView->setColumnWidth(
                0, CurrentTagView->fontMetrics().width(theModel->newKeyText())+10
            );
        connect(CurrentTagView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(on_tag_selection_changed(QItemSelection,QItemSelection)));
        CurrentTagView->horizontalHeader()->setStretchLastSection(true);
        CurrentTagView->installEventFilter(shortcutFilter);
    }
}

void PropertiesDock::on_TrackPointLat_editingFinished()
{
    if (TrackPointUi.Latitude->text().isEmpty()) return;
    Node* Pt = dynamic_cast<Node*>(selection(0));
    if (Pt)
    {
        CUR_DOCUMENT->addHistory(
            new MoveNodeCommand(Pt,
                Coord(Pt->position().x(), TrackPointUi.Latitude->text().toDouble()), CUR_DOCUMENT->getDirtyOrOriginLayer(Pt->layer()) ));
        CUR_MAINWINDOW->invalidateView(false);
    }
}

void PropertiesDock::on_TrackPointLon_editingFinished()
{
    if (TrackPointUi.Longitude->text().isEmpty()) return;
    Node* Pt = dynamic_cast<Node*>(selection(0));
    if (Pt)
    {
        CUR_DOCUMENT->addHistory(
            new MoveNodeCommand(Pt,
                Coord(TrackPointUi.Longitude->text().toDouble(), Pt->position().y()), CUR_DOCUMENT->getDirtyOrOriginLayer(Pt->layer()) ));
        CUR_MAINWINDOW->invalidateView(false);
    }
}

void PropertiesDock::on_tag_selection_changed(const QItemSelection & selected, const QItemSelection & deselected)
{
    QModelIndexList indexes = selected.indexes();
    if (indexes.isEmpty()) return;

    switch(NowShowing) {
    case NoUiShowing:
        return;

    case TrackPointUiShowing:
        TrackPointUi.RemoveTagButton->setEnabled(false);
        break;

    case DefaultUiShowing:
        DefaultUi.RemoveTagButton->setEnabled(false);
        break;

    case MultiShowing:
        MultiUi.RemoveTagButton->setEnabled(false);
        break;
    }

    while (!indexes.isEmpty()) {
        QModelIndex index = indexes.takeLast();
        QModelIndex idx = index.sibling(index.row(),0);
        QVariant Content(theModel->data(idx,Qt::DisplayRole));
        if (Content.isValid() && Content.toString() != TagModel::newKeyText())
        {
            switch(NowShowing) {
            case NoUiShowing:
                return;

            case TrackPointUiShowing:
                TrackPointUi.RemoveTagButton->setEnabled(true);
                break;

            case DefaultUiShowing:
                DefaultUi.RemoveTagButton->setEnabled(true);
                break;

            case MultiShowing:
                MultiUi.RemoveTagButton->setEnabled(true);
                break;
            }
            break;
        }
    }
}

void PropertiesDock::on_tag_changed(QString k, QString v)
{
    if (!FullSelection.size())
        return;
    Feature* F = FullSelection[0];
    if (F->tagValue(k, "__NULL__") != v) {
        CUR_DOCUMENT->addHistory(new SetTagCommand(F,k,v,CUR_DOCUMENT->getDirtyOrOriginLayer(F->layer())));
        CUR_MAINWINDOW->invalidateView();
    }
}

void PropertiesDock::on_tag_cleared(QString k)
{
    if (!FullSelection.size())
        return;
    Feature* F = FullSelection[0];
    CUR_DOCUMENT->addHistory(new ClearTagCommand(F,k,CUR_DOCUMENT->getDirtyOrOriginLayer(F->layer())));
    CUR_MAINWINDOW->invalidateView();
}

void PropertiesDock::on_RemoveTagButton_clicked()
{
    if (!CurrentTagView)
        return;

    QModelIndexList indexes = CurrentTagView->selectionModel()->selectedIndexes();
    if (indexes.isEmpty()) return;

    CommandList *L = 0;
    if (indexes.count()==1)
    {
        QModelIndex index = indexes.at(0);
        QModelIndex idx = index.sibling(index.row(),0);
        QVariant Content(theModel->data(idx,Qt::DisplayRole));
        if (Content.isValid())
        {
            QString KeyName = Content.toString();
            L = new CommandList(MainWindow::tr("Clear Tag '%1' on %2").arg(KeyName).arg(Selection[0]->id().numId), Selection[0]);
            for (int i=0; i<Selection.size(); ++i)
                if (Selection[i]->findKey(KeyName) != -1)
                    L->add(new ClearTagCommand(Selection[i],KeyName,CUR_DOCUMENT->getDirtyOrOriginLayer(Selection[i]->layer())));
        }
    }
    else
    {
        L = new CommandList(MainWindow::tr("Clear %1 tags on %2").arg(indexes.count()).arg(Selection[0]->id().numId), Selection[0]);
        while (!indexes.isEmpty()) {
            QModelIndex index = indexes.takeLast();
            QModelIndex idx = index.sibling(index.row(),0);
            QVariant Content(theModel->data(idx,Qt::DisplayRole));
            if (Content.isValid())
            {
                QString KeyName = Content.toString();
                for (int i=0; i<Selection.size(); ++i)
                    if (Selection[i]->findKey(KeyName) != -1)
                        L->add(new ClearTagCommand(Selection[i],KeyName,CUR_DOCUMENT->getDirtyOrOriginLayer(Selection[i]->layer())));
            }
        }
    }
    if (!L) return;
    if (L->empty()) {
        delete L;
    } else {
        CUR_DOCUMENT->addHistory(L);
        CUR_MAINWINDOW->invalidateView();
    }
}

void PropertiesDock::on_SourceTagButton_clicked()
{
    QStringList sl = CUR_DOCUMENT->getCurrentSourceTags();
    if (!sl.size())
        return;

    QString src = Selection[0]->tagValue("source", "");
    if (!src.isEmpty())
        sl.prepend(src);

    CommandList* L = new CommandList(MainWindow::tr("Set \"source\" tag on %1").arg(Selection[0]->id().numId), Selection[0]);
    L->add(new SetTagCommand(Selection[0], "source", sl.join(";")));
    CUR_DOCUMENT->addHistory(L);
    CUR_MAINWINDOW->invalidateView();
}

void PropertiesDock::on_SelectionList_customContextMenuRequested(const QPoint & pos)
{
    QListWidgetItem *it = MultiUi.SelectionList->itemAt(pos);
    if (it) {
        QMenu menu(MultiUi.SelectionList);
        menu.addAction(centerAction);
        menu.addAction(centerZoomAction);
        menu.exec(MultiUi.SelectionList->mapToGlobal(pos));
    }
}

void PropertiesDock::on_centerAction_triggered()
{
    CoordBox cb;
    if (CurrentTagView) {
        CUR_MAINWINDOW->setUpdatesEnabled(false);
        int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
        cb = FullSelection[idx]->boundingBox();
        for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
            idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
            cb.merge(FullSelection[idx]->boundingBox());
        }
    }
    Coord c = cb.center();
    CUR_VIEW->setCenter(c, CUR_VIEW->rect());
    CUR_MAINWINDOW->setUpdatesEnabled(true);
    CUR_MAINWINDOW->invalidateView(false);
}

void PropertiesDock::on_centerZoomAction_triggered()
{
    CoordBox cb;
    if (CurrentTagView) {
        CUR_MAINWINDOW->setUpdatesEnabled(false);
        int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
        cb = FullSelection[idx]->boundingBox();
        for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
            idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
            cb.merge(FullSelection[idx]->boundingBox());
        }
        CoordBox mini(cb.center()-COORD_ENLARGE, cb.center()+COORD_ENLARGE);
        cb.merge(mini);
        cb = cb.zoomed(1.1);
    }
    CUR_VIEW->setViewport(cb, CUR_VIEW->rect());
    CUR_MAINWINDOW->setUpdatesEnabled(true);
    CUR_MAINWINDOW->invalidateView(false);
}

bool PropertiesDock::loadTemplates(const QString& filename)
{
    SAFE_DELETE(theTemplates);

    QFile File;
    if (!filename.isEmpty())
        File.setFileName(filename);
    else
        File.setFileName(M_PREFS->getDefaultTemplate());

    if (!File.open(QIODevice::ReadOnly)) {
        qDebug() << tr("Error reading template file");
        return false;
    }

    QDomDocument DomDoc;
    QString ErrorStr;
    int ErrorLine;
    int ErrorColumn;

    if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
    {
        File.close();
        QMessageBox::warning(CUR_MAINWINDOW,"Parse error",
            QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
        return false;
    }

    QDomElement root = DomDoc.documentElement();
    theTemplates = TagTemplates::fromXml(root);
    if (theTemplates) {
        connect(theTemplates, SIGNAL(tagChanged(QString, QString)), this, SLOT(on_tag_changed(QString, QString)));
        connect(theTemplates, SIGNAL(tagCleared(QString)), this, SLOT(on_tag_cleared(QString)));
        connect(theTemplates, SIGNAL(templateChanged(TagTemplate*)), this, SLOT(on_template_changed(TagTemplate*)));
    } else {
        QMessageBox::warning(CUR_MAINWINDOW,"Template read error", "Error parsing template file");
        return false;
    }

    return true;
}

bool PropertiesDock::mergeTemplates(const QString& filename)
{
    QFile File;
    if (!filename.isEmpty())
        File.setFileName(filename);
    else
        return false;

    if (!File.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(CUR_MAINWINDOW,"Template read error", "Error reading template file");
        return false;
    }

    QDomDocument DomDoc;
    QString ErrorStr;
    int ErrorLine;
    int ErrorColumn;

    if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
    {
        File.close();
        QMessageBox::warning(CUR_MAINWINDOW,"Parse error",
            QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
        return false;
    }

    QDomElement root = DomDoc.documentElement();
    if (!theTemplates->mergeXml(root)) {
        QMessageBox::warning(CUR_MAINWINDOW,"Template read error", "Error parsing template file");
        return false;
    }

    return true;
}

bool PropertiesDock::saveTemplates(const QString& filename)
{
    if (!theTemplates)
        return false;

    QDomDocument theXmlDoc;
    theXmlDoc.appendChild(theXmlDoc.createProcessingInstruction("xml", "version=\"1.0\""));

    if (!theTemplates->toXML(theXmlDoc)) {
        QMessageBox::warning(CUR_MAINWINDOW,"Tag templates write error", "Unable to generate XML document");
        return false;
    }

    QFile File(filename);
    if (!File.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(CUR_MAINWINDOW,"Tag templates write error", "Error opening template file for writing");
        return false;
    }
    File.write(theXmlDoc.toString().toUtf8());

    return true;
}

void PropertiesDock::on_template_changed(TagTemplate* /* aNewTemplate */)
{
    resetValues();
}

void PropertiesDock::changeEvent(QEvent * event)
{
    if (event->type() == QEvent::LanguageChange)
        retranslateUi();
    MDockAncestor::changeEvent(event);
}

void PropertiesDock::retranslateUi()
{
    setWindowTitle(tr("Properties"));
    centerAction->setText(tr("Center map"));
    centerZoomAction->setText(tr("Center && Zoom map"));
}

int PropertiesDock::highlightedSize() const
{
    if (!isVisible())
        return 0;
    return Highlighted.size();
}

Feature* PropertiesDock::highlighted(int idx)
{
    if (idx < Highlighted.size())
        return Highlighted[idx];
    return 0;
}

QList<Feature*> PropertiesDock::highlighted()
{
    return Highlighted;
}

