#include "Maps/Road.h"

#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Maps/Coord.h"
#include "Maps/Painting.h"
#include "MapView.h"
#include "Maps/TrackPoint.h"
#include "Utils/LineF.h"
#include "Utils/MDiscardableDialog.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QMessageBox>
#include <QProgressDialog>

#include <algorithm>
#include <QList>

#ifndef _MOBILE
#include <ggl/ggl.hpp>
#include <ggl/geometries/cartesian2d.hpp>

#include <ggl/geometries/adapted/std_as_linestring.hpp>
#endif


class RoadPrivate
{
	public:
		RoadPrivate()
		: SmoothedUpToDate(false), BBox(Coord(0,0),Coord(0,0)), BBoxUpToDate(false), Area(0), Distance(0), Width(0),
			MetaUpToDate(false)
		{
		}
		std::vector<TrackPointPtr> Nodes;
 		QList<Coord> Smoothed;
 		bool SmoothedUpToDate;
		CoordBox BBox;
		bool BBoxUpToDate;

		bool IsCoastline;
		double Area;
		double Distance;
		double Width;
        bool NotEverythingDownloaded;
		bool MetaUpToDate;
		RenderPriority theRenderPriority;
		QPainterPath thePath;

		void updateSmoothed(bool DoSmooth);
		void addSmoothedBezier(int i, int j, int k, int l);
};

void RoadPrivate::addSmoothedBezier(int i, int j, int k, int l)
{
	Coord A(Nodes[i]->position());
	Coord B(Nodes[j]->position());
	Coord C(Nodes[k]->position());
	Coord D(Nodes[l]->position());

	Coord Ctrl1(B+(C-A)*(1/6));
	Coord Ctrl2(C-(D-B)*(1/6));


	Smoothed.push_back(Ctrl1);
	Smoothed.push_back(Ctrl2);
	Smoothed.push_back(C);
}

void RoadPrivate::updateSmoothed(bool DoSmooth)
{
	SmoothedUpToDate = true;
	Smoothed.clear();
	if ( (Nodes.size() < 3) || !DoSmooth )
		return;
	Smoothed.push_back(Nodes[0]->position());
	addSmoothedBezier(0,0,1,2);
	for (unsigned int i=1; i<Nodes.size()-2; ++i)
		addSmoothedBezier(i-1,i,i+1,i+2);
	int Last = Nodes.size()-1;
	addSmoothedBezier(Last-2,Last-1,Last,Last);
}

Road::Road(void)
: p(new RoadPrivate)
{
}

Road::Road(const Road& other)
: MapFeature(other), p(new RoadPrivate)
{
}

Road::~Road(void)
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i])
			p->Nodes[i]->unsetParentFeature(this);
	delete p;
}

void Road::setLayer(MapLayer* L)
{
	if (L)
		for (unsigned int i=0; i<p->Nodes.size(); ++i)
			if (p->Nodes[i])
				p->Nodes[i]->setParentFeature(this);
	else
		for (unsigned int i=0; i<p->Nodes.size(); ++i)
			if (p->Nodes[i])
				p->Nodes[i]->unsetParentFeature(this);
	MapFeature::setLayer(L);
}

