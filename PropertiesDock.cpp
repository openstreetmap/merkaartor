#include "PropertiesDock.h"
#include "MainWindow.h"
#include "TagModel.h"
#include "Command/FeatureCommands.h"
#include "Command/TrackPointCommands.h"
#include "Command/WayCommands.h"
#include "Map/Coord.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Map/Way.h"

#include <QtGui/QHeaderView>
#include <QtGui/QLineEdit>
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
	Selection.clear();
	if (S)
		Selection.push_back(S);
	switchUi();
}

void PropertiesDock::toggleSelection(MapFeature* S)
{
	std::vector<MapFeature*>::iterator i = std::find(Selection.begin(),Selection.end(),S);
	if (i == Selection.end())
		Selection.push_back(S);
	else
		Selection.erase(i);
	switchUi();
}

void PropertiesDock::switchUi()
{
	if (Selection.size() == 0)
		switchToNoUi();
	else if (Selection.size() == 1)
	{
		if (dynamic_cast<Way*>(Selection[0]))
			switchToWayUi();
		else if (dynamic_cast<TrackPoint*>(Selection[0]))
			switchToTrackPointUi();
		else if (dynamic_cast<Road*>(Selection[0]))
			switchToRoadUi();
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
	setWindowTitle(tr("Properties - Multiple elements"));
}

void PropertiesDock::switchToTrackPointUi()
{
	if (NowShowing == TrackPointUiShowing) return;
	NowShowing = TrackPointUiShowing;
	QWidget* NewUi = new QWidget(this);
	TrackPointUi.setupUi(NewUi);
	TrackPointUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(TrackPointUi.Longitude,SIGNAL(textChanged(const QString&)),this, SLOT(on_TrackPointLon_textChanged(const QString&)));
	connect(TrackPointUi.Latitude,SIGNAL(textChanged(const QString&)),this, SLOT(on_TrackPointLat_textChanged(const QString&)));
	connect(TrackPointUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Trackpoint"));
}

void PropertiesDock::switchToWayUi()
{
	if (NowShowing == WayUiShowing) return;
	NowShowing = WayUiShowing;
	QWidget* NewUi = new QWidget(this);
	WayUi.setupUi(NewUi);
	WayUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(WayUi.Width,SIGNAL(textChanged(const QString&)),this, SLOT(on_WayWidth_textChanged(const QString&)));
	connect(WayUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Link"));
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

void PropertiesDock::switchToRoadUi()
{
	if (NowShowing == RoadUiShowing) return;
	NowShowing = RoadUiShowing;
	QWidget* NewUi = new QWidget(this);
	RoadUi.setupUi(NewUi);
	RoadUi.TagView->verticalHeader()->hide();
	setWidget(NewUi);
	if (CurrentUi)
		delete CurrentUi;
	CurrentUi = NewUi;
	connect(RoadUi.Name,SIGNAL(textChanged(const QString&)),this, SLOT(on_RoadName_textChanged(const QString&)));
	connect(RoadUi.TrafficDirection,SIGNAL(activated(int)), this, SLOT(on_TrafficDirection_activated(int)));
	connect(RoadUi.Highway,SIGNAL(activated(int)), this, SLOT(on_Highway_activated(int)));
	connect(RoadUi.RemoveTagButton,SIGNAL(clicked()),this, SLOT(on_RemoveTagButton_clicked()));
	setWindowTitle(tr("Properties - Road"));
}

void PropertiesDock::resetValues()
{
	// to prevent slots to change the values also
	std::vector<MapFeature*> Current = Selection;
	Selection.clear();
	if (Current.size() == 1)
	{
		if (Way* W = dynamic_cast<Way*>(Current[0]))
		{
			WayUi.Width->setText(QString::number(W->width()));
			WayUi.Id->setText(W->id());
			WayUi.TagView->setModel(theModel);
		}
		else if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(Current[0]))
		{
			TrackPointUi.Id->setText(Pt->id());
			TrackPointUi.Latitude->setText(QString::number(radToAng(Pt->position().lat()),'g',8));
			TrackPointUi.Longitude->setText(QString::number(radToAng(Pt->position().lon()),'g',8));
			TrackPointUi.TagView->setModel(theModel);
		}
		else if (Road* R = dynamic_cast<Road*>(Current[0]))
		{
			RoadUi.Id->setText(R->id());
			RoadUi.Name->setText(R->tagValue("name",""));
			RoadUi.TrafficDirection->setCurrentIndex(trafficDirection(R));
			RoadUi.TagView->setModel(theModel);
			unsigned int idx = RoadUi.Highway->findText(R->tagValue("highway","Unknown"));
			if (idx == -1)
				idx = 0;
			RoadUi.Highway->setCurrentIndex(idx);
		}
	}
	else if (Current.size() > 1)
		MultiUi.TagView->setModel(theModel);
	theModel->setFeature(Current);
	Selection = Current;
}

void PropertiesDock::on_WayWidth_textChanged(const QString& )
{
	if (WayUi.Width->text().isEmpty()) return;
	Way* W = dynamic_cast<Way*>(selection(0));
	if (W)
	{
		W->setLastUpdated(MapFeature::User);
		Main->document()->history().add(
			new WaySetWidthCommand(W,WayUi.Width->text().toDouble()));
		Main->invalidateView(false);
		theModel->setFeature(Selection);
	}
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
	Road* R = dynamic_cast<Road*>(selection(0));
	if (R)
	{
		if (idx == 0)
			Main->document()->history().add(new ClearTagCommand(R,"highway"));
		else
			Main->document()->history().add(new SetTagCommand(R,"highway",RoadUi.Highway->currentText()));
		Main->invalidateView();
	}
}

void PropertiesDock::on_RemoveTagButton_clicked()
{
	QTableView* TagTable = 0;
	switch (NowShowing)
	{
	case WayUiShowing:
		TagTable = WayUi.TagView; break;
	case TrackPointUiShowing:
		TagTable = TrackPointUi.TagView; break;
	case RoadUiShowing:
		TagTable = RoadUi.TagView; break;
	case MultiShowing:
		TagTable = MultiUi.TagView; break;
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