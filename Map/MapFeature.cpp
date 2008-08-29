#include "Map/MapFeature.h"
#include "Map/Relation.h"
#include "Map/Road.h"
#include "Map/TrackPoint.h"
#include "Command/Command.h"
#include "Command/DocumentCommands.h"
#include "Command/FeatureCommands.h"
#include "Command/RelationCommands.h"
#include "Command/RoadCommands.h"
#include "Command/TrackPointCommands.h"
#include "Map/MapDocument.h"
#include "Map/MapLayer.h"
#include "PaintStyle/EditPaintStyle.h"
#include "PaintStyle/TagSelector.h"

#include <QtCore/QUuid>
#include <QProgressDialog>

#include <algorithm>

static QString randomId()
{
	return QUuid::createUuid().toString();
}

void copyTags(MapFeature* Dest, MapFeature* Src)
{
	for (unsigned int i=0; i<Src->tagSize(); ++i)
		Dest->setTag(Src->tagKey(i),Src->tagValue(i));
}

class MapFeaturePrivate
{
	public:
		MapFeaturePrivate()
			: LastActor(MapFeature::User), theLayer(0),
				PossiblePaintersUpToDate(false),
			  	PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
				theFeature(0), LastPartNotification(0), Time(QDateTime::currentDateTime()),
				TagsSize(0)
		{
			initVersionNumber();
		}
		MapFeaturePrivate(const MapFeaturePrivate& other)
			: Tags(other.Tags), LastActor(other.LastActor), theLayer(0),
				PossiblePaintersUpToDate(false),
			  	PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
				theFeature(0), LastPartNotification(0), Time(other.Time),
				TagsSize(other.TagsSize)
		{
			initVersionNumber();
		}

		void updatePainters(double PixelPerM);
		void blankPainters(double PixelPerM);
		void initVersionNumber();

		mutable QString Id;
		std::vector<std::pair<QString, QString> > Tags;
		unsigned int TagsSize;
		MapFeature::ActorType LastActor;
		MapLayer* theLayer;
		std::vector<FeaturePainter*> PossiblePainters;
		bool PossiblePaintersUpToDate;
		double PixelPerMForPainter;
		FeaturePainter* CurrentPainter;
		bool HasPainter;
		MapFeature* theFeature;
		std::vector<MapFeature*> Parents;
		unsigned int LastPartNotification;
		QDateTime Time;
		QString User;
		int VersionNumber;
};

void MapFeaturePrivate::initVersionNumber()
{
	static int VN = -1;
	VersionNumber = VN--;
}

MapFeature::MapFeature()
: p(0)
{
	 p = new MapFeaturePrivate;
	 p->theFeature = this;
}

MapFeature::MapFeature(const MapFeature& other)
{
	p = new MapFeaturePrivate(*other.p);
	p->theFeature = this;
}

MapFeature::~MapFeature(void)
{
	if (p->theLayer)
		p->theLayer->notifyIdUpdate(p->Id,0);
	delete p;
}

void MapFeature::setVersionNumber(int vn)
{
	p->VersionNumber = vn;
}

int MapFeature::versionNumber() const
{
	return p->VersionNumber;
}

void MapFeature::setLayer(MapLayer* aLayer)
{
	p->theLayer = aLayer;
}

MapLayer* MapFeature::layer()
{
	return p->theLayer;
}

void MapFeature::setLastUpdated(MapFeature::ActorType A)
{
	p->LastActor = A;
}

MapFeature::ActorType MapFeature::lastUpdated() const
{
	return p->LastActor;
}

void MapFeature::setId(const QString& id)
{
	if (p->theLayer)
	{
		p->theLayer->notifyIdUpdate(p->Id,0);
		p->theLayer->notifyIdUpdate(id,this);
	}
	p->Id = id;
}

const QString& MapFeature::id() const
{
	if (p->Id == "")
	{
		p->Id = randomId();
		if (p->theLayer)
			p->theLayer->notifyIdUpdate(p->Id,const_cast<MapFeature*>(this));
	}
	return p->Id;
}

qint64 MapFeature::idToLong() const
{
	bool ok;

	if (hasOSMId()) {
		QString s = stripToOSMId(id());
		qint64 l = s.toLongLong(&ok);
		Q_ASSERT(ok);
		return l;
	} else {
		return (((qint64)this) * -1);
	}
}

