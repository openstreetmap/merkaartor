#include "PropertiesDock.h"
#include "InfoDock.h"
#include "MainWindow.h"
#include "MapView.h"
#include "TagModel.h"
#include "Utils/EditCompleterDelegate.h"
#include "Utils/ShortcutOverrideFilter.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/TrackPointCommands.h"
#include "Command/RelationCommands.h"
#include "Map/Coord.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/FeatureManipulations.h"
#include "Map/TrackPoint.h"
#include "TagTemplate/TagTemplate.h"

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

#include <algorithm>

PropertiesDock::PropertiesDock(MainWindow* aParent)
: MDockAncestor(aParent), Main(aParent), CurrentUi(0), Selection(0),
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

	loadTemplates();

	retranslateUi();
}

PropertiesDock::~PropertiesDock(void)
{
	delete theModel;
	delete theTemplates;
	delete shortcutFilter;
}

static bool isChildOfSingleRoadInner(MapFeature *mapFeature)
{
	return Road::GetSingleParentRoadInner(mapFeature) != NULL;
}

static bool isChildOfArea(MapFeature *mapFeature)
{
	Road* R =  Road::GetSingleParentRoadInner(mapFeature);
	if (R)
		return (R->area() > 0.0);
	return false;
}

static bool isChildOfSingleRoad(MapFeature *mapFeature)
{
	return Road::GetSingleParentRoad(mapFeature) != NULL;
}

static bool isChildOfSingleRelation(MapFeature *mapFeature)
{
	unsigned int parents = mapFeature->sizeParents();

	if (parents == 0)
		return false;

	unsigned int parentRelations = 0;

	unsigned int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		bool isParentRelation = dynamic_cast<Relation*>(parent) != 0;
		if (isParentRelation)
			parentRelations++;
			if (parentRelations > 1)
				return false;
	}

	return (parentRelations == 1);
}

static bool isChildOfRelation(MapFeature *mapFeature)
{
	unsigned int parents = mapFeature->sizeParents();

	if (parents == 0)
		return false;

	unsigned int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		if (dynamic_cast<Relation*>(parent))
			return true;
	}

	return false;
}

void PropertiesDock::checkMenuStatus()
{
	bool IsPoint = false;
	bool IsRoad = false;
	bool IsParentRoad = false;
	bool IsParentRoadInner = false;
	bool IsParentRelation = false;
	bool IsParentArea = false;
	unsigned int NumRoads = 0;
	unsigned int NumCommitableFeature = 0;
	unsigned int NumPoints = 0;
	unsigned int NumRelation = 0;
	unsigned int NumRelationChild = 0;
	unsigned int NumAreas = 0;
	if (Selection.size() == 1)
	{
		IsPoint = dynamic_cast<TrackPoint*>(Selection[0]) != 0;
		IsRoad = dynamic_cast<Road*>(Selection[0]) != 0;
		IsParentRoad = IsPoint && isChildOfSingleRoad(Selection[0]);
		IsParentRoadInner = IsPoint && isChildOfSingleRoadInner(Selection[0]);
		IsParentRelation = isChildOfSingleRelation(Selection[0]);
		IsParentArea = isChildOfArea(Selection[0]);
	}
	for (unsigned int i=0; i<Selection.size(); ++i)
	{
		if (CAST_NODE(Selection[i]))
			++NumPoints;
		if (Road* R = dynamic_cast<Road*>(Selection[i]))
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

		if (Selection[i]->layer() && !Selection[i]->layer()->isUploadable())
			++NumCommitableFeature;
	}
	Main->createRelationAction->setEnabled(Selection.size());
	Main->editRemoveAction->setEnabled(Selection.size());
	Main->editMoveAction->setEnabled(true);
	Main->editReverseAction->setEnabled(IsRoad);
	Main->roadJoinAction->setEnabled(NumRoads > 1 && canJoinRoads(this));
	Main->roadSplitAction->setEnabled((IsParentRoadInner && !IsParentArea) || (NumRoads && NumPoints) || (NumAreas && NumPoints));
	Main->roadBreakAction->setEnabled(IsParentRoadInner || (NumRoads == 1 && NumPoints) || (NumRoads > 1 && canBreakRoads(this)));
	Main->featureCommitAction->setEnabled(NumCommitableFeature);
	Main->nodeMergeAction->setEnabled(NumPoints > 1);
	Main->nodeAlignAction->setEnabled(NumPoints > 2);
	Main->nodeDetachAction->setEnabled(NumPoints && canDetachNodes(this));
	Main->relationAddMemberAction->setEnabled(NumRelation && Selection.size() > 1);
	Main->relationRemoveMemberAction->setEnabled((NumRelation && Selection.size() > 1 && NumRelationChild) || IsParentRelation);

	Main->editCopyAction->setEnabled(Selection.size());
	Main->clipboardChanged();
}