void Road::partChanged(MapFeature*, int ChangeId)
{
	if (isDeleted())
		return;

	if (layer())
		layer()->getRTree()->remove(p->BBox, this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
	if (layer()) {
		CoordBox bb = boundingBox();
		layer()->getRTree()->insert(bb, this);
	}
	notifyParents(ChangeId);
}

QString Road::description() const
{
	QString s(tagValue("name",""));
	if (!s.isEmpty())
		return QString("%1 (%2)").arg(s).arg(id());
	return QString("%1").arg(id());
}

RenderPriority Road::renderPriority() 
{
	// FIWME Segments of a road with different layers are wrongly painted (rounded corners)
	return p->theRenderPriority;
}

RenderPriority Road::getRenderPriority()
{
	return p->theRenderPriority;
}

void Road::add(TrackPoint* Pt)
{
	p->Nodes.push_back(Pt);
	Pt->setParentFeature(this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

void Road::add(TrackPoint* Pt, int Idx)
{
	p->Nodes.push_back(Pt);
	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
	Pt->setParentFeature(this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

int Road::find(MapFeature* Pt) const
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == Pt)
			return i;
	return p->Nodes.size();
}

void Road::remove(int idx)
{
	if (p->Nodes[idx]) {
		TrackPoint* Pt = p->Nodes[idx];
		Pt->unsetParentFeature(this);
	}
	p->Nodes.erase(p->Nodes.begin()+idx);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

void Road::remove(MapFeature* F)
{
	for (int i=p->Nodes.size(); i; --i)
		if (p->Nodes[i-1] == F)
			remove(i-1);
}

int Road::size() const
{
	return p->Nodes.size();
}

TrackPoint* Road::getNode(int idx)
{
	return p->Nodes[idx];
}

const TrackPoint* Road::getNode(int idx) const
{
	return p->Nodes[idx];
}

const std::vector<TrackPointPtr>& Road::getNodes() const
{
	return p->Nodes;
}


MapFeature* Road::get(int idx)
{
	return p->Nodes[idx];
}

const MapFeature* Road::get(int idx) const
{
	return p->Nodes[idx];
}

bool Road::isNull() const
{
	return (p->Nodes.size() == 0);
}

bool Road::notEverythingDownloaded() const
{
   	if (p->MetaUpToDate == false)
		updateMeta();

	return p->NotEverythingDownloaded;
}

CoordBox Road::boundingBox() const
{
	if (!p->BBoxUpToDate)
	{
		if (p->Nodes.size())
		{
			p->BBox = CoordBox(p->Nodes[0]->position(),p->Nodes[0]->position());
			for (unsigned int i=1; i<p->Nodes.size(); ++i)
				p->BBox.merge(p->Nodes[i]->position());
		}
		else
			p->BBox = CoordBox(Coord(0,0),Coord(0,0));
		p->BBoxUpToDate = true;
	}
	return p->BBox;
}

void Road::updateMeta() const
{
	p->Area = 0;
	p->Distance = 0;
	p->IsCoastline = false;

	p->NotEverythingDownloaded = false;
	if (lastUpdated() == MapFeature::NotYetDownloaded)
		p->NotEverythingDownloaded = true;
	else
		for (unsigned int i=0; i<p->Nodes.size(); ++i)
			if (p->Nodes.at(i) && p->Nodes.at(i)->notEverythingDownloaded()) {
				p->NotEverythingDownloaded = true;
				break;
			}

	if (p->Nodes.size() == 0)
	{
		p->MetaUpToDate = true;
		return;
	}

	bool isArea = (p->Nodes[0] == p->Nodes[p->Nodes.size()-1]);

	for (unsigned int i=0; (i+1)<p->Nodes.size(); ++i)
	{
		if (p->Nodes[i] && p->Nodes[i+1]) {
			const Coord & here = p->Nodes[i]->position();
			const Coord & next = p->Nodes[i+1]->position();

			p->Distance += next.distanceFrom(here);
			//if (isArea)
				//p->Area += here.lat() * next.lon() - next.lat() * here.lon();
		}
	}

	if (isArea) {
		p->Area = p->Distance;
		p->theRenderPriority = RenderPriority(RenderPriority::IsArea,-fabs(p->Area));
	} else {
		qreal Priority = tagValue("layer","0").toInt();
		if (Priority >= 0)
			Priority++;
		// dummy number to get a deterministic feature sort
		Priority *= sin(intToRad(boundingBox().lonDiff()));
		p->theRenderPriority = RenderPriority(RenderPriority::IsLinear,Priority);
	}

	if (tagValue("natural","") == "coastline")
		p->IsCoastline = true;

    
	p->MetaUpToDate = true;
}

double Road::distance() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->Distance;
}

bool Road::isCoastline() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->IsCoastline;
}

bool Road::isClosed() const
{
	return (p->Area > 0.0);
}

double Road::area() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->Area;
}

void Road::draw(QPainter&, MapView* )
{
}

void Road::drawHover(QPainter& thePainter, MapView* theView, bool solid)
{
	// FIXME Selected route
	if (!size())
		return;

	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(MerkaartorPreferences::instance()->getHoverColor());
	TP.setWidth(MerkaartorPreferences::instance()->getHoverWidth());
	if (!solid) {
		TP.setDashPattern(getParentDashes());
	}
	thePainter.setPen(TP);
	thePainter.setBrush(Qt::NoBrush);
	//QRect clipRect = thePainter.clipRegion().boundingRect().adjusted(int(-20), int(-20), int(20), int(20));
	//buildPath(theProjection, clipRect);
	thePainter.drawPath(theView->transform().map(p->thePath));
	if (solid) {
		TP.setWidth(MerkaartorPreferences::instance()->getHoverWidth()*3);
		TP.setCapStyle(Qt::RoundCap);
		thePainter.setPen(TP);
		QPolygonF Pl;
		buildPolygonFromRoad(this,theView->projection(),Pl);
		thePainter.drawPoints(theView->transform().map(Pl));

		if (M_PREFS->getShowParents()) {
			for (int i=0; i<sizeParents(); ++i)
				if (!getParent(i)->isDeleted())
					getParent(i)->drawHover(thePainter, theView, false);
		}
	}
}

void Road::drawHighlight(QPainter& thePainter, MapView* theView, bool solid)
{
	// FIXME Selected route
	if (!size())
		return;

	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(MerkaartorPreferences::instance()->getHighlightColor());
	TP.setWidth(MerkaartorPreferences::instance()->getHighlightWidth());
	if (!solid) {
		TP.setDashPattern(getParentDashes());
	}
	thePainter.setPen(TP);
	thePainter.setBrush(Qt::NoBrush);
	//QRect clipRect = thePainter.clipRegion().boundingRect().adjusted(int(-20), int(-20), int(20), int(20));
	//buildPath(theProjection, clipRect);
	thePainter.drawPath(theView->transform().map(p->thePath));
	if (solid) {
		TP.setWidth(MerkaartorPreferences::instance()->getHighlightWidth()*3);
		TP.setCapStyle(Qt::RoundCap);
		thePainter.setPen(TP);
		QPolygonF Pl;
		buildPolygonFromRoad(this,theView->projection(),Pl);
		thePainter.drawPoints(theView->transform().map(Pl));

//		if (M_PREFS->getShowParents()) {
//			for (int i=0; i<sizeParents(); ++i)
//				if (!getParent(i)->isDeleted())
//					getParent(i)->drawHover(thePainter, theView, false);
//		}
	}
}

void Road::drawFocus(QPainter& thePainter, MapView* theView, bool solid)
{
	// FIXME Selected route
	if (!size())
		return;

	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(MerkaartorPreferences::instance()->getFocusColor());
	TP.setWidth(MerkaartorPreferences::instance()->getFocusWidth());
	if (!solid) {
		TP.setDashPattern(getParentDashes());
	}
	thePainter.setPen(TP);
	thePainter.setBrush(Qt::NoBrush);
	//QRect clipRect = thePainter.clipRegion().boundingRect().adjusted(int(-20), int(-20), int(20), int(20));
	//buildPath(theProjection, clipRect);
	thePainter.drawPath(theView->transform().map(p->thePath));
	if (solid) {
		TP.setWidth(MerkaartorPreferences::instance()->getFocusWidth()*3);
		TP.setCapStyle(Qt::RoundCap);
		thePainter.setPen(TP);
		QPolygonF Pl;
		buildPolygonFromRoad(this,theView->projection(),Pl);
		thePainter.drawPoints(theView->transform().map(Pl));

		if (M_PREFS->getShowParents()) {
			for (int i=0; i<sizeParents(); ++i)
				if (!getParent(i)->isDeleted())
					getParent(i)->drawFocus(thePainter, theView, false);
		}
	}
}

double Road::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection, const QTransform& theTransform) const
{
	double Best = 1000000;
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
	{
		if (p->Nodes.at(i)) {
			double x = ::distance(Target,theTransform.map(theProjection.project(p->Nodes.at(i))));
			if (x<ClearEndDistance)
				return Best;
		}
	}
	if (smoothed().size())
		for (int i=3; i <p->Smoothed.size(); i += 3)
		{
			BezierF F(
				theTransform.map(theProjection.project(p->Smoothed[i-3])),
				theTransform.map(theProjection.project(p->Smoothed[i-2])),
				theTransform.map(theProjection.project(p->Smoothed[i-1])),
				theTransform.map(theProjection.project(p->Smoothed[i])));
			double D = F.distance(Target);
			if (D < ClearEndDistance)
				Best = D;
		}
	else
		for (unsigned int i=1; i<p->Nodes.size(); ++i)
		{
			if (p->Nodes.at(i) && p->Nodes.at(i-1)) {
				LineF F(theTransform.map(theProjection.project(p->Nodes.at(i-1))),theTransform.map(theProjection.project(p->Nodes.at(i))));
				double D = F.capDistance(Target);
				if (D < ClearEndDistance)
					Best = D;
			}
		}
	return Best;
}

