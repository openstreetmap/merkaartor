#include "PaintStyle/EditPaintStyle.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "PaintStyle/TagSelector.h"
#include "Utils/LineF.h"

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QtXml/QDomDocument>
#include <QtXml/QDomNode>


#include <math.h>
#include <utility>

#define LOCALZOOM		0.05
#define REGIONALZOOM	0.01
#define GLOBALZOOM		0.002

EditPaintStyle* EditPaintStyle::m_EPSInstance = 0;

//static bool localZoom(const Projection& theProjection)
//{
//	return theProjection.pixelPerM() < LOCALZOOM;
//}

static bool regionalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < REGIONALZOOM;
}

static bool globalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < GLOBALZOOM;
}

class EditPaintStylePrivate
{
	public:
		EditPaintStylePrivate(QPainter& P, const Projection& aProj)
			: thePainter(P), theProjection(aProj)
		{
			First.setP(this);
			Second.setP(this);
			Third.setP(this);
			Fourth.setP(this);
		}

		QPainter& thePainter;
		const Projection& theProjection;
		EPBackgroundLayer First;
		EPForegroundLayer Second;
		EPTouchupLayer Third;
		EPLabelLayer Fourth;
		bool isTrackPointVisible;
		bool isTrackSegmentVisible;
};

#define ALWAYS 10e6

/* Zoom boundaries : expressed in Pixel per Meter

   eg 0.01->ALWAYS means show a feature from a zoom level of 0.01 Pixel Per M,
   or 100 Meter per Pixel. For a screen of 1000px wide this is when viewing
   100km or less across.

   eg 0.2->ALWAYS means show a feature from a zoom level 0.2 Px/M or 5M/Px which
   is viewing 5km or less across a screen of 1000Px. */

void EPBackgroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}


void EPBackgroundLayer::draw(Road* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawBackground(R,p->thePainter,p->theProjection);
	else if (!globalZoom(p->theProjection) && !R->hasEditPainter())
	{
		QPen thePen(QColor(0,0,0),1);
		if (regionalZoom(p->theProjection))
			thePen = QPen(QColor(0x77,0x77,0x77),1);
		p->thePainter.setPen(thePen);
		p->thePainter.setBrush(Qt::NoBrush);
		p->thePainter.drawPath(R->getPath());
	}
}

void EPBackgroundLayer::draw(Relation* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawBackground(R,p->thePainter,p->theProjection);
}


void EPBackgroundLayer::draw(TrackPoint*)
{
}

void EPForegroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPForegroundLayer::draw(Road* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theProjection);
}

void EPForegroundLayer::draw(Relation* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theProjection);
}

void EPForegroundLayer::draw(TrackPoint*)
{
}

void EPTouchupLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPTouchupLayer::draw(Road* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(R,p->thePainter,p->theProjection);
	else {
		if ( M_PREFS->getDirectionalArrowsVisible() != DirectionalArrows_Never )
		{
			MapFeature::TrafficDirectionType TT = trafficDirection(R);
			if ( (TT != MapFeature::UnknownDirection) || (M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Always) ) 
			{
				double theWidth = p->theProjection.pixelPerM()*R->widthOf()-4;
				if (theWidth > 8)
					theWidth = 8;
				double DistFromCenter = 2*(theWidth+4);
				if (theWidth > 0)
				{
					for (unsigned int i=1; i<R->size(); ++i)
					{
						QPointF FromF(p->theProjection.project(R->getNode(i-1)->position()));
						QPointF ToF(p->theProjection.project(R->getNode(i)->position()));
						if (distance(FromF,ToF) > (DistFromCenter*2+4))
						{
							QPointF H(FromF+ToF);
							H *= 0.5;
							double A = angle(FromF-ToF);
							QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
							QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
							QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));
							if ( M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Oneway )
							{
								if ( (TT == MapFeature::OtherWay) || (TT == MapFeature::BothWays) )
								{
									p->thePainter.setPen(QColor(0,0,0));
									p->thePainter.drawLine(H+T,H+T-V1);
									p->thePainter.drawLine(H+T,H+T-V2);
								}
								if ( (TT == MapFeature::OneWay) || (TT == MapFeature::BothWays) )
								{
									p->thePainter.setPen(QColor(0,0,0));
									p->thePainter.drawLine(H-T,H-T+V1);
									p->thePainter.drawLine(H-T,H-T+V2);
								}
							} 
							else
							{
								p->thePainter.setPen(QColor(255,0,0));
								p->thePainter.drawLine(H-T,H-T+V1);
								p->thePainter.drawLine(H-T,H-T+V2);
							}
						}
					}
				}
			}
		}
	}
}

