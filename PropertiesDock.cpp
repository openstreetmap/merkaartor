#include "PropertiesDock.h"
#include "InfoDock.h"
#include "MainWindow.h"
#include "MapView.h"
#include "TagModel.h"
#include "Utils/EditCompleterDelegate.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/Coord.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/PreDefinedTags.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"

#include <QtCore/QTimer>
#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>
#include <QtGui/QTableView>
#include <QClipboard>

#include <algorithm>

PropertiesDock::PropertiesDock(MainWindow* aParent)
: QDockWidget(aParent), Main(aParent), CurrentUi(0), Selection(0), NowShowing(NoUiShowing)
{
	setMinimumSize(220,100);
	switchToNoUi();
	setWindowTitle(tr("Properties"));
	setObjectName("propertiesDock");
	theModel = new TagModel(aParent);
	delegate = new EditCompleterDelegate(aParent);

	centerAction = new QAction(tr("Center map"), this);
	connect(centerAction, SIGNAL(triggered()), this, SLOT(on_centerAction_triggered()));
	centerZoomAction = new QAction(tr("Center && Zoom map"), this);
	connect(centerZoomAction, SIGNAL(triggered()), this, SLOT(on_centerZoomAction_triggered()));
}

PropertiesDock::~PropertiesDock(void)
{
	delete theModel;
}

static bool isChildOfSingleRoad(MapFeature *mapFeature)
{
	unsigned int parents = mapFeature->sizeParents();

	if (parents == 0)
		return false;

	unsigned int parentRoads = 0;

	unsigned int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		bool isParentRoad = dynamic_cast<Road*>(parent) != 0;
		if (isParentRoad)
			parentRoads++;
	}

	return (parentRoads == 1);
}

