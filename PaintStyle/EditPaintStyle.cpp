#include "PaintStyle/EditPaintStyle.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Map/Road.h"
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

static bool localZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < LOCALZOOM;
}

static bool regionalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < REGIONALZOOM;
}

static bool globalZoom(const Projection& theProjection)
{
	return theProjection.pixelPerM() < GLOBALZOOM;
}


/* FEATUREPAINTSELECTOR */

class EPBackgroundLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EPForegroundLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EPTouchupLayer : public PaintStyleLayer
{
	public:
		void setP(EditPaintStylePrivate* p);
		virtual void draw(Road* R);
		virtual void draw(TrackPoint* Pt);
		virtual void draw(Relation* R);
	private:
		EditPaintStylePrivate* p;
};

class EditPaintStylePrivate
{
	public:
		EditPaintStylePrivate(QPainter& P, const Projection& aProj)
			: thePainter(P), theProjection(aProj)
		{
			First.setP(this);
			Second.setP(this);
			Third.setP(this);
			initPainters();
		}

		void initPainters();

		QPainter& thePainter;
		const Projection& theProjection;
		EPBackgroundLayer First;
		EPForegroundLayer Second;
		EPTouchupLayer Third;
};

#define ALWAYS 10e6

/* Zoom boundaries : expressed in Pixel per Meter

   eg 0.01->ALWAYS means show a feature from a zoom level of 0.01 Pixel Per M,
   or 100 Meter per Pixel. For a screen of 1000px wide this is when viewing
   100km or less across.

   eg 0.2->ALWAYS means show a feature from a zoom level 0.2 Px/M or 5M/Px which
   is viewing 5km or less across a screen of 1000Px. */

