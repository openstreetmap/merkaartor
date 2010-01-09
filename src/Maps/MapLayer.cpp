#include "Maps/MapLayer.h"

#include "Maps/MapFeature.h"

#include "Maps/MapDocument.h"
#include "Maps/Road.h"
#include "Maps/Relation.h"
#include "Maps/TrackPoint.h"
#include "Maps/TrackSegment.h"
#include "LayerWidget.h"
#include "Maps/ImportOSM.h"

#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RoadCommands.h"

#include "Utils/LineF.h"
#include "Utils/SortAccordingToRenderingPriority.h"

#include "ImportExport/ImportExportOsmBin.h"

#include <QtCore/QString>
#include <QMultiMap>
#include <QProgressDialog>
#include <QUuid>

#include <algorithm>
#include <QMap>
#include <QList>

/* MAPLAYER */

class MapLayerPrivate
{
public:
	MapLayerPrivate()
		: RenderPriorityUpToDate(false)
	{
		theDocument = NULL;
		selected = false;
		Enabled = true;
		Readonly = false;
		Uploadable = true;

		theRTree = new MyRTree(7, 2);
		IndexingBlocked = false;

	}
	~MapLayerPrivate()
	{
		delete theRTree;
		//for (int i=0; i<Features.size(); ++i)
		//	if (Features[i])
		//		delete Features[i];
	}
	QList<MapFeaturePtr> Features;
	MyRTree* theRTree;

	QHash<QString, MapFeaturePtr> IdMap;
	QString Name;
	QString Description;
	bool Visible;
	bool selected;
	bool Enabled;
	bool Readonly;
	bool Uploadable;
	bool IndexingBlocked;
	qreal alpha;
	int dirtyLevel;

	bool RenderPriorityUpToDate;
	MapDocument* theDocument;

	void sortRenderingPriority();
};

 void MapLayerPrivate::sortRenderingPriority()
{
	qSort(Features.begin(),Features.end(),SortAccordingToRenderingPriority());
	RenderPriorityUpToDate = true;
}

MapLayer::MapLayer()
:  p(new MapLayerPrivate), theWidget(0)
{
	p->alpha = 1.0;
	p->dirtyLevel = 0;
}

MapLayer::MapLayer(const QString& aName)
:  p(new MapLayerPrivate), theWidget(0)
{
	p->Name = aName;
	p->alpha = 1.0;
	p->dirtyLevel = 0;
}

MapLayer::MapLayer(const MapLayer&)
: QObject(), p(0), theWidget(0)
{
}

MapLayer::~MapLayer()
{
	SAFE_DELETE(p);
}

void MapLayer::sortRenderingPriority()
{
	if (!p->RenderPriorityUpToDate)
		p->sortRenderingPriority();
}

void MapLayer::invalidateRenderPriority()
{
	p->RenderPriorityUpToDate = false;
}

void MapLayer::setName(const QString& s)
{
	p->Name = s;
}

const QString& MapLayer::name() const
{
	return p->Name;
}

void MapLayer::setDescription(const QString& s)
{
	p->Description = s;
}

const QString& MapLayer::description() const
{
	return p->Description;
}

void MapLayer::setVisible(bool b) {
	p->Visible = b;
}

bool MapLayer::isVisible() const
{
	return p->Visible;
}

void MapLayer::setSelected(bool b) {
	p->selected = b;
}

bool MapLayer::isSelected() const
{
	return p->selected;
}

void MapLayer::setEnabled(bool b) {
	p->Enabled = b;
	if (theWidget) {
		theWidget->setVisible(b);
		theWidget->getAssociatedMenu()->menuAction()->setVisible(b);
	}
}

bool MapLayer::isEnabled() const
{
	return p->Enabled;
}

void MapLayer::setReadonly(bool b) {
	p->Readonly = b;
}

bool MapLayer::isReadonly() const
{
	return p->Readonly;
}

void MapLayer::setUploadable(bool b) {
	p->Uploadable = b;
}