void EPTouchupLayer::draw(Relation* /* R */)
{
}

void EPTouchupLayer::draw(TrackPoint* Pt)
{
	const FeaturePainter* paintsel = Pt->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(Pt,p->thePainter,p->theProjection);
	else if (!Pt->hasEditPainter()) {
		if (p->isTrackPointVisible || (Pt->lastUpdated() == MapFeature::Log && !p->isTrackSegmentVisible)) {
			bool Draw = p->theProjection.pixelPerM() > 1;
			if (!Draw && !Pt->sizeParents() && (p->theProjection.pixelPerM() > LOCALZOOM) )
				Draw = true;
			if (Pt->lastUpdated() == MapFeature::Log && !p->isTrackSegmentVisible)
				Draw = true;
			if (Draw)
			{
				QPointF P(p->theProjection.project(Pt->position()));

				if (Pt->findKey("_waypoint_") != Pt->tagSize()) {
					QRectF R(P-QPointF(4,4),QSize(8,8)); 
					p->thePainter.fillRect(R,QColor(255,0,0,128)); 
				}
				
				QRectF R(P-QPointF(2,2),QSize(4,4));
				p->thePainter.fillRect(R,QColor(0,0,0,128));
			}
		}
	}
}

void EPLabelLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPLabelLayer::draw(Road* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawLabel(R,p->thePainter,p->theProjection);
}

void EPLabelLayer::draw(Relation* /* R */)
{
}

void EPLabelLayer::draw(TrackPoint* Pt)
{
	const FeaturePainter* paintsel = Pt->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawLabel(Pt,p->thePainter,p->theProjection);
}

/* EDITPAINTSTYLE */

EditPaintStyle::EditPaintStyle()
	: p(0)
{
}

EditPaintStyle::~EditPaintStyle(void)
{
	delete p;
}

void EditPaintStyle::initialize(QPainter& P, const Projection& theProjection)
{
	if (p) {
		Layers.clear();
		delete p;
	}

	p = new EditPaintStylePrivate(P,theProjection);
	add(&p->First);
	add(&p->Second);
	add(&p->Third);
	add(&p->Fourth);

	p->isTrackPointVisible = M_PREFS->getTrackPointsVisible();
	p->isTrackSegmentVisible = M_PREFS->getTrackSegmentsVisible();
}

void EditPaintStyle::savePainters(const QString& filename)
{
	QFile data(filename);
	if (data.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out(&data);
		out << "<mapStyle>\n";
		out << globalPainter.toXML();
		for (unsigned int i=0; i<Painters.size(); ++i)
		{
			QString s = Painters[i].toXML();
			out << s;
		}
		out << "</mapStyle>\n";
	}
}

void EditPaintStyle::loadPainters(const QString& filename)
{
	QDomDocument doc;
	QFile file(filename);
	if (!file.open(QIODevice::ReadOnly))
		return;
	if (!doc.setContent(&file))
	{
		file.close();
		return;
	}
	file.close();
	GlobalPainter gp;
	globalPainter = gp;
	Painters.clear();
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	while(!n.isNull())
	{
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull() && e.tagName() == "global")
		{
			globalPainter = GlobalPainter::fromXML(e);
		} else
		if(!e.isNull() && e.tagName() == "painter")
		{
			FeaturePainter FP = FeaturePainter::fromXML(e);
			Painters.push_back(FP);
		}
		n = n.nextSibling();
	}
}

int EditPaintStyle::painterSize()
{
	return Painters.size();
}

const GlobalPainter& EditPaintStyle::getGlobalPainter() const
{
	return globalPainter;
}

void EditPaintStyle::setGlobalPainter(GlobalPainter aGlobalPainter)
{
	globalPainter = aGlobalPainter;
}

const FeaturePainter* EditPaintStyle::getPainter(int i) const
{
	return &(Painters[i]);
}

QVector<FeaturePainter> EditPaintStyle::getPainters() const
{
	return Painters;
}

void EditPaintStyle::setPainters(QVector<FeaturePainter> aPainters)
{
	Painters = aPainters;
}


