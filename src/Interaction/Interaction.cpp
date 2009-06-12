#include "Interaction.h"

#include "MapView.h"
#include "Maps/MapDocument.h"
#include "Maps/Projection.h"
#include "Maps/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <math.h>

Interaction::Interaction(MapView* aView)
: theView(aView), Panning(false), Dragging(false), StartDrag(0,0), EndDrag(0,0)
{
	connect(this, SIGNAL(requestCustomContextMenu(const QPoint &)), theView, SLOT(on_customContextMenuRequested(const QPoint &)));
}

Interaction::~Interaction()
{
}

bool Interaction::panning() const
{
	return (Panning && (LastPan != FirstPan));
}

MainWindow* Interaction::main()
{
	return theView->main();
}

MapView* Interaction::view()
{
	return theView;
}

MapDocument* Interaction::document()
{
	return theView->document();
}

const Projection& Interaction::projection() const
{
	return theView->projection();
}

const QTransform& Interaction::transform() const
{
	return theView->transform();
}

void Interaction::mousePressEvent(QMouseEvent * anEvent)
{
#if defined(Q_OS_MAC)
	// In the name of beautifull code, Steve, add a right mouse button
	if (	(anEvent->modifiers() & Qt::MetaModifier) ||
			(M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
			(!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
		)
#else
	if (
		(M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
		(!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
		)
#endif // Q_OS_MAC
	{
		if (anEvent->modifiers() & Qt::ControlModifier) {
			EndDrag = StartDrag = XY_TO_COORD(anEvent->pos());
			Dragging = true;
		} else 
		if (anEvent->modifiers() & Qt::ShiftModifier) {
		} else {
			Panning = true;
			FirstPan = LastPan = anEvent->pos();
		}
	}
}

void Interaction::mouseReleaseEvent(QMouseEvent * anEvent)
{
	if (Panning) {
		if (FirstPan != LastPan)
			view()->invalidate(true, true);
#ifndef _MOBILE
		else
			if (anEvent->button() == Qt::RightButton)
				emit(requestCustomContextMenu(anEvent->pos()));
#endif
		Panning = false;
	} else
	if (Dragging)
	{
		CoordBox DragBox(StartDrag,XY_TO_COORD(anEvent->pos()));
		if (!DragBox.isEmpty()) {
			view()->setViewport(DragBox,view()->rect());
			view()->invalidate(true, true);
			view()->launch(0);
		}
		Dragging = false;
	} else
		if (anEvent->button() == Qt::RightButton)
			emit(requestCustomContextMenu(anEvent->pos()));
}

void Interaction::mouseMoveEvent(QMouseEvent* anEvent)
{
#if defined(Q_OS_MAC)
	// In the name of beautifull code, Steve, add a right mouse button
	if (	(anEvent->modifiers() & Qt::MetaModifier) ||
			(M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
			(!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
		)
#else
	if (
		(M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::LeftButton)) ||
		(!M_PREFS->getMouseSingleButton() && (anEvent->buttons() & Qt::RightButton))
		)
#endif // Q_OS_MAC
	{
		if (Panning)
		{
			QPoint Delta = LastPan;
			Delta -= anEvent->pos();
			view()->panScreen(-Delta);
			LastPan = anEvent->pos();
#if defined(ENABLE_NVIDIA_HACK)
			view()->invalidate(true, false);
#endif // ENABLE_NVIDIA_HACK
		} else
		if (Dragging)
		{
			EndDrag = XY_TO_COORD(anEvent->pos());
			view()->update();
		}
	}
}

void Interaction::paintEvent(QPaintEvent*, QPainter& thePainter)
{
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(0,0,255),1,Qt::DotLine));
		thePainter.drawRect(QRectF(COORD_TO_XY(StartDrag),COORD_TO_XY(EndDrag)));
	}
}

#ifndef Q_OS_SYMBIAN
QCursor Interaction::cursor() const
{
	return QCursor(Qt::ArrowCursor);
}
#endif