bool MapLayer::isUploadable() const
{
	return p->Uploadable;
}

void MapLayer::add(MapFeature* aFeature)
{
	aFeature->setLayer(this);
	if (!aFeature->isDeleted())
		indexAdd(aFeature->boundingBox(), aFeature);
	p->Features.push_back(aFeature);
	notifyIdUpdate(aFeature->id(),aFeature);
	p->RenderPriorityUpToDate = false;
}

void MapLayer::add(MapFeature* aFeature, int Idx)
{
	add(aFeature);
	std::rotate(p->Features.begin()+Idx,p->Features.end()-1,p->Features.end());
}

void MapLayer::notifyIdUpdate(const QString& id, MapFeature* aFeature)
{
	p->IdMap[id] = aFeature;
}

void MapLayer::remove(MapFeature* aFeature)
{
	if (p->Features.removeOne(aFeature))
	{
		aFeature->setLayer(0);
		if (!aFeature->isDeleted())
			indexRemove(aFeature->boundingBox(), aFeature);
		notifyIdUpdate(aFeature->id(),0);
		p->RenderPriorityUpToDate = false;
	}
}

void MapLayer::deleteFeature(MapFeature* aFeature)
{
	if (p->Features.removeOne(aFeature))
	{
		aFeature->setLayer(0);
		if (!aFeature->isDeleted())
			indexRemove(aFeature->boundingBox(), aFeature);
		notifyIdUpdate(aFeature->id(),0);
		p->RenderPriorityUpToDate = false;
	}

	delete aFeature;
}

void MapLayer::clear()
{
	QList<MapFeaturePtr>::iterator i;
	for (i=p->Features.begin(); i != p->Features.end();)
	{
		(*i)->setLayer(0);
		notifyIdUpdate((*i)->id(),0);
		p->RenderPriorityUpToDate = false;
		i = p->Features.erase(i);
	}
	reIndex();
}

bool MapLayer::exists(MapFeature* F) const
{
	int i = p->Features.indexOf(F);
	return (i != -1);
}

int MapLayer::size() const
{
	return p->Features.size();
}

void MapLayer::setDocument(MapDocument* aDocument)
{
	p->theDocument = aDocument;
}

MapDocument* MapLayer::getDocument()
{
	return p->theDocument;
}

int MapLayer::get(MapFeature* aFeature)
{
	for (int i=0; i<p->Features.size(); ++i)
		if (p->Features[i] == aFeature)
			return i;

	return -1;
}

QList<MapFeature *> MapLayer::get()
{
	QList<MapFeature *> theList;
	for (int i=0; i<p->Features.size(); ++i)
		if (p->Features[i])
			theList.append(p->Features[i]);
	return theList;
}


MapFeature* MapLayer::get(int i)
{
	return p->Features.at(i);
}

MapFeature* MapLayer::get(const QString& id, bool exact)
{
	QHash<QString, MapFeaturePtr>::const_iterator i;

	i = p->IdMap.find(id);
	if (i != p->IdMap.end())
		return i.value();

	if (!exact) {
		i = p->IdMap.find(QString("node_"+id));
		if (i != p->IdMap.end())
			return i.value();
		i = p->IdMap.find(QString("way_"+id));
		if (i != p->IdMap.end())
			return i.value();
		i = p->IdMap.find(QString("rel_"+id));
		if (i != p->IdMap.end())
			return i.value();
	}
	return 0;
}

const MapFeature* MapLayer::get(int i) const
{
	if((int)i>=p->Features.size()) return 0;
	return p->Features[i];
}

LayerWidget* MapLayer::getWidget(void)
{
	return theWidget;
}

void MapLayer::deleteWidget(void)
{
//	theWidget->deleteLater();
	delete theWidget;
	theWidget = NULL;
}

void MapLayer::setAlpha(const qreal a)
{
	p->alpha = a;
}

qreal MapLayer::getAlpha() const
{
	return p->alpha;
}

void MapLayer::setId(const QString& id)
{
	Id = id;
}

