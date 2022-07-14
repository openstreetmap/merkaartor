#include "PropertiesDock.h"
#include "InfoDock.h"
#include "MainWindow.h"
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

#include <QTimer>
#include <QHeaderView>
#include <QLineEdit>
#include <QListWidget>
#include <QTableView>
#include <QClipboard>
#include <QMessageBox>
#include <QMenu>

#include <algorithm>

PropertiesDock::PropertiesDock(MainWindow* aParent)
: MDockAncestor(aParent), Main(aParent), CurrentUi(0),
    theTemplates(0), CurrentTagView(0), CurrentMembersView(0), NowShowing(NoUiShowing)
{
    setMinimumSize(220,100);
    switchToNoUi();
    setObjectName("propertiesDock");
    theModel = new TagModel(aParent);
    delegate = new EditCompleterDelegate(aParent);

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
    selectAction = new QAction(NULL, this);
    connect(selectAction, SIGNAL(triggered()), this, SLOT(on_Member_selected()));

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
    Q_UNUSED(IsParentRoad);
    bool IsParentRoadInner = false;
    bool IsParentRelation = false;
    bool IsParentArea = false;
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
    Main->ui->createRelationAction->setEnabled(Selection.size());
    Main->ui->editRemoveAction->setEnabled(Selection.size());
    Main->ui->editMoveAction->setEnabled(true);
    Main->ui->editReverseAction->setEnabled(IsRoad || NumAreas + NumRoads > 0);
    Main->ui->roadAddStreetNumbersAction->setEnabled(NumRoads >= 1);
    Main->ui->roadJoinAction->setEnabled(NumRoads >= 1 && canJoinRoads(this));
    Main->ui->roadCreateJunctionAction->setEnabled(NumRoads > 1 && canCreateJunction(this));
    Main->ui->roadSplitAction->setEnabled((IsParentRoadInner && !IsParentArea) || (NumRoads && NumPoints) || (NumAreas && NumPoints > 1));
    Main->ui->roadBreakAction->setEnabled(IsParentRoadInner || ((NumRoads == 1 || NumAreas == 1) && NumPoints) || (NumRoads > 1 && canBreakRoads(this)));
    Main->ui->roadSimplifyAction->setEnabled(IsRoad || NumRoads > 0 || NumAreas > 0);
    Main->ui->roadSubdivideAction->setEnabled((NumRoads + NumAreas) == 1 && (!NumPoints || NumPoints == 2) && canSubdivideRoad(this));
    Main->ui->roadAxisAlignAction->setEnabled((NumRoads + NumAreas) > 0 || (NumRelation > 0 && canAxisAlignRoads(this)));
    Main->ui->areaJoinAction->setEnabled(NumAreas > 1);
    Main->ui->areaSplitAction->setEnabled(NumAreas == 1 && NumPoints == 2 && canSplitArea(this));
    Main->ui->areaTerraceAction->setEnabled(NumAreas == 1 && NumRoads == 0 && canTerraceArea(this));
    Main->ui->featureSelectChildrenAction->setEnabled(NumChildren);
    Main->ui->featureSelectParentsAction->setEnabled(NumParents);
    Main->ui->featureDownloadMissingChildrenAction->setEnabled(NumIncomplete);
    Main->ui->featureDeleteAction->setEnabled((IsPoint || IsRoad || IsRelation) && !Selection[0]->isDirty());
    Main->ui->featureCommitAction->setEnabled(NumCommitableFeature);
    Main->ui->nodeMergeAction->setEnabled(NumPoints > 1);
    Main->ui->nodeAlignAction->setEnabled(NumPoints > 2);
    Main->ui->nodeSpreadAction->setEnabled(NumPoints > 2);
    Main->ui->nodeDetachAction->setEnabled(NumPoints && canDetachNodes(this));
    Main->ui->relationAddMemberAction->setEnabled(NumRelation && Selection.size() > 1);
    Main->ui->relationRemoveMemberAction->setEnabled((NumRelation && Selection.size() > 1 && NumRelationChild) || IsParentRelation);
    Main->ui->relationAddToMultipolygonAction->setEnabled((NumAreas > 1) || (NumAreas >0 && NumRelation == 1));

    Main->ui->editCopyAction->setEnabled(Selection.size());
    Main->clipboardChanged();
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
    emit selectionChanged();
}