unsigned int PropertiesDock::size() const
{
	return Selection.size();
}

MapFeature* PropertiesDock::selection(unsigned int idx)
{
	if (idx < Selection.size())
		return Selection[idx];
	return 0;
}

QVector<MapFeature*> PropertiesDock::selection()
{
	return QVector<MapFeature*>::fromStdVector(Selection);
}

void PropertiesDock::setSelection(MapFeature*aFeature)
{
	cleanUpUi();
	Selection.clear();
	if (aFeature)
		Selection.push_back(aFeature);
	FullSelection = Selection;
	switchUi();
	fillMultiUiSelectionBox();
}

void PropertiesDock::setMultiSelection(const std::vector<MapFeature*>& aFeatureList)
{
	cleanUpUi();
	Selection.clear();
	for (unsigned int i=0; i<aFeatureList.size(); ++i)
		Selection.push_back(aFeatureList[i]);
	FullSelection = Selection;
	switchToMultiUi();
	// to prevent slots to change the values also
	std::vector<MapFeature*> Current = Selection;
	Selection.clear();
	MultiUi.TagView->setModel(theModel);
	MultiUi.TagView->setItemDelegate(delegate);
	theModel->setFeature(Current);
	Selection = Current;
	fillMultiUiSelectionBox();
}

void PropertiesDock::toggleSelection(MapFeature* S)
{
	cleanUpUi();
	std::vector<MapFeature*>::iterator i = std::find(Selection.begin(),Selection.end(),S);
	if (i == Selection.end())
		Selection.push_back(S);
	else
		Selection.erase(i);
	FullSelection = Selection;
	switchUi();
	fillMultiUiSelectionBox();
}

void PropertiesDock::addSelection(MapFeature* S)
{
	cleanUpUi();
	std::vector<MapFeature*>::iterator i = std::find(Selection.begin(),Selection.end(),S);
	if (i == Selection.end())
		Selection.push_back(S);
	FullSelection = Selection;
	switchUi();
	fillMultiUiSelectionBox();
}

void PropertiesDock::adjustSelection()
{
	QVector<MapFeature*> aSelection;
	unsigned int cnt = Selection.size();

	for (unsigned int i=0; i<FullSelection.size(); ++i) {
		if (Main->document()->exists(FullSelection[i])) {
			aSelection.push_back(FullSelection[i]);
		} else {
			std::vector<MapFeature*>::iterator it = std::find(Selection.begin(),Selection.end(),FullSelection[i]);
			if (it != Selection.end())
				Selection.erase(it);
		}
	}

	FullSelection = aSelection.toStdVector();
	if (Selection.size() != cnt)
		switchUi();
}

bool PropertiesDock::isSelected(MapFeature *aFeature)
{
	std::vector<MapFeature*>::iterator i = std::find(Selection.begin(),Selection.end(),aFeature);
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
		for (unsigned int i=0; i<FullSelection.size(); ++i)
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
		for (unsigned int i=0; i<FullSelection.size(); ++i)
			if (MultiUi.SelectionList->item(i)->isSelected())
				Selection.push_back(FullSelection[i]);
		if (Selection.size() == 1) {
			Main->info()->setHtml(Selection[0]->toHtml());

			#ifdef GEOIMAGE
			TrackPoint *Pt;
			if ((Pt = dynamic_cast<TrackPoint*>(Selection[0]))) Main->geoImage()->setImage(Pt->getImageId());
			#endif
		}
		theModel->setFeature(Selection);
		MultiUi.lbStatus->setText(tr("%1/%2 selected item(s)").arg(Selection.size()).arg(FullSelection.size()));
		Main->view()->update();
	}
}

