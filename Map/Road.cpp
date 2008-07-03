#include "Map/Road.h"

#include "Command/DocumentCommands.h"
#include "Command/RoadCommands.h"
#include "Map/Coord.h"
#include "Map/Painting.h"
#include "Map/Projection.h"
#include "Map/TrackPoint.h"
#include "Utils/LineF.h"

#include <QtGui/QPainter>
#include <QtGui/QPainterPath>
#include <QMessageBox>

#include <algorithm>
#include <vector>

class RoadPrivate
{
	public:
		RoadPrivate()
		: SmoothedUpToDate(false), BBox(Coord(0,0),Coord(0,0)), BBoxUpToDate(true), Area(0), Distance(0), MetaUpToDate(true)
		{
		}
		std::vector<TrackPoint*> Nodes;
 		std::vector<Coord> Smoothed;
 		bool SmoothedUpToDate;
		CoordBox BBox;
		bool BBoxUpToDate;

		double Area;
		double Distance;
		bool MetaUpToDate;

		void updateSmoothed(bool DoSmooth);
		void addSmoothedBezier(unsigned int i, unsigned int j, unsigned int k, unsigned int l);
};

void RoadPrivate::addSmoothedBezier(unsigned int i, unsigned int j, unsigned int k, unsigned int l)
{
	Coord A(Nodes[i]->position());
	Coord B(Nodes[j]->position());
	Coord C(Nodes[k]->position());
	Coord D(Nodes[l]->position());

	Coord Ctrl1(B+(C-A)*(1.0/6));
	Coord Ctrl2(C-(D-B)*(1.0/6));


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
	unsigned int Last = Nodes.size()-1;
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
	delete p;
}

void Road::setLayer(MapLayer* L)
{
	if (L)
		for (unsigned int i=0; i<p->Nodes.size(); ++i)
			p->Nodes[i]->setParent(this);
	else
		for (unsigned int i=0; i<p->Nodes.size(); ++i)
			p->Nodes[i]->unsetParent(this);
	MapFeature::setLayer(L);
}

void Road::partChanged(MapFeature*, unsigned int ChangeId)
{
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
	notifyParents(ChangeId);
}

QString Road::description() const
{
	QString s(tagValue("name",""));
	if (!s.isEmpty())
		return QString("%1 (%2)").arg(s).arg(id());
	return QString("%1").arg(id());
}

RenderPriority Road::renderPriority(double aPixelPerM) const
{
	FeaturePainter* thePainter = getEditPainter(aPixelPerM);
	if (thePainter && thePainter->isFilled())
		return RenderPriority(RenderPriority::IsArea,fabs(area()));
	return RenderPriority(RenderPriority::IsLinear,0);
}