const QString& MapLayer::id() const
{
	if (Id == "")
		Id = QUuid::createUuid().toString();
	return Id;
}

void MapLayer::blockIndexing(bool val)
{
	p->IndexingBlocked = val;
}

void MapLayer::indexAdd(const CoordBox& bb, const MapFeaturePtr aFeat)
{
	if (bb.isNull())
		return;
	if (!p->IndexingBlocked)
		p->theRTree->insert(bb, aFeat);
}

void MapLayer::indexRemove(const CoordBox& bb, const MapFeaturePtr aFeat)
{
	if (bb.isNull())
		return;
	if (!p->IndexingBlocked)
		p->theRTree->remove(bb, aFeat);
}

std::deque<MapFeature*> MapLayer::indexFind(const CoordBox& vp)
{
	return p->theRTree->find(vp);
}

void MapLayer::reIndex()
{
	delete p->theRTree;
	p->theRTree = new MyRTree(7, 2);

	for (int i=0; i<p->Features.size(); ++i) {
		if (p->Features.at(i)->isDeleted() || p->Features.at(i)->isVirtual())
			continue;
		MapFeature* f = p->Features.at(i);
		CoordBox bb = f->boundingBox();
		if (!bb.isNull()) {
			//Q_ASSERT((bb.bottomLeft().lon() <= bb.topRight().lon()) && (bb.bottomLeft().lat() <= bb.topRight().lat()));
			//Q_ASSERT((bb.bottomLeft().lon() < 100000000) && (bb.bottomLeft().lat() > 100000000));
			//Q_ASSERT((bb.topRight().lon() < 100000000) && (bb.topRight().lat() > 100000000));
			p->theRTree->insert(bb, f);
		}
	}
}

CoordBox MapLayer::boundingBox()
{
	if(p->Features.size()==0) return CoordBox(Coord(0,0),Coord(0,0));
	CoordBox Box;
	bool haveFirst = false;
	for (int i=0; i<p->Features.size(); ++i) {
		if (p->Features.at(i)->isDeleted())
			continue;
		if (p->Features.at(i)->notEverythingDownloaded())
			continue;
		if (haveFirst)
			Box.merge(p->Features.at(i)->boundingBox());
		else {
			Box = p->Features.at(i)->boundingBox();
			haveFirst = true;
		}
	}
	return Box;
}

int MapLayer::incDirtyLevel(int inc)
{
	return p->dirtyLevel += inc;
}

int MapLayer::decDirtyLevel(int inc)
{
	return p->dirtyLevel -= inc;
}

int MapLayer::setDirtyLevel(int newLevel)
{
	return (p->dirtyLevel = newLevel);
}

int MapLayer::getDirtyLevel()
{
	return p->dirtyLevel;
}

int MapLayer::getDirtySize()
{
	int dirtyObjects = 0;

	QList<MapFeaturePtr>::const_iterator i;
	for (i = p->Features.constBegin(); i != p->Features.constEnd(); i++)
		if (!((*i)->isDeleted()) || ((*i)->isDeleted() && (*i)->hasOSMId()))
			++dirtyObjects;

	return dirtyObjects;
}

bool MapLayer::canDelete() const
{
	return (p->dirtyLevel == 0);
}

QString MapLayer::toMainHtml()
{
	QString desc;
	desc = QString("<big><b>%1</b></big><br/>").arg(p->Name);
	if (!p->Description.isEmpty())
		desc += QString("<b>%1</b><br/>").arg(p->Description);
	desc += QString("<small>(%1)</small>").arg(id());

	QString S =
	"<html><head/><body>"
	"<small><i>" + QString(metaObject()->className()) + "</i></small><br/>"
	+ desc;
	S += "<hr/>";
		S += "<i>"+QApplication::translate("MapLayer", "Size")+": </i>" + QApplication::translate("MapLayer", "%n features", "", QCoreApplication::CodecForTr, size())+"<br/>";
	S += "%1";
	S += "</body></html>";

	return S;
}

QString MapLayer::toHtml()
{
	return toMainHtml().arg("");
}


