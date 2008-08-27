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
#include "Map/Relation.h"
#include "Map/RoadManipulations.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <vector>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0)
{
	connect(main(),SIGNAL(remove_triggered()),this,SLOT(on_remove_triggered()));
	connect(main(),SIGNAL(reverse_triggered()), this,SLOT(on_reverse_triggered()));
	view()->properties()->checkMenuStatus();
}

EditInteraction::~EditInteraction(void)
{
	if(main())
	{
		main()->editRemoveAction->setEnabled(false);
		main()->editReverseAction->setEnabled(false);
	}
}

void EditInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	for (unsigned int i=0; i<view()->properties()->size(); ++i)
		view()->properties()->selection(i)->drawFocus(thePainter, projection());
#ifndef _MOBILE
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
		thePainter.drawRect(QRectF(projection().project(StartDrag),projection().project(EndDrag)));
	}
#endif
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

void EditInteraction::snapMousePressEvent(QMouseEvent * ev, MapFeature* aLast)
{
	if (ev->buttons() & Qt::LeftButton)
	{
		if (ev->modifiers()) {
			if (ev->modifiers() & Qt::ControlModifier)
			{
				if (aLast)
					view()->properties()->toggleSelection(aLast);
			}
			if (ev->modifiers() & Qt::ShiftModifier)
			{
				if (aLast)
					view()->properties()->addSelection(aLast);
			}
		} else {
			view()->properties()->setSelection(aLast);
		}
#ifndef _MOBILE
		if (!aLast)
		{
			EndDrag = StartDrag = projection().inverse(ev->pos());
			Dragging = true;
		}
#endif
		view()->properties()->checkMenuStatus();
		view()->update();
	}
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * ev , MapFeature* )
{
	Q_UNUSED(ev);
#ifndef _MOBILE
	if (Dragging)
	{
		std::vector<MapFeature*> List;
		CoordBox DragBox(StartDrag,projection().inverse(ev->pos()));
		for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
			if (ev->modifiers() & Qt::ShiftModifier) {
				if (!DragBox.intersects(it.get()->boundingBox()))
					continue;
				if (DragBox.contains(it.get()->boundingBox()))
					List.push_back(it.get());
				else {
					Coord A, B;
					if (Road* R = dynamic_cast<Road*>(it.get())) {
						for (unsigned int j=1; j<R->size(); ++j) {
							A = R->get(j-1)->position();
							B = R->get(j)->position();
							if (CoordBox::visibleLine(DragBox, A, B)) {
								List.push_back(R);
								break;
							}
						}
					} else 
					if (Relation* r = dynamic_cast<Relation*>(it.get())) {
						for (unsigned int k=0; k<r->size(); ++k) {
							if (Road* R = dynamic_cast<Road*>(r->get(k))) {
								for (unsigned int j=1; j<R->size(); ++j) {
									A = R->get(j-1)->position();
									B = R->get(j)->position();
									if (CoordBox::visibleLine(DragBox, A, B)) {
										List.push_back(r);
										break;
									}
								}
							}
						}
					}
				}
			} else {
				if (DragBox.contains(it.get()->boundingBox()))
					List.push_back(it.get());
			}
		view()->properties()->setSelection(List);
		view()->properties()->checkMenuStatus();
		Dragging = false;
		view()->update();
	}
#endif
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, MapFeature* )
{
	Q_UNUSED(anEvent);
#ifndef _MOBILE
	if (Dragging)
	{
		EndDrag = projection().inverse(anEvent->pos());
		view()->update();
	}
#endif
}

void EditInteraction::on_remove_triggered()
{
	std::vector<MapFeature*> Sel;
	for (unsigned int i=0; i<view()->properties()->size(); ++i)
		Sel.push_back(view()->properties()->selection(i));
	if (Sel.size() == 0) return;
	CommandList* theList  = new CommandList(MainWindow::tr("Remove feature %1").arg(Sel[0]->id()), Sel[0]);
	for (unsigned int i=0; i<Sel.size(); ++i)
		if (document()->exists(Sel[i]))
		{
			if (Sel[i]->deleteChildren(document(), theList)) {
				std::vector<MapFeature*> Alternatives;
				theList->add(new RemoveFeatureCommand(document(), Sel[i], Alternatives));
			}
		}

	if (theList->size())
		document()->addHistory(theList);
	else
		delete theList;
	view()->properties()->setSelection(0);
	view()->properties()->checkMenuStatus();
	view()->invalidate(true, false);
}

void EditInteraction::on_reverse_triggered()
{
	MapFeature* Selection = view()->properties()->selection(0);
	if (Road* R = dynamic_cast<Road*>(Selection))
	{
		CommandList* theList  = new CommandList(MainWindow::tr("Reverse Road %1").arg(R->id()), R);
		reversePoints(document(),theList,R);
		document()->addHistory(theList);
	}
	view()->invalidate(true, false);
}
