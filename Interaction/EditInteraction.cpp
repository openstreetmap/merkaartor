#include "EditInteraction.h"
#include "MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Interaction/EditRoadInteraction.h"
#include "Interaction/MoveTrackPointInteraction.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/RoadManipulations.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView)
{
	connect(main(),SIGNAL(remove_triggered()),this,SLOT(on_remove_triggered()));
	connect(main(),SIGNAL(move_triggered()),this,SLOT(on_move_triggered()));
	connect(main(),SIGNAL(add_triggered()),this,SLOT(on_add_triggered()));
	connect(main(),SIGNAL(reverse_triggered()), this,SLOT(on_reverse_triggered()));
	view()->properties()->checkMenuStatus();
}

EditInteraction::~EditInteraction(void)
{
	main()->editRemoveAction->setEnabled(false);
	main()->editMoveAction->setEnabled(false);
	main()->editAddAction->setEnabled(false);
	main()->editReverseAction->setEnabled(false);
}

void EditInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	for (unsigned int i=0; i<view()->properties()->size(); ++i)
		view()->properties()->selection(i)->drawFocus(thePainter, projection());
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

void EditInteraction::snapMousePressEvent(QMouseEvent * event, MapFeature* aLast)
{
	if (event->buttons() & Qt::LeftButton)
	{
		if (event->modifiers() & Qt::ControlModifier)
		{
			if (aLast)
				view()->properties()->toggleSelection(aLast);
		}
		else
			view()->properties()->setSelection(aLast);
		view()->properties()->checkMenuStatus();
		view()->update();
	}
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * , MapFeature* )
{
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* )
{
}

void EditInteraction::on_remove_triggered()
{
	MapFeature* Selection = view()->properties()->selection(0);
	if (Selection)
	{
		view()->properties()->setSelection(0);
		view()->properties()->checkMenuStatus();
		std::vector<MapFeature*> Alternatives;
		CommandList* theList = new CommandList;
		for (FeatureIterator it(document()); !it.isEnd(); ++it)
			it.get()->cascadedRemoveIfUsing(document(), Selection, theList, Alternatives);
		theList->add(new RemoveFeatureCommand(document(), Selection));
		document()->history().add(theList);
		view()->invalidate();
	}
}

void EditInteraction::on_move_triggered()
{
	view()->launch(new MoveTrackPointInteraction(view()));
}

void EditInteraction::on_add_triggered()
{
//	view()->launch(new EditRoadInteraction(view(),dynamic_cast<Road*>(view()->properties()->selection(0))));
}

void EditInteraction::on_reverse_triggered()
{
	MapFeature* Selection = view()->properties()->selection(0);
	if (Road* R = dynamic_cast<Road*>(Selection))
	{
		CommandList* theList = new CommandList;
		reversePoints(theList,R);
		document()->history().add(theList);
	}
	view()->invalidate();
}
