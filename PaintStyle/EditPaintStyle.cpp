#include "PaintStyle/EditPaintStyle.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>

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
			: thePainter(P), theProjection(aProj), CurrentZoomLevel(FeaturePainter::NoZoomLimit)
		{
			First.setP(this);
			Second.setP(this);
			Third.setP(this);
			initPainters();
			double PM = theProjection.pixelPerM();
			if (PM > LOCALZOOM)
				CurrentZoomLevel = FeaturePainter::LocalZoom;
			else if (PM > REGIONALZOOM)
				CurrentZoomLevel = FeaturePainter::RegionalZoom;
			else if (PM > GLOBALZOOM)
				CurrentZoomLevel = FeaturePainter::GlobalZoom;
			else
				CurrentZoomLevel = FeaturePainter::NoZoomLimit;
		}

		void initPainters();

		QPainter& thePainter;
		const Projection& theProjection;
		EPBackgroundLayer First;
		EPForegroundLayer Second;
		EPTouchupLayer Third;
		FeaturePainter::ZoomType CurrentZoomLevel;
};

void EditPaintStylePrivate::initPainters()
{
	if (EditPaintStyle::Painters.size()) return;

	FeaturePainter MotorWay;
	MotorWay.background(QColor(0xff,0,0),1,6).foreground(QColor(0xff,0xff,0),0.5,2);
	MotorWay.selectOnTag("highway","motorway","motorway_link");
	MotorWay.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(MotorWay);

	FeaturePainter Trunk;
	Trunk.foreground(QColor(0xff,0,0),1,4);
	Trunk.selectOnTag("highway","trunk","trunk_link");
	Trunk.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Trunk);

	FeaturePainter Primary;
	Primary.foreground(QColor(0,0xff,0),1,3);
	Primary.selectOnTag("highway","primary","primary_link").limitToZoom(FeaturePainter::GlobalZoom);
	Primary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Primary);

	FeaturePainter Secondary;
	Secondary.foreground(QColor(0xff,0xaa,0),1,2);
	Secondary.selectOnTag("highway","secondary","secondary_link").limitToZoom(FeaturePainter::RegionalZoom);
	Secondary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Secondary);

	FeaturePainter Tertiary;
	Tertiary.foreground(QColor(0xff,0x55,0x55),1,1.5);
	Tertiary.selectOnTag("highway","tertiary","tertiary_link").limitToZoom(FeaturePainter::RegionalZoom);
	Tertiary.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Tertiary);

	FeaturePainter Cycleway;
	Cycleway.foreground(QColor(0,0,0xff),1,0).foregroundDash(2,2);
	Cycleway.selectOnTag("highway","cycleway").limitToZoom(FeaturePainter::LocalZoom);
	EditPaintStyle::Painters.push_back(Cycleway);

	FeaturePainter Footway;
	Footway.foreground(QColor(0,0,0),1,0).foregroundDash(2,2);
	Footway.selectOnTag("highway","footway","track").limitToZoom(FeaturePainter::LocalZoom);
	EditPaintStyle::Painters.push_back(Footway);

	FeaturePainter Pedestrian;
	Pedestrian.foreground(QColor(0xaa,0xaa,0xaa),1,0);
	Pedestrian.selectOnTag("highway","pedestrian").limitToZoom(FeaturePainter::LocalZoom);
	EditPaintStyle::Painters.push_back(Pedestrian);

	FeaturePainter Residential;
	Residential.background(QColor(0x77,0x77,0x77),1,1).foreground(QColor(0xff,0xff,0xff),1,-1);
	Residential.selectOnTag("highway","residential","unclassified").limitToZoom(FeaturePainter::LocalZoom);
	Residential.drawTrafficDirectionMarks();
	EditPaintStyle::Painters.push_back(Residential);

	FeaturePainter Railway;
	Railway.background(QColor(0,0,0),1,4).foreground(QColor(0xff,0xff,0xff),1,2).touchup(QColor(0,0,0),1,2).touchupDash(3,3);
	Railway.selectOnTag("railway","rail").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Railway);

	FeaturePainter Park;
	Park.foregroundFill(QColor(0x77,0xff,0x77,0x77)).foreground(QColor(0,0x77,0),0,1);
	Park.selectOnTag("leisure","park").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Park);

	FeaturePainter Wood;
	Wood.foregroundFill(QColor(0x22,0x99,0x22,0x77)).foreground(QColor(0,0x77,0),0,1);
	Wood.selectOnTag("natural","wood").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Wood);

	FeaturePainter Forest;
	Forest.foregroundFill(QColor(0x22,0xcc,0x22,0x77)).foreground(QColor(0,0x77,0),0,1);
	Forest.selectOnTag("landuse","forest").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Forest);

	FeaturePainter Scrub;
	Scrub.foregroundFill(QColor(0xBC,0xCF,0x8F,0x77)).foreground(QColor(0,0x77,0),0,1);
	Scrub.selectOnTag("natural","scrub").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Scrub);

	FeaturePainter Heath;
	Heath.foregroundFill(QColor(0x55,0x99,0x44,0x77)).foreground(QColor(0,0x77,0),0,1);
	Heath.selectOnTag("natural","scrub").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Heath);

	FeaturePainter Pitch;
	Pitch.foregroundFill(QColor(0xff,0x77,0x77,0x77)).foreground(QColor(0x77,0,0),0,1);
	Pitch.selectOnTag("leisure","pitch").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Pitch);

	FeaturePainter Water;
	Water.foregroundFill(QColor(0x77,0x77,0xff,0x77)).foreground(QColor(0,0,0x77),0,1);
	Water.selectOnTag("natural","water").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(Water);

	FeaturePainter Parking;
	Parking.trackPointIcon(":/Art/Mapnik/parking.png").limitToZoom(FeaturePainter::LocalZoom);
	Parking.foregroundFill(QColor(0xf6,0xee,0xb6,0x77));
	Parking.selectOnTag("amenity","parking");
	Parking.limitToZoom(FeaturePainter::LocalZoom);
	EditPaintStyle::Painters.push_back(Parking);

	FeaturePainter School;
	//School.trackPointIcon(":/Art/Mapnik/school.png").limitToZoom(FeaturePainter::LocalZoom);
	School.foregroundFill(QColor(0xF0,0xF0,0xD9,0x77)).foreground(QColor(0xDB,0xBA,0xAB),0,1);
	School.selectOnTag("amenity","school");
	School.limitToZoom(FeaturePainter::LocalZoom);
	EditPaintStyle::Painters.push_back(School);

	FeaturePainter LanduseFarm;
	LanduseFarm.foregroundFill(QColor(0xBB,0xE1,0xC9,0x77));
	LanduseFarm.selectOnTag("landuse","farm").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseFarm);

	FeaturePainter LanduseForest;
	LanduseForest.foregroundFill(QColor(0x70,0xBD,0x7E,0x77));
	LanduseForest.selectOnTag("landuse","forest").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseForest);

	FeaturePainter RecreationGround;
	RecreationGround.foregroundFill(QColor(0xD0,0xEB,0xA9,0x77));
	RecreationGround.selectOnTag("landuse","recreation_ground").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(RecreationGround);

	FeaturePainter VillageGreen;
	VillageGreen.foregroundFill(QColor(0xD0,0xEB,0xA9,0x77));
	VillageGreen.selectOnTag("landuse","village_green").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(VillageGreen);

	FeaturePainter LanduseResidential;
	LanduseResidential.foregroundFill(QColor(0xDC,0xDC,0xDC,0x55));
	LanduseResidential.selectOnTag("landuse","residential").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseResidential);

	FeaturePainter LanduseIndustrial;
	LanduseIndustrial.foregroundFill(QColor(0xFE,0xAD,0xBA,0x55));
	LanduseIndustrial.selectOnTag("landuse","industrial").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseIndustrial);

	FeaturePainter LanduseRetail;
	LanduseRetail.foregroundFill(QColor(0xF4,0x98,0x96,0x55));
	LanduseRetail.selectOnTag("landuse","retail").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseRetail);

	FeaturePainter LanduseCommercial;
	LanduseCommercial.foregroundFill(QColor(0xFB,0xFD,0xC8,0x55));
	LanduseCommercial.selectOnTag("landuse","commercial").limitToZoom(FeaturePainter::GlobalZoom);
	EditPaintStyle::Painters.push_back(LanduseCommercial);
}