void EditPaintStylePrivate::initPainters()
{
	if (EditPaintStyle::Painters.size()) return;

	FeaturePainter MotorWay;
	MotorWay.background(QColor(0xff,0,0),1,6).foreground(QColor(0xff,0xff,0),0.5,2);
	MotorWay.selectOnTag("highway","motorway","motorway_link");
	MotorWay.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(MotorWay);

	// Highways
	FeaturePainter Trunk;
	Trunk.foreground(QColor(0xff,0,0),1,4);
	Trunk.selectOnTag("highway","trunk","trunk_link");
	Trunk.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Trunk);

	FeaturePainter Primary;
	Primary.foreground(QColor(0,0xff,0),1,3);
	Primary.selectOnTag("highway","primary","primary_link").zoomBoundary(0.01,ALWAYS);
	Primary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Primary);

	FeaturePainter Secondary;
	Secondary.foreground(QColor(0xff,0xaa,0),1,2);
	Secondary.selectOnTag("highway","secondary","secondary_link").zoomBoundary(0.02,ALWAYS);
	Secondary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Secondary);

	FeaturePainter Tertiary;
	Tertiary.foreground(QColor(0xff,0x55,0x55),1,1.5);
	Tertiary.selectOnTag("highway","tertiary","tertiary_link").zoomBoundary(0.05,ALWAYS);
	Tertiary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Tertiary);

	FeaturePainter Residential;
	Residential.background(QColor(0x77,0x77,0x77),1,1).foreground(QColor(0xff,0xff,0xff),1,-1);
	Residential.selectOnTag("highway","residential","unclassified").zoomBoundary(0.1,ALWAYS);
	Residential.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Residential);

	FeaturePainter Cycleway;
	Cycleway.foreground(QColor(0,0,0xff),1,0).foregroundDash(2,2);
	Cycleway.selectOnTag("highway","cycleway").zoomBoundary(0.2,ALWAYS);
	EditPaintStyle::Painters.push_back(Cycleway);

	FeaturePainter Footway;
	Footway.foreground(QColor(0,0,0),1,0).foregroundDash(2,2);
	Footway.selectOnTag("highway","footway","track").zoomBoundary(0.2,ALWAYS);
	EditPaintStyle::Painters.push_back(Footway);

	FeaturePainter Pedestrian;
	Pedestrian.foreground(QColor(0xaa,0xaa,0xaa),1,0);
	Pedestrian.selectOnTag("highway","pedestrian").zoomBoundary(0.1,ALWAYS);
	EditPaintStyle::Painters.push_back(Pedestrian);

	FeaturePainter Railway;
	Railway.background(QColor(0,0,0),1,4).foreground(QColor(0xff,0xff,0xff),1,2).touchup(QColor(0,0,0),1,2).touchupDash(3,3);
	Railway.selectOnTag("railway","rail").zoomBoundary(0.01,ALWAYS);
	EditPaintStyle::Painters.push_back(Railway);

	// leasure areas
	FeaturePainter Park;
	Park.foregroundFill(QColor(0x77,0xff,0x77,0x77)).foreground(QColor(0,0x77,0),0,1);
	Park.selectOnTag("leisure","park").zoomBoundary(0.02,ALWAYS);
	EditPaintStyle::Painters.push_back(Park);

	FeaturePainter Pitch;
	Pitch.foregroundFill(QColor(0xff,0x77,0x77,0x77)).foreground(QColor(0x77,0,0),0,1);
	Pitch.selectOnTag("leisure","pitch").zoomBoundary(0.02,ALWAYS);
	EditPaintStyle::Painters.push_back(Pitch);

	// natural areas
	FeaturePainter Wood;
	Wood.foregroundFill(QColor(0x22,0x99,0x22,0x77)).foreground(QColor(0,0x77,0),0,1);
	Wood.selectOnTag("natural","wood").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(Wood);

	FeaturePainter Scrub;
	Scrub.foregroundFill(QColor(0xBC,0xCF,0x8F,0x77)).foreground(QColor(0,0x77,0),0,1);
	Scrub.selectOnTag("natural","scrub").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(Scrub);

	FeaturePainter Heath;
	Heath.foregroundFill(QColor(0x55,0x99,0x44,0x77)).foreground(QColor(0,0x77,0),0,1);
	Heath.selectOnTag("natural","heath").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(Heath);

	FeaturePainter Water;
	Water.foregroundFill(QColor(0x77,0x77,0xff,0x77)).foreground(QColor(0,0,0x77),0,1);
	Water.selectOnTag("natural","water").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(Water);

	// Point amenities
	FeaturePainter Parking;
	Parking.trackPointIcon(":/Art/Mapnik/parking.png").zoomBoundary(0.03,ALWAYS);
	Parking.foregroundFill(QColor(0xf6,0xee,0xb6,0x77));
	Parking.selectOnTag("amenity","parking");
	EditPaintStyle::Painters.push_back(Parking);

	FeaturePainter Hospital;
	Hospital.trackPointIcon(":/Art/Mapnik/hospital.png").zoomBoundary(0.01,ALWAYS);
	Hospital.foregroundFill(QColor(0xf6,0x66,0x66,0x77));
	Hospital.selectOnTag("amenity","hospital");
	EditPaintStyle::Painters.push_back(Hospital);

	FeaturePainter School;
	//School.trackPointIcon(":/Art/Mapnik/school.png");
	School.foregroundFill(QColor(0xF0,0xF0,0xD9,0x77)).foreground(QColor(0xDB,0xBA,0xAB),0,1);
	School.selectOnTag("amenity","school");
	School.zoomBoundary(0.02,ALWAYS);
	EditPaintStyle::Painters.push_back(School);

	// Landuse
	FeaturePainter LanduseFarm;
	LanduseFarm.foregroundFill(QColor(0xBB,0xE1,0xC9,0x77));
	LanduseFarm.selectOnTag("landuse","farm").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(LanduseFarm);

	FeaturePainter Forest;
	Forest.foregroundFill(QColor(0x22,0xcc,0x22,0x77)).foreground(QColor(0,0x77,0),0,1);
	Forest.selectOnTag("landuse","forest").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(Forest);

	FeaturePainter RecreationGround;
	RecreationGround.foregroundFill(QColor(0xD0,0xEB,0xA9,0x77));
	RecreationGround.selectOnTag("landuse","recreation_ground").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(RecreationGround);

	FeaturePainter VillageGreen;
	VillageGreen.foregroundFill(QColor(0xD0,0xEB,0xA9,0x77));
	VillageGreen.selectOnTag("landuse","village_green").zoomBoundary(0.01,ALWAYS);
	EditPaintStyle::Painters.push_back(VillageGreen);

	FeaturePainter LanduseResidential;
	LanduseResidential.foregroundFill(QColor(0xDC,0xDC,0xDC,0x55));
	LanduseResidential.selectOnTag("landuse","residential").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(LanduseResidential);

	FeaturePainter LanduseIndustrial;
	LanduseIndustrial.foregroundFill(QColor(0xFE,0xAD,0xBA,0x55));
	LanduseIndustrial.selectOnTag("landuse","industrial").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(LanduseIndustrial);

	FeaturePainter LanduseRetail;
	LanduseRetail.foregroundFill(QColor(0xF4,0x98,0x96,0x55));
	LanduseRetail.selectOnTag("landuse","retail").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(LanduseRetail);

	FeaturePainter LanduseCommercial;
	LanduseCommercial.foregroundFill(QColor(0xFB,0xFD,0xC8,0x55));
	LanduseCommercial.selectOnTag("landuse","commercial").zoomBoundary(0.002,ALWAYS);
	EditPaintStyle::Painters.push_back(LanduseCommercial);
}

void EPBackgroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}


void EPBackgroundLayer::draw(Road* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawBackground(R,p->thePainter,p->theProjection);
	else if (!globalZoom(p->theProjection) && !R->hasEditPainter())
	{
		QPen thePen(QColor(0,0,0),1);
		if (regionalZoom(p->theProjection))
			thePen = QPen(QColor(0x77,0x77,0x77),1);
		QPainterPath Path;
		buildPathFromRoad(R, p->theProjection, Path);
		p->thePainter.strokePath(Path,thePen);
	}
}