// DrawingMapLayer

DrawingMapLayer::DrawingMapLayer(const QString & aName)
	: MapLayer(aName)
{
	p->Visible = true;
}

DrawingMapLayer::~ DrawingMapLayer()
{
}

LayerWidget* DrawingMapLayer::newWidget(void)
{
//	delete theWidget;
	theWidget = new DrawingLayerWidget(this);
	return theWidget;
}


bool DrawingMapLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
	e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("osm");
	e.appendChild(o);
	o.setAttribute("version", "0.5");
	o.setAttribute("generator", "Merkaartor");

	if (p->Features.size()) {
		QDomElement bb = xParent.ownerDocument().createElement("bound");
		o.appendChild(bb);
		CoordBox layBB = boundingBox();
		QString S = QString().number(intToAng(layBB.bottomLeft().lat()),'f',6) + ",";
		S += QString().number(intToAng(layBB.bottomLeft().lon()),'f',6) + ",";
		S += QString().number(intToAng(layBB.topRight().lat()),'f',6) + ",";
		S += QString().number(intToAng(layBB.topRight().lon()),'f',6);
		bb.setAttribute("box", S);
		bb.setAttribute("origin", QString("http://www.openstreetmap.org/api/%1").arg(M_PREFS->apiVersion()));
	}

	QList<MapFeaturePtr>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++)
		(*it)->toXML(o, progress);

	return OK;
}

DrawingMapLayer * DrawingMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress)
{
	DrawingMapLayer* l = new DrawingMapLayer(e.attribute("name"));
	d->add(l);
	if (!DrawingMapLayer::doFromXML(l, d, e, progress)) {
		d->remove(l);
		delete l;
		return NULL;
	}
	return l;
}

DrawingMapLayer * DrawingMapLayer::doFromXML(DrawingMapLayer* l, MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	l->blockIndexing(true);

	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));
	l->setReadonly((e.attribute("readonly") == "true" ? true : false));

	QDomElement c = e.firstChildElement();
	if (c.tagName() != "osm")
		return NULL;

// 	QByteArray ba;
// 	QTextStream out(&ba);
// 	c.save(out,2);
//
// 	bool importOK = importOSM(NULL, ba, d, l,NULL);
// 	if (importOK == false) {
// 		d->remove(l);
// 		delete l;
// 		return NULL;
// 	}


	int i=0;
	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "bound") {
		} else
		if (c.tagName() == "way") {
			/* Road* R = */ Road::fromXML(d, l, c);
//			l->add(R);
			i++;
		} else
		if (c.tagName() == "relation") {
			/* Relation* r = */ Relation::fromXML(d, l, c);
//			l->add(r);
			i++;
		} else
		if (c.tagName() == "node") {
			/* TrackPoint* N = */ TrackPoint::fromXML(d, l, c);
//			l->add(N);
			i++;
		} else
		if (c.tagName() == "trkseg") {
			TrackSegment* T = TrackSegment::fromXML(d, l, c, progress);
			l->add(T);
			i++;
		}

		if (i >= progress.maximum()/100) {
			progress.setValue(progress.value()+i);
			i=0;
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}

	if (i > 0) progress.setValue(progress.value()+i);

	l->blockIndexing(false);
	l->reIndex();

	return l;
}

// TrackMapLayer

TrackMapLayer::TrackMapLayer(const QString & aName, const QString& filename)
	: MapLayer(aName), Filename(filename)
{
	p->Visible = true;
	p->Readonly = M_PREFS->getReadonlyTracksDefault();
}

TrackMapLayer::~ TrackMapLayer()
{
}

LayerWidget* TrackMapLayer::newWidget(void)
{
	theWidget = new TrackLayerWidget(this);
	return theWidget;
}