void PropertiesDock::checkMenuStatus()
{
	bool IsPoint = false;
	bool IsRoad = false;
	bool IsParentRoad = false;
	unsigned int NumRoads = 0;
	unsigned int NumCommitableFeature = 0;
	unsigned int NumPoints = 0;
	if (Selection.size() == 1)
	{
		IsPoint = dynamic_cast<TrackPoint*>(Selection[0]) != 0;
		IsRoad = dynamic_cast<Road*>(Selection[0]) != 0;
		IsParentRoad = IsPoint && isChildOfSingleRoad(Selection[0]);
	}
	for (unsigned int i=0; i<Selection.size(); ++i)
	{
		if (dynamic_cast<TrackPoint*>(Selection[i]))
			++NumPoints;
		if (dynamic_cast<Road*>(Selection[i])) {
			++NumRoads;
		if (!Selection[i]->layer()->isUploadable())
			++NumCommitableFeature;
		}
	}
	Main->createRelationAction->setEnabled(Selection.size());
	Main->editRemoveAction->setEnabled(Selection.size());
	Main->editMoveAction->setEnabled(true);
	Main->editReverseAction->setEnabled(IsRoad);
	Main->roadJoinAction->setEnabled(NumRoads > 1);
	Main->roadSplitAction->setEnabled(IsParentRoad || (NumRoads && NumPoints));
	Main->roadBreakAction->setEnabled(NumRoads > 1);
	Main->featureCommitAction->setEnabled(NumCommitableFeature);
	Main->nodeMergeAction->setEnabled(NumPoints > 1);
	Main->nodeAlignAction->setEnabled(NumPoints > 2);
	Main->fileDownloadMoreAction->setEnabled(Main->document()->getLastDownloadLayer() != NULL);

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
		if (Selection.size() == 1)
			Main->info()->setHtml(Selection[0]->toHtml());
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
	if (FullSelection.size() == 0)
		switchToNoUi();
	else if (FullSelection.size() == 1)
	{
		if (dynamic_cast<TrackPoint*>(FullSelection[0]))
			switchToTrackPointUi();
		else if (dynamic_cast<Road*>(FullSelection[0]))
			switchToRoadUi();
		else if (dynamic_cast<Relation*>(FullSelection[0]))
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
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(MultiUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	connect(MultiUi.SelectionList,SIGNAL(itemSelectionChanged()),this,SLOT(on_SelectionList_itemSelectionChanged()));
	connect(MultiUi.SelectionList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(on_SelectionList_itemDoubleClicked(QListWidgetItem*)));
	connect(MultiUi.SelectionList, SIGNAL(customContextMenuRequested(const QPoint &)), this, SLOT(on_SelectionList_customContextMenuRequested(const QPoint &)));
	setWindowTitle(tr("Properties - Multiple elements"));
}

void PropertiesDock::switchToTrackPointUi()
{
	if (NowShowing == TrackPointUiShowing) return;
	NowShowing = TrackPointUiShowing;
	QWidget* NewUi = new QWidget(this);
	TrackPointUi.setupUi(NewUi);
	fillAmenities(TrackPointUi.Amenity);
	TrackPointUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(TrackPointUi.Longitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLon_editingFinished()));
	connect(TrackPointUi.Latitude,SIGNAL(editingFinished()),this, SLOT(on_TrackPointLat_editingFinished()));
	connect(TrackPointUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	connect(TrackPointUi.Amenity,SIGNAL(activated(int)),this,SLOT(on_Amenity_activated(int)));
	setWindowTitle(tr("Properties - Trackpoint"));
}


void PropertiesDock::switchToRelationUi()
{
	if (NowShowing == RelationUiShowing) return;
	NowShowing = RelationUiShowing;
	QWidget* NewUi = new QWidget(this);
	RelationUi.setupUi(NewUi);
	fillLandUse(RelationUi.LandUse);
	RelationUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(RelationUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	connect(RelationUi.LandUse,SIGNAL(activated(int)), this, SLOT(on_LandUse_activated(int)));
	setWindowTitle(tr("Properties - Relation"));
}

void PropertiesDock::switchToRoadUi()
{
	if (NowShowing == RoadUiShowing) return;
	NowShowing = RoadUiShowing;
	QWidget* NewUi = new QWidget(this);
	RoadUi.setupUi(NewUi);
	fillHighway(RoadUi.Highway);
	fillLandUse(RoadUi.LandUse);
	RoadUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(RoadUi.Name,SIGNAL(editingFinished()),this, SLOT(on_RoadName_editingFinished()));
	connect(RoadUi.TrafficDirection,SIGNAL(activated(int)), this, SLOT(on_TrafficDirection_activated(int)));
	connect(RoadUi.Highway,SIGNAL(activated(int)), this, SLOT(on_Highway_activated(int)));
	connect(RoadUi.LandUse,SIGNAL(activated(int)), this, SLOT(on_LandUse_activated(int)));
	connect(RoadUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Road"));
}

void PropertiesDock::switchToNoUi()
{
	if (NowShowing == NoUiShowing) return;
	NowShowing = NoUiShowing;
	QWidget* NewUi = new QWidget(this);
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	setWindowTitle(tr("Properties"));
}

void PropertiesDock::resetValues()
{
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
			resetTagComboBox(TrackPointUi.Amenity,Pt,"amenity");
		}
		else if (Road* R = dynamic_cast<Road*>(FullSelection[0]))
		{
			RoadUi.Id->setText(R->id());
			RoadUi.Name->setText(R->tagValue("name",""));
			RoadUi.TrafficDirection->setCurrentIndex(trafficDirection(R));
			RoadUi.TagView->setModel(theModel);
                        RoadUi.TagView->setItemDelegate(delegate);
                        resetTagComboBox(RoadUi.Highway,R,"highway");
			resetTagComboBox(RoadUi.LandUse,R,"landuse");
		}
		else if (Relation* R = dynamic_cast<Relation*>(FullSelection[0]))
		{
			RelationUi.MembersView->setModel(R->referenceMemberModel(Main));
			RelationUi.TagView->setModel(theModel);
                        RelationUi.TagView->setItemDelegate(delegate);
                        resetTagComboBox(RelationUi.LandUse,R,"landuse");
		}
	}
	else if (FullSelection.size() > 1)
	{
		Main->info()->setHtml("");
		MultiUi.TagView->setModel(theModel);
		MultiUi.TagView->setItemDelegate(delegate);
	}
	theModel->setFeature(Current);
	Selection = Current;
}

void PropertiesDock::on_TrackPointLat_editingFinished()
{
	if (TrackPointUi.Latitude->text().isEmpty()) return;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0));
	if (Pt)
	{
		Pt->setLastUpdated(MapFeature::User);
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
		Pt->setLastUpdated(MapFeature::User);
		Main->document()->addHistory(
			new MoveTrackPointCommand(Pt,
				Coord(Pt->position().lat(),angToInt(TrackPointUi.Longitude->text().toDouble())), Main->document()->getDirtyOrOriginLayer(Pt->layer()) ));
		Main->invalidateView(false);
	}
}

