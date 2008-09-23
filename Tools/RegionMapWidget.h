//
// C++ Interface: RegionMapWidget
//
// Description: 
//
//
// Author: Chris Browet <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef REGIONMAPWIDGET_H
#define REGIONMAPWIDGET_H

#include "Utils/SlippyMapWidget.h"

#include <QHash>

class RegionMapWidget : public SlippyMapWidget
{
	Q_OBJECT

	public:
		RegionMapWidget(QWidget* aParent);
		virtual ~RegionMapWidget();

		void setShowGrid(bool val);
		bool getShowGrid();

		virtual void paintEvent(QPaintEvent* ev);
		virtual void mouseReleaseEvent(QMouseEvent* ev);

		QHash <quint32, bool> SelectedRegions;
		QHash <quint32, bool> ExistingRegions;

	private:
		bool showGrid;
};

#endif
