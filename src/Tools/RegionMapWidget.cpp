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
	CoordBox v = CoordBox(Coord(R.y(), R.x()), Coord(R.y()+R.height(), R.x()+R.width()));
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
            P.drawLine(int((x * REGION_WIDTH - v.bottomLeft().lon()) * sx), 0, int((x * REGION_WIDTH - v.bottomLeft().lon()) * sx), height());
	}
	for (qint64 y = t; y<= v.topRight().lat() / REGION_WIDTH; ++y) {
            P.drawLine(0, int(height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy)), width(), int(height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy)));
	}
	for (qint64 x = l; x<= v.topRight().lon() / REGION_WIDTH; ++x)
		for (qint64 y = t; y<= v.topRight().lat() / REGION_WIDTH; ++y) {
			quint32 rg = (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);
            QRect rgRect = QRect(int((x * REGION_WIDTH - v.bottomLeft().lon()) * sx),
                            int(height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy)),
                            int(REGION_WIDTH * sx), int(-REGION_WIDTH * sy));
			//P.setClipRect(rgRect.adjusted(-1, -1, 1, 1));
			if (ExistingRegions[rg])
                P.drawText(int((x * REGION_WIDTH - v.bottomLeft().lon()) * sx),
                            int(height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy)),
							QString("%1 - %2").arg(QString::number(rg)).arg(DateRegions[rg].toString("yyyy-MM-dd")));
			else
                P.drawText(int((x * REGION_WIDTH - v.bottomLeft().lon()) * sx),
                            int(height() - ((y * REGION_WIDTH - v.bottomLeft().lat()) * sy)),
							QString("%1").arg(QString::number(rg)));

			if (ExistingRegions[rg] && !DeleteRegions[rg]) {
				//qDebug() << (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);
				P.save();
				P.setPen(Qt::NoPen);
				P.setBrush(QBrush(Qt::green, Qt::FDiagPattern));

				P.drawRect(rgRect);

				P.restore();
			}
			if (SelectedRegions[rg] && !DeleteRegions[rg]) {
				//qDebug() << (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);
				P.save();
				P.setPen(Qt::NoPen);
				P.setBrush(QBrush(Qt::blue, Qt::BDiagPattern));

				P.drawRect(rgRect);

				P.restore();
			}
		}


}

void RegionMapWidget::mouseReleaseEvent(QMouseEvent* ev)
{
	if (!isDragging()) {
		QRect R(viewArea());
		CoordBox v = CoordBox(Coord(R.y(), R.x()), Coord(R.y()+R.height(), R.x()+R.width()));
		QPointF P = ev->pos();

        Coord Pt(int(((height()-P.y()) / height() * v.latDiff()) + v.bottomLeft().lat()), int((P.x() / width() * v.lonDiff()) + v.bottomLeft().lon()));
		int x = int(((qint64)Pt.lon()) / REGION_WIDTH);
		x = (x < 0) ? x-1 :x;
		int y = int(((qint64)Pt.lat()) / REGION_WIDTH);
		y = (y < 0) ? y-1 :y;
		int rg = (y + NUM_REGIONS/2)*NUM_REGIONS + (x + NUM_REGIONS/2);

		if (SelectedRegions[rg] && ExistingRegions[rg]) {
			SelectedRegions[rg] = false;
			DeleteRegions[rg] = true;
		} else
			if (!SelectedRegions[rg] && DeleteRegions[rg])
				DeleteRegions[rg] = false;
			else
				SelectedRegions[rg] = !SelectedRegions[rg];

		update();
	}

	SlippyMapWidget::mouseReleaseEvent(ev);
}