void Road::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const QList<MapFeature*>& Proposals)
{
	for (unsigned int i=0; i<p->Nodes.size();) {
		if (p->Nodes[i] == aFeature)
		{
			QList<TrackPoint*> Alternatives;
			for (int j=0; j<Proposals.size(); ++j)
			{
				TrackPoint* Pt = dynamic_cast<TrackPoint*>(Proposals[j]);
				if (Pt)
					Alternatives.push_back(Pt);
			}
			if ( (p->Nodes.size() == 1) && (Alternatives.size() == 0) ) {
				if (!isDeleted())
					theList->add(new RemoveFeatureCommand(theDocument,this));
			}
			else
			{
				for (int j=0; j<Alternatives.size(); ++j)
				{
					if (i < p->Nodes.size())
					{
						if (p->Nodes[i+j] != Alternatives[j])
						{
							if ((i+j) == 0)
								theList->add(new RoadAddTrackPointCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(layer())));
							else if (p->Nodes[i+j-1] != Alternatives[j])
								theList->add(new RoadAddTrackPointCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(layer())));
						}
					}
				}
				theList->add(new RoadRemoveTrackPointCommand(this, (TrackPoint*)aFeature,theDocument->getDirtyOrOriginLayer(layer())));
			}
		}
		++i;
	}
}