void EPBackgroundLayer::setP(EditPaintStylePrivate* ap)
{
	p = ap;
}


void EPBackgroundLayer::draw(Road* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->CurrentZoomLevel);
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
	FeaturePainter* paintsel = R->getEditPainter(p->CurrentZoomLevel);
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
	FeaturePainter* paintsel = R->getEditPainter(p->CurrentZoomLevel);
	if (paintsel)
		paintsel->drawForeground(R,p->thePainter,p->theProjection);
}

void EPForegroundLayer::draw(Relation* R)
{
	if (p->theProjection.viewport().disjunctFrom(R->boundingBox())) return;
	FeaturePainter* paintsel = R->getEditPainter(p->CurrentZoomLevel);
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
	FeaturePainter* paintsel = R->getEditPainter(p->CurrentZoomLevel);
	if (paintsel)
		paintsel->drawTouchup(R,p->thePainter,p->theProjection);
}

void EPTouchupLayer::draw(Relation* R)
{
}

void EPTouchupLayer::draw(TrackPoint* Pt)
{
	if (p->theProjection.viewport().disjunctFrom(Pt->boundingBox())) return;
	FeaturePainter* paintsel = Pt->getEditPainter(p->CurrentZoomLevel);
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

FeaturePainter::ZoomType EditPaintStyle::zoom() const
{
	return p->CurrentZoomLevel;
}