void PropertiesDock::setMultiSelection(const QList<Feature*>& aFeatureList)
{
    cleanUpUi();
    Selection.clear();
    for (int i=0; i<aFeatureList.size(); ++i)
        Selection.push_back(aFeatureList[i]);
    FullSelection = Selection;
    switchToMultiUi();
    // to prevent slots to change the values also
    QList<Feature*> Current = Selection;
    Selection.clear();
    MultiUi.TagView->setModel(theModel);
    MultiUi.TagView->setItemDelegate(delegate);
    Main->info()->setHtml("");
    #ifdef GEOIMAGE
    Main->geoImage()->setImage((Node *)NULL);
    #endif
    CurrentTagView = MultiUi.TagView;
    theModel->setFeature(Current);
    Selection = Current;
    fillMultiUiSelectionBox();
    emit selectionChanged();
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
    emit selectionChanged();
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
    emit selectionChanged();
}

void PropertiesDock::adjustSelection()
{
    QList<Feature*> aSelection;
    int cnt = Selection.size();

    for (int i=0; i<FullSelection.size(); ++i) {
        if (Main->document()->exists(FullSelection[i]) && FullSelection[i] && !FullSelection[i]->isDeleted()) {
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
    emit selectionChanged();
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
        Main->setUpdatesEnabled(false);
        MultiUi.SelectionList->clear();
        for (int i=0; i<FullSelection.size(); ++i)
        {
            QListWidgetItem* it = new QListWidgetItem(FullSelection[i]->description(),MultiUi.SelectionList);
            it->setData(Qt::UserRole,QVariant(i));
            it->setSelected(true);
        }
        MultiUi.lbStatus->setText(tr("%1/%1 selected item(s)").arg(FullSelection.size()));
        Main->setUpdatesEnabled(true);
        NowShowing = MultiShowing;
    }
}

void PropertiesDock::on_SelectionList_itemSelectionChanged()
{
    if (NowShowing == MultiShowing)
    {
        Selection.clear();
        for (int i=0; i<FullSelection.size(); ++i)
            if (MultiUi.SelectionList->item(i)->isSelected())
                Selection.push_back(FullSelection[i]);
        if (Selection.size() == 1) {
            Main->info()->setHtml(Selection[0]->toHtml());

            #ifdef GEOIMAGE
            Node *Pt;
            if ((Pt = dynamic_cast<Node*>(Selection[0]))) Main->geoImage()->setImage(Pt);
            #endif
        }
        theModel->setFeature(Selection);
        MultiUi.lbStatus->setText(tr("%1/%2 selected item(s)").arg(Selection.size()).arg(FullSelection.size()));
        Main->view()->update();
        emit selectionChanged();
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
    if (NowShowing == RelationUiShowing)
    {
        RelationUi.MembersView->setModel(0);
        Relation* R = dynamic_cast<Relation*>(FullSelection[0]);
        R->releaseMemberModel();
    }
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
        else if (CAST_WAY(FullSelection[0]))
            switchToWayUi();
        else if (CAST_RELATION(FullSelection[0]))
            switchToRelationUi();
        else
            switchToNoUi();
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

void PropertiesDock::switchToWayUi()
{
    NowShowing = RoadUiShowing;
    QWidget* NewUi = new QWidget(this);
    RoadUi.setupUi(NewUi);
    RoadUi.TagView->verticalHeader()->hide();
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    connect(RoadUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
    connect(RoadUi.SourceTagButton,SIGNAL(clicked()),this, SLOT(on_SourceTagButton_clicked()));
    setWindowTitle(tr("Properties - Way"));
}

void PropertiesDock::switchToRelationUi()
{
    NowShowing = RelationUiShowing;
    QWidget* NewUi = new QWidget(this);
    RelationUi.setupUi(NewUi);
    RelationUi.TagView->verticalHeader()->hide();
    setWidget(NewUi);
    if (CurrentUi)
        CurrentUi->deleteLater();
    CurrentUi = NewUi;
    RelationUi.MembersView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(RelationUi.MembersView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_Member_customContextMenuRequested(const QPoint &)));
    connect(RelationUi.MembersView, SIGNAL(clicked(QModelIndex)), this, SLOT(on_Member_clicked(QModelIndex)));
    connect(RelationUi.RemoveMemberButton,SIGNAL(clicked()),this, SLOT(on_RemoveMemberButton_clicked()));
    connect(RelationUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
    connect(RelationUi.SourceTagButton,SIGNAL(clicked()),this, SLOT(on_SourceTagButton_clicked()));
    connect(RelationUi.btMemberUp, SIGNAL(clicked()), SLOT(on_btMemberUp_clicked()));
    connect(RelationUi.btMemberDown, SIGNAL(clicked()), SLOT(on_btMemberDown_clicked()));
    setWindowTitle(tr("Properties - Relation"));
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
    CurrentMembersView = NULL;

    // to prevent slots to change the values also
    QList<Feature*> Current = Selection;
    Selection.clear();
    if (FullSelection.size() == 1)
    {
        Main->info()->setHtml(FullSelection[0]->toHtml());

        Node* Pt = dynamic_cast<Node*>(FullSelection[0]);
        Way* R = dynamic_cast<Way*>(FullSelection[0]);
        Relation* L = dynamic_cast<Relation*>(FullSelection[0]);

        if ((Pt) && (NowShowing == TrackPointUiShowing))
        {
            TrackPointUi.Id->setText(QString::number(Pt->id().numId));
            TrackPointUi.Latitude->setText(COORD2STRING(Pt->position().y()));
            TrackPointUi.Longitude->setText(COORD2STRING(Pt->position().x()));
            TrackPointUi.TagView->setModel(theModel);
            TrackPointUi.TagView->setItemDelegate(delegate);

            QWidget* w;
            for (int i=0; i<TrackPointUi.variableLayout->count(); ++i) {
                w = TrackPointUi.variableLayout->itemAt(i)->widget();
                if (w) {
                    w->hide();
                    w->deleteLater();
                }
            }
            if (theTemplates) {
                w = theTemplates->getWidget(Pt, Main->view());
                w->installEventFilter(shortcutFilter);
                TrackPointUi.variableLayout->addWidget(w);
            }

            CurrentTagView = TrackPointUi.TagView;

            #ifdef GEOIMAGE
            Main->geoImage()->setImage(Pt);
            #endif
        }
        else if ((R) && (NowShowing == RoadUiShowing))
        {
            RoadUi.Id->setText(QString::number(R->id().numId));
            //RoadUi.Name->setText(R->tagValue("name",""));
            RoadUi.TagView->setModel(theModel);
            RoadUi.TagView->setItemDelegate(delegate);

            QWidget* w;
            for (int i=0; i<RoadUi.variableLayout->count(); ++i) {
                w = RoadUi.variableLayout->itemAt(i)->widget();
                if (w) {
                    w->hide();
                    w->deleteLater();
                }
            }
            if (theTemplates) {
                w = theTemplates->getWidget(R, Main->view());
                w->installEventFilter(shortcutFilter);
                RoadUi.variableLayout->addWidget(w);
            }

            CurrentTagView = RoadUi.TagView;
        }
        else if ((L) && (NowShowing == RelationUiShowing))
        {
            RelationUi.MembersView->setModel(L->referenceMemberModel(Main));
            RelationUi.TagView->setModel(theModel);
            RelationUi.TagView->setItemDelegate(delegate);

            QWidget* w;
            for (int i=0; i<RelationUi.variableLayout->count(); ++i) {
                w = RelationUi.variableLayout->itemAt(i)->widget();
                if (w) {
                    w->hide();
                    w->deleteLater();
                }
            }
            if (theTemplates) {
                w = theTemplates->getWidget(L, Main->view());
                w->installEventFilter(shortcutFilter);
                RelationUi.variableLayout->addWidget(w);
            }

            CurrentTagView     = RelationUi.TagView;
            CurrentMembersView = RelationUi.MembersView;
        }

        if (theTemplates)
            theTemplates->apply(FullSelection[0]);
    }
    else if ((FullSelection.size() > 1)  && (NowShowing == MultiShowing))
    {
        Main->info()->setHtml("");
        #ifdef GEOIMAGE
        Main->geoImage()->setImage((Node *)NULL);
        #endif
        MultiUi.TagView->setModel(theModel);
        MultiUi.TagView->setItemDelegate(delegate);
        CurrentTagView = MultiUi.TagView;
    }
    theModel->setFeature(Current);
    Selection = Current;
    checkMenuStatus();
    emit selectionChanged();

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
                0, CurrentTagView->fontMetrics().horizontalAdvance(theModel->newKeyText())+10
            );
        CurrentTagView->horizontalHeader()->setStretchLastSection(true);
        CurrentTagView->installEventFilter(shortcutFilter);
    }
    if (CurrentMembersView) {
        CurrentMembersView->setColumnWidth(
            0, CurrentMembersView->fontMetrics().horizontalAdvance(theModel->newKeyText())+10
        );
        CurrentMembersView->horizontalHeader()->setStretchLastSection(true);
        CurrentMembersView->installEventFilter(shortcutFilter);
    }
}

void PropertiesDock::on_TrackPointLat_editingFinished()
{
    if (TrackPointUi.Latitude->text().isEmpty()) return;
    Node* Pt = dynamic_cast<Node*>(selection(0));
    if (Pt)
    {
        Main->document()->addHistory(
            new MoveNodeCommand(Pt,
                Coord(Pt->position().x(), TrackPointUi.Latitude->text().toDouble()), Main->document()->getDirtyOrOriginLayer(Pt->layer()) ));
        Main->invalidateView(false);
    }
}

void PropertiesDock::on_TrackPointLon_editingFinished()
{
    if (TrackPointUi.Longitude->text().isEmpty()) return;
    Node* Pt = dynamic_cast<Node*>(selection(0));
    if (Pt)
    {
        Main->document()->addHistory(
            new MoveNodeCommand(Pt,
                Coord(TrackPointUi.Longitude->text().toDouble(), Pt->position().y()), Main->document()->getDirtyOrOriginLayer(Pt->layer()) ));
        Main->invalidateView(false);
    }
}

void PropertiesDock::on_tag_changed(QString k, QString v)
{
    if (!FullSelection.size())
        return;
    Feature* F = FullSelection[0];
    if (F->tagValue(k, "__NULL__") != v) {
        Main->document()->addHistory(new SetTagCommand(F,k,v,Main->document()->getDirtyOrOriginLayer(F->layer())));
        Main->invalidateView();
    }
}

void PropertiesDock::on_tag_cleared(QString k)
{
    if (!FullSelection.size())
        return;
    Feature* F = FullSelection[0];
    Main->document()->addHistory(new ClearTagCommand(F,k,Main->document()->getDirtyOrOriginLayer(F->layer())));
    Main->invalidateView();
}

void PropertiesDock::on_RemoveTagButton_clicked()
{
    QTableView* TagTable = 0;
    switch (NowShowing)
    {
    case TrackPointUiShowing:
        TagTable = TrackPointUi.TagView; break;
    case RoadUiShowing:
        TagTable = RoadUi.TagView; break;
    case MultiShowing:
        TagTable = MultiUi.TagView; break;
    case RelationUiShowing:
        TagTable = RelationUi.TagView; break;
    default: break;
    }
    if (!TagTable) return;

    QModelIndexList indexes = TagTable->selectionModel()->selectedIndexes();
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
                    L->add(new ClearTagCommand(Selection[i],KeyName,Main->document()->getDirtyOrOriginLayer(Selection[i]->layer())));
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
                        L->add(new ClearTagCommand(Selection[i],KeyName,Main->document()->getDirtyOrOriginLayer(Selection[i]->layer())));
            }
        }
    }
    if (!L) return;
    if (L->empty()) {
        delete L;
    } else {
        Main->document()->addHistory(L);
        Main->invalidateView();
    }
}