QPainterPath Road::getPath()
{
	return p->thePath;
}

#ifndef _MOBILE
void Road::buildPath(const Projection &theProjection, const QTransform& /*theTransform*/, const QRectF& cr)
{
	using namespace ggl;

	p->thePath = QPainterPath();

	if (p->Nodes.size() < 2)
		return;

	// Ensure nodes' projection is up-to-date
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		theProjection.project(p->Nodes[i]);

	QPointF pbl = theProjection.project(p->BBox.bottomLeft());
    QPointF ptr = theProjection.project(p->BBox.topRight());
    box_2d roadRect (
        make<point_2d>(pbl.x(), pbl.y()), 
        make<point_2d>(ptr.x(), ptr.y())
        );
	box_2d clipRect (make<point_2d>(cr.bottomLeft().x(), cr.topRight().y()), make<point_2d>(cr.topRight().x(), cr.bottomLeft().y()));
    bool toClip = !ggl::within(roadRect, clipRect);
    if (!toClip) {
		p->thePath.moveTo(p->Nodes.at(0)->projection());
	    for (unsigned int i=1; i<p->Nodes.size(); ++i) {
			p->thePath.lineTo(p->Nodes.at(i)->projection());
        }
    } else {

	    if (area() <= 0.0) {
//	        linestring_2d in;
//	        for (unsigned int i=0; i<p->Nodes.size(); ++i) {
//		        QPointF P = p->Nodes[i]->projection();
//		        append(in, make<point_2d>(P.x(), P.y()));
//	        }

			std::vector<linestring_2d> clipped;
			intersection <linestring_2d, box_2d, /*linestring_2d*/ std::vector<TrackPointPtr>, std::back_insert_iterator <std::vector<linestring_2d> > >
				(clipRect, /*in*/ p->Nodes, std::back_inserter(clipped));

	        for (std::vector<linestring_2d>::const_iterator it = clipped.begin(); it != clipped.end(); it++)
	        {
		        if (!(*it).empty()) {
			        p->thePath.moveTo(QPointF((*it)[0].x(), (*it)[0].y()));
		        }
		        for (linestring_2d::const_iterator itl = (*it).begin()+1; itl != (*it).end(); itl++)
		        {
			        p->thePath.lineTo(QPointF((*itl).x(), (*itl).y()));
		        }
	        }
	    } 
	    else 
	    {
			polygon_2d in;
			for (unsigned int i=0; i<p->Nodes.size(); ++i) {
				QPointF P = p->Nodes[i]->projection();
				append(in, make<point_2d>(P.x(), P.y()));
			}
//	        correct(in);

	        std::vector<polygon_2d> clipped;
			intersection <polygon_2d, box_2d, polygon_2d /*std::vector<TrackPointPtr>*/, std::back_insert_iterator <std::vector<polygon_2d> > >
				(clipRect, in /*p->Nodes*/, std::back_inserter(clipped));

	        for (std::vector<polygon_2d>::const_iterator it = clipped.begin(); it != clipped.end(); it++)
	        {
		        if (!(*it).outer().empty()) {
			        p->thePath.moveTo(QPointF((*it).outer()[0].x(), (*it).outer()[0].y()));
		        }
		        for (ring_2d::const_iterator itl = (*it).outer().begin()+1; itl != (*it).outer().end(); itl++)
		        {
			        p->thePath.lineTo(QPointF((*itl).x(), (*itl).y()));
		        }
		        p->thePath.lineTo(QPointF((*it).outer()[0].x(), (*it).outer()[0].y()));
	        }
        }
	}
	//p->thePath = theTransform.map(p->thePath);
}
#else
void Road::buildPath(Projection const &theProjection, const QRect& clipRect)
{
	p->thePath = QPainterPath();
	if (!p->Nodes.size())
		return;

	bool lastPointVisible = true;
	QPoint lastPoint = theProjection.project(p->Nodes[0]);
	QPoint aP = lastPoint;

	double PixelPerM = theProjection.pixelPerM();
	double WW = PixelPerM*widthOf()*10+10;


	if (M_PREFS->getDrawingHack()) {
		if (!clipRect.contains(aP)) {
			aP.setX(qMax(clipRect.left(), aP.x()));
			aP.setX(qMin(clipRect.right(), aP.x()));
			aP.setY(qMax(clipRect.top(), aP.y()));
			aP.setY(qMin(clipRect.bottom(), aP.y()));
			lastPointVisible = false;
		}
	}
	p->thePath.moveTo(aP);
	QPoint firstPoint = aP;
	if (smoothed().size())
	{
		for (int i=3; i<smoothed().size(); i+=3)
			p->thePath.cubicTo(
				theProjection.project(smoothed()[i-2]),
				theProjection.project(smoothed()[i-1]),
				theProjection.project(smoothed()[i]));
	}
	else
		for (int j=1; j<size(); ++j) {
			aP = theProjection.project(p->Nodes[j]);
			if (M_PREFS->getDrawingHack()) {
				QLine l(lastPoint, aP);
				if (!clipRect.contains(aP)) {
					if (!lastPointVisible) {
						QPoint a, b;
						if (QRectInterstects(clipRect, l, a, b)) {
							p->thePath.lineTo(a);
							lastPoint = aP;
							aP = b;
						} else {
							lastPoint = aP;
							aP.setX(qMax(clipRect.left(), aP.x()));
							aP.setX(qMin(clipRect.right(), aP.x()));
							aP.setY(qMax(clipRect.top(), aP.y()));
							aP.setY(qMin(clipRect.bottom(), aP.y()));
						}
					} else {
						QPoint a, b;
						QRectInterstects(clipRect, l, a, b);
						lastPoint = aP;
						aP = a;
					}
					lastPointVisible = false;
				} else {
					if (!lastPointVisible) {
						QPoint a, b;
						QRectInterstects(clipRect, l, a, b);
						p->thePath.lineTo(a);
					}
					lastPoint = aP;
					lastPointVisible = true;
				}
			}
			p->thePath.lineTo(aP);
		}
		if (area() > 0.0 && !lastPointVisible)
			p->thePath.lineTo(firstPoint);
}
#endif