void Road::add(TrackPoint* Pt)
{
	p->Nodes.push_back(Pt);
	Pt->setParent(this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

void Road::add(TrackPoint* Pt, unsigned int Idx)
{
	p->Nodes.push_back(Pt);
	std::rotate(p->Nodes.begin()+Idx,p->Nodes.end()-1,p->Nodes.end());
	Pt->setParent(this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

unsigned int Road::find(TrackPoint* Pt) const
{
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i] == Pt)
			return i;
	return p->Nodes.size();
}

void Road::remove(unsigned int idx)
{
	TrackPoint* Pt = p->Nodes[idx];
	p->Nodes.erase(p->Nodes.begin()+idx);
	Pt->unsetParent(this);
	p->BBoxUpToDate = false;
	p->MetaUpToDate = false;
	p->SmoothedUpToDate = false;
}

unsigned int Road::size() const
{
	return p->Nodes.size();
}

TrackPoint* Road::get(unsigned int idx)
{
	return p->Nodes[idx];
}

const TrackPoint* Road::get(unsigned int idx) const
{
	return p->Nodes[idx];
}

bool Road::notEverythingDownloaded() const
{
	if (lastUpdated() == MapFeature::NotYetDownloaded)
		return true;
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
		if (p->Nodes[i]->notEverythingDownloaded())
			return true;
	return false;
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

	if (p->Nodes.size() == 0)
	{
		p->MetaUpToDate = true;
		return;
	}

	bool isArea = (p->Nodes[0] == p->Nodes[p->Nodes.size()-1]);

	for (unsigned int i=0; (i+1)<p->Nodes.size(); ++i)
	{
		const Coord & here = p->Nodes[i]->position();
		const Coord & next = p->Nodes[i+1]->position();

		if (isArea)
			p->Area += here.lat() * next.lon() - next.lat() * here.lon();

		p->Distance += next.distanceFrom(here);
	}

	p->Area /= 2;
	p->MetaUpToDate = true;
}

double Road::distance() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->Distance;
}

double Road::area() const
{
	if (p->MetaUpToDate == false)
		updateMeta();

	return p->Area;
}

void Road::draw(QPainter& /* thePainter */, const Projection& )
{
}

void Road::drawHover(QPainter& thePainter, const Projection& theProjection)
{
	// FIXME Selected route
	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(MerkaartorPreferences::instance()->getHoverColor());
	TP.setWidth(5);
	thePainter.setPen(TP);
	QPainterPath Pt;
	buildPathFromRoad(this,theProjection,Pt);
	thePainter.drawPath(Pt);
	TP.setWidth(15);
	TP.setCapStyle(Qt::RoundCap);
	thePainter.setPen(TP);
	QPolygonF Pl;
	buildPolygonFromRoad(this,theProjection,Pl);
	thePainter.drawPoints(Pl);
}

void Road::drawFocus(QPainter& thePainter, const Projection& theProjection)
{
	// FIXME Selected route
	QFont F(thePainter.font());
	F.setPointSize(10);
	F.setBold(true);
	F.setWeight(QFont::Black);
	thePainter.setFont(F);
	QPen TP(MerkaartorPreferences::instance()->getFocusColor());
	TP.setWidth(5);
	thePainter.setPen(TP);
	QPainterPath Pt;
	buildPathFromRoad(this,theProjection,Pt);
	thePainter.drawPath(Pt);
	TP.setWidth(15);
	TP.setCapStyle(Qt::RoundCap);
	thePainter.setPen(TP);
	QPolygonF Pl;
	buildPolygonFromRoad(this,theProjection,Pl);
	thePainter.drawPoints(Pl);
}

double Road::pixelDistance(const QPointF& Target, double ClearEndDistance, const Projection& theProjection) const
{
	double Best = 1000000;
	for (unsigned int i=0; i<p->Nodes.size(); ++i)
	{
		double x = ::distance(Target,theProjection.project(p->Nodes[i]->position()));
		if (x<ClearEndDistance)
			return Best;
	}
	if (smoothed().size())
		for (unsigned int i=3; i <p->Smoothed.size(); i += 3)
		{
			BezierF F(
				theProjection.project(p->Smoothed[i-3]),
				theProjection.project(p->Smoothed[i-2]),
				theProjection.project(p->Smoothed[i-1]),
				theProjection.project(p->Smoothed[i]));
			double D = F.distance(Target);
			if (D < ClearEndDistance)
				Best = D;
		}
	else
		for (unsigned int i=1; i<p->Nodes.size(); ++i)
		{
			LineF F(theProjection.project(p->Nodes[i-1]->position()),theProjection.project(p->Nodes[i]->position()));
			double D = F.capDistance(Target);
			if (D < ClearEndDistance)
				Best = D;
		}
	return Best;
}

void Road::cascadedRemoveIfUsing(MapDocument* theDocument, MapFeature* aFeature, CommandList* theList, const std::vector<MapFeature*>& Proposals)
{
	for (unsigned int i=0; i<p->Nodes.size();) {
		if (p->Nodes[i] == aFeature)
		{
			std::vector<TrackPoint*> Alternatives;
			for (unsigned int j=0; j<Proposals.size(); ++j)
			{
				TrackPoint* Pt = dynamic_cast<TrackPoint*>(Proposals[j]);
				if (Pt)
					Alternatives.push_back(Pt);
			}
			if ( (p->Nodes.size() == 1) && (Alternatives.size() == 0) )
				theList->add(new RemoveFeatureCommand(theDocument,this));
			else
			{
				for (unsigned int j=0; j<Alternatives.size(); ++j)
					if (i < p->Nodes.size())
						if (p->Nodes[i+j] != Alternatives[j])
							if ((i+j) == 0)
								theList->add(new RoadAddTrackPointCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(Alternatives[j]->layer())));
							else if (p->Nodes[i+j-1] != Alternatives[j])
								theList->add(new RoadAddTrackPointCommand(this, Alternatives[j], i+j,theDocument->getDirtyOrOriginLayer(Alternatives[j]->layer())));
				theList->add(new RoadRemoveTrackPointCommand(this, (TrackPoint*)aFeature,theDocument->getDirtyOrOriginLayer(aFeature->layer())));
			}
		}
		++i;
	}
}

