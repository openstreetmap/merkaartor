#include "EditInteraction.h"
#include "MainWindow.h"
#include "MapView.h"
#include "PropertiesDock.h"
#include "InfoDock.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Interaction/MoveTrackPointInteraction.h"
#include "Map/MapDocument.h"
#include "Map/MapFeature.h"
#include "Map/Road.h"
#include "Map/Relation.h"
#include "Map/FeatureManipulations.h"
#include "Map/TrackPoint.h"
#include "Map/Projection.h"
#include "Utils/LineF.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QMessageBox>

#include <vector>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0),
  StartDragPosition(0,0), MoveMode(false), Moved(false)
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

QCursor EditInteraction::moveCursor() const
{
	QPixmap pm(":/Icons/move.xpm");
	return QCursor(pm);
}

Coord EditInteraction::calculateNewPosition(QMouseEvent *event, MapFeature *aLast, CommandList* theList)
{
	Coord TargetC = projection().inverse(event->pos());
	QPoint Target(TargetC.lat(),TargetC.lon());
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aLast))
		return Pt->position();
	else if (Road* R = dynamic_cast<Road*>(aLast))
	{
		LineF L1(R->getNode(0)->position(),R->getNode(1)->position());
		double Dist = L1.capDistance(TargetC);
		QPoint BestTarget = L1.project(Target).toPoint();
		unsigned int BestIdx = 1;
		for (unsigned int i=2; i<R->size(); ++i)
		{
			LineF L2(R->getNode(i-1)->position(),R->getNode(i)->position());
			double Dist2 = L2.capDistance(TargetC);
			if (Dist2 < Dist)
			{
				Dist = Dist2;
				BestTarget = L2.project(Target).toPoint();
				BestIdx = i;
			}
		}
		if (theList && (Moving.size() == 1))
			theList->add(new
				RoadAddTrackPointCommand(R,Moving[0],BestIdx,document()->getDirtyOrOriginLayer(R->layer())));
		return Coord(BestTarget.x(),BestTarget.y());
	}
	return projection().inverse(event->pos());
}

void EditInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
		thePainter.drawRect(QRectF(projection().project(StartDrag),projection().project(EndDrag)));
	}
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