bool Road::deleteChildren(MapDocument* theDocument, CommandList* theList)
{
	if (lastUpdated() == MapFeature::OSMServerConflict)
		return true;

	MDiscardableMessage dlg(NULL,
		MainWindow::tr("Delete Children."),
		MainWindow::tr("Do you want to delete the children nodes also?"));
	if (dlg.check() == QDialog::Accepted) {
		QList<MapFeature*> Alternatives;
		QMap<MapFeature*, int> ToDelete;
		for (int i=(int)p->Nodes.size()-1; i>=0; --i) {
			TrackPoint* N = p->Nodes[i];
			if (N->sizeParents() < 2) {
				ToDelete[N] = i;
			}
		}
		QList<MapFeature*> ToDeleteKeys = ToDelete.uniqueKeys();
		for (int i=0; i<ToDeleteKeys.size(); ++i) {
			if (!ToDeleteKeys[i]->isDeleted())
				theList->add(new RemoveFeatureCommand(theDocument, ToDeleteKeys[i], Alternatives));
		}
	}
	return true;
}

#define DEFAULTWIDTH 6
#define LANEWIDTH 4

double Road::widthOf()
{
	if (p->Width)
		return p->Width;

	QString s(tagValue("width",QString()));
	if (!s.isNull())
		p->Width = s.toDouble();
	QString h = tagValue("highway",QString());
	if ( (h == "motorway") || (h=="motorway_link") )
		p->Width =  4*LANEWIDTH; // 3 lanes plus emergency
	else if ( (h == "trunk") || (h=="trunk_link") )
		p->Width =  3*LANEWIDTH; // 2 lanes plus emergency
	else if ( (h == "primary") || (h=="primary_link") )
		p->Width =  2*LANEWIDTH; // 2 lanes
	else if (h == "secondary")
		p->Width =  2*LANEWIDTH; // 2 lanes
	else if (h == "tertiary")
		p->Width =  1.5*LANEWIDTH; // shared middle lane
	else if (h == "cycleway")
		p->Width =  1.5;
	p->Width = DEFAULTWIDTH;

	return p->Width;
}

