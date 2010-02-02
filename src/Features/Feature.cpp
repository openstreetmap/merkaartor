#include "Features.h"
#include "Command.h"
#include "DocumentCommands.h"
#include "FeatureCommands.h"
#include "RelationCommands.h"
#include "WayCommands.h"
#include "NodeCommands.h"
#include "Document.h"
#include "Layer.h"
#include "PaintStyle/MasPaintStyle.h"
#include "PaintStyle/TagSelector.h"

#include <QtCore/QUuid>
#include <QProgressDialog>

#include <algorithm>

const QString encodeAttributes(const QString & text);

//static QString randomId()
//{
//	QUuid uuid = QUuid::createUuid();
//#ifdef _MOBILE
//	return uuid.toString();
//#else
//	// This is fairly horrible, but it's also around 10 times faster than QUuid::toString()
//	// and randomId() is called a lot during large imports
//
//	// Lookup table of hex value-pairs representing a byte
//	static char hex[(2*256)+1] = "000102030405060708090a0b0c0d0e0f101112131415161718191a1b1c1d1e1f202122232425262728292a2b2c2d2e2f303132333435363738393a3b3c3d3e3f404142434445464748494a4b4c4d4e4f505152535455565758595a5b5c5d5e5f606162636465666768696a6b6c6d6e6f707172737475767778797a7b7c7d7e7f808182838485868788898a8b8c8d8e8f909192939495969798999a9b9c9d9e9fa0a1a2a3a4a5a6a7a8a9aaabacadaeafb0b1b2b3b4b5b6b7b8b9babbbcbdbebfc0c1c2c3c4c5c6c7c8c9cacbcccdcecfd0d1d2d3d4d5d6d7d8d9dadbdcdddedfe0e1e2e3e4e5e6e7e8e9eaebecedeeeff0f1f2f3f4f5f6f7f8f9fafbfcfdfeff";
//
//	char buffer[39] = "{________-____-____-____-____________}";
//
//
//
//	// {__%08x__-____-____-____-____________}
//	memcpy(&buffer[ 1], &hex[((uuid.data1 >> 24) & 0xFF)], 2);
//	memcpy(&buffer[ 3], &hex[((uuid.data1 >> 16) & 0xFF)], 2);
//	memcpy(&buffer[ 5], &hex[((uuid.data1 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[ 7], &hex[((uuid.data1 >>  0) & 0xFF)], 2);
//
//	// {________-%04x-____-____-____________}
//	memcpy(&buffer[10], &hex[((uuid.data2 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[12], &hex[((uuid.data2 >>  0) & 0xFF)], 2);
//
//	// {________-____-%04x-____-____________}
//	memcpy(&buffer[15], &hex[((uuid.data3 >>  8) & 0xFF)], 2);
//	memcpy(&buffer[17], &hex[((uuid.data3 >>  0) & 0xFF)], 2);
//
//	// {________-____-____-%04x-____________}
//	memcpy(&buffer[20], &hex[uuid.data4[0]], 2);
//	memcpy(&buffer[22], &hex[uuid.data4[1]], 2);
//
//	// {________-____-____-____-___%012x____}
//	for (int i=0; i<6; i++) {
//		memcpy(&buffer[25+(i*2)], &hex[uuid.data4[i+2]], 2);
//	}
//
//	return QString::fromAscii(buffer,38);
//#endif
//}

namespace boost
{
	void intrusive_ptr_add_ref(Feature * p)
	{
		++(p->m_references);
	}
	void intrusive_ptr_release(Feature * p)
	{
		if (--(p->m_references) == 0) {
			if (p->layer())
				p->layer()->deleteFeature(p);
			else
				delete p;
		}
	}
} // namespace boost


void copyTags(Feature* Dest, Feature* Src)
{
	for (int i=0; i<Src->tagSize(); ++i)
		Dest->setTag(Src->tagKey(i),Src->tagValue(i));
}

class MapFeaturePrivate
{
	public:
		MapFeaturePrivate()
			:  TagsSize(0), LastActor(Feature::User),
				PossiblePaintersUpToDate(false),
				PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
				theFeature(0), LastPartNotification(0),
				Time(QDateTime::currentDateTime()), Deleted(false), Visible(true), Uploaded(false), LongId(0)
				, Virtual(false)
		{
			initVersionNumber();
		}
		MapFeaturePrivate(const MapFeaturePrivate& other)
			: Tags(other.Tags), TagsSize(other.TagsSize), LastActor(other.LastActor),
				PossiblePaintersUpToDate(false),
				PixelPerMForPainter(-1), CurrentPainter(0), HasPainter(false),
				theFeature(0), LastPartNotification(0),
				Time(other.Time), Deleted(false), Visible(true), Uploaded(false), LongId(0)
				, Virtual(other.Virtual)
		{
			initVersionNumber();
		}