bool Road::deleteChildren(MapDocument* theDocument, CommandList* theList)
{
	QMessageBox::StandardButton resp = QMessageBox::question(NULL, MainWindow::tr("Delete Children"),
								 MainWindow::tr("Do you want to delete the children nodes also?"),
								 QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
	switch(resp) {
		case QMessageBox::No:
			return true;

		case QMessageBox::Cancel:
			return false;

		case QMessageBox::Yes:
			for (unsigned int i=0; i<p->Nodes.size(); ++i) {
				if (p->Nodes[i]->sizeParents() < 2)
					theList->add(new RemoveFeatureCommand(theDocument, p->Nodes[i]));
			}
			return true;

		default:
			return false;
	}
}


QString Road::toXML(unsigned int lvl)
{
	QString S(lvl*2, ' ');
	S += QString("<way id=\"%1\">\n").arg(stripToOSMId(id()));
	for (unsigned int i=0; i<size(); ++i)
		S += QString((lvl+1)*2, ' ') + QString("<nd ref=\"%1\"/>\n").arg(stripToOSMId(get(i)->id()));
	S += tagsToXML(lvl+1);
	S += QString(lvl*2, ' ') + "</way>\n";
	return S;
}

bool Road::toXML(QDomElement xParent)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement("way");
	xParent.appendChild(e);

	e.setAttribute("id", xmlId());
	e.setAttribute("timestamp", time().toString(Qt::ISODate));
	e.setAttribute("user", user());

	for (unsigned int i=0; i<size(); ++i) {
		QDomElement n = xParent.ownerDocument().createElement("nd");
		e.appendChild(n);

		n.setAttribute("ref", get(i)->xmlId());
	}

	tagsToXML(e);

	return OK;
}

Road * Road::fromXML(MapDocument* d, MapLayer * L, const QDomElement e)
{
	QString id = e.attribute("id");
	if (!id.startsWith('{'))
		id = "way_" + id;
	QDateTime time = QDateTime::fromString(e.attribute("timestamp"), Qt::ISODate);
	QString user = e.attribute("user");

	Road* R = dynamic_cast<Road*>(d->getFeature(id));

	if (!R) {
		R = new Road();
		R->setId(id);
		R->setLastUpdated(MapFeature::OSMServer);
	} else {
		R->layer()->remove(R);
	}
	R->setTime(time);
	R->setUser(user);

	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "nd") {
			QString nId = c.attribute("ref");
			if (!nId.startsWith('{'))
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
	L->add(R);

	MapFeature::tagsFromXML(d, R, e);

	return R;
}