void TrackMapLayer::extractLayer()
{
	DrawingMapLayer* extL = new DrawingMapLayer(tr("Extract - %1").arg(name()));
	extL->setUploadable(false);

	TrackPoint* P;
	QList<TrackPoint*> PL;

	const double coordPer10M = (double(INT_MAX) * 2 / 40080000) * 2;

	for (int i=0; i < size(); i++) {
		if (TrackSegment* S = dynamic_cast<TrackSegment*>(get(i))) {

			if (S->size() < 2)
				continue;

			// Cope with walking tracks
			double konstant = coordPer10M;
			double meanSpeed = S->distance() / S->duration() * 3600;
			if (meanSpeed < 10.)
				konstant /= 3.;


			PL.clear();

			P = new TrackPoint( S->getNode(0)->position() );
			if (M_PREFS->apiVersionNum() < 0.6)
				P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			P->setTime(S->getNode(0)->time());
			P->setElevation(S->getNode(0)->elevation());
			P->setSpeed(S->getNode(0)->speed());
			//P->setTag("ele", QString::number(S->get(0)->elevation()));
			PL.append(P);
			int startP = 0;

			P = new TrackPoint( S->getNode(1)->position() );
			if (M_PREFS->apiVersionNum() < 0.6)
				P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			P->setTime(S->getNode(1)->time());
			P->setElevation(S->getNode(1)->elevation());
			P->setSpeed(S->getNode(1)->speed());
			//P->setTag("ele", QString::number(S->get(1)->elevation()));
			PL.append(P);
			int endP = 1;

			for (int j=2; j < S->size(); j++) {
				P = new TrackPoint( S->getNode(j)->position() );
				if (M_PREFS->apiVersionNum() < 0.6)
					P->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
				P->setTime(S->getNode(j)->time());
				P->setElevation(S->getNode(j)->elevation());
				P->setSpeed(S->getNode(j)->speed());
				//P->setTag("ele", QString::number(S->get(j)->elevation()));
				PL.append(P);
				endP = PL.size()-1;

				LineF l(toQt(PL[startP]->position()), toQt(PL[endP]->position()));
				for (int k=startP+1; k < endP; k++) {
					double d = l.distance(toQt(PL[k]->position()));
					if (d < konstant) {
						TrackPoint* P = PL[k];
						PL.removeAt(k);
						delete P;
						endP--;
					} else
						startP = k;
				}
			}

			Road* R = new Road();
			R->setLastUpdated(MapFeature::OSMServer);
			if (M_PREFS->apiVersionNum() < 0.6)
				R->setTag("created_by", QString("Merkaartor v%1%2").arg(STRINGIFY(VERSION)).arg(STRINGIFY(REVISION)));
			extL->add(R);
			for (int i=0; i < PL.size(); i++) {
				extL->add(PL[i]);
				R->add(PL[i]);
			}
		}
	}

	p->theDocument->add(extL);
}

const QString TrackMapLayer::getFilename()
{
	return Filename;
}

QString TrackMapLayer::toHtml()
{
	QString S;

	int totSegment = 0;
	int totSec = 0;
	double totDistance = 0;
	for (int i=0; i < size(); ++i) {
		if (TrackSegment* S = CAST_SEGMENT(get(i))) {
			totSegment++;
			totSec += S->duration();
			totDistance += S->distance();
		}
	}

	S += "<i>"+QApplication::translate("TrackMapLayer", "# of track segments")+": </i>" + QApplication::translate("TrackMapLayer", "%1").arg(QLocale().toString(totSegment))+"<br/>";
	S += "<i>"+QApplication::translate("TrackMapLayer", "Total distance")+": </i>" + QApplication::translate("TrackMapLayer", "%1 km").arg(QLocale().toString(totDistance, 'g', 3))+"<br/>";
	S += "<i>"+QApplication::translate("TrackMapLayer", "Total duration")+": </i>" + QApplication::translate("TrackMapLayer", "%1h %2m").arg(QLocale().toString(totSec/3600)).arg(QLocale().toString((totSec%3600)/60))+"<br/>";

	return toMainHtml().arg(S);
}

