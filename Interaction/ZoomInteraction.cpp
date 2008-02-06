#include "Interaction/ZoomInteraction.h"

#include "MapView.h"
#include "Map/MapDocument.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

ZoomInteraction::ZoomInteraction(MapView* aView)
: Interaction(aView), HaveFirstPoint(false)
{
}

ZoomInteraction::~ZoomInteraction(void)
{
}

void ZoomInteraction::paintEvent(QPaintEvent*, QPainter& thePainter)
{
	if (HaveFirstPoint)
	{
		QPen TP(Qt::DashDotLine);
		thePainter.setBrush(Qt::NoBrush);
		TP.setColor(QColor(255,0,0));
		thePainter.setPen(TP);
		thePainter.drawRect(QRectF(P1,QSize(P2.x()-P1.x(),P2.y()-P1.y())));
	}
}

void ZoomInteraction::mouseReleaseEvent(QMouseEvent * event)
{
	if (!HaveFirstPoint)
	{
		P1 = P2 = event->pos();
		HaveFirstPoint = true;
	}
	else
	{
		P2 = event->pos();
		view()->projection()->setViewport(CoordBox(projection()->inverse(P1),projection()->inverse(P2)),view()->rect());
		view()->invalidate();
		view()->launch(0);
	}
}

void ZoomInteraction::mouseMoveEvent(QMouseEvent* event)
{
	if (HaveFirstPoint)
	{
		P2 = event->pos();
		view()->update();
	}
}

QCursor ZoomInteraction::cursor() const
{
	QPixmap pm(":/Icons/zoomico.xpm");
	return QCursor(pm,11,12);
}