void Road::setTag(const QString& key, const QString& value, bool addToTagList)
{
	MapFeature::setTag(key, value, addToTagList);
	p->MetaUpToDate = false;
	p->Width = 0;
}

void Road::setTag(int index, const QString& key, const QString& value, bool addToTagList)
{
	MapFeature::setTag(index, key, value, addToTagList);
	p->MetaUpToDate = false;
	p->Width = 0;
}

void Road::clearTags()
{
	MapFeature::clearTags();
	p->MetaUpToDate = false;
	p->Width = 0;
}

void Road::clearTag(const QString& k)
{
	MapFeature::clearTag(k);
	p->MetaUpToDate = false;
	p->Width = 0;
}

bool Road::toGPX(QDomElement xParent, QProgressDialog & progress, bool forExport)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("rte");
	xParent.appendChild(e);

	if (!forExport)
		e.setAttribute("xml:id", xmlId());
	QString s = tagValue("name","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("name");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}
	s = tagValue("_comment_","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("cmt");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}
	s = tagValue("_description_","");
	if (!s.isEmpty()) {
		QDomElement c = xParent.ownerDocument().createElement("desc");
		e.appendChild(c);
		QDomText v = c.ownerDocument().createTextNode(s);
		c.appendChild(v);
	}

	for (int i=0; i<size(); ++i) {
		dynamic_cast <TrackPoint*> (get(i))->toGPX(e, progress, forExport);
	}

	return OK;
}

QString Road::toXML(int lvl, QProgressDialog * progress)
{
	if (!size()) return "";

	if (progress)
		progress->setValue(progress->value()+1);

	QString S(lvl*2, ' ');
	S += QString("<way id=\"%1\">\n").arg(stripToOSMId(id()));

	S += QString((lvl+1)*2, ' ') + QString("<nd ref=\"%1\"/>\n").arg(stripToOSMId(get(0)->id()));
	for (int i=1; i<size(); ++i)
		if (get(i)->id() != get(i-1)->id())
			S += QString((lvl+1)*2, ' ') + QString("<nd ref=\"%1\"/>\n").arg(stripToOSMId(get(i)->id()));
	S += tagsToXML(lvl+1);
	S += QString(lvl*2, ' ') + "</way>\n";
	return S;
}

bool Road::toXML(QDomElement xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("way");
	xParent.appendChild(e);

	e.setAttribute("id", xmlId());
	e.setAttribute("timestamp", time().toString(Qt::ISODate)+"Z");
	e.setAttribute("user", user());
	e.setAttribute("actor", (int)lastUpdated());
	e.setAttribute("version", versionNumber());
	if (isDeleted())
		e.setAttribute("deleted","true");

	if (size()) {
		QDomElement n = xParent.ownerDocument().createElement("nd");
		e.appendChild(n);
		n.setAttribute("ref", get(0)->xmlId());

		for (int i=1; i<size(); ++i) {
			if (get(i)->xmlId() != get(i-1)->xmlId()) {
				QDomElement n = xParent.ownerDocument().createElement("nd");
				e.appendChild(n);

				n.setAttribute("ref", get(i)->xmlId());
			}
		}
	}

	tagsToXML(e);

	progress.setValue(progress.value()+1);
	return OK;
}

Road * Road::fromXML(MapDocument* d, MapLayer * L, const QDomElement e)
{
	QString id = e.attribute("id");
	if (!id.startsWith('{') && !id.startsWith('-'))
		id = "way_" + id;
	QDateTime time = QDateTime::fromString(e.attribute("timestamp").left(19), Qt::ISODate);
	QString user = e.attribute("user");
	bool Deleted = (e.attribute("deleted") == "true");
	int Version = e.attribute("version").toInt();
	MapFeature::ActorType A;
	if (e.hasAttribute("actor"))
		A = (MapFeature::ActorType)(e.attribute("actor", "2").toInt());
	else
		if ((L == d->getDirtyOrOriginLayer()))
			A = MapFeature::User;
		else
			A = MapFeature::OSMServer;

	Road* R = dynamic_cast<Road*>(d->getFeature(id));

	if (!R) {
		R = new Road();
		R->setId(id);
		R->setLastUpdated(A);
		L->add(R);
	} else {
		if (R->layer() != L) {
			R->layer()->remove(R);
			L->add(R);
		}
		if (R->lastUpdated() == MapFeature::NotYetDownloaded)
			R->setLastUpdated(A);
	}
	R->setTime(time);
	R->setUser(user);
	R->setDeleted(Deleted);
	R->setVersionNumber(Version);

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "nd") {
			QString nId = c.attribute("ref");
			if (!nId.startsWith('{') && !nId.startsWith('-'))
				nId = "node_" + nId;
			//TrackPoint* Part = MapFeature::getTrackPointOrCreatePlaceHolder(d, L, NULL, c.attribute("ref"));
			TrackPoint* Part = dynamic_cast<TrackPoint*>(d->getFeature(nId));
			if (!Part)
			{
				Part = new TrackPoint(Coord(0,0));
				Part->setId(nId);
				Part->setLastUpdated(MapFeature::NotYetDownloaded);
				L->add(Part);
			}
			R->add(Part);
		}
		c = c.nextSiblingElement();
	}

	MapFeature::tagsFromXML(d, R, e);

	return R;
}

