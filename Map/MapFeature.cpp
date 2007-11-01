#include "Map/MapFeature.h"
#include "Map/MapDocument.h"
#include "PaintStyle/EditPaintStyle.h"

#include <QtCore/QUuid>

static QString randomId()
{
	return QUuid::createUuid().toString(); 
}

class MapFeaturePrivate
{
	public:
		MapFeaturePrivate()
			: LastActor(MapFeature::User), theLayer(0), PaintersUpToDate(false),
		      PainterOnLocalZoom(0), PainterOnRegionalZoom(0), 
			  PainterOnGlobalZoom(0), PainterOnAnyZoom(0),
			  theFeature(0) { }
		MapFeaturePrivate(const MapFeaturePrivate& other)
			: Tags(other.Tags), LastActor(MapFeature::User), theLayer(0),
			  PaintersUpToDate(other.PaintersUpToDate),
			  PainterOnLocalZoom(other.PainterOnLocalZoom),
			  PainterOnRegionalZoom(other.PainterOnRegionalZoom),
			  PainterOnGlobalZoom(other.PainterOnGlobalZoom),
			  PainterOnAnyZoom(other.PainterOnAnyZoom),
			  theFeature(0) { }

		void updatePainters();

		mutable QString Id;
		std::vector<std::pair<QString, QString> > Tags;
		MapFeature::ActorType LastActor;
		MapLayer* theLayer;
		bool PaintersUpToDate;
		FeaturePainter* PainterOnLocalZoom;
		FeaturePainter* PainterOnRegionalZoom;
		FeaturePainter* PainterOnGlobalZoom;
		FeaturePainter* PainterOnAnyZoom;
		MapFeature* theFeature;
};

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
	p->PaintersUpToDate = false;
	p->Tags[idx] = std::make_pair(k,v);
}

void MapFeature::setTag(const QString& k, const QString& v)
{
	p->PaintersUpToDate = false;
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
	p->PaintersUpToDate = false;
	p->Tags.clear();
}

void MapFeature::clearTag(const QString& k)
{
	p->PaintersUpToDate = false;
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
	p->PaintersUpToDate = false;
	p->Tags.erase(p->Tags.begin()+idx);
}

QString MapFeature::tagValue(const QString& k, const QString& Default) const
{
	for (unsigned int i=0; i<p->Tags.size(); ++i)
		if (p->Tags[i].first == k)
			return p->Tags[i].second;
	return Default;
}

void MapFeaturePrivate::updatePainters()
{
	PainterOnLocalZoom = PainterOnRegionalZoom = PainterOnGlobalZoom = PainterOnAnyZoom = 0;
	for (unsigned int i=0; i<EditPaintStyle::Painters.size(); ++i)
	{
		FeaturePainter* Current = &EditPaintStyle::Painters[i];
		if (!PainterOnLocalZoom && Current->isHit(theFeature,FeaturePainter::LocalZoom))
			PainterOnLocalZoom = Current;
		if (!PainterOnRegionalZoom && Current->isHit(theFeature,FeaturePainter::RegionalZoom))
			PainterOnRegionalZoom = Current;
		if (!PainterOnGlobalZoom && Current->isHit(theFeature,FeaturePainter::GlobalZoom))
			PainterOnGlobalZoom = Current;
		if (!PainterOnAnyZoom && Current->isHit(theFeature,FeaturePainter::NoZoomLimit))
			PainterOnAnyZoom = Current;
	}
	PaintersUpToDate = true;
}

FeaturePainter* MapFeature::getEditPainter(FeaturePainter::ZoomType Zoom) const
{
	if (!p->PaintersUpToDate)
		p->updatePainters();
	switch (Zoom)
	{
	case FeaturePainter::LocalZoom:
		return p->PainterOnLocalZoom;
	case FeaturePainter::RegionalZoom:
		return p->PainterOnRegionalZoom;
	case FeaturePainter::GlobalZoom:
		return p->PainterOnGlobalZoom;
	case FeaturePainter::NoZoomLimit:
		return p->PainterOnAnyZoom;
	}
	return 0;
}

bool MapFeature::hasEditPainter() const
{
	if (!p->PaintersUpToDate)
		p->updatePainters();
	return p->PainterOnGlobalZoom || p->PainterOnLocalZoom || p->PainterOnGlobalZoom || p->PainterOnAnyZoom;
}

