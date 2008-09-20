//
// C++ Implementation: RegionMapWidget
//
// Description:
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "RegionMapWidget.h"
#include "Utils/SlippyMapWidget.h"

#include "ImportExport/ImportExportOsmBin.h"

#include <QPainter>
#include <QMouseEvent>

RegionMapWidget::RegionMapWidget(QWidget* aParent)
: SlippyMapWidget(aParent), showGrid(false)
{
}

RegionMapWidget::~RegionMapWidget(void)
{
}

void RegionMapWidget::setShowGrid(bool val)
{
	showGrid = val;
}

bool RegionMapWidget::getShowGrid()
{
	return showGrid;
}

void RegionMapWidget::paintEvent(QPaintEvent* anEvent)
{
	SlippyMapWidget::paintEvent(anEvent);

	if (!showGrid) return;

	QPainter P(this);
	QRect R(viewArea());
	CoordBox v = CoordBox(Coord(R.x(), R.y()), Coord(R.x()+R.width(), R.y()+R.height()));
	P.setPen(QPen(Qt::blue,2));
	P.setBrush(Qt::NoBrush);

	double sx = (double)width() / v.lonDiff();
	if (REGION_WIDTH * sx < 6) return;

	double sy = (double)height() / v.latDiff();

	qint32 l = v.bottomLeft().lon() / REGION_WIDTH;
	qint32 t = v.bottomLeft().lat() / REGION_WIDTH;
	//for (int x = l*REGION_WIDTH; x<v.width(); x+=REGION_WIDTH) {
	//	for (int y = t*REGION_WIDTH; y<v.height(); y+=REGION_WIDTH) {
	//		P.drawRect(x * sx, y * sy, REGION_WIDTH * sx, REGION_WIDTH * sy);
	//	}
	//}
	for (qint64 x = l; x<= v.topRight().lon() / REGION_WIDTH; ++x) {
			P.drawLine((x * REGION_WIDTH - v.bottomLeft().lon()) * sx, 0, (x * REGION_WIDTH - v.bottomLeft().lon()) * sx, height());
	}
	for (qint64 y = t; y<= v.topRight().lat() / REGION_WIDTH; ++y) {
			P.drawLine(0, height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy), width(), height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy));
	}
	for (qint64 x = l; x<= v.topRight().lon() / REGION_WIDTH; ++x)
		for (qint64 y = t; y<= v.topRight().lat() / REGION_WIDTH; ++y) {
			P.drawText((x * REGION_WIDTH - v.bottomLeft().lon()) * sx, 
			height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy),
			QString::number((y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2)));

			if (SelectedRegions[(y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2)]) {
				qDebug() << (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);
				P.save();
				P.setPen(Qt::NoPen);
				P.setBrush(QBrush(Qt::blue, Qt::BDiagPattern));

				P.drawRect(
					QRect((x * REGION_WIDTH - v.bottomLeft().lon()) * sx, 
							height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy), REGION_WIDTH * sx, -REGION_WIDTH * sy));

				P.restore();
			}
		}


}

void RegionMapWidget::mouseReleaseEvent(QMouseEvent* ev)
{
	if (!isDragging()) {
		QRect R(viewArea());
		CoordBox v = CoordBox(Coord(R.x(), R.y()), Coord(R.x()+R.width(), R.y()+R.height()));
		QPointF P = ev->pos();

		Coord Pt(((height()-P.y()) / height() * v.latDiff()) + v.bottomLeft().lat(), (P.x() / width() * v.lonDiff()) + v.bottomLeft().lon());
		int x = int(((qint64)Pt.lon()) / REGION_WIDTH);
		int y = int(((qint64)Pt.lat()) / REGION_WIDTH);
		int rg = (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);

		SelectedRegions[rg] = !SelectedRegions[rg];

		update();
	}

	SlippyMapWidget::mouseReleaseEvent(ev);
}

