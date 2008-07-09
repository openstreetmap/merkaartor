#include "Interaction.h"

#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"

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
	return Panning;
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

void Interaction::mousePressEvent(QMouseEvent * anEvent)
{
#if defined(Q_OS_MAC)
	// In the name of beautifull code, Steve, add a right mouse button
	if ( (anEvent->modifiers() & Qt::MetaModifier) ||
	   (anEvent->buttons() & Qt::RightButton) )
#else
	if (anEvent->buttons() & Qt::RightButton)
#endif
	{
		if (anEvent->modifiers() & Qt::ShiftModifier) {
			EndDrag = StartDrag = projection().inverse(anEvent->pos());
			Dragging = true;
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
		else
			emit(requestCustomContextMenu(anEvent->pos()));
		Panning = false;
	}
	if (Dragging)
	{
		CoordBox DragBox(StartDrag,projection().inverse(anEvent->pos()));
		Dragging = false;
		view()->projection().setViewport(DragBox,view()->rect());
		view()->invalidate(true, true);
		view()->launch(0);
	}
}

void Interaction::mouseMoveEvent(QMouseEvent* anEvent)
{
	if (Panning)
	{
		QPoint Delta = LastPan;
		Delta -= anEvent->pos();
		view()->panScreen(-Delta);
		LastPan = anEvent->pos();
	}
	if (Dragging)
	{
		EndDrag = projection().inverse(anEvent->pos());
		view()->update();
	}
}

void Interaction::paintEvent(QPaintEvent*, QPainter& thePainter)
{
	if (Dragging)
	{
		thePainter.setPen(QPen(QColor(0,0,255),1,Qt::DotLine));
		thePainter.drawRect(QRectF(projection().project(StartDrag),projection().project(EndDrag)));
	}
}

QCursor Interaction::cursor() const
{
	return QCursor(Qt::ArrowCursor);
}