QString MapFeature::xmlId() const
{
	return stripToOSMId(id());
}

bool MapFeature::hasOSMId() const
{
	if (p->Id.left(5) == "node_")
		return true;
	if (p->Id.left(4) == "way_")
		return true;
	if (p->Id.left(4) == "rel_")
		return true;
	return false;
}

const QDateTime& MapFeature::time() const
{
	return p->Time;
}

void MapFeature::setTime(const QDateTime& time)
{
	p->Time = time;
	notifyChanges();
}

const QString& MapFeature::user() const
{
	return p->User;
}

void MapFeature::setUser(const QString& user)
{
	p->User = user;
	notifyChanges();
}

bool MapFeature::isDirty()
{
	return (p->theLayer->className() == "DirtyMapLayer");
}

bool MapFeature::isUploadable()
{
	return (p->theLayer->isUploadable());
}

void MapFeature::setTag(unsigned int index, const QString& key, const QString& value, bool addToTagList)
{
	p->PixelPerMForPainter = -1;
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == key)
		{
			p->Tags[i].second = value;
			notifyChanges();
			return;
		}
	p->Tags.insert(p->Tags.begin() + index, std::make_pair(key,value));
	p->TagsSize++;
	if (p->theLayer && addToTagList)
		if (p->theLayer->getDocument())
	  		p->theLayer->getDocument()->addToTagList(key, value);
	notifyChanges();
}

void MapFeature::setTag(const QString& key, const QString& value, bool addToTagList)
{
	p->PixelPerMForPainter = -1;
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == key)
		{
			p->Tags[i].second = value;
			notifyChanges();
			return;
		}
	p->Tags.push_back(std::make_pair(key,value));
	p->TagsSize++;
	if (p->theLayer && addToTagList)
		if (p->theLayer->getDocument())
  			p->theLayer->getDocument()->addToTagList(key, value);
	notifyChanges();
}

void MapFeature::clearTags()
{
	p->PixelPerMForPainter = -1;
	p->Tags.clear();
	p->TagsSize = 0;
	notifyChanges();
}

void MapFeature::invalidatePainter()
{
	p->PossiblePaintersUpToDate = false;
	p->PixelPerMForPainter = -1;
}

void MapFeature::clearTag(const QString& k)
{
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
		{
			p->PixelPerMForPainter = -1;
			p->Tags.erase(p->Tags.begin()+i);
			p->TagsSize--;
			notifyChanges();
			return;
		}
}

void MapFeature::removeTag(unsigned int idx)
{
	p->PixelPerMForPainter = -1;
	p->Tags.erase(p->Tags.begin()+idx);
	p->TagsSize--;
	notifyChanges();
}

unsigned int MapFeature::tagSize() const
{
	return p->TagsSize;
}

QString MapFeature::tagValue(unsigned int i) const
{
	return p->Tags[i].second;
}

QString MapFeature::tagKey(unsigned int i) const
{
	return p->Tags[i].first;
}

unsigned int MapFeature::findKey(const QString &k) const
{
	for (unsigned int i=0; i<p->TagsSize; ++i)
		if (p->Tags[i].first == k)
			return i;
	return p->Tags.size();
}

QString MapFeature::tagValue(const QString& k, const QString& Default) const
{
	for (unsigned int i=0; i<p->TagsSize; ++i)
		if (p->Tags[i].first == k)
			return p->Tags[i].second;
	return Default;
}

