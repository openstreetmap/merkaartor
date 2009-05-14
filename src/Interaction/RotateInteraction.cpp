#include "Interaction/RotateInteraction.h"

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

RotateInteraction::RotateInteraction(MapView* aView)
: FeatureSnapInteraction(aView), StartDragPosition(0,0)
{
}

RotateInteraction::~RotateInteraction(void)
{
}

#ifndef Q_OS_SYMBIAN
QCursor RotateInteraction::cursor() const
{
	QPixmap pm(":/Icons/rotate.svg");
	return QCursor(pm.scaledToWidth(22));
}
#endif


void RotateInteraction::snapMousePressEvent(QMouseEvent * event, MapFeature* aLast)
{
	QList<MapFeature*> sel;
	if (view()->isSelectionLocked()) {
		if (view()->properties()->selection(0))
			sel.append(view()->properties()->selection(0));
		else
			sel.append(aLast);
	} else {
		sel = view()->properties()->selection();
		if (!sel.size() && aLast)
			sel.append(aLast);
	}
	clearNoSnap();
	Rotating.clear();
	OriginalPosition.clear();

	if (!sel.size())
		return;

	StartDragPosition = projection().inverse(event->pos());
	CoordBox selBB = sel[0]->boundingBox();
	for (int j=0; j<sel.size(); j++) {
		selBB.merge(sel[j]->boundingBox());
		if (TrackPoint* Pt = dynamic_cast<TrackPoint*>(sel[j]))
		{
			Rotating.push_back(Pt);
			StartDragPosition = Pt->position();
		}
		else if (Road* R = dynamic_cast<Road*>(sel[j])) {
			for (int i=0; i<R->size(); ++i)
				if (std::find(Rotating.begin(),Rotating.end(),R->get(i)) == Rotating.end())
					Rotating.push_back(R->getNode(i));
			addToNoSnap(R);
		}
	}
	if (Rotating.size() > 1) {
		RotationCenter = selBB.center();
		for (int i=0; i<Rotating.size(); ++i)
		{
			OriginalPosition.push_back(Rotating[i]->position());
			addToNoSnap(Rotating[i]);
		}
	} else
		Rotating.clear();
}

void RotateInteraction::snapMouseReleaseEvent(QMouseEvent * event, MapFeature* /*Closer*/)
{
	if (Rotating.size() && !panning())
	{
		CommandList* theList;
		theList = new CommandList(MainWindow::tr("Rotate Nodes").arg(Rotating[0]->id()), Rotating[0]);
		double Angle = calculateNewAngle(event);
		for (int i=0; i<Rotating.size(); ++i)
		{
			Rotating[i]->setPosition(OriginalPosition[i]);
			if (Rotating[i]->layer()->isTrack())
				theList->add(new MoveTrackPointCommand(Rotating[i],rotatePosition(RotationCenter, OriginalPosition[i], Angle), Rotating[i]->layer()));
			else
				theList->add(new MoveTrackPointCommand(Rotating[i],rotatePosition(RotationCenter, OriginalPosition[i], Angle), document()->getDirtyOrOriginLayer(Rotating[i]->layer())));
		}
		
		
		document()->addHistory(theList);
		view()->invalidate(true, false);
	}
	Rotating.clear();
	OriginalPosition.clear();
	clearNoSnap();
}

void RotateInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* /*Closer*/)
{
	if (Rotating.size() && !panning())
	{
		double Angle = calculateNewAngle(event);
		for (int i=0; i<Rotating.size(); ++i)
			Rotating[i]->setPosition(rotatePosition(RotationCenter, OriginalPosition[i], Angle));
		view()->invalidate(true, false);
	}
}

Coord RotateInteraction::rotatePosition(Coord center, Coord position, double angle)
{
	QPointF c = projection().project(center);
	QPointF p = projection().project(position);
	QLineF v(c, p);
	v.setAngle(v.angle() + angle);

	return projection().inverse(v.p2());
}

double RotateInteraction::calculateNewAngle(QMouseEvent *event)
{
	QPointF c = projection().project(RotationCenter);
	QPointF p1 = projection().project(StartDragPosition);
	QLineF v1(c, p1);
	QLineF v2(c, event->pos());

	return v1.angleTo(v2);
}

void RotateInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (!RotationCenter.isNull())
	{
		thePainter.setPen(QPen(QColor(255,0,0),1));
		thePainter.drawEllipse(projection().project(RotationCenter), 5, 5);
	}
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