bool TrackMapLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
	e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

	QDomElement o = xParent.ownerDocument().createElement("gpx");
	e.appendChild(o);
	o.setAttribute("version", "1.1");
	o.setAttribute("creator", "Merkaartor");
	o.setAttribute("xmlns", "http://www.topografix.com/GPX/1/1");

	QList<TrackPoint*>	waypoints;
	QList<TrackSegment*>	segments;
	QList<MapFeaturePtr>::iterator it;
	for(it = p->Features.begin(); it != p->Features.end(); it++) {
		if (TrackSegment* S = qobject_cast<TrackSegment*>(*it))
			segments.push_back(S);
		if (TrackPoint* P = qobject_cast<TrackPoint*>(*it))
			if (!P->tagValue("_waypoint_","").isEmpty())
				waypoints.push_back(P);
	}

	for (int i=0; i < waypoints.size(); ++i) {
		waypoints[i]->toGPX(o, progress);
	}

	QDomElement t = o.ownerDocument().createElement("trk");
	o.appendChild(t);

	for (int i=0; i < segments.size(); ++i)
		segments[i]->toXML(t, progress);

	return OK;
}

TrackMapLayer * TrackMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress)
{
	TrackMapLayer* l = new TrackMapLayer(e.attribute("name"));
	l->blockIndexing(true);

	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));
	l->setReadonly((e.attribute("readonly") == "true" ? true : false));

	d->add(l);

	QDomElement c = e.firstChildElement();
	if (c.tagName() != "gpx")
		return NULL;

	c = c.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "trk") {
			QDomElement t = c.firstChildElement();
			while(!t.isNull()) {
				if (t.tagName() == "trkseg") {
					TrackSegment* N = TrackSegment::fromXML(d, l, t, progress);
					l->add(N);
				}

				t = t.nextSiblingElement();
			}
		}
		if (c.tagName() == "wpt") {
			/* TrackPoint* N = */ TrackPoint::fromGPX(d, l, c);
			//l->add(N);
			progress.setValue(progress.value()+1);
		}

		if (progress.wasCanceled())
			break;

		c = c.nextSiblingElement();
	}

	l->blockIndexing(false);
	l->reIndex();

	return l;
}

// DirtyMapLayer

DirtyMapLayer::DirtyMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = true;
}

DirtyMapLayer::~ DirtyMapLayer()
{
}

DirtyMapLayer* DirtyMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	DirtyMapLayer* l = new DirtyMapLayer(e.attribute("name"));
	d->add(l);
	d->setDirtyLayer(l);
	DrawingMapLayer::doFromXML(l, d, e, progress);
	return l;
}

LayerWidget* DirtyMapLayer::newWidget(void)
{
	theWidget = new DirtyLayerWidget(this);
	return theWidget;
}

// UploadedMapLayer

UploadedMapLayer::UploadedMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = true;
}

UploadedMapLayer::~ UploadedMapLayer()
{
}

UploadedMapLayer* UploadedMapLayer::fromXML(MapDocument* d, const QDomElement e, QProgressDialog & progress)
{
	UploadedMapLayer* l = new UploadedMapLayer(e.attribute("name"));
	d->add(l);
	d->setUploadedLayer(l);
	DrawingMapLayer::doFromXML(l, d, e, progress);
	return l;
}

LayerWidget* UploadedMapLayer::newWidget(void)
{
	theWidget = new UploadedLayerWidget(this);
	return theWidget;
}

// DeletedMapLayer

DeletedMapLayer::DeletedMapLayer(const QString & aName)
	: DrawingMapLayer(aName)
{
	p->Visible = false;
	p->Enabled = false;
}

DeletedMapLayer::~ DeletedMapLayer()
{
}

bool DeletedMapLayer::toXML(QDomElement& , QProgressDialog & )
{
	return true;
}

DeletedMapLayer* DeletedMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress)
{
	/* Only keep DeletedMapLayer for backward compatibility with MDC */
	DrawingMapLayer::doFromXML(dynamic_cast<DrawingMapLayer*>(d->getDirtyOrOriginLayer()), d, e, progress);
	return NULL;
}