void EPBackgroundLayer::draw(Relation* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
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
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theProjection);
}

void EPForegroundLayer::draw(Relation* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
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
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(R,p->thePainter,p->theProjection);
}

void EPTouchupLayer::draw(Relation* R)
{
}

void EPTouchupLayer::draw(TrackPoint* Pt)
{
	if (p->theProjection.viewport().disjunctFrom(Pt->boundingBox())) return;
	FeaturePainter* paintsel = Pt->getEditPainter(p->theProjection.pixelPerM());
	if (paintsel)
		paintsel->drawTouchup(Pt,p->thePainter,p->theProjection);
	else if (!Pt->hasEditPainter())
	{
		bool Draw = p->theProjection.pixelPerM() > 1;
		if (!Draw && !Pt->sizeParents() && (p->theProjection.pixelPerM() > LOCALZOOM) )
			Draw = true;
		if (Draw)
		{
			QPointF P(p->theProjection.project(Pt->position()));
			QRectF R(P-QPointF(2,2),QSize(4,4));
			p->thePainter.fillRect(R,QColor(0,0,0,128));
		}
	}
}

/* EDITPAINTSTYLE */

std::vector<FeaturePainter> EditPaintStyle::Painters;

EditPaintStyle::EditPaintStyle(QPainter& P, const Projection& theProjection)
{
	p = new EditPaintStylePrivate(P,theProjection);
	add(&p->First);
	add(&p->Second);
	add(&p->Third);
}

EditPaintStyle::~EditPaintStyle(void)
{
	delete p;
}



void savePainters(const QString& filename)
{
	QFile data(filename);
	if (data.open(QFile::WriteOnly | QFile::Truncate))
	{
		QTextStream out(&data);
		out << "<mapStyle>\n";
		for (unsigned int i=0; i<EditPaintStyle::Painters.size(); ++i)
		{
			QString s = EditPaintStyle::Painters[i].asXML();
			out << s;
		}
		out << "</mapStyle>\n";
	}
}

QColor toColor(const QString& s)
{
	return
		QColor(
			s.mid(1,2).toInt(0,16),
			s.mid(3,2).toInt(0,16),
			s.mid(5,2).toInt(0,16),
			s.mid(7,2).toInt(0,16));
}

void readFromNode(const QDomElement& e, FeaturePainter& FP)
{
	if (e.hasAttribute("zoomUnder") || e.hasAttribute("zoomPpper"))
		FP.zoomBoundary(e.attribute("zoomUnder","0").toDouble(),e.attribute("zoomUpper","10e6").toDouble());
	if (e.hasAttribute("foregroundColor"))
	{
		FP.foreground(
			toColor(e.attribute("foregroundColor")),e.attribute("foregroundScale").toDouble(),e.attribute("foregroundOffset").toDouble());
		if (e.hasAttribute("foregroundDashDown"))
			FP.foregroundDash(e.attribute("foregroundDashDown").toDouble(),e.attribute("foregroundDashUp").toDouble());
	}
	if (e.hasAttribute("backgroundColor"))
		FP.background(
			toColor(e.attribute("backgroundColor")),e.attribute("backgroundScale").toDouble(),e.attribute("backgroundOffset").toDouble());
	if (e.hasAttribute("touchupColor"))
	{
		FP.touchup(
			toColor(e.attribute("touchupColor")),e.attribute("touchupScale").toDouble(),e.attribute("touchupOffset").toDouble());
		if (e.hasAttribute("touchupDashDown"))
			FP.touchupDash(e.attribute("touchupDashDown").toDouble(),e.attribute("touchupDashUp").toDouble());
	}
	if (e.hasAttribute("fillColor"))
		FP.foregroundFill(toColor(e.attribute("fillColor")));
	if (e.hasAttribute("icon"))
		FP.trackPointIcon(e.attribute("icon"));
	if (e.attribute("drawTrafficDirectionMarks") == "yes")
		FP.drawTrafficDirectionMarks();
	QDomNode n = e.firstChild();
	while (!n.isNull())
	{
		if (n.isElement())
		{
			QDomElement t = n.toElement();
			if (t.tagName() == "selector")
				FP.selectOnTag(t.attribute("key"),t.attribute("value"));
		}
		n = n.nextSibling();
	}
}

void loadPainters(const QString& filename)
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
	EditPaintStyle::Painters.clear();
	QDomElement docElem = doc.documentElement();
	QDomNode n = docElem.firstChild();
	while(!n.isNull())
	{
		QDomElement e = n.toElement(); // try to convert the node to an element.
		if(!e.isNull() && e.tagName() == "painter")
		{
			FeaturePainter FP;
			readFromNode(e,FP);
			EditPaintStyle::Painters.push_back(FP);
		}
		n = n.nextSibling();
	}
}