		void updatePossiblePainters();
		void blankPainters();
		void updatePainters(double PixelPerM);
		void initVersionNumber();

		mutable QString Id;
		QList<QPair<QString, QString> > Tags;
		int TagsSize;
		Feature::ActorType LastActor;
		QList<const FeaturePainter*> PossiblePainters;
		bool PossiblePaintersUpToDate;
		double PixelPerMForPainter;
		const FeaturePainter* CurrentPainter;
		bool HasPainter;
		Feature* theFeature;
		QList<Feature*> Parents;
		int LastPartNotification;
		QDateTime Time;
		QString User;
		int VersionNumber;
		bool Deleted;
		bool Visible;
		bool Uploaded;
		qint64 LongId;
		RenderPriority theRenderPriority;
		bool Virtual;
};

void MapFeaturePrivate::initVersionNumber()
{
	static int VN = -1;
	VersionNumber = VN--;
}

Feature::Feature()
: p(0), MetaUpToDate(false), m_references(0)
{
	 p = new MapFeaturePrivate;
	 p->theFeature = this;
}

Feature::Feature(const Feature& other)
: QObject(), MetaUpToDate(false), m_references(0)
{
	p = new MapFeaturePrivate(*other.p);
	p->theFeature = this;
}

Feature::~Feature(void)
{
	while (sizeParents())
		getParent(0)->remove(this);
	delete p;
}

void Feature::setVersionNumber(int vn)
{
	p->VersionNumber = vn;
}

int Feature::versionNumber() const
{
	return p->VersionNumber;
}

void Feature::setLayer(Layer* aLayer)
{
	setParent(aLayer);
}

Layer* Feature::layer()
{
	return dynamic_cast<Layer*>(parent());
}

const RenderPriority& Feature::renderPriority()
{
	return p->theRenderPriority;
}

void Feature::setRenderPriority(const RenderPriority& aPriority)
{
	p->theRenderPriority = aPriority;
}

void Feature::setLastUpdated(Feature::ActorType A)
{
	p->LastActor = A;
}

Feature::ActorType Feature::lastUpdated() const
{
	Layer* L = dynamic_cast<Layer*>(parent());
	if (L && L->classType() == Layer::DirtyLayerType)
		return Feature::User;
	else
		return p->LastActor;
}

void Feature::setId(const QString& id)
{
	if (parent())
	{
		dynamic_cast<Layer*>(parent())->notifyIdUpdate(p->Id,0);
		dynamic_cast<Layer*>(parent())->notifyIdUpdate(id,this);
	}
	p->Id = id;
}

const QString& Feature::resetId()
{
	p->Id = QString::number((((qint64)this) * -1));
	if (parent())
		dynamic_cast<Layer*>(parent())->notifyIdUpdate(p->Id,const_cast<Feature*>(this));
	return p->Id;
}

const QString& Feature::id() const
{
	if (p->Id.isEmpty())
	{
		p->Id = QString::number((((qint64)this) * -1));
		Layer* L = dynamic_cast<Layer*>(parent());
		if (L)
			L->notifyIdUpdate(p->Id,const_cast<Feature*>(this));
	}
	return p->Id;
}

qint64 Feature::idToLong() const
{
	if (p->LongId)
		return p->LongId;

	if (hasOSMId()) {
		bool ok;
		QString s = stripToOSMId(id());
		p->LongId = s.toLongLong(&ok);
		Q_ASSERT(ok);
	} else {
		p->LongId = (((qint64)this) * -1);
	}

	return p->LongId;
}

QString Feature::xmlId() const
{
	return stripToOSMId(id());
}

bool Feature::hasOSMId() const
{
	if (p->Id.left(5) == "node_")
		return true;
	if (p->Id.left(4) == "way_")
		return true;
	if (p->Id.left(4) == "rel_")
		return true;
	return false;
}

const QDateTime& Feature::time() const
{
	return p->Time;
}

void Feature::setTime(const QDateTime& time)
{
	p->Time = time;
}

const QString& Feature::user() const
{
	return p->User;
}

void Feature::setUser(const QString& user)
{
	p->User = user;
}

bool Feature::isDirty() const
{
	if (parent())
		return (dynamic_cast<Layer*>(parent())->classType() == Layer::DirtyLayerType);
	else
		return false;
}