void MapFeaturePrivate::updatePainters(double PixelPerM)
{
	//if the object has no tags or only the created_by tag, we don't check for style
	//search is about 15 times faster like that !!!
	//However, still match features with no tags and no parent, i.e. "lost" trackpoints
	if ( (theFeature->layer()->isTrack()) && MerkaartorPreferences::instance()->getDisableStyleForTracks() ) return blankPainters(PixelPerM);

	if ( (theFeature->layer()->isTrack()) || theFeature->sizeParents() ) {
		unsigned int i;
		for (i=0; i<theFeature->tagSize(); ++i)
			if ((theFeature->tagKey(i) != "created_by") && (theFeature->tagKey(i) != "ele"))
				break;

		if (i == theFeature->tagSize()) return blankPainters(PixelPerM);
	}

	if (PixelPerMForPainter < 0)
	{
		PossiblePainters.clear();
		std::vector<FeaturePainter*> DefaultPainters;
		for (unsigned int i=0; i<EditPaintStyle::Painters.size(); ++i)
		{
			FeaturePainter* Current = &EditPaintStyle::Painters[i];
			switch (Current->matchesTag(theFeature)) {
				case TagSelect_Match:
					PossiblePainters.push_back(Current);
					break;
				case TagSelect_DefaultMatch:
					DefaultPainters.push_back(Current);
					break;
				default:
					break;
			}
		}
		if (!PossiblePainters.size())
			PossiblePainters = DefaultPainters;
		PossiblePaintersUpToDate = true;
		HasPainter = PossiblePainters.size();
	}
	CurrentPainter = 0;
	PixelPerMForPainter = PixelPerM;
	for (unsigned int i=0; i<PossiblePainters.size(); ++i)
		if (PossiblePainters[i]->matchesZoom(PixelPerM))
		{
			CurrentPainter = PossiblePainters[i];
			return;
		}
}

void MapFeaturePrivate::blankPainters(double PixelPerM)
{
	CurrentPainter = 0;
	PixelPerMForPainter = PixelPerM;
	PossiblePainters.clear();
	HasPainter = false;
}

FeaturePainter* MapFeature::getEditPainter(double PixelPerM) const
{
	if (p->PixelPerMForPainter != PixelPerM)
		p->updatePainters(PixelPerM);
	return p->CurrentPainter;
}

FeaturePainter* MapFeature::getCurrentEditPainter() const
{
	return p->CurrentPainter;
}

bool MapFeature::hasEditPainter() const
{
	return p->HasPainter;
}

void MapFeature::setParent(MapFeature* F)
{
	if (std::find(p->Parents.begin(),p->Parents.end(),F) == p->Parents.end())
		p->Parents.push_back(F);
}

void MapFeature::unsetParent(MapFeature* F)
{
	for (unsigned int i=0; i<p->Parents.size(); ++i)
		if (p->Parents[i] == F)
		{
			p->Parents.erase(p->Parents.begin()+i);
			return;
		}
}

unsigned int MapFeature::sizeParents() const
{
	return p->Parents.size();
}

MapFeature* MapFeature::getParent(unsigned int i)
{
	return p->Parents[i];
}

const MapFeature* MapFeature::getParent(unsigned int i) const
{
	return p->Parents[i];
}


void MapFeature::notifyChanges()
{
	static unsigned int Id = 0;
	notifyParents(++Id);
	if (p->theLayer)
		p->theLayer->invalidateRenderPriority();
}

void MapFeature::notifyParents(unsigned int Id)
{
	if (Id != p->LastPartNotification)
	{
		p->LastPartNotification = Id;
		for (unsigned int i=0; i<p->Parents.size(); ++i)
			p->Parents[i]->partChanged(this, Id);
	}
}

QString MapFeature::tagsToXML(unsigned int lvl)
{
	QString S;
	for (unsigned int i=0; i<tagSize(); ++i)
	{
		S += QString(lvl*2, ' ') + QString("<tag k=\"%1\" v=\"%2\"/>\n").arg(tagKey(i)).arg(tagValue(i));
	}
	return S;
}

bool MapFeature::tagsToXML(QDomElement xParent)
{
	bool OK = true;

	for (unsigned int i=0; i<tagSize(); ++i)
	{
		QDomElement e = xParent.ownerDocument().createElement("tag");
		xParent.appendChild(e);

		e.setAttribute("k", tagKey(i));
		e.setAttribute("v", tagValue(i));
	}

	return OK;
}

void MapFeature::tagsFromXML(MapDocument* d, MapFeature * f, QDomElement e)
{
	Q_UNUSED(d)
	QDomElement c = e.firstChildElement();
	while(!c.isNull()) {
		if (c.tagName() == "tag") {
			f->setTag(c.attribute("k"),c.attribute("v"));
		}
		c = c.nextSiblingElement();
	}
}

bool MapFeature::tagsToBinary(QDataStream& ds)
{
	bool OK = true;
	qint32 k, v;

	ds << (qint32)tagSize();
	for (unsigned int i=0; i<tagSize(); ++i) {
		k = (qint32)(p->theLayer->getDocument()->getTagKeyIndex(tagKey(i)));
		Q_ASSERT(!(k<0));
		v = (qint32)(p->theLayer->getDocument()->getTagValueIndex(tagValue(i)));
		Q_ASSERT(!(v<0));
		ds << k;
		ds << v;
	}

	return OK;
}

