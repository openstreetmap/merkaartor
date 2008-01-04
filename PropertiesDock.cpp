#include "PropertiesDock.h"
#include "MainWindow.h"
#include "MapView.h"
#include "TagModel.h"
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

#include <algorithm>

PropertiesDock::PropertiesDock(MainWindow* aParent)
: QDockWidget(aParent), Main(aParent), CurrentUi(0), Selection(0), NowShowing(NoUiShowing)
{
	setMinimumSize(220,100);
	switchToNoUi();
	setWindowTitle(tr("Properties"));
	theModel = new TagModel(aParent);
}

PropertiesDock::~PropertiesDock(void)
{
	delete theModel;
}

void PropertiesDock::checkMenuStatus()
{
	bool IsPoint = false;
	bool IsRoad = false;
	unsigned int NumRoads = 0;
	unsigned int NumPoints = 0;
	if (Selection.size() == 1)
	{
		IsPoint = dynamic_cast<TrackPoint*>(Selection[0]) != 0;
		IsRoad = dynamic_cast<Road*>(Selection[0]) != 0;
	}
	for (unsigned int i=0; i<Selection.size(); ++i)
	{
		if (dynamic_cast<TrackPoint*>(Selection[i]))
			++NumPoints;
		if (dynamic_cast<Road*>(Selection[i]))
			++NumRoads;
	}
	Main->createRelationAction->setEnabled(Selection.size());
	Main->editRemoveAction->setEnabled(Selection.size());
	Main->editMoveAction->setEnabled(true);
	Main->editAddAction->setEnabled(IsRoad);
	Main->editReverseAction->setEnabled(IsRoad);
	Main->roadJoinAction->setEnabled(NumRoads > 1);
	Main->roadSplitAction->setEnabled(NumRoads && NumPoints);
	Main->roadBreakAction->setEnabled(NumRoads > 1);
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

void PropertiesDock::setSelection(MapFeature* S)
{
	cleanUpUi();
	Selection.clear();
	if (S)
		Selection.push_back(S);
	FullSelection = Selection;
	switchUi();
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

void PropertiesDock::fillMultiUiSelectionBox()
{
	if (NowShowing == MultiShowing)
	{
		// to prevent on_SelectionList_itemSelectionChanged to kick in
		NowShowing = NoUiShowing;
		MultiUi.SelectionList->clear();
		for (unsigned int i=0; i<FullSelection.size(); ++i)
		{
			QListWidgetItem* it = new QListWidgetItem(FullSelection[i]->description(),MultiUi.SelectionList);
			it->setData(Qt::UserRole,QVariant(i));
			it->setSelected(true);
		}
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
		theModel->setFeature(Selection);
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
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(MultiUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	connect(MultiUi.SelectionList,SIGNAL(itemSelectionChanged()),this,SLOT(on_SelectionList_itemSelectionChanged()));
	connect(MultiUi.SelectionList,SIGNAL(itemDoubleClicked(QListWidgetItem*)),this,SLOT(on_SelectionList_itemDoubleClicked(QListWidgetItem*)));
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
	connect(TrackPointUi.Longitude,SIGNAL(textChanged(const QString&)),this, SLOT(on_TrackPointLon_textChanged(const QString&)));
	connect(TrackPointUi.Latitude,SIGNAL(textChanged(const QString&)),this, SLOT(on_TrackPointLat_textChanged(const QString&)));
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
	connect(RoadUi.Name,SIGNAL(textChanged(const QString&)),this, SLOT(on_RoadName_textChanged(const QString&)));
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
		if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(FullSelection[0]))
		{
			TrackPointUi.Id->setText(Pt->id());
			TrackPointUi.Latitude->setText(QString::number(radToAng(Pt->position().lat()),'g',8));
			TrackPointUi.Longitude->setText(QString::number(radToAng(Pt->position().lon()),'g',8));
			TrackPointUi.TagView->setModel(theModel);
			resetTagComboBox(TrackPointUi.Amenity,Pt,"amenity");
		}
		else if (Road* R = dynamic_cast<Road*>(FullSelection[0]))
		{
			RoadUi.Id->setText(R->id());
			RoadUi.Name->setText(R->tagValue("name",""));
			RoadUi.TrafficDirection->setCurrentIndex(trafficDirection(R));
			RoadUi.TagView->setModel(theModel);
			resetTagComboBox(RoadUi.Highway,R,"highway");
			resetTagComboBox(RoadUi.LandUse,R,"landuse");
		}
		else if (Relation* R = dynamic_cast<Relation*>(FullSelection[0]))
		{
			RelationUi.MembersView->setModel(R->referenceMemberModel(Main));
			RelationUi.TagView->setModel(theModel);
			resetTagComboBox(RelationUi.LandUse,R,"landuse");
		}
	}
	else if (FullSelection.size() > 1)
		MultiUi.TagView->setModel(theModel);
	theModel->setFeature(Current);
	Selection = Current;
}

void PropertiesDock::on_TrackPointLat_textChanged(const QString&)
{
	if (TrackPointUi.Latitude->text().isEmpty()) return;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0));
	if (Pt)
	{
		Pt->setLastUpdated(MapFeature::User);
		Main->document()->history().add(
			new MoveTrackPointCommand(Pt,
				Coord(angToRad(TrackPointUi.Latitude->text().toDouble()),Pt->position().lon())));
		Main->invalidateView(false);
	}
}

void PropertiesDock::on_TrackPointLon_textChanged(const QString&)
{
	if (TrackPointUi.Longitude->text().isEmpty()) return;
	TrackPoint* Pt = dynamic_cast<TrackPoint*>(selection(0));
	if (Pt)
	{
		Pt->setLastUpdated(MapFeature::User);
		Main->document()->history().add(
			new MoveTrackPointCommand(Pt,
				Coord(Pt->position().lat(),angToRad(TrackPointUi.Longitude->text().toDouble()))));
		Main->invalidateView(false);
	}
}

void PropertiesDock::on_RoadName_textChanged(const QString&)
{
	if (selection(0))
	{
		if (RoadUi.Name->text().isEmpty())
			Main->document()->history().add(
				new ClearTagCommand(selection(0),"name"));
		else
			Main->document()->history().add(
				new SetTagCommand(selection(0),"name",RoadUi.Name->text()));
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
				Main->document()->history().add(new SetTagCommand(R,"oneway","yes")); break;
			case MapFeature::BothWays:
				Main->document()->history().add(new SetTagCommand(R,"oneway","no")); break;
			case MapFeature::OtherWay:
				Main->document()->history().add(new SetTagCommand(R,"oneway","-1")); break;
			default:
				Main->document()->history().add(new ClearTagCommand(R,"oneway")); break;
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
				CommandList* L = new CommandList;
				QString KeyName = Content.toString();
				for (unsigned int i=0; i<Selection.size(); ++i)
					if (Selection[i]->findKey(KeyName) < Selection[i]->tagSize())
						L->add(new ClearTagCommand(Selection[i],KeyName));
				if (L->empty())
					delete L;
				else
				{
					Main->document()->history().add(L);
					Main->invalidateView();
					return;
				}
			}
		}
	}
}