void PropertiesDock::on_SourceTagButton_clicked()
{
    QStringList sl = Main->document()->getCurrentSourceTags();
    if (!sl.size())
        return;

    QString src = Selection[0]->tagValue("source", "");
    if (!src.isEmpty())
        sl.prepend(src);

    CommandList* L = new CommandList(MainWindow::tr("Set \"source\" tag on %1").arg(Selection[0]->id().numId), Selection[0]);
    L->add(new SetTagCommand(Selection[0], "source", sl.join(";")));
    Main->document()->addHistory(L);
    Main->invalidateView();
}

void PropertiesDock::on_RemoveMemberButton_clicked()
{
    if (CurrentMembersView)
    {
        Relation* R = dynamic_cast<Relation*>(Selection[0]);
        if (R) {
            QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedIndexes();
            QModelIndex index;

            foreach(index, indexes)
            {
                QModelIndex idx = index.sibling(index.row(),0);
                QVariant Content(R->referenceMemberModel(Main)->data(idx,Qt::UserRole));
                if (Content.isValid())
                {
                    Feature* F = Content.value<Feature*>();
                    if (F) {
                        CommandList* L = new CommandList(MainWindow::tr("Remove member '%1' on %2").arg(F->description()).arg(R->description()), R);
                        if (R->find(F) < R->size())
                            L->add(new RelationRemoveFeatureCommand(R,F,Main->document()->getDirtyOrOriginLayer(R->layer())));
                        if (L->empty())
                            delete L;
                        else
                        {
                            Main->document()->addHistory(L);
                            Main->invalidateView();
                            return;
                        }
                    }
                }
            }
        }
    }
}

