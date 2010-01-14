#include "Interaction/MoveTrackPointInteraction.h"

#include "MapView.h"
#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Maps/Coord.h"
#include "Maps/MapDocument.h"
#include "Maps/Projection.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"
#include "Utils/MDiscardableDialog.h"

#include <QtGui/QCursor>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>
#include <QMessageBox>

#include <QList>

MoveTrackPointInteraction::MoveTrackPointInteraction(MapView* aView)
	: FeatureSnapInteraction(aView)
	, StartDragPosition(0,0)
{
	if (M_PREFS->getSeparateMoveMode()) {
		setDontSelectVirtual(false);
	}
}

MoveTrackPointInteraction::~MoveTrackPointInteraction(void)
{
}

QString MoveTrackPointInteraction::toHtml()
{
	QString help;
	help = (MainWindow::tr("LEFT-CLICK to select;LEFT-DRAG to move"));

	QStringList helpList = help.split(";");

	QString desc;
	desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Move node Interaction"));

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

#ifndef Q_OS_SYMBIAN
QCursor MoveTrackPointInteraction::cursor() const
{
	QPixmap pm(":/Icons/move.xpm");
	return QCursor(pm);
}
#endif


void MoveTrackPointInteraction::snapMousePressEvent(QMouseEvent * event, MapFeature* aLast)
{
	QList<MapFeature*> sel;
	if (view()->isSelectionLocked()) {
		if (view()->properties()->selection(0))
			sel.append(view()->properties()->selection(0));
		else
			sel.append(aLast);
	} else {
		if (aLast) {
			if (view()->properties()->selection().size())
				sel = view()->properties()->selection();
			else
				sel.append(aLast);
		}
	}
	clearNoSnap();
	Moving.clear();
	OriginalPosition.clear();
	StartDragPosition = XY_TO_COORD(event->pos());
	for (int j=0; j<sel.size(); j++) {
		if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(sel[j]))
		{
			Moving.push_back(Pt);
			if (sel.size() == 1)
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
	Virtual = false;
	theList = new CommandList;
}

void MoveTrackPointInteraction::snapMouseReleaseEvent(QMouseEvent * event, MapFeature* Closer)
{
	Coord Diff(calculateNewPosition(event,Closer, theList)-StartDragPosition);
	if (Moving.size() && !panning() && !Diff.isNull())
	{
		if (Moving.size() > 1) {
			theList->setDescription(MainWindow::tr("Move Nodes"));
			theList->setFeature(Moving[0]);
		} else {
			if (!Virtual) {
				theList->setDescription(MainWindow::tr("Move Node %1").arg(Moving[0]->id()));
				theList->setFeature(Moving[0]);
			}
		}
		for (int i=0; i<Moving.size(); ++i)
		{
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
				TrackPoint* visPt = CAST_NODE(it.get());
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
	}
	Moving.clear();
	OriginalPosition.clear();
	clearNoSnap();
}

void MoveTrackPointInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* Closer)
{
	Coord Diff = calculateNewPosition(event,Closer,NULL)-StartDragPosition;
	if (Moving.size() && !panning() && !Diff.isNull())
	{
		for (int i=0; i<Moving.size(); ++i) {
			if (Moving[i]->isVirtual()) {
				Virtual = true;
				TrackPoint* v = Moving[i];
				Road* aRoad = CAST_WAY(v->getParent(0));
				theList->setDescription(MainWindow::tr("Create node in Road: %1").arg(aRoad->id()));
				theList->setFeature(aRoad);
				int SnapIdx = aRoad->findVirtual(v)+1;
				TrackPoint* N = new TrackPoint(*v);
				N->setVirtual(false);
				N->setPosition(OriginalPosition[i]+Diff);
				if (M_PREFS->apiVersionNum() < 0.6)
					N->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));

				if (view()->properties()->isSelected(v)) {
					view()->properties()->toggleSelection(v);
					view()->properties()->toggleSelection(N);
				}
				theList->add(new AddFeatureCommand(main()->document()->getDirtyOrOriginLayer(aRoad->layer()),N,true));
				theList->add(new RoadAddTrackPointCommand(aRoad,N,SnapIdx,main()->document()->getDirtyOrOriginLayer(aRoad->layer())));

				Moving[i] = N;
			} else {
				Moving[i]->setPosition(OriginalPosition[i]+Diff);
				for (int j=0; j<Moving[i]->sizeParents(); ++j) {
					if (Road* aRoad = CAST_WAY(Moving[i]->getParent(j)))
						aRoad->updateVirtuals();
				}
			}
		}
		view()->invalidate(true, false);
	}
}

Coord MoveTrackPointInteraction::calculateNewPosition(QMouseEvent *event, MapFeature *aLast, CommandList* theList)
{
	Coord TargetC = XY_TO_COORD(event->pos());
	if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(aLast))
		return Pt->position();
	else if (Road* R = dynamic_cast<Road*>(aLast))
	{
		QPoint Target(TargetC.lat(),TargetC.lon());
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
	return TargetC;
}