LayerWidget* DeletedMapLayer::newWidget(void)
{
	return NULL;
}

// OsbMapLayer

class OsbMapLayerPrivate
{
public:
	OsbMapLayerPrivate()
		: theImp(0)
	{
	}
	OsbMapLayer* theLayer;
	ImportExportOsmBin* theImp;
	QList<qint32> loadedTiles;
	QList<qint32> loadedRegions;

	int rl;
	QMap< qint32, quint64 >::const_iterator ri;
	void loadRegion(MapDocument* d, int rt);
	void clearRegion(MapDocument* d, int rt);
	void handleTile(MapDocument* d, int rt);
};

OsbMapLayer::OsbMapLayer(const QString & aName)
	: MapLayer(aName)
{
	p->Visible = true;
	pp = new OsbMapLayerPrivate();
	pp->theLayer = this;
	pp->theImp = new ImportExportOsmBin(NULL);
}

OsbMapLayer::OsbMapLayer(const QString & aName, const QString & filename)
	: MapLayer(aName)
{
	p->Visible = true;
	pp = new OsbMapLayerPrivate();
	pp->theLayer = this;
	pp->theImp = new ImportExportOsmBin(NULL);
	if (pp->theImp->loadFile(filename))
		pp->theImp->import(this);
}

OsbMapLayer::~ OsbMapLayer()
{
	delete pp->theImp;
	delete pp;
}

bool OsbMapLayer::arePointsDrawable()
{
	return true;
}

void OsbMapLayer::setFilename(const QString& filename)
{
	delete pp->theImp;

	pp->theImp = new ImportExportOsmBin(NULL);
	if (pp->theImp->loadFile(filename))
		pp->theImp->import(this);
}

LayerWidget* OsbMapLayer::newWidget(void)
{
	theWidget = new OsbLayerWidget(this);
	return theWidget;
}

void OsbMapLayerPrivate::loadRegion(MapDocument* d, int rt)
{
	while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
		if (loadedRegions.at(rl) == rt) {
			++rl;
			return;
		}
		while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
			++rl;
		}
	}
	while(ri != theImp->theRegionToc.constEnd() && ri.key() < rt) {
		++ri;
	}
	if (ri != theImp->theRegionToc.constEnd() && ri.key() == rt) {
		if (theImp->loadRegion(rt, d, theLayer)) {
			loadedRegions.insert(rl, rt);
			++rl;
			++ri;
		}
	}
}

void OsbMapLayerPrivate::handleTile(MapDocument* d, int rt)
{
	while (rl < loadedTiles.size() && loadedTiles.at(rl) <= rt) {
		if (loadedTiles.at(rl) == rt) {
			++rl;
			return;
		}
		while (rl < loadedTiles.size() && loadedTiles.at(rl) < rt ) {
			if (theImp->clearTile(loadedTiles.at(rl), d, theLayer))
				loadedTiles.removeAt(rl);
			else
				++rl;
		}
	}
	if (theImp->loadTile(rt, d, theLayer)) {
		loadedTiles.insert(rl, rt);
		++rl;
	}
}

void OsbMapLayerPrivate::clearRegion(MapDocument* d, int rt)
{
	while (rl < loadedRegions.size() && loadedRegions.at(rl) <= rt) {
		if (loadedRegions.at(rl) == rt) {
			++rl;
			return;
		}
		while (rl < loadedRegions.size() && loadedRegions.at(rl) < rt ) {
			if (theImp->clearRegion(loadedRegions.at(rl), d, theLayer))
				loadedRegions.removeAt(rl);
			else
				++rl;
		}
	}
}