void PropertiesDock::on_Member_customContextMenuRequested(const QPoint & pos)
{
    QModelIndex ix = CurrentMembersView->indexAt(pos);
    if (ix.isValid()) {
        QMenu menu(CurrentMembersView);
        menu.addAction(centerAction);
        menu.addAction(centerZoomAction);
        menu.addAction(selectAction);
        menu.exec(CurrentMembersView->mapToGlobal(pos));
    }
}

void PropertiesDock::on_Member_clicked(const QModelIndex & index)
{
    Highlighted.clear();

    Relation* R = dynamic_cast<Relation*>(Selection[0]);
    if (R) {
        QVariant Content(R->referenceMemberModel(Main)->data(index,Qt::UserRole));
        if (Content.isValid())
        {
            Feature* F = Content.value<Feature*>();
            if (F)
                Highlighted.push_back(F);
        }
    }
    Main->view()->update();
}

void PropertiesDock::on_Member_selected()
{
    QList<Feature*> Features;
    Relation* R = dynamic_cast<Relation*>(Selection[0]);
    if (R) {
        QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedIndexes();
        QModelIndex index;

        foreach(index, indexes)
        {
            QModelIndex idx = index.sibling(index.row(),0);
            QVariant Content(R->referenceMemberModel(Main)->data(idx,Qt::UserRole));
            if (Content.isValid())
            {
                Feature* F = Content.value<Feature*>();
                if (F) {
                    Features.append(F);
                }
            }
        }
        if (!Features.isEmpty())
            setMultiSelection(Features);
    }
    Main->invalidateView(false);
}

