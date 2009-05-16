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
#include "Maps/MapDocument.h"
#include "Maps/MapFeature.h"
#include "Maps/Road.h"
#include "Maps/Relation.h"
#include "Maps/FeatureManipulations.h"
#include "Maps/TrackPoint.h"
#include "Maps/Projection.h"
#include "Utils/LineF.h"
#include "Utils/MDiscardableDialog.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>
#include <QMessageBox>

#include <QList>

EditInteraction::EditInteraction(MapView* theView)
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0),
	StartDragPosition(0,0), currentMode(EditMode), Moved(false)
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

#ifndef Q_OS_SYMBIAN
QCursor EditInteraction::moveCursor() const
{
	QPixmap pm(":/Icons/move.xpm");
	return QCursor(pm);
}
#endif

QString EditInteraction::toHtml()
{
	QString help;
	if (!M_PREFS->getMouseSingleButton())
		help = (MainWindow::tr("LEFT-CLICK to select;RIGHT-CLICK to pan;CTRL-LEFT-CLICK to toggle selction;SHIFT-LEFT-CLICK to add to selection;LEFT-DRAG for area selection;CTRL-RIGHT-DRAG for zoom;"));
	else
		help = (MainWindow::tr("CLICK to select/move;CTRL-CLICK to toggle selction;SHIFT-CLICK to add to selection;SHIFT-DRAG for area selection;CTRL-DRAG for zoom;"));

	QStringList helpList = help.split(";");

	QString desc;
	desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Edit Interaction"));

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "<hr/>";
	S += "<ul style=\"margin-left: 0px; padding-left: 0px;\">";
	for (int i=0; i<helpList.size(); ++i) {
		S+= "<li>" + helpList[i] + "</li>";
	}
	S += "</ul>";
	S += "</body></html>";

	return S;
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
		int BestIdx = 1;
		for (int i=2; i<R->size(); ++i)
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
	if (currentMode == MoveMode) {
		QList<MapFeature*> sel;
		if (view()->isSelectionLocked()) {
			if (view()->properties()->selection(0))
				sel.append(view()->properties()->selection(0));
			else
				sel.append(aLast);
		} else {
			sel = view()->properties()->selection();
		}
		clearNoSnap();
		Moving.clear();
		OriginalPosition.clear();
		StartDragPosition = projection().inverse(ev->pos());
		for (int j=0; j<sel.size(); j++) {
			if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(sel[j]))
			{
				Moving.push_back(Pt);
				StartDragPosition = Pt->position();
			}
			else if (Road* R = dynamic_cast<Road*>(sel[j])) {
				for (int i=0; i<R->size(); ++i)
					if (std::find(Moving.begin(),Moving.end(),R->get(i)) == Moving.end())
						Moving.push_back(R->getNode(i));
				addToNoSnap(R);
			}
		}
		for (int i=0; i<Moving.size(); ++i)
		{
			OriginalPosition.push_back(Moving[i]->position());
			addToNoSnap(Moving[i]);
		}
	} 
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
	if (currentMode == MoveMode) {
		// Check if we actually moved 
		if (Moving.size() && Moved)
		{
			CommandList* theList;
			if (Moving.size() > 1)
				theList = new CommandList(MainWindow::tr("Move Nodes").arg(Moving[0]->id()), Moving[0]);
			else
			theList = new CommandList(MainWindow::tr("Move Node %1").arg(Moving[0]->id()), Moving[0]);
			Coord Diff(calculateNewPosition(ev, aLast, theList)-StartDragPosition);
			for (int i=0; i<Moving.size(); ++i)
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
				QList<TrackPoint*> samePosPts;
				for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it)
				{
					TrackPoint* visPt = dynamic_cast<TrackPoint*>(it.get());
					if (visPt && visPt->layer()->classType() != MapLayer::TrackMapLayerType)
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
					MDiscardableMessage dlg(view(),
						MainWindow::tr("Nodes at the same position found."),
						MainWindow::tr("Do you want to merge all nodes at the drop position?"));
					if (dlg.check() == QDialog::Accepted)
					{
						// Merge all nodes from the same position

						// from MainWindow::on_nodeMergeAction_triggered()
						// Merge all nodes into the first node that has been found (not the node being moved)
						MapFeature* F = samePosPts[0];
                        // Change the command description to reflect the merge
						theList->setDescription(MainWindow::tr("Merge Nodes into %1").arg(F->id()));
						theList->setFeature(F);
						
						// from mergeNodes(theDocument, theList, theProperties);
						QList<MapFeature*> alt;
						TrackPoint* merged = samePosPts[0];
						alt.push_back(merged);
						for (int i = 1; i < samePosPts.size(); ++i) {
							MapFeature::mergeTags(document(), theList, merged, samePosPts[i]);
							theList->add(new RemoveFeatureCommand(document(), samePosPts[i], alt));
						}
						
						view()->properties()->setSelection(F);
					}
				}
			}

			document()->addHistory(theList);
			view()->invalidate(true, false);
		} else
		if (ev->modifiers()) {
			if ((ev->modifiers() & Qt::ControlModifier) && aLast)
				view()->properties()->toggleSelection(aLast);

			if ((ev->modifiers() & Qt::ShiftModifier) && aLast)
				view()->properties()->addSelection(aLast);
		} 
		Moving.clear();
		OriginalPosition.clear();
		clearNoSnap();
		Moved = false;
	} else
	if (Dragging)
	{
		QList<MapFeature*> List;
		CoordBox DragBox(StartDrag,projection().inverse(ev->pos()));
		for (VisibleFeatureIterator it(document()); !it.isEnd(); ++it) {
			if (it.get()->layer()->isReadonly())
				continue;
			else if (
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
						for (int j=1; j<R->size(); ++j) {
							A = R->getNode(j-1)->position();
							B = R->getNode(j)->position();
							if (CoordBox::visibleLine(DragBox, A, B)) {
								List.push_back(R);
								break;
							}
						}
					} else 
					if (Relation* r = dynamic_cast<Relation*>(it.get())) {
						for (int k=0; k<r->size(); ++k) {
							if (Road* R = dynamic_cast<Road*>(r->get(k))) {
								for (int j=1; j<R->size(); ++j) {
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
		}
		view()->properties()->setSelection(List);
		view()->properties()->checkMenuStatus();
		Dragging = false;
		view()->update();
	} else {
		if (!panning() && !ev->modifiers()) {
			view()->properties()->setSelection(aLast);
			if (view()->properties()->isSelected(aLast) && !M_PREFS->getSeparateMoveMode()) {
				currentMode = MoveMode;
				Moved = false;
#ifndef Q_OS_SYMBIAN
				view()->setCursor(moveCursor());
#endif
			}
			view()->properties()->checkMenuStatus();
			view()->update();
		}
	}
}

void EditInteraction::snapMouseMoveEvent(QMouseEvent* anEvent, MapFeature* aLast)
{
	Q_UNUSED(anEvent);
	if (currentMode == MoveMode) {
		if (anEvent->buttons() & Qt::LeftButton && Moving.size())
		{
			Moved = true;
			Coord Diff = calculateNewPosition(anEvent, aLast, 0)-StartDragPosition;
			for (int i=0; i<Moving.size(); ++i)
				Moving[i]->setPosition(OriginalPosition[i]+Diff);
			view()->invalidate(true, false);
		} else
		if ((!aLast || !(view()->properties()->isSelected(aLast))) && !M_PREFS->getSeparateMoveMode())
		{
#ifndef Q_OS_SYMBIAN
			view()->setCursor(cursor());
#endif
			currentMode = EditMode;
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
#ifndef Q_OS_SYMBIAN
		view()->setCursor(moveCursor());
#endif
		currentMode = MoveMode;
		Moved = false;
	} else
	{
#ifndef Q_OS_SYMBIAN
		view()->setCursor(cursor());
#endif
		currentMode = EditMode;
		Moved = false;
	}
}

void EditInteraction::on_remove_triggered()
{
	QList<MapFeature*> Sel;
	for (int i=0; i<view()->properties()->size(); ++i)
		Sel.push_back(view()->properties()->selection(i));
	if (Sel.size() == 0) return;
	CommandList* theList  = new CommandList(MainWindow::tr("Remove feature %1").arg(Sel[0]->id()), Sel[0]);
	bool deleteChildrenOK = true;
	for (int i=0; i<Sel.size() && deleteChildrenOK; ++i) {
		if (document()->exists(Sel[i])) {
			QList<MapFeature*> Alternatives;
			theList->add(new RemoveFeatureCommand(document(), Sel[i], Alternatives));

			deleteChildrenOK = Sel[i]->deleteChildren(document(), theList);
		}
	}

	if (!deleteChildrenOK) {
		theList->undo();
		delete theList;
	} else
		if (theList->size()) {
			document()->addHistory(theList);
			view()->properties()->setSelection(0);
			view()->properties()->checkMenuStatus();
		}
		else
			delete theList;
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
