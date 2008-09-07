#include "SatelliteStrengthView.h"

#include <QtGui/QPainter>

void sortSatellitesById(std::vector<Satellite>& List)
{
	for (unsigned int i=0; i<List.size(); ++i)
		for (unsigned int j=i+1; j<List.size(); ++j)
			if (List[i].Id>List[j].Id)
				std::swap(List[i],List[j]);
}

SatelliteStrengthView::SatelliteStrengthView(QWidget* aParent)
: QWidget(aParent)
{
}

void SatelliteStrengthView::setSatellites(const std::vector<Satellite>& aList)
{
	List = aList;
	update();
}

void SatelliteStrengthView::paintEvent(QPaintEvent* ev)
{
	QPainter p(this);
	p.drawLine(0,height()-1,width()-1,height()-1);
	if (List.size())
	{
		int w=width()/List.size();
		p.setBrush(QBrush(QColor(0,0,0)));
		for (unsigned int i=0; i<List.size(); ++i)
		{
			// typical s/r values seem to be 0-50?
			// this may turn out higher, I tested it on a
			// very rainy day...
			int x = List[i].SignalStrength*height()/50;
			x = height()-x;
			if (x<0) x = 0;
			p.setPen(QColor(0,0,0));
			p.drawRect(w*i,x,w,height()-x);
			p.setPen(QColor(127,127,127));
			p.drawText(QRect(w*i,0,w,height()),
				Qt::AlignCenter,QString::number(List[i].Id));
		}
	}
	else
		p.drawText(rect(),Qt::AlignCenter,tr("No satellites"));
}