void Feature::setUploaded(bool state)
{
	p->Uploaded = state;
}

bool Feature::isUploaded() const
{
	return p->Uploaded;
}

bool Feature::isUploadable() const
{
	if (parent())
		return (dynamic_cast<Layer*>(parent())->isUploadable());
	else
		return false;
}

void Feature::setDeleted(bool delState)
{
	if (delState == p->Deleted)
		return;

	if (layer()) {
		if (delState)
			layer()->indexRemove(boundingBox(), this);
		else
			layer()->indexAdd(boundingBox(), this);
	}
	p->Deleted = delState;
}

bool Feature::isDeleted() const
{
	return p->Deleted;
}

void Feature::setVisible(bool state)
{
	if (state == p->Visible)
		return;

	if (layer()) {
		if (!state)
			layer()->indexRemove(boundingBox(), this);
		else
			layer()->indexAdd(boundingBox(), this);
	}
	p->Visible = state;
}

bool Feature::isVisible() const
{
	return p->Visible;
}

bool Feature::isHidden() const
{
	return !p->Visible;
}

void Feature::setVirtual(bool val)
{
	p->Virtual = val;
}

bool Feature::isVirtual() const
{
	return p->Virtual;
}

void Feature::setTag(int index, const QString& key, const QString& value, bool addToTagList)
{
	int i = 0;
	for (; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == key)
		{
			if (p->Tags[i].second == value)
				return;
			p->Tags[i].second = value;
			break;
		}
	if (i == p->Tags.size()) {
		p->Tags.insert(p->Tags.begin() + index, qMakePair(key,value));
		p->TagsSize++;
	}
	p->PixelPerMForPainter = -1;
	invalidatePainter();
	invalidateMeta();

	if (parent() && addToTagList)
		if (dynamic_cast<Layer*>(parent())->getDocument())
			dynamic_cast<Layer*>(parent())->getDocument()->addToTagList(key, value);
}

void Feature::setTag(const QString& key, const QString& value, bool addToTagList)
{
	int i = 0;
	for (; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == key)
		{
			if (p->Tags[i].second == value)
				return;
			p->Tags[i].second = value;
			break;
		}
	if (i == p->Tags.size()) {
		p->Tags.push_back(qMakePair(key,value));
		p->TagsSize++;
	}
	p->PixelPerMForPainter = -1;
	invalidateMeta();
	invalidatePainter();

	if (parent() && addToTagList)
		if (dynamic_cast<Layer*>(parent())->getDocument())
			dynamic_cast<Layer*>(parent())->getDocument()->addToTagList(key, value);
}

void Feature::clearTags()
{
	p->PixelPerMForPainter = -1;
	p->Tags.clear();
	p->TagsSize = 0;
	invalidateMeta();
	invalidatePainter();
}

void Feature::invalidateMeta()
{
	MetaUpToDate = false;
}

void Feature::invalidatePainter()
{
	p->PossiblePaintersUpToDate = false;
	p->PixelPerMForPainter = -1;
}

