#include "EditInteraction.h"
#include "MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Interaction/MoveTrackPointInteraction.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/RoadManipulations.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <vector>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0)
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
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
		thePainter.drawRect(QRectF(projection().project(StartDrag),projection().project(EndDrag)));
	}
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

void EditInteraction::snapMousePressEvent(QMouseEvent * ev, MapFeature* aLast)
{
	if (ev->buttons() & Qt::LeftButton)
	{
		if (ev->modifiers() & Qt::ControlModifier)
		{
			if (aLast)
				view()->properties()->toggleSelection(aLast);
		}
		else {
			view()->properties()->setSelection(aLast);
			if (aLast)
				view()->info()->setHtml(aLast->toHtml());
			else
				view()->info()->setHtml("");
		}
		if (!aLast)
		{
			EndDrag = StartDrag = projection().inverse(ev->pos());
			Dragging = true;
		}
		view()->properties()->checkMenuStatus();
		view()->update();
	}
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * ev , MapFeature* )
{
	if (Dragging)
	{
		std::vector<MapFeature*> List;
		CoordBox DragBox(StartDrag,projection().inverse(ev->pos()));
		for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
			if (DragBox.contains(it.get()->boundingBox()))
				List.push_back(it.get());
		view()->properties()->setSelection(List);
		view()->properties()->checkMenuStatus();
		Dragging = false;
		view()->update();
	}
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* )
{
	if (Dragging)
	{
		EndDrag = projection().inverse(event->pos());
		view()->update();
	}
}

void EditInteraction::on_remove_triggered()
{
	std::vector<MapFeature*> Sel;
	for (unsigned int i=0; i<view()->properties()->size(); ++i)
		Sel.push_back(view()->properties()->selection(i));
	if (Sel.size() == 0) return;
	view()->properties()->setSelection(0);
	view()->properties()->checkMenuStatus();
	CommandList* theList = new CommandList;
	for (unsigned int i=0; i<Sel.size(); ++i)
		if (document()->exists(Sel[i]))
		{
			std::vector<MapFeature*> Alternatives;
			for (FeatureIterator it(document()); !it.isEnd(); ++it)
				it.get()->cascadedRemoveIfUsing(document(), Sel[i], theList, Alternatives);
			theList->add(new RemoveFeatureCommand(document(), Sel[i]));
		}
	document()->history().add(theList);
	view()->invalidate();
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
