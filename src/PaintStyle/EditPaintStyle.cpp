#include "MapView.h"
#include "PaintStyle/EditPaintStyle.h"
#include "Maps/Painting.h"
#include "Maps/Projection.h"
#include "Features.h"
#include "Layer.h"
#include "ImageMapLayer.h"
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

static bool regionalZoom(const MapView& theView)
{
	return theView.pixelPerM() < REGIONALZOOM;
}

//static bool globalZoom(const MapView& theView)
//{
//	return theView.pixelPerM() < GLOBALZOOM;
//}

class EditPaintStylePrivate
{
	public:
		EditPaintStylePrivate(QPainter& P, MapView& aView)
			: thePainter(P), theView(aView)
		{
			bgLayer.setP(this);
			fgLayer.setP(this);
			tchLayer.setP(this);
			lblLayer.setP(this);
		}

		QPainter& thePainter;
		MapView& theView;
		EPBackgroundLayer bgLayer;
		EPForegroundLayer fgLayer;
		EPTouchupLayer tchLayer;
		EPLabelLayer lblLayer;
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


void EPBackgroundLayer::draw(Way* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel) {
		paintsel->drawBackground(R,p->thePainter,p->theView);
		return;
	}
	for (int i=0; i<R->sizeParents(); ++i) {
		if ((paintsel = R->getParent(i)->getEditPainter(p->theView.pixelPerM())))
			return;
	}
	if (/*!globalZoom(p->theProjection) && */!R->hasEditPainter()) //FIXME Untagged roads level of zoom?
	{
		QPen thePen(QColor(0,0,0),1);

		p->thePainter.setBrush(Qt::NoBrush);
		if (dynamic_cast<ImageMapLayer*>(R->layer()) && M_PREFS->getUseShapefileForBackground()) {
			thePen = QPen(QColor(0xc0,0xc0,0xc0),1);
			if (!R->isCoastline()) {
				if (M_PREFS->getBackgroundOverwriteStyle() || !M_STYLE->getGlobalPainter().getDrawBackground())
					p->thePainter.setBrush(M_PREFS->getBgColor());
				else
					p->thePainter.setBrush(QBrush(M_STYLE->getGlobalPainter().getBackgroundColor()));
			}
		} else {
			if (regionalZoom(p->theView))
				thePen = QPen(QColor(0x77,0x77,0x77),1);
		}

		p->thePainter.setPen(thePen);
		p->thePainter.drawPath(p->theView.transform().map(R->getPath()));
	}
}

void EPBackgroundLayer::draw(Relation* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawBackground(R,p->thePainter,p->theView);
}


void EPBackgroundLayer::draw(Node*)
{
}

void EPForegroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPForegroundLayer::draw(Way* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theView);
}

void EPForegroundLayer::draw(Relation* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theView);
}

void EPForegroundLayer::draw(Node*)
{
}

EPTouchupLayer::EPTouchupLayer()
{
}

void EPTouchupLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPTouchupLayer::draw(Way* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(R,p->thePainter,p->theView);
	else {
		if ( M_PREFS->getDirectionalArrowsVisible() != DirectionalArrows_Never )
		{
			Feature::TrafficDirectionType TT = trafficDirection(R);
			if ( (TT != Feature::UnknownDirection) || (M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Always) )
			{
				double theWidth = p->theView.pixelPerM()*R->widthOf()-4;
				if (theWidth > 8)
					theWidth = 8;
				double DistFromCenter = 2*(theWidth+4);
				if (theWidth > 0)
				{
					for (int i=1; i<R->size(); ++i)
					{
						QPointF FromF(p->theView.transform().map(p->theView.projection().project(R->getNode(i-1))));
						QPointF ToF(p->theView.transform().map(p->theView.projection().project(R->getNode(i))));
						if (distance(FromF,ToF) > (DistFromCenter*2+4))
						{
							QPointF H(FromF+ToF);
							H *= 0.5;
							double A = angle(FromF-ToF);
							QPointF T(DistFromCenter*cos(A),DistFromCenter*sin(A));
							QPointF V1(theWidth*cos(A+M_PI/6),theWidth*sin(A+M_PI/6));
							QPointF V2(theWidth*cos(A-M_PI/6),theWidth*sin(A-M_PI/6));
							if ( (TT == Feature::OtherWay) || (TT == Feature::BothWays) )
							{
								p->thePainter.setPen(QPen(QColor(0,0,255), 2));
								p->thePainter.drawLine(H+T,H+T-V1);
								p->thePainter.drawLine(H+T,H+T-V2);
							}
							if ( (TT == Feature::OneWay) || (TT == Feature::BothWays) )
							{
								p->thePainter.setPen(QPen(QColor(0,0,255), 2));
								p->thePainter.drawLine(H-T,H-T+V1);
								p->thePainter.drawLine(H-T,H-T+V2);
							}
							else
							{
								if ( M_PREFS->getDirectionalArrowsVisible() == DirectionalArrows_Always )
								{
									p->thePainter.setPen(QPen(QColor(255,0,0), 2));
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
}

void EPTouchupLayer::draw(Relation* /* R */)
{
}

void EPTouchupLayer::draw(Node* Pt)
{
	const FeaturePainter* paintsel = Pt->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(Pt,p->thePainter,p->theView);
	else if (!Pt->hasEditPainter()) {
		if (p->isTrackPointVisible || (Pt->lastUpdated() == Feature::Log && !p->isTrackSegmentVisible)) {
			bool Draw = p->theView.pixelPerM() > 1;
			// Do not draw GPX nodes when simple GPX track appearance is enabled
			if (M_PREFS->getSimpleGpxTrack() && Pt->layer()->isTrack())
				Draw = false;
			if (!Draw) {
				if (!Pt->sizeParents() && (p->theView.pixelPerM() > LOCALZOOM) )
					Draw = true;
				else if (Pt->lastUpdated() == Feature::Log && !p->isTrackSegmentVisible)
					Draw = true;
			}
			if (Draw)
			{
				QPoint P = p->theView.transform().map(p->theView.projection().project(Pt)).toPoint();

				if (Pt->isVirtual()) {
					if (M_PREFS->getVirtualNodesVisible()) {
						p->thePainter.save();
						p->thePainter.setPen(QColor(0,0,0));
						p->thePainter.drawLine(P+QPoint(-3, -3), P+QPoint(3, 3));
						p->thePainter.drawLine(P+QPoint(3, -3), P+QPoint(-3, 3));
						p->thePainter.restore();
					}
				} else {
					if (Pt->isWaypoint()) {
						QRect R2(P-QPoint(4,4),QSize(8,8));
						p->thePainter.fillRect(R2,QColor(255,0,0,128));
					}

					QRect R(P-QPoint(3,3),QSize(6,6));
					p->thePainter.fillRect(R,QColor(0,0,0,128));
				}
			}
		}
	}
}

void EPLabelLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}

void EPLabelLayer::draw(Way* R)
{
	const FeaturePainter* paintsel = R->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawLabel(R,p->thePainter,p->theView);
}

void EPLabelLayer::draw(Relation* /* R */)
{
}

void EPLabelLayer::draw(Node* Pt)
{
	const FeaturePainter* paintsel = Pt->getEditPainter(p->theView.pixelPerM());
	if (paintsel)
		paintsel->drawLabel(Pt,p->thePainter,p->theView);
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

void EditPaintStyle::initialize(QPainter& P, MapView& theView)
{
	if (p) {
		Layers.clear();
		delete p;
	}

	p = new EditPaintStylePrivate(P,theView);
	if (M_PREFS->getStyleBackgroundVisible())
		add(&p->bgLayer);
	if (M_PREFS->getStyleForegroundVisible())
		add(&p->fgLayer);
	if (M_PREFS->getStyleTouchupVisible())
		add(&p->tchLayer);
	if (M_PREFS->getNamesVisible())
		add(&p->lblLayer);

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
		for (int i=0; i<Painters.size(); ++i)
		{
			QString s = Painters[i].toXML(filename);
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
			FeaturePainter FP = FeaturePainter::fromXML(e, filename);
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

QList<FeaturePainter> EditPaintStyle::getPainters() const
{
	return Painters;
}

void EditPaintStyle::setPainters(QList<FeaturePainter> aPainters)
{
	Painters = aPainters;
}


