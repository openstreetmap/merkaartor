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
#include <QPainter>

RotateInteraction::RotateInteraction(MapView* aView)
: FeatureSnapInteraction(aView), StartDragPosition(0,0)
{
}

RotateInteraction::~RotateInteraction(void)
{
}

QString RotateInteraction::toHtml()
{
	QString help;
	help = (MainWindow::tr("HOVER to select;LEFT-DRAG to rotate/scale"));

	QStringList helpList = help.split(";");

	QString desc;
	desc = QString("<big><b>%1</b></big>").arg(MainWindow::tr("Rotate Interaction"));

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
QCursor RotateInteraction::cursor() const
{
	QPixmap pm(":/Icons/rotate.png");
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
	Angle = 0.0;
	Radius = 1.0;
	clearNoSnap();
	Rotating.clear();
	OriginalPosition.clear();

	if (!sel.size())
		return;

	StartDragPosition = XY_TO_COORD(event->pos());
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
		RotationCenter = COORD_TO_XY(selBB.center());
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
	Q_UNUSED(event);

	if ((Angle != 0.0 || Radius != 1.0) && Rotating.size() && !panning())
	{
		CommandList* theList;
		theList = new CommandList(MainWindow::tr("Scale/Rotate Nodes").arg(Rotating[0]->id()), Rotating[0]);
		for (int i=0; i<Rotating.size(); ++i)
		{
			Rotating[i]->setPosition(OriginalPosition[i]);
			if (Rotating[i]->layer()->isTrack())
				theList->add(new MoveTrackPointCommand(Rotating[i],rotatePosition(OriginalPosition[i], Angle, Radius), Rotating[i]->layer()));
			else
				theList->add(new MoveTrackPointCommand(Rotating[i],rotatePosition(OriginalPosition[i], Angle, Radius), document()->getDirtyOrOriginLayer(Rotating[i]->layer())));
		}
		
		
		document()->addHistory(theList);
		view()->invalidate(true, false);
	}
	Angle = 0.0;
	Radius = 1.0;
	Rotating.clear();
	OriginalPosition.clear();
	clearNoSnap();
}

void RotateInteraction::snapMouseMoveEvent(QMouseEvent* event, MapFeature* /*Closer*/)
{
	if (Rotating.size() && !panning())
	{
		if (!(event->modifiers() & Qt::ControlModifier))
			Radius = distance(RotationCenter,event->pos()) / distance(RotationCenter, COORD_TO_XY(StartDragPosition));
		if (!(event->modifiers() & Qt::ShiftModifier))
			Angle = calculateNewAngle(event);
		for (int i=0; i<Rotating.size(); ++i)
			Rotating[i]->setPosition(rotatePosition(OriginalPosition[i], Angle, Radius));
		view()->invalidate(true, false);
	}
}

Coord RotateInteraction::rotatePosition(Coord position, double angle, double radius)
{
	QPointF p = COORD_TO_XY(position);
	QLineF v(RotationCenter, p);
	v.setAngle(v.angle() + angle);
	v.setLength(v.length() * radius);

	return XY_TO_COORD(v.p2());
}

double RotateInteraction::calculateNewAngle(QMouseEvent *event)
{
	QPointF p1 = COORD_TO_XY(StartDragPosition);
	QLineF v1(RotationCenter, p1);
	QLineF v2(RotationCenter, event->pos());

	return v1.angleTo(v2);
}

void RotateInteraction::paintEvent(QPaintEvent* anEvent, QPainter& thePainter)
{
	if (!RotationCenter.isNull())
	{
		thePainter.setPen(QPen(QColor(255,0,0),1));
		thePainter.drawEllipse(COORD_TO_XY(RotationCenter), 5, 5);
	}
	FeatureSnapInteraction::paintEvent(anEvent, thePainter);
}