void PropertiesDock::on_SelectionList_itemDoubleClicked(QListWidgetItem* item)
{
	unsigned int i=item->data(Qt::UserRole).toUInt();
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
        M_PREFS->setTagListFirstColumnWidth(CurrentTagView->columnWidth(0));

    if (FullSelection.size() == 0)
		switchToNoUi();
	else if (FullSelection.size() == 1)
	{
		if (dynamic_cast<TrackPoint*>(FullSelection[0]))
			switchToTrackPointUi(FullSelection[0]);
		else if (dynamic_cast<Road*>(FullSelection[0]))
			switchToRoadUi(FullSelection[0]);
		else if (dynamic_cast<Relation*>(FullSelection[0]))
			switchToRelationUi(FullSelection[0]);
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

void PropertiesDock::switchToTrackPointUi(MapFeature* F)
{
	NowShowing = TrackPointUiShowing;
	QWidget* NewUi = new QWidget(this);
	TrackPointUi.setupUi(NewUi);
	if (theTemplates)
		TrackPointUi.variableLayout->addWidget(theTemplates->getWidget(F));
	TrackPointUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		CurrentUi->deleteLater();
	CurrentUi = NewUi;
	connect(TrackPointUi.Longitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLon_editingFinished()));
	connect(TrackPointUi.Latitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLat_editingFinished()));
	connect(TrackPointUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Trackpoint"));
}

void PropertiesDock::switchToRoadUi(MapFeature* F)
{
	NowShowing = RoadUiShowing;
	QWidget* NewUi = new QWidget(this);
	RoadUi.setupUi(NewUi);
	if (theTemplates)
		RoadUi.variableLayout->addWidget(theTemplates->getWidget(F));
	RoadUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		CurrentUi->deleteLater();
	CurrentUi = NewUi;
	connect(RoadUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Road"));
}

void PropertiesDock::switchToRelationUi(MapFeature* F)
{
	NowShowing = RelationUiShowing;
	QWidget* NewUi = new QWidget(this);
	RelationUi.setupUi(NewUi);
	if (theTemplates)
		RelationUi.variableLayout->addWidget(theTemplates->getWidget(F));
	RelationUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		CurrentUi->deleteLater();
	CurrentUi = NewUi;
	RelationUi.MembersView->setContextMenuPolicy(Qt::CustomContextMenu);
	connect(RelationUi.MembersView, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_Member_customContextMenuRequested(const QPoint &)));
	connect(RelationUi.RemoveMemberButton,SIGNAL(clicked()),this, SLOT(on_RemoveMemberButton_clicked()));
	connect(RelationUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
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
	// Tables that might need column sizing
    CurrentTagView = NULL;
    CurrentMembersView = NULL;

	// to prevent slots to change the values also
	std::vector<MapFeature*> Current = Selection;
	Selection.clear();
	if (FullSelection.size() == 1)
	{
		Main->info()->setHtml(FullSelection[0]->toHtml());

		if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(FullSelection[0]))
		{
			TrackPointUi.Id->setText(Pt->id());
			TrackPointUi.Latitude->setText(QString::number(intToAng(Pt->position().lat()),'g',8));
			TrackPointUi.Longitude->setText(QString::number(intToAng(Pt->position().lon()),'g',8));
			TrackPointUi.TagView->setModel(theModel);
			TrackPointUi.TagView->setItemDelegate(delegate);

			QWidget* w = TrackPointUi.variableLayout->takeAt(0)->widget();
			w->deleteLater();
			if (theTemplates)
				TrackPointUi.variableLayout->addWidget(theTemplates->getWidget(Pt));

			CurrentTagView = TrackPointUi.TagView;
 
 			#ifdef GEOIMAGE
 			Main->geoImage()->setImage(Pt->getImageId());
 			#endif
		}
		else if (Road* R = dynamic_cast<Road*>(FullSelection[0]))
		{
			RoadUi.Id->setText(R->id());
			//RoadUi.Name->setText(R->tagValue("name",""));
			RoadUi.TagView->setModel(theModel);
			RoadUi.TagView->setItemDelegate(delegate);

			QWidget* w = RoadUi.variableLayout->takeAt(0)->widget();
			w->deleteLater();
			if (theTemplates)
				RoadUi.variableLayout->addWidget(theTemplates->getWidget(R));

			CurrentTagView = RoadUi.TagView;
		}
		else if (Relation* R = dynamic_cast<Relation*>(FullSelection[0]))
		{
			RelationUi.MembersView->setModel(R->referenceMemberModel(Main));
			RelationUi.TagView->setModel(theModel);
			RelationUi.TagView->setItemDelegate(delegate);

			QWidget* w = RelationUi.variableLayout->takeAt(0)->widget();
			w->deleteLater();
			if (theTemplates)
				RelationUi.variableLayout->addWidget(theTemplates->getWidget(R));
			
			CurrentTagView     = RelationUi.TagView;
			CurrentMembersView = RelationUi.MembersView;
		}

		if (theTemplates)
			theTemplates->apply(FullSelection[0]);
	}
	else if (FullSelection.size() > 1)
	{
		Main->info()->setHtml("");
		#ifdef GEOIMAGE
		Main->geoImage()->setImage(-1);
		#endif
		MultiUi.TagView->setModel(theModel);
		MultiUi.TagView->setItemDelegate(delegate);
		CurrentTagView = MultiUi.TagView;
	}
	theModel->setFeature(Current);
	Selection = Current;
	
	/* If we have standard TableViews in the current UI, set it so that the */
	/* first column is the width of the default text (Edit this to add...)  */
	/* And the rest of the space is assigned to the second column           */
    if (CurrentTagView) {
        if (M_PREFS->getTagListFirstColumnWidth())
            CurrentTagView->setColumnWidth(
                0, M_PREFS->getTagListFirstColumnWidth()
            );
        else
            CurrentTagView->setColumnWidth(
                0, CurrentTagView->fontMetrics().width(theModel->newKeyText())+10
            );
		CurrentTagView->horizontalHeader()->setStretchLastSection(true);
		CurrentTagView->installEventFilter(shortcutFilter);
	}
	if (CurrentMembersView) {
		CurrentMembersView->setColumnWidth(
			0, CurrentMembersView->fontMetrics().width(theModel->newKeyText())+10
		);
		CurrentMembersView->horizontalHeader()->setStretchLastSection(true);
		CurrentMembersView->installEventFilter(shortcutFilter);
	}
}

void PropertiesDock::on_TrackPointLat_editingFinished()
{
	if (TrackPointUi.Latitude->text().isEmpty()) return;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0));
	if (Pt)
	{
		Main->document()->addHistory(
			new MoveTrackPointCommand(Pt,
				Coord(angToInt(TrackPointUi.Latitude->text().toDouble()),Pt->position().lon()), Main->document()->getDirtyOrOriginLayer(Pt->layer()) ));
		Main->invalidateView(false);
	}
}