void Feature::clearTag(const QString& k)
{
	for (int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
		{
			p->PixelPerMForPainter = -1;
			p->Tags.erase(p->Tags.begin()+i);
			p->TagsSize--;
			return;
		}
	invalidateMeta();
	invalidatePainter();
}

void Feature::removeTag(int idx)
{
	p->PixelPerMForPainter = -1;
	p->Tags.erase(p->Tags.begin()+idx);
	p->TagsSize--;
	invalidateMeta();
	invalidatePainter();
}

int Feature::tagSize() const
{
	return p->TagsSize;
}

QString Feature::tagValue(int i) const
{
	return p->Tags[i].second;
}

QString Feature::tagKey(int i) const
{
	return p->Tags[i].first;
}

int Feature::findKey(const QString &k) const
{
	for (int i=0; i<p->TagsSize; ++i)
		if (p->Tags[i].first == k)
			return i;
	return p->Tags.size();
}

QString Feature::tagValue(const QString& k, const QString& Default) const
{
	for (int i=0; i<p->TagsSize; ++i)
		if (p->Tags[i].first == k)
			return p->Tags[i].second;
	return Default;
}

void MapFeaturePrivate::updatePossiblePainters()
{
	//if the object has no tags or only the created_by tag, we don't check for style
	//search is about 15 times faster like that !!!
	//However, still match features with no tags and no parent, i.e. "lost" trackpoints
	if ( (theFeature->layer()->isTrack()) && MerkaartorPreferences::instance()->getDisableStyleForTracks() ) return blankPainters();

	if ( (theFeature->layer()->isTrack()) || theFeature->sizeParents() ) {
		int i;
		for (i=0; i<theFeature->tagSize(); ++i)
			if ((theFeature->tagKey(i) != "created_by") && (theFeature->tagKey(i) != "ele"))
				break;

		if (i == theFeature->tagSize()) return blankPainters();
	}

	PossiblePainters.clear();
	QList<const FeaturePainter*> DefaultPainters;
	for (int i=0; i<M_STYLE->painterSize(); ++i)
	{
		const FeaturePainter* Current = M_STYLE->getPainter(i);
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
	HasPainter = (PossiblePainters.size() > 0);
}

void MapFeaturePrivate::updatePainters(double PixelPerM)
{
	if (!PossiblePaintersUpToDate)
		updatePossiblePainters();

	CurrentPainter = 0;
	PixelPerMForPainter = PixelPerM;
	for (int i=0; i<PossiblePainters.size(); ++i)
		if (PossiblePainters[i]->matchesZoom(PixelPerM))
		{
			CurrentPainter = PossiblePainters[i];
			return;
		}
}

void MapFeaturePrivate::blankPainters()
{
	CurrentPainter = 0;
	PossiblePainters.clear();
	PossiblePaintersUpToDate = true;
	HasPainter = false;
}

const FeaturePainter* Feature::getEditPainter(double PixelPerM) const
{
	if (p->PixelPerMForPainter != PixelPerM)
		p->updatePainters(PixelPerM);
	return p->CurrentPainter;
}

const FeaturePainter* Feature::getCurrentEditPainter() const
{
	return p->CurrentPainter;
}

bool Feature::hasEditPainter() const
{
	if (!p->PossiblePaintersUpToDate)
		p->updatePossiblePainters();

	return p->HasPainter;
}

void Feature::setParentFeature(Feature* F)
{
	if (std::find(p->Parents.begin(),p->Parents.end(),F) == p->Parents.end())
		p->Parents.push_back(F);
}

void Feature::unsetParentFeature(Feature* F)
{
	for (int i=0; i<p->Parents.size(); ++i)
		if (p->Parents[i] == F)
		{
			p->Parents.erase(p->Parents.begin()+i);
			return;
		}
}

int Feature::sizeParents() const
{
	return p->Parents.size();
}

Feature* Feature::getParent(int i)
{
	return p->Parents[i];
}

const Feature* Feature::getParent(int i) const
{
	return p->Parents[i];
}

void Feature::notifyChanges()
{
	static int Id = 0;
	notifyParents(++Id);
}

void Feature::notifyParents(int Id)
{
	if (Id != p->LastPartNotification)
	{
		p->LastPartNotification = Id;
		for (int i=0; i<p->Parents.size(); ++i)
			p->Parents[i]->partChanged(this, Id);
	}
}

QString Feature::tagsToXML(int lvl)
{
	QString S;
	for (int i=0; i<tagSize(); ++i)
	{
		S += QString(lvl*2, ' ') + QString("<tag k=\"%1\" v=\"%2\"/>\n").arg(encodeAttributes(tagKey(i))).arg(encodeAttributes(tagValue(i)));
	}
	return S;
}

bool Feature::tagsToXML(QDomElement xParent)
{
	bool OK = true;

	for (int i=0; i<tagSize(); ++i)
	{
		QDomElement e = xParent.ownerDocument().createElement("tag");
		xParent.appendChild(e);

		e.setAttribute("k", tagKey(i));
		e.setAttribute("v", tagValue(i));
	}

	return OK;
}

void Feature::tagsFromXML(Document* d, Feature * f, QDomElement e)
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

QString Feature::stripToOSMId(const QString& id)
{
	int f = id.lastIndexOf("_");
	if (f>0)
		return id.right(id.length()-(f+1));
	return id;
}

Relation * Feature::GetSingleParentRelation(Feature * mapFeature)
{
	int parents = mapFeature->sizeParents();

	if (parents == 0)
		return NULL;

	Relation * parentRelation = NULL;

	int i;
	for (i=0; i<parents; i++)
	{
		Feature * parent = mapFeature->getParent(i);
		Relation * rel = dynamic_cast<Relation*>(parent);

		if (rel == NULL)
			continue;

		if (parentRelation)
			return NULL;

		if (rel->layer()->isEnabled())
			parentRelation = rel;
	}

	return parentRelation;
}

//Static
Node* Feature::getTrackPointOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id)
{
	Node* Part = dynamic_cast<Node*>(theDocument->getFeature("node_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<Node*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new Node(Coord(0,0));
			if (Id.startsWith('{') || Id.startsWith('-'))
				Part->setId(Id);
			else
				Part->setId("node_"+Id);
			Part->setLastUpdated(Feature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

Way* Feature::getWayOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id)
{
	Way* Part = dynamic_cast<Way*>(theDocument->getFeature("way_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<Way*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new Way;
			if (Id.startsWith('{') || Id.startsWith('-'))
				Part->setId(Id);
			else
				Part->setId("way_"+Id);
			Part->setLastUpdated(Feature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

Relation* Feature::getRelationOrCreatePlaceHolder(Document *theDocument, Layer *theLayer, const QString& Id)
{
	Relation* Part = dynamic_cast<Relation*>(theDocument->getFeature("rel_"+Id));
	if (!Part)
	{
		Part = dynamic_cast<Relation*>(theDocument->getFeature(Id));
		if (!Part)
		{
			Part = new Relation;
			if (Id.startsWith('{') || Id.startsWith('-'))
				Part->setId(Id);
			else
				Part->setId("rel_"+Id);
			Part->setLastUpdated(Feature::NotYetDownloaded);
			theLayer->add(Part);
		}
	}
	return Part;
}

void Feature::mergeTags(Document* theDocument, CommandList* L, Feature* Dest, Feature* Src)
{
	for (int i=0; i<Src->tagSize(); ++i)
	{
		QString k = Src->tagKey(i);
		QString v1 = Src->tagValue(i);
		int j = Dest->findKey(k);
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


QString Feature::toMainHtml(QString type, QString systemtype)
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
		"<a href='/api/" + M_PREFS->apiVersion() + "/" + systemtype + "/" + xmlId() + "/history'>"+QApplication::translate("MapFeature", "History")+"</a>";
		if (systemtype == "node") {
			S += "<br/>"
			"<a href='/api/" + M_PREFS->apiVersion() + "/" + systemtype + "/" + xmlId() + "/ways'>"+QApplication::translate("MapFeature", "Referenced by ways")+"</a>";
		}
		S += "<br/>"
		"<a href='/api/" + M_PREFS->apiVersion() + "/" + systemtype + "/" + xmlId() + "/relations'>"+QApplication::translate("MapFeature", "Referenced by relation")+"</a>";
	}
//	QString bbox("bbox=%1,%2,%3,%4");
//	bbox = bbox.arg(intToAng(boundingBox().bottomLeft().lon()), 0, 'f').arg(intToAng(boundingBox().bottomLeft().lat()), 0, 'f').arg(intToAng(boundingBox().topRight().lon()), 0, 'f').arg(intToAng(boundingBox().topRight().lat()), 0, 'f');
//	S += "<br/>"
//		"<a href='/api/" + M_PREFS->apiVersion() + "/changesets?" + bbox + "'>"+QApplication::translate("MapFeature", "Changesets in feature bbox")+"</a>";
	S += "</body></html>";

	return S;
}

bool Feature::QRectInterstects(const QRect& r, const QLine& l, QPoint& a, QPoint& b)
{
	QLineF lF = QLineF(l);
	QPointF pF;
	bool hasP1 = false;
	bool hasP2 = false;

	if (QLineF(r.topLeft(), r.bottomLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
		a = pF.toPoint();
		hasP1 = true;
	}
	if (QLineF(r.bottomLeft(), r.bottomRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
		if (hasP1) {
			b = pF.toPoint();
			hasP2 = true;
		} else {
			a = pF.toPoint();
			hasP1 = true;
		}
	}
	if (QLineF(r.bottomRight(), r.topRight()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
		if (hasP1) {
			b = pF.toPoint();
			hasP2 = true;
		} else {
			a = pF.toPoint();
			hasP1 = true;
		}
	}
	if (QLineF(r.topRight(), r.topLeft()).intersect(lF, &pF) == QLineF::BoundedIntersection) {
		if (hasP1) {
			b = pF.toPoint();
			hasP2 = true;
		} else {
			a = pF.toPoint();
			hasP1 = true;
		}
	}

	if (hasP1 && hasP2) {
#if (QT_VERSION >= QT_VERSION_CHECK(4, 4, 0))
		double la1 = QLineF(a,b).angleTo(lF);
#else
		double la1 = QLineF(a,b).angle(lF);
#endif
		if (la1 > 15.0 && la1 < 345.0) {
			QPoint t = b;
			b = a;
			a = t;
		}
	}
	if (hasP1)
		return true;
	else
		return false;
}

