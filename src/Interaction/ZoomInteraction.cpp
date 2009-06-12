#include "Interaction/ZoomInteraction.h"

#include "MapView.h"
#include "Maps/MapDocument.h"
#include "Maps/Projection.h"
#include "Maps/TrackPoint.h"

#include <QtGui/QMouseEvent>
#include <QtGui/QPainter>

ZoomInteraction::ZoomInteraction(MapView* aView)
: Interaction(aView), HaveFirstPoint(false)
{
}

ZoomInteraction::~ZoomInteraction(void)
{
}

QString ZoomInteraction::toHtml()
{
	QString help;
	help = (MainWindow::tr("LEFT-CLICK to first corner -> LEFT-DRAG to specify area -> LEFT-CLICK to zoom"));

	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(MainWindow::tr("Zoom Interaction"));
	desc += QString("<b>%1</b><br/>").arg(help);

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "</body></html>";

	return S;
}

void ZoomInteraction::paintEvent(QPaintEvent*, QPainter& thePainter)
{
	if (HaveFirstPoint)
	{
		QPen TP(Qt::DashDotLine);
		thePainter.setBrush(Qt::NoBrush);
		TP.setColor(QColor(255,0,0));
		thePainter.setPen(TP);
		thePainter.drawRect(QRectF(P1,QSize(int(P2.x()-P1.x()),int(P2.y()-P1.y()))));
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
		view()->setViewport(CoordBox(XY_TO_COORD(P1),XY_TO_COORD(P2)),view()->rect());
		view()->invalidate(true, true);
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

#ifndef Q_OS_SYMBIAN
QCursor ZoomInteraction::cursor() const
{
	QPixmap pm(":/Icons/zoomico.xpm");
	return QCursor(pm,11,12);
}
#endif


