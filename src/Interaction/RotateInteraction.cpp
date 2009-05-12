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
	MapFeature* sel = aLast;
	if (view()->isSelectionLocked()) {
		sel = view()->properties()->selection(0);
		if (!sel)
			sel = aLast;
	}
	clearNoSnap();
	Rotating.clear();
	OriginalPosition.clear();
	StartDragPosition = projection().inverse(event->pos());
	if (Road* R = dynamic_cast<Road*>(sel)) {
		for (int i=0; i<R->size(); ++i)
			if (std::find(Rotating.begin(),Rotating.end(),R->get(i)) == Rotating.end())
				Rotating.push_back(R->getNode(i));
		addToNoSnap(R);
		RotationCenter = R->boundingBox().center();
	}
	for (int i=0; i<Rotating.size(); ++i)
	{
		OriginalPosition.push_back(Rotating[i]->position());
		addToNoSnap(Rotating[i]);
	}
}

void RotateInteraction::snapMouseReleaseEvent(QMouseEvent * event, MapFeature* Closer)
{
	if (Rotating.size())
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

void RotateInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* Closer)
{
	if (Rotating.size())
	{
		double Angle = calculateNewAngle(event);
		for (int i=0; i<Rotating.size(); ++i)
			Rotating[i]->setPosition(rotatePosition(RotationCenter, OriginalPosition[i], Angle));
		view()->invalidate(true, false);
	}
}

Coord RotateInteraction::rotatePosition(Coord center, Coord position, double angle)
{
	QPointF c = QPointF(center.lon(), center.lat());
	QPointF p = QPointF(position.lon(), position.lat());
	QLineF v(c, p);
	v.setAngle(v.angle() + angle);

	return Coord(v.p2().y(), v.p2().x());
}

double RotateInteraction::calculateNewAngle(QMouseEvent *event)
{
	QPointF c = QPointF(RotationCenter.lon(), RotationCenter.lat());
	QPointF p1 = QPointF(StartDragPosition.lon(), StartDragPosition.lat());
	QLineF v1(c, p1);


	Coord TargetC = projection().inverse(event->pos());
	QPointF p2 = QPointF(TargetC.lon(), TargetC.lat());
	QLineF v2(c, p2);

	return v1.angleTo(v2);
}
