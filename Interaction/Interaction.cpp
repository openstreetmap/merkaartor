#include "Interaction.h"

#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

#include <math.h>

Interaction::Interaction(MapView* aView)
: theView(aView), Panning(false)
{
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

const Projection* Interaction::projection() const
{
	return theView->projection();
}

void Interaction::mousePressEvent(QMouseEvent * anEvent)
{
	if (anEvent->buttons() & Qt::RightButton)
	{
		Panning = true;
		LastPan = anEvent->pos();
	}
}

void Interaction::mouseReleaseEvent(QMouseEvent * )
{
	if (Panning)
	{
		Panning = false;
		view()->invalidate();
	}
}

void Interaction::mouseMoveEvent(QMouseEvent* anEvent)
{
	if (Panning)
	{
		QPoint Delta = LastPan;
		Delta -= anEvent->pos();
		view()->projection()->panScreen(-Delta,view()->rect());
		view()->invalidate();
		LastPan = anEvent->pos();
	}
}

void Interaction::paintEvent(QPaintEvent*, QPainter&)
{
}

QCursor Interaction::cursor() const
{
	return QCursor(Qt::ArrowCursor);
}