void PropertiesDock::on_RoadName_editingFinished()
{
	if (selection(0))
	{
		if (RoadUi.Name->text().isEmpty())
			Main->document()->addHistory(
				new ClearTagCommand(selection(0),"name",Main->document()->getDirtyOrOriginLayer(selection(0)->layer())));
		else {
			CommandList* theList  = new CommandList(MainWindow::tr("Set Tag 'name' to '%1' on %2").arg(RoadUi.Name->text()).arg(selection(0)->description()), selection(0));
			if (!selection(0)->isDirty() && !selection(0)->hasOSMId() && selection(0)->isUploadable())
				theList->add(new AddFeatureCommand(Main->document()->getDirtyLayer(),selection(0),false));
			theList->add(new SetTagCommand(selection(0),"name",RoadUi.Name->text(),Main->document()->getDirtyOrOriginLayer(selection(0)->layer())));
			Main->document()->addHistory(theList);
		}
		theModel->setFeature(Selection);
	}
}

void PropertiesDock::on_TrafficDirection_activated(int idx)
{
	Road* R = dynamic_cast<Road*>(selection(0));
	if (R && (idx != trafficDirection(R)) )
	{
		switch (idx)
		{
			case MapFeature::OneWay:
				Main->document()->addHistory(new SetTagCommand(R,"oneway","yes",Main->document()->getDirtyOrOriginLayer(R->layer()))); break;
			case MapFeature::BothWays:
				Main->document()->addHistory(new SetTagCommand(R,"oneway","no",Main->document()->getDirtyOrOriginLayer(R->layer()))); break;
			case MapFeature::OtherWay:
				Main->document()->addHistory(new SetTagCommand(R,"oneway","-1",Main->document()->getDirtyOrOriginLayer(R->layer()))); break;
			default:
				Main->document()->addHistory(new ClearTagCommand(R,"oneway",Main->document()->getDirtyOrOriginLayer(R->layer()))); break;
		}
		Main->invalidateView();
	}
}

void PropertiesDock::on_Highway_activated(int idx)
{
	if (Road* R = dynamic_cast<Road*>(selection(0)))
	{
		tagComboBoxActivated(RoadUi.Highway,idx,R,"highway",Main->document());
		Main->invalidateView();
	}
}

void PropertiesDock::on_Amenity_activated(int idx)
{
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0)))
	{
		tagComboBoxActivated(TrackPointUi.Amenity,idx,Pt, "amenity",Main->document());
		Main->invalidateView();
	}
}

void PropertiesDock::on_LandUse_activated(int idx)
{
	if (Road* R = dynamic_cast<Road*>(selection(0)))
		tagComboBoxActivated(RoadUi.LandUse,idx,R,"landuse",Main->document());
	else if (Relation* Rel = dynamic_cast<Relation*>(selection(0)))
		tagComboBoxActivated(RelationUi.LandUse,idx,Rel,"landuse",Main->document());
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
	Main->setUpdatesEnabled(false);
	unsigned int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
	CoordBox cb = FullSelection[idx]->boundingBox();
	for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
		idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
		cb.merge(FullSelection[idx]->boundingBox());
	}
	Coord c = cb.center();
	Main->view()->projection().setCenter(c, Main->view()->rect());
	Main->setUpdatesEnabled(true);
	Main->invalidateView(false);
}

void PropertiesDock::on_centerZoomAction_triggered()
{
	Main->setUpdatesEnabled(false);
	unsigned int idx = MultiUi.SelectionList->selectedItems()[0]->data(Qt::UserRole).toUInt();
	CoordBox cb = FullSelection[idx]->boundingBox();
	for (int i=1; i < MultiUi.SelectionList->selectedItems().size(); i++) {
		idx = MultiUi.SelectionList->selectedItems()[i]->data(Qt::UserRole).toUInt();
		cb.merge(FullSelection[idx]->boundingBox());
	}
	CoordBox min(cb.center()-2000, cb.center()+2000);
	cb.merge(min);
	cb = cb.zoomed(1.1);
	Main->view()->projection().setViewport(cb, Main->view()->rect());
	Main->setUpdatesEnabled(true);
	Main->invalidateView(false);
}