MapFeature::TrafficDirectionType trafficDirection(const Road* R)
{
	// TODO some duplication with Way trafficDirection
	QString d;
	int idx=R->findKey("oneway");
	if (idx<R->tagSize())
	{
		d = R->tagValue(idx);
		if ( (d == "yes") || (d == "1") || (d == "true")) return MapFeature::OneWay;
		if (d == "-1") return MapFeature::OtherWay;
		if ((d == "no") || (d == "false") || (d == "0")) return MapFeature::BothWays;
	}

	idx=R->findKey("junction");
	if (idx<R->tagSize())
	{
		d = R->tagValue(idx);
		if(d=="roundabout") return MapFeature::OneWay;
		//TODO For motorway and motorway_link, this is still discussed on the wiki.
	}
	return MapFeature::UnknownDirection;
}

int findSnapPointIndex(const Road* R, Coord& P)
{
	if (R->smoothed().size())
	{
		BezierF L(R->smoothed()[0],R->smoothed()[1],R->smoothed()[2],R->smoothed()[3]);
		int BestIdx = 3;
		double BestDistance = L.distance(toQt(P));
		for (int i=3; i<R->smoothed().size(); i+=3)
		{
			BezierF L(R->smoothed()[i-3],R->smoothed()[i-2],R->smoothed()[i-1],R->smoothed()[i]);
			double Distance = L.distance(toQt(P));
			if (Distance < BestDistance)
			{
				BestIdx = i;
				BestDistance = Distance;
			}
		}
		BezierF B(R->smoothed()[BestIdx-3],R->smoothed()[BestIdx-2],R->smoothed()[BestIdx-1],R->smoothed()[BestIdx]);
		P = toCoord(B.project(toQt(P)));
		return BestIdx/3;
	}
	LineF L(R->getNode(0)->position(),R->getNode(1)->position());
	int BestIdx = 1;
	double BestDistance = L.capDistance(P);
	for (int i = 2; i<R->size(); ++i)
	{
		LineF L(R->getNode(i-1)->position(),R->getNode(i)->position());
		double Distance = L.capDistance(P);
		if (Distance < BestDistance)
		{
			BestIdx = i;
			BestDistance = Distance;
		}
	}
	LineF F(R->getNode(BestIdx-1)->position(),R->getNode(BestIdx)->position());
	P = F.project(Coord(P));
	return BestIdx;
}

const QList<Coord>& Road::smoothed() const
{
	if (!p->SmoothedUpToDate)
		p->updateSmoothed(tagValue("smooth","") == "yes");
	return p->Smoothed;
}

QString Road::toHtml()
{
	QString distanceLabel;
	if (distance() < 1.0)
		distanceLabel = QString("%1 m").arg(int(distance() * 1000));
	else
		distanceLabel = QString("%1 km").arg(distance(), 0, 'f', 3);

	QString D;

	D += "<i>"+QApplication::translate("MapFeature", "Length")+": </i>" + distanceLabel;
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Size")+": </i>" + QApplication::translate("MapFeature", "%1 nodes").arg(size());
	CoordBox bb = boundingBox();
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + QString::number(intToAng(bb.topLeft().lat()), 'f', 4) + " / " + QString::number(intToAng(bb.topLeft().lon()), 'f', 4);
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + QString::number(intToAng(bb.bottomRight().lat()), 'f', 4) + " / " + QString::number(intToAng(bb.bottomRight().lon()), 'f', 4);

	return MapFeature::toMainHtml(QApplication::translate("MapFeature", "Way"), "way").arg(D);
}

