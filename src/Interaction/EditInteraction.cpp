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
: FeatureSnapInteraction(theView), Dragging(false), StartDrag(0,0), EndDrag(0,0)
    , currentMode(EditMode)
    , theMoveInteraction(0)
{
	connect(main(),SIGNAL(remove_triggered()),this,SLOT(on_remove_triggered()));
	connect(main(),SIGNAL(reverse_triggered()), this,SLOT(on_reverse_triggered()));
	view()->properties()->checkMenuStatus();

    theMoveInteraction = new MoveTrackPointInteraction(theView);
}

EditInteraction::~EditInteraction(void)
{
	if(main())
	{
		main()->editRemoveAction->setEnabled(false);
		main()->editReverseAction->setEnabled(false);
	}
    SAFE_DELETE(theMoveInteraction);
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
		help = (MainWindow::tr("LEFT-CLICK to select;RIGHT-CLICK to pan;CTRL-LEFT-CLICK to toggle selection;SHIFT-LEFT-CLICK to add to selection;LEFT-DRAG for area selection;CTRL-RIGHT-DRAG for zoom;"));
	else
		help = (MainWindow::tr("CLICK to select/move;CTRL-CLICK to toggle selection;SHIFT-CLICK to add to selection;SHIFT-DRAG for area selection;CTRL-DRAG for zoom;"));

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

void EditInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(255,0,0),1,Qt::DotLine));
		thePainter.drawRect(QRectF(COORD_TO_XY(StartDrag),COORD_TO_XY(EndDrag)));
	}
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

void EditInteraction::snapMousePressEvent(QMouseEvent * ev, MapFeature* aLast)
{
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
			if (view()->properties()->selection().size() == 0) {
				if (
					(M_PREFS->getMouseSingleButton() && (ev->modifiers() & Qt::ShiftModifier) && !aLast) ||
					(!M_PREFS->getMouseSingleButton() && !aLast)
					)
				{
					EndDrag = StartDrag = XY_TO_COORD(ev->pos());
					Dragging = true;
				}
			}
		}
		view()->properties()->checkMenuStatus();
		view()->update();
	}
    if (currentMode == MoveMode) {
        theMoveInteraction->snapMousePressEvent(ev, aLast);
    }
}

void EditInteraction::snapMouseReleaseEvent(QMouseEvent * ev , MapFeature* aLast)
{
	Q_UNUSED(ev);
	if (currentMode == MoveMode) {
        theMoveInteraction->snapMouseReleaseEvent(ev, aLast);
    } else
	if (Dragging)
	{
		QList<MapFeature*> List;
		CoordBox DragBox(StartDrag,XY_TO_COORD(ev->pos()));
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
        theMoveInteraction->snapMouseMoveEvent(anEvent, aLast);
    }
    if (Dragging)
	{
		EndDrag = XY_TO_COORD(anEvent->pos());
		view()->update();
	} else
    if (anEvent->buttons() == Qt::NoButton) {
        if (aLast && view()->properties()->isSelected(aLast) && !M_PREFS->getSeparateMoveMode())
        {
    #ifndef Q_OS_SYMBIAN
            view()->setCursor(moveCursor());
    #endif
            currentMode = MoveMode;
        } else
        {
    #ifndef Q_OS_SYMBIAN
            view()->setCursor(cursor());
    #endif
            currentMode = EditMode;
        }
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