void PropertiesDock::on_btMemberUp_clicked()
{
    Relation* R = dynamic_cast<Relation*>(Selection[0]);
    if (R) {
        CommandList* theList = new CommandList(MainWindow::tr("Reorder members in relation %1").arg(R->id().numId), R);

        QModelIndex index;
        foreach(index, CurrentMembersView->selectionModel()->selectedIndexes())
        {
            CurrentMembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedRows(0);
        QModelIndexList newSel;
        foreach(index, indexes)
        {
            QVariant Content(R->referenceMemberModel(Main)->data(index,Qt::UserRole));
            if (Content.isValid())
            {
                Feature* F = Content.value<Feature*>();
                if (F) {
                    int pos = R->find(F);
                    if (!pos)
                        break;
                    QString role = R->getRole(pos);
                    theList->add(new RelationRemoveFeatureCommand(R, pos, Main->document()->getDirtyOrOriginLayer(R->layer())));
                    theList->add(new RelationAddFeatureCommand(R, role, F, pos-1, Main->document()->getDirtyOrOriginLayer(R->layer())));
                    newSel.append(CurrentMembersView->model()->index(pos-1, 0));
                }
            }
        }
        CurrentMembersView->selectionModel()->clearSelection();
        foreach(index, newSel)
        {
            CurrentMembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        if (theList->empty())
            delete theList;
        else
        {
            Main->document()->addHistory(theList);
            Main->invalidateView();
        }
    }
}

void PropertiesDock::on_btMemberDown_clicked()
{
    Relation* R = dynamic_cast<Relation*>(Selection[0]);
    if (R) {
        CommandList* theList = new CommandList(MainWindow::tr("Reorder members in relation %1").arg(R->id().numId), R);

        QModelIndex index;
        foreach(index, CurrentMembersView->selectionModel()->selectedIndexes())
        {
            CurrentMembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedRows(0);
        QModelIndexList newSel;
        // We need to iterate backwards so that earlier moves don't corrupt the inputs to later ones
        for (int i = indexes.count()-1;  i >= 0;  i--)
        {
            index = indexes[i];
            QVariant Content(R->referenceMemberModel(Main)->data(index,Qt::UserRole));
            if (Content.isValid())
            {
                Feature* F = Content.value<Feature*>();
                if (F) {
                    int pos = R->find(F);
                    if (pos >= R->size()-1)
                        break;
                    QString role = R->getRole(pos);
                    theList->add(new RelationRemoveFeatureCommand(R, pos, Main->document()->getDirtyOrOriginLayer(R->layer())));
                    theList->add(new RelationAddFeatureCommand(R, role, F, pos+1, Main->document()->getDirtyOrOriginLayer(R->layer())));
                    newSel.append(CurrentMembersView->model()->index(pos+1, 0));
                }
            }
        }
        CurrentMembersView->selectionModel()->clearSelection();
        foreach(index, newSel)
        {
            CurrentMembersView->selectionModel()->select(index, QItemSelectionModel::Select | QItemSelectionModel::Rows);
        }
        if (theList->empty())
            delete theList;
        else
        {
            Main->document()->addHistory(theList);
            Main->invalidateView();
        }
    }
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
    if (CurrentMembersView)
    {
        Relation* R = dynamic_cast<Relation*>(Selection[0]);
        if (R) {
            QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedIndexes();
            QModelIndex index;

            foreach(index, indexes)
            {
                QModelIndex idx = index.sibling(index.row(),0);
                QVariant Content(R->referenceMemberModel(Main)->data(idx,Qt::UserRole));
                if (Content.isValid())
                {
                    Feature* F = Content.value<Feature*>();
                    if (F) {
                        //setSelection(F);
                        cb = F->boundingBox();
                    }
                }
            }
        }
    } else
    if (CurrentTagView) {
        Main->setUpdatesEnabled(false);
        int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
        cb = FullSelection[idx]->boundingBox();
        for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
            idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
            cb.merge(FullSelection[idx]->boundingBox());
        }
    }
    Coord c = cb.center();
    Main->view()->setCenter(c, Main->view()->rect());
    Main->setUpdatesEnabled(true);
    Main->invalidateView(false);
}

void PropertiesDock::on_centerZoomAction_triggered()
{
    CoordBox cb;
    if (CurrentMembersView)
    {
        Relation* R = dynamic_cast<Relation*>(Selection[0]);
        if (R) {
            QModelIndexList indexes = CurrentMembersView->selectionModel()->selectedIndexes();
            QModelIndex index;

            foreach(index, indexes)
            {
                QModelIndex idx = index.sibling(index.row(),0);
                QVariant Content(R->referenceMemberModel(Main)->data(idx,Qt::UserRole));
                if (Content.isValid())
                {
                    Feature* F = Content.value<Feature*>();
                    if (F) {
                        //setSelection(F);
                        cb = F->boundingBox();
                        CoordBox mini(cb.center()-COORD_ENLARGE, cb.center()+COORD_ENLARGE);
                        cb.merge(mini);
                        cb = cb.zoomed(1.1);
                    }
                }
            }
        }
    } else
    if (CurrentTagView) {
        Main->setUpdatesEnabled(false);
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
    Main->view()->setViewport(cb, Main->view()->rect());
    Main->setUpdatesEnabled(true);
    Main->invalidateView(false);
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
        QMessageBox::warning(Main,"Parse error",
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
        QMessageBox::warning(Main,"Template read error", "Error parsing template file");
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
        QMessageBox::warning(Main,"Template read error", "Error reading template file");
        return false;
    }

    QDomDocument DomDoc;
    QString ErrorStr;
    int ErrorLine;
    int ErrorColumn;

    if (!DomDoc.setContent(&File, true, &ErrorStr, &ErrorLine,&ErrorColumn))
    {
        File.close();
        QMessageBox::warning(Main,"Parse error",
            QString("Parse error at line %1, column %2:\n%3")
                                  .arg(ErrorLine)
                                  .arg(ErrorColumn)
                                  .arg(ErrorStr));
        return false;
    }

    QDomElement root = DomDoc.documentElement();
    if (!theTemplates->mergeXml(root)) {
        QMessageBox::warning(Main,"Template read error", "Error parsing template file");
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
        QMessageBox::warning(Main,"Tag templates write error", "Unable to generate XML document");
        return false;
    }

    QFile File(filename);
    if (!File.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(Main,"Tag templates write error", "Error opening template file for writing");
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
    selectAction->setText(tr("Select member"));
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