MapFeature::TrafficDirectionType trafficDirection(const Road* R)
{
	// TODO some duplication with Way trafficDirection
	QString d;
	unsigned int idx=R->findKey("oneway");
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

#define DEFAULTWIDTH 6
#define LANEWIDTH 4

double widthOf(const Road* R)
{
	QString s(R->tagValue("width",QString()));
	if (!s.isNull())
		return s.toDouble();
	QString h = R->tagValue("highway",QString());
	if ( (h == "motorway") || (h=="motorway_link") )
		return 4*LANEWIDTH; // 3 lanes plus emergency
	else if ( (h == "trunk") || (h=="trunk_link") )
		return 3*LANEWIDTH; // 2 lanes plus emergency
	else if ( (h == "primary") || (h=="primary_link") )
		return 2*LANEWIDTH; // 2 lanes
	else if (h == "secondary")
		return 2*LANEWIDTH; // 2 lanes
	else if (h == "tertiary")
		return 1.5*LANEWIDTH; // shared middle lane
	else if (h == "cycleway")
		return 1.5;
	return DEFAULTWIDTH;
}

unsigned int findSnapPointIndex(const Road* R, Coord& P)
{
	if (R->smoothed().size())
	{
		BezierF L(R->smoothed()[0],R->smoothed()[1],R->smoothed()[2],R->smoothed()[3]);
		unsigned int BestIdx = 3;
		double BestDistance = L.distance(toQt(P));
		for (unsigned int i=3; i<R->smoothed().size(); i+=3)
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
	LineF L(R->get(0)->position(),R->get(1)->position());
	unsigned int BestIdx = 1;
	double BestDistance = L.capDistance(P);
	for (unsigned int i = 2; i<R->size(); ++i)
	{
		LineF L(R->get(i-1)->position(),R->get(i)->position());
		double Distance = L.capDistance(P);
		if (Distance < BestDistance)
		{
			BestIdx = i;
			BestDistance = Distance;
		}
	}
	LineF F(R->get(BestIdx-1)->position(),R->get(BestIdx)->position());
	P = F.project(Coord(P));
	return BestIdx;
}

bool isClosed(const Road* R)
{
	return R->size() && (R->get(0) == R->get(R->size()-1));
}

const std::vector<Coord>& Road::smoothed() const
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
	D += "<i>"+QApplication::translate("MapFeature", "Topleft")+": </i>" + QString::number(radToAng(bb.topLeft().lat()), 'f', 4) + " / " + QString::number(radToAng(bb.topLeft().lon()), 'f', 4);
	D += "<br/>";
	D += "<i>"+QApplication::translate("MapFeature", "Botright")+": </i>" + QString::number(radToAng(bb.bottomRight().lat()), 'f', 4) + " / " + QString::number(radToAng(bb.bottomRight().lon()), 'f', 4);

	return MapFeature::toMainHtml(QApplication::translate("MapFeature", "Way"), "way").arg(D);
}

void Road::toBinary(QDataStream& ds)
{
	ds << (qint8)'R';
	ds << idToLong();
	ds << (qint32)size();
	for (unsigned int i=0; i < size(); ++i) {
		ds << (qint64)(get(i)->idToLong());
	}
	tagsToBinary(ds);
}

Road* Road::fromBinary(MapDocument* d, MapLayer* /* L */, QDataStream& ds)
{
	qint8	c;
	qint64	id;
	qint32	fSize;

	ds >> c; if (c != 'R') return NULL;
	ds >> id;
	ds >> fSize;

	Road* R = new Road();
	R->setId(QString::number(id));
	if (id < 0)
		R->setId(QString::number(id));
	else
		R->setId("way_"+QString::number(id));

	for (int i=0; i < fSize; ++i) {
		qint64 refId;
		ds >> refId;

		QString sRefId;
		if (refId < 0)
			sRefId = QString::number(refId);
		else
			sRefId = "node_" + QString::number(refId);
		TrackPoint* N = dynamic_cast<TrackPoint*> (d->getFeature(sRefId));
		Q_ASSERT(N);
		if (N)
			R->add(N);
	}
	MapFeature::tagsFromBinary(d, R, ds);

	return R;
}