void PropertiesDock::on_TrackPointLon_editingFinished()
{
	if (TrackPointUi.Longitude->text().isEmpty()) return;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0));
	if (Pt)
	{
		Main->document()->addHistory(
			new MoveTrackPointCommand(Pt,
				Coord(Pt->position().lat(),angToInt(TrackPointUi.Longitude->text().toDouble())), Main->document()->getDirtyOrOriginLayer(Pt->layer()) ));
		Main->invalidateView(false);
	}
}

void PropertiesDock::on_tag_changed(QString k, QString v)
{
	MapFeature* F = FullSelection[0];
	if (F->tagValue(k, "__NULL__") != v) {
		Main->document()->addHistory(new SetTagCommand(F,k,v,Main->document()->getDirtyOrOriginLayer(F->layer())));
		Main->invalidateView();

		resetValues();
	}
}

void PropertiesDock::on_tag_cleared(QString k)
{
	MapFeature* F = FullSelection[0];
	Main->document()->addHistory(new ClearTagCommand(F,k,Main->document()->getDirtyOrOriginLayer(F->layer())));
	Main->invalidateView();

	resetValues();
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
	if (TagTable)
	{
		QModelIndexList indexes = TagTable->selectionModel()->selectedIndexes();
		QModelIndex index;

		foreach(index, indexes)
		{
			QModelIndex idx = index.sibling(index.row(),0);
			QVariant Content(theModel->data(idx,Qt::DisplayRole));
			if (Content.isValid())
			{
				QString KeyName = Content.toString();
				CommandList* L = new CommandList(MainWindow::tr("Clear Tag '%1' on %2").arg(KeyName).arg(Selection[0]->id()), Selection[0]);
				for (unsigned int i=0; i<Selection.size(); ++i)
					if (Selection[i]->findKey(KeyName) < Selection[i]->tagSize())
						L->add(new ClearTagCommand(Selection[i],KeyName,Main->document()->getDirtyOrOriginLayer(Selection[i]->layer())));
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
					MapFeature* F = Content.value<MapFeature*>();
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
		menu.exec(CurrentMembersView->mapToGlobal(pos));
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
					MapFeature* F = Content.value<MapFeature*>();
					if (F) {
						setSelection(F);
						cb = F->boundingBox();
					}
				}
			}
		}
	} else
	if (CurrentTagView) {
		Main->setUpdatesEnabled(false);
		unsigned int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
		cb = FullSelection[idx]->boundingBox();
		for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
			idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
			cb.merge(FullSelection[idx]->boundingBox());
		}
	}
	Coord c = cb.center();
	Main->view()->projection().setCenter(c, Main->view()->rect());
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
					MapFeature* F = Content.value<MapFeature*>();
					if (F) {
						setSelection(F);
						cb = F->boundingBox();
						CoordBox mini(cb.center()-2000, cb.center()+2000);
						cb.merge(mini);
						cb = cb.zoomed(1.1);
					}
				}
			}
		}
	} else
	if (CurrentTagView) {
		Main->setUpdatesEnabled(false);
		unsigned int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
		CoordBox cb = FullSelection[idx]->boundingBox();
		for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
			idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
			cb.merge(FullSelection[idx]->boundingBox());
		}
		CoordBox mini(cb.center()-2000, cb.center()+2000);
		cb.merge(mini);
		cb = cb.zoomed(1.1);
	}
	Main->view()->projection().setViewport(cb, Main->view()->rect());
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


}