void MapFeature::tagsFromBinary(MapDocument* d, MapFeature * f, QDataStream& ds)
{
	quint32 numTags;
	quint32 k,v;
	QString K, V;

	ds >> numTags;
	for (unsigned int i=0; i < numTags; ++i) {
		ds >> k;
		ds >> v;
		K = d->getTagKey(k);
		V = d->getTagValue(v);
		f->setTag(K,V);
	}
}

QString MapFeature::stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

//Static
TrackPoint* MapFeature::getTrackPointOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id)
{
	TrackPoint* Part = dynamic_cast<TrackPoint*>(theDocument->getFeature("node_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<TrackPoint*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new TrackPoint(Coord(0,0));
			if (Id.startsWith('{'))
				Part->setId(Id);
			else
				Part->setId("node_"+Id);
			Part->setLastUpdated(MapFeature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

Road* MapFeature::getWayOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id)
{
	Road* Part = dynamic_cast<Road*>(theDocument->getFeature("way_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<Road*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new Road;
			if (Id.startsWith('{'))
				Part->setId(Id);
			else
				Part->setId("way_"+Id);
			Part->setLastUpdated(MapFeature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

Relation* MapFeature::getRelationOrCreatePlaceHolder(MapDocument *theDocument, MapLayer *theLayer, const QString& Id)
{
	Relation* Part = dynamic_cast<Relation*>(theDocument->getFeature("rel_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<Relation*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new Relation;
			if (Id.startsWith('{'))
				Part->setId(Id);
			else
				Part->setId("rel_"+Id);
			Part->setLastUpdated(MapFeature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

void MapFeature::mergeTags(MapDocument* theDocument, CommandList* L, MapFeature* Dest, MapFeature* Src)
{
	for (unsigned int i=0; i<Src->tagSize(); ++i)
	{
		QString k = Src->tagKey(i);
		QString v1 = Src->tagValue(i);
		unsigned int j = Dest->findKey(k);
		if (j == Dest->tagSize())
			L->add(new SetTagCommand(Dest,k,v1, theDocument->getDirtyOrOriginLayer(Dest->layer())));
		else
		{
			QString v2 = Dest->tagValue(j);
			if (v1 != v2 && k !="created_by")
			{
				L->add(new SetTagCommand(Dest,k,QString("%1;%2").arg(v2).arg(v1), theDocument->getDirtyOrOriginLayer(Dest->layer())));
			}
		}
	}
}


QString MapFeature::toMainHtml(QString type, QString systemtype)
{
	QString desc;
	QString name(tagValue("name",""));
	if (!name.isEmpty())
		desc = QString("<big><b>%1</b></big><br/><small>(%2)</small>").arg(name).arg(id());
	else
		desc = QString("<big><b>%1</b></big>").arg(id());

	QString S =
	"<html><head/><body>"
	"<small><i>" + type + "</i></small><br/>"
	+ desc + 
	"<br/>"
	"<small>";
	if (!user().isEmpty())
		S += QApplication::translate("MapFeature", "<i>last: </i><b>%1</b> by <b>%2</b>").arg(time().toString(Qt::SystemLocaleDate)).arg(user());
        else
		S += QApplication::translate("MapFeature", "<i>last: </i><b>%1</b>").arg(time().toString(Qt::SystemLocaleDate));
	S += "</small>"
	"<hr/>"
	"%1";
	int f = id().lastIndexOf("_");
	if (f>0) {
		S += "<hr/>"
		"<a href='/api/0.5/" + systemtype + "/" + xmlId() + "/history'>"+QApplication::translate("MapFeature", "History")+"</a>";
		if (systemtype == "node") {
			S += "<br/>"
			"<a href='/api/0.5/" + systemtype + "/" + xmlId() + "/ways'>"+QApplication::translate("MapFeature", "Referenced by ways")+"</a>";
		}
		S += "<br/>"
		"<a href='/api/0.5/" + systemtype + "/" + xmlId() + "/relations'>"+QApplication::translate("MapFeature", "Referenced by relation")+"</a>";
	}
	S += "</body></html>";

	return S;
}