void OsbMapLayer::invalidate(MapDocument* d, CoordBox vp)
{
	if (!isVisible())
		return;

	QRectF r(vp.toQRectF());

	int xr1 = int((r.topLeft().x() + INT_MAX) / REGION_WIDTH);
	int yr1 = int((r.topLeft().y() + INT_MAX) / REGION_WIDTH);
	int xr2 = int((r.bottomRight().x() + INT_MAX) / REGION_WIDTH);
	int yr2 = int((r.bottomRight().y() + INT_MAX) / REGION_WIDTH);

	int xt1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH);
	int yt1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
	int xt2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
	int yt2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);

	pp->rl = 0;
	pp->ri = pp->theImp->theRegionToc.constBegin();

	pp->loadRegion(d, 0);
	for (int j=yr1; j <= yr2; ++j)
		for (int i=xr1; i <= xr2; ++i) {
			pp->loadRegion(d, j*NUM_REGIONS+i);
		}

	pp->rl = 0;
	if (intToAng(vp.lonDiff()) <= M_PREFS->getTileToRegionThreshold()) {
		for (int j=yt1; j <= yt2; ++j)
			for (int i=xt1; i <= xt2; ++i)
				pp->handleTile(d, j*NUM_TILES+i);
	}
	while (pp->rl < pp->loadedTiles.size() ) {
		if (pp->theImp->clearTile(pp->loadedTiles.at(pp->rl), d, this))
			pp->loadedTiles.removeAt(pp->rl);
		else
			++pp->rl;
	}

	pp->rl = 0;
	for (int j=yr1; j <= yr2; ++j)
		for (int i=xr1; i <= xr2; ++i) {
			pp->clearRegion(d, j*NUM_REGIONS+i);
		}

	while (pp->rl < pp->loadedRegions.size()) {
		if (pp->theImp->clearRegion(pp->loadedRegions.at(pp->rl), d, this))
			pp->loadedRegions.removeAt(pp->rl);
		else
			++pp->rl;
	}
}

//MapFeature*  OsbMapLayer::getFeatureByRef(MapDocument* d, quint64 ref)
//{
//	return pp->theImp->getFeature(d, this, ref);
//}

bool OsbMapLayer::toXML(QDomElement& xParent, QProgressDialog & progress)
{
	Q_UNUSED(progress);

	bool OK = true;

	QDomElement e = xParent.ownerDocument().createElement(metaObject()->className());
	xParent.appendChild(e);

	e.setAttribute("xml:id", id());
	e.setAttribute("name", p->Name);
	e.setAttribute("alpha", QString::number(p->alpha,'f',2));
	e.setAttribute("visible", QString((p->Visible ? "true" : "false")));
	e.setAttribute("selected", QString((p->selected ? "true" : "false")));
	e.setAttribute("enabled", QString((p->Enabled ? "true" : "false")));
	e.setAttribute("readonly", QString((p->Readonly ? "true" : "false")));

	e.setAttribute("filename", pp->theImp->getFilename());

	return OK;
}

OsbMapLayer * OsbMapLayer::fromXML(MapDocument* d, const QDomElement& e, QProgressDialog & progress)
{
	Q_UNUSED(progress);

	OsbMapLayer* l = new OsbMapLayer(e.attribute("name"));

	l->setId(e.attribute("xml:id"));
	l->setAlpha(e.attribute("alpha").toDouble());
	l->setVisible((e.attribute("visible") == "true" ? true : false));
	l->setSelected((e.attribute("selected") == "true" ? true : false));
	l->setEnabled((e.attribute("enabled") == "false" ? false : true));
	l->setReadonly((e.attribute("readonly") == "true" ? true : false));

	if (l->pp->theImp->loadFile(e.attribute("filename")))
		l->pp->theImp->import(l);
	else {
		delete l;
		return NULL;
	}

	d->add(l);
	return l;
}

QString OsbMapLayer::toHtml()
{
	QString S;

	S += "<i>"+QApplication::translate("OsbMapLayer", "# of loaded Regions")+": </i>" + QApplication::translate("OsbMapLayer", "%1").arg(QLocale().toString(pp->loadedRegions.size()))+"<br/>";
	S += "<i>"+QApplication::translate("OsbMapLayer", "# of loaded Tiles")+": </i>" + QApplication::translate("OsbMapLayer", "%1").arg(QLocale().toString(pp->loadedTiles.size()))+"<br/>";

	return toMainHtml().arg(S);
}