void Road::toBinary(QDataStream& ds, QHash <QString, quint64>& theIndex)
{
	theIndex["R" + QString::number(idToLong())] = ds.device()->pos();
	ds << (qint8)'R';
	ds << idToLong();
	ds << (qint32)size();
	for (int i=0; i < size(); ++i) {
		ds << (qint64)(get(i)->idToLong());
//		ds << (qint64)(theIndex["N" + QString::number(get(i)->idToLong())]);
	}
}

Road* Road::fromBinary(MapDocument* d, OsbMapLayer* L, QDataStream& ds, qint8 c, qint64 id)
{
	Q_UNUSED(c);

	qint32	fSize;
	QString strId;
	qint64 refId;

	ds >> fSize;

	if (!L) {
		for (int i=0; i < fSize; ++i)
			ds >> refId;
		return NULL;
	}

	if (id < 1)
		strId = QString::number(id);
	else
		strId = QString("way_%1").arg(QString::number(id));

	Road* R = dynamic_cast<Road*>(d->getFeature(strId));
	if (!R) {
		R = new Road();
		R->setId(strId);
		R->setLastUpdated(MapFeature::OSMServer);
		L->add(R);
	} else {
		if (R->lastUpdated() == MapFeature::NotYetDownloaded)
			R->setLastUpdated(MapFeature::OSMServer);
		else  {
			for (int i=0; i < fSize; ++i)
				ds >> refId;
			return R;
		}
	}

	for (int i=0; i < fSize; ++i) {
		ds >> refId;

		QString sRefId;
		if (refId < 0)
			sRefId = QString::number(refId);
		else
			sRefId = "node_" + QString::number(refId);
		TrackPoint* N = dynamic_cast<TrackPoint*> (d->getFeature(sRefId));
//		TrackPoint* N = dynamic_cast<TrackPoint*> (L->getFeature(d, refId)); QString("node_"+id));
//		TrackPoint* N = dynamic_cast<TrackPoint*> (L->get(QString("node_%1").arg(refId)));
//		TrackPoint* N = dynamic_cast<TrackPoint*> (d->getFeature(QString("node_%1").arg(refId)));
		Q_ASSERT(N);
		R->add(N);
	}

	return R;
}

bool Road::isExtrimity(TrackPoint* node)
{
	if (p->Nodes[0] == node)
		return true;

	if (p->Nodes[p->Nodes.size()-1] == node)
		return true;

	return false;
}

Road * Road::GetSingleParentRoad(MapFeature * mapFeature)
{
	int parents = mapFeature->sizeParents();

	if (parents == 0)
		return NULL;

	Road * parentRoad = NULL;

	int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		Road * road = CAST_WAY(parent);

		if (road == NULL)
			continue;

		if (parentRoad && road != parentRoad)
			return NULL;

		//FIXME This test shouldn't be necessary, but there is at least a case where the road has a NULL layer. The root cause must be found.
		if (!(road->isDeleted()) && road->layer() && road->layer()->isEnabled())
			parentRoad = road;
	}

	return parentRoad;
}

Road * Road::GetSingleParentRoadInner(MapFeature * mapFeature)
{
	int parents = mapFeature->sizeParents();

	if (parents == 0)
		return NULL;

	Road * parentRoad = NULL;
	TrackPoint* trackPoint = dynamic_cast<TrackPoint*>(mapFeature);

	int i;
	for (i=0; i<parents; i++)
	{
		MapFeature * parent = mapFeature->getParent(i);
		Road * road = dynamic_cast<Road*>(parent);

		if (road == NULL)
			continue;

		if (road->isExtrimity(trackPoint) && !road->isClosed())
			continue;
		
		if (parentRoad && road != parentRoad)
			return NULL;

		//FIXME This test shouldn't be necessary, but there is at least a case where the road has a NULL layer. The root cause must be found.
		if (!(road->isDeleted()) && road->layer() && road->layer()->isEnabled())
			parentRoad = road;
	}

	return parentRoad;
}