void EditInteraction::snapMousePressEvent(QMouseEvent * ev, MapFeature* aLast)
{
	if (MoveMode) {
		MapFeature* sel = aLast;
		if (view()->isSelectionLocked()) {
			sel = view()->properties()->selection(0);
			if (!sel)
				sel = aLast;
		}
		clearNoSnap();
		Moving.clear();
		OriginalPosition.clear();
		StartDragPosition = projection().inverse(ev->pos());
		if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(sel))
		{
			Moving.push_back(Pt);
			StartDragPosition = Pt->position();
		}
		else if (Road* R = dynamic_cast<Road*>(sel)) {
			for (unsigned int i=0; i<R->size(); ++i)
				if (std::find(Moving.begin(),Moving.end(),R->get(i)) == Moving.end())
					Moving.push_back(R->getNode(i));
			addToNoSnap(R);
		}
		for (unsigned int i=0; i<Moving.size(); ++i)
		{
			OriginalPosition.push_back(Moving[i]->position());
			addToNoSnap(Moving[i]);
		}
	} else
	if (!view()->isSelectionLocked()) {
		if (ev->buttons() & Qt::LeftButton)
		{
			if (ev->modifiers()) {
				if ((ev->modifiers() & Qt::ControlModifier) && aLast)
					view()->properties()->toggleSelection(aLast);

				if ((ev->modifiers() & Qt::ShiftModifier) && aLast)
					view()->properties()->addSelection(aLast);
			} else {
				StackSnap = SnapList;
//				if (aLast)
//					view()->properties()->setSelection(aLast);
			}
			if (
				(M_PREFS->getMouseSingleButton() && (ev->modifiers() & Qt::ShiftModifier) && !aLast) ||
				(!M_PREFS->getMouseSingleButton() && !aLast)
				)
			{
				EndDrag = StartDrag = projection().inverse(ev->pos());
				Dragging = true;
			}
		}
		view()->properties()->checkMenuStatus();
		view()->update();
	}
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * ev , MapFeature* aLast)
{
	Q_UNUSED(ev);
	if (MoveMode) {
		// Check if we actually moved 
		if (Moving.size() && Moved)
		{
			CommandList* theList = new CommandList(MainWindow::tr("Move Point %1").arg(Moving[0]->id()), Moving[0]);
			Coord Diff(calculateNewPosition(ev, aLast, theList)-StartDragPosition);
			for (unsigned int i=0; i<Moving.size(); ++i)
			{
				Moving[i]->setPosition(OriginalPosition[i]);
				if (Moving[i]->layer()->isTrack())
					theList->add(new MoveTrackPointCommand(Moving[i],OriginalPosition[i]+Diff, Moving[i]->layer()));
				else
					theList->add(new MoveTrackPointCommand(Moving[i],OriginalPosition[i]+Diff, document()->getDirtyOrOriginLayer(Moving[i]->layer())));
			}

			// If moving a single node (not a track node), see if it got dropped onto another node
			if (Moving.size() == 1 && !Moving[0]->layer()->isTrack())
			{
				Coord newPos = OriginalPosition[0] + Diff;
				std::vector<TrackPoint*> samePosPts;
				for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
				{
					TrackPoint* visPt = dynamic_cast<TrackPoint*>(it.get());
					if (visPt)
					{
						if (visPt == Moving[0])
							continue;

						if (visPt->position() == newPos)
						{
							samePosPts.push_back(visPt);
						}
					}
				}
				// Ensure the node being moved is at the end of the list.
				// (This is not the node that all nodes will be merged into,
				// they are always merged into a node that already was at that position.)
				samePosPts.push_back(Moving[0]);

				if (samePosPts.size() > 1)   // Ignore the node we're moving, see if there are more
				{
					int ret = QMessageBox::question(view(),
						MainWindow::tr("Nodes at the same position found."),
						MainWindow::tr("Do you want to merge all nodes at the drop position?"),
						QMessageBox::Yes | QMessageBox::No);
					if (ret == QMessageBox::Yes)
					{
						// Merge all nodes from the same position

						// from MainWindow::on_nodeMergeAction_triggered()
						// Merge all nodes into the first node that has been found (not the node being moved)
						MapFeature* F = samePosPts[0];
                        // Change the command description to reflect the merge
						theList->setDescription(MainWindow::tr("Merge Nodes into %1").arg(F->id()));
						theList->setFeature(F);
						
						// from mergeNodes(theDocument, theList, theProperties);
						std::vector<MapFeature*> alt;
						TrackPoint* merged = samePosPts[0];
						alt.push_back(merged);
						for (unsigned int i = 1; i < samePosPts.size(); ++i) {
							MapFeature::mergeTags(document(), theList, merged, samePosPts[i]);
							theList->add(new RemoveFeatureCommand(document(), samePosPts[i], alt));
						}
						
						view()->properties()->setSelection(F);
					}
				}
			}

			document()->addHistory(theList);
			view()->invalidate(true, false);
		}
		Moving.clear();
		OriginalPosition.clear();
		clearNoSnap();
		Moved = false;
	} else
	if (Dragging)
	{
		std::vector<MapFeature*> List;
		CoordBox DragBox(StartDrag,projection().inverse(ev->pos()));
		for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
			if (
				(M_PREFS->getMouseSingleButton() && ev->modifiers().testFlag(Qt::ShiftModifier) && ev->modifiers().testFlag(Qt::AltModifier)) ||
				(!M_PREFS->getMouseSingleButton() && ev->modifiers().testFlag(Qt::ShiftModifier))
				)
			{
				if (!DragBox.intersects(it.get()->boundingBox()))
					continue;
				if (DragBox.contains(it.get()->boundingBox()))
					List.push_back(it.get());
				else {
					Coord A, B;
					if (Road* R = dynamic_cast<Road*>(it.get())) {
						for (unsigned int j=1; j<R->size(); ++j) {
							A = R->getNode(j-1)->position();
							B = R->getNode(j)->position();
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
									A = R->getNode(j-1)->position();
									B = R->getNode(j)->position();
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
	} else {
		if (!panning() && !ev->modifiers()) {
			view()->properties()->setSelection(aLast);
			if (view()->properties()->isSelected(aLast) && !M_PREFS->getSeparateMoveMode()) {
				MoveMode = true;
				Moved = false;
				view()->setCursor(moveCursor());
			}
			view()->properties()->checkMenuStatus();
			view()->update();
		}
	}
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, MapFeature* aLast)
{
	Q_UNUSED(anEvent);
	if (MoveMode) {
		if (anEvent->buttons() & Qt::LeftButton && Moving.size())
		{
			Moved = true;
			Coord Diff = calculateNewPosition(anEvent, aLast, 0)-StartDragPosition;
			for (unsigned int i=0; i<Moving.size(); ++i)
				Moving[i]->setPosition(OriginalPosition[i]+Diff);
			view()->invalidate(true, false);
		} else
		if ((!aLast || !(view()->properties()->isSelected(aLast))) && !M_PREFS->getSeparateMoveMode())
		{
			view()->setCursor(cursor());
			MoveMode = false;
			Moved = false;
		}
	} else
	if (Dragging)
	{
		EndDrag = projection().inverse(anEvent->pos());
		view()->update();
	} else
	if (aLast && view()->properties()->isSelected(aLast) && !M_PREFS->getSeparateMoveMode())
	{
		view()->setCursor(moveCursor());
		MoveMode = true;
		Moved = false;
	} else
	{
		view()->setCursor(cursor());
		MoveMode = false;
		Moved = false;
	}
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
			std::vector<MapFeature*> Alternatives;
			theList->add(new RemoveFeatureCommand(document(), Sel[i], Alternatives));
		}
	for (unsigned int i=0; i<Sel.size(); ++i)
		if (document()->exists(Sel[i]))
			Sel[i]->deleteChildren(document(), theList);

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
