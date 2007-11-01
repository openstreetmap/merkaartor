#include "Map/MapFeature.h"
#include "Map/MapDocument.h"

#include <QtCore/QUuid>

static QString randomId()
{
	return QUuid::createUuid().toString(); 
}

class MapFeaturePrivate
{
	public:
		MapFeaturePrivate()
			: LastActor(MapFeature::User), theLayer(0) { }
		MapFeaturePrivate(const MapFeaturePrivate& other)
			: Tags(other.Tags), LastActor(MapFeature::User), theLayer(0) { }

		mutable QString Id;
		std::vector<std::pair<QString, QString> > Tags;
		MapFeature::ActorType LastActor;
		MapLayer* theLayer;
};

MapFeature::MapFeature()
: p(0)
{
	 p = new MapFeaturePrivate;
}

MapFeature::MapFeature(const MapFeature& other)
{
	p = new MapFeaturePrivate(*other.p);
}

MapFeature::~MapFeature(void)
{
	if (p->theLayer)
		p->theLayer->notifyIdUpdate(p->Id,0);
	delete p;
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

void MapFeature::setTag(unsigned int idx, const QString& k, const QString& v)
{
	p->Tags[idx] = std::make_pair(k,v);
}

void MapFeature::setTag(const QString& k, const QString& v)
{
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
		{
			p->Tags[i].second = v;
			return;
		}
	p->Tags.push_back(std::make_pair(k,v));
}

void MapFeature::clearTags()
{
	p->Tags.clear();
}

void MapFeature::clearTag(const QString& k)
{
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
		{
			p->Tags.erase(p->Tags.begin()+i);
			return;
		}
}

unsigned int MapFeature::tagSize() const
{
	return p->Tags.size();
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
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
			return i;
	return p->Tags.size();
}

void MapFeature::removeTag(unsigned int idx)
{
	p->Tags.erase(p->Tags.begin()+idx);
}

QString MapFeature::tagValue(const QString& k, const QString& Default) const
{
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
			return p->Tags[i].second;
	return Default;
}


