//
// C++ Implementation: ImportExportOsmBin
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtGui>

#include "../ImportExport/ImportExportOsmBin.h"

ImportExportOsmBin::ImportExportOsmBin(MapDocument* doc)
 : IImportExport(doc)
{
}

ImportExportOsmBin::~ImportExportOsmBin()
{
}

void ImportExportOsmBin::addTileIndex(MapFeature* F, qint64 pos)
{
	QRectF r = F->boundingBox().toQRectF();
	int x1 = int((r.topLeft().x() + INT_MAX) / REGION_WIDTH);
	int y1 = int((r.topLeft().y() + INT_MAX) / REGION_WIDTH);
	int x2 = int((r.bottomRight().x() + INT_MAX) / REGION_WIDTH);
	int y2 = int((r.bottomRight().y() + INT_MAX) / REGION_WIDTH);

	for (int i=x1; i <= x2; ++i)
		for (int j=y1; j <= y2; ++j)
			theTileIndex[j*NUM_TILES+i].push_back(pos);
}

void ImportExportOsmBin::tagsToBinary(MapFeature* F, QDataStream& ds)
{
	qint64 k, v;

	ds << (qint32)F->tagSize();
	for (unsigned int i=0; i<F->tagSize(); ++i) {
		k = theTagKeysIndex[F->layer()->getDocument()->getTagKeyIndex(F->tagKey(i))];
		Q_ASSERT((k>0));
		v = theTagValuesIndex[F->layer()->getDocument()->getTagValueIndex(F->tagValue(i))];
		Q_ASSERT((v>0));
		ds << k;
		ds << v;
	}
}

void ImportExportOsmBin::tagsFromBinary(MapFeature * F, QDataStream& ds)
{
	quint32 numTags;
	quint64 k,v;
	QString K, V;
	quint64 cur_pos;

	ds >> numTags;
	for (unsigned int i=0; i < numTags; ++i) {
		ds >> k;
		ds >> v;
		cur_pos = Device->pos();
		Device->seek(k);
		ds >> K;
		Device->seek(v);
		ds >> V;
		F->setTag(K,V);
		Device->seek(cur_pos);
	}
}

bool ImportExportOsmBin::prepare()
{
	for (int i=0; i< theFeatures.size(); ++i) {
		qint64 idx = theFeatures[i]->idToLong();
		if (TrackPoint* N = dynamic_cast <TrackPoint*> (theFeatures[i])) {
			theNodes[idx] = N;
		}
		if (Road* R = dynamic_cast <Road*> (theFeatures[i])) {
			theRoads[idx] = R;
		}
		if (Relation* L = dynamic_cast <Relation*> (theFeatures[i])) {
			theRelations[idx] = L;
		}
	}
	return true;
}

#define HEADER_SIZE 5
bool ImportExportOsmBin::writeHeader(QDataStream& ds)
{
	quint16 osbVersion = 2;

	ds << (qint8)'O' << (qint8)'S' << (qint8)'B' << /* ds.version() << */ osbVersion;
	ds << (qint64)0 /* RegionToc offset */ << (qint64)0 /* tagKeys offset */ << (qint64)0 /* tag Values offset */;

	return true;
}

bool ImportExportOsmBin::writeIndex(QDataStream& ds)
{
	QMapIterator< qint32, QList<quint64> > i(theTileIndex);
	while (i.hasNext()) {
		i.next();

		int y = int(i.key() / NUM_TILES);
		int x = (i.key() % NUM_TILES);
		int rg = (y * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (x * NUM_REGIONS / NUM_TILES);

		theRegionIndex[rg].append(QPair < qint32, quint64 > (i.key(), Device->pos()));
		ds << i.value();
	}

	QMapIterator < qint32, QList< QPair < qint32, quint64 > > > j(theRegionIndex);
	while (j.hasNext()) {
		j.next();

		theRegionToc[j.key()] = Device->pos();
		ds << j.value();
	}
	tocPos = Device->pos();
	ds << theRegionToc;

	return true;
}

bool ImportExportOsmBin::writeTagLists(QDataStream& ds)
{
	tagKeysPos = Device->pos();
	ds << (qint32) theDoc->getTagKeys().size();
	for (int i=0; i<theDoc->getTagKeys().size(); i++) {
		theTagKeysIndex << Device->pos();
		ds << theDoc->getTagKeys().at(i);
	}
	tagValuesPos = Device->pos();
	ds << (qint32) theDoc->getTagValues().size();
	for (int i=0; i<theDoc->getTagValues().size(); i++) {
		theTagValuesIndex << Device->pos();
		ds << theDoc->getTagValues().at(i);
	}

	return true;
}

bool ImportExportOsmBin::writeNodes(QDataStream& ds)
{
	ds << (qint32)theNodes.size();
	QMapIterator<quint64, TrackPoint*> i(theNodes);
	i.toBack();
	while (i.hasPrevious()) {
		i.previous();

		addTileIndex(i.value(), Device->pos());
		theFeatureIndex["N" +  QString::number(i.value()->idToLong())] = Device->pos();
		i.value()->toBinary(ds, theFeatureIndex);
		tagsToBinary(i.value(), ds);
	}
	return true;
}

bool ImportExportOsmBin::writeRoads(QDataStream& ds)
{
	ds << (qint32)theRoads.size();
	QMapIterator<quint64,Road*> i(theRoads);
	i.toBack();
	while (i.hasPrevious()) {
		i.previous();

		addTileIndex(i.value(), Device->pos());
		theFeatureIndex["R" + QString::number(i.value()->idToLong())] = Device->pos();
		i.value()->toBinary(ds, theFeatureIndex);
		tagsToBinary(i.value(), ds);
	}
	return true;
}

bool ImportExportOsmBin::writeRelations(QDataStream& ds)
{
	ds << (qint32)theRelations.size();
	QMapIterator<quint64,Relation*> i(theRelations);
	i.toBack();
	while (i.hasPrevious()) {
		i.previous();

		addTileIndex(i.value(), Device->pos());
		theFeatureIndex["L" + QString::number(i.value()->idToLong())] = Device->pos();
		i.value()->toBinary(ds, theFeatureIndex);
		tagsToBinary(i.value(), ds);
	}
	return true;
}

bool ImportExportOsmBin::readHeader(QDataStream& ds)
{
	qint8 c;
	quint16 osbVersion;

	ds >> c; if (c != 'O') return false;
	ds >> c; if (c != 'S') return false;
	ds >> c; if (c != 'B') return false;

	ds >> osbVersion;

	ds >> tocPos;
	ds >> tagKeysPos;
	ds >> tagValuesPos;

	return true;
}

bool ImportExportOsmBin::readRegionToc(QDataStream& ds)
{
	Device->seek(tocPos);
	ds >> theRegionToc;

	return true;
}

bool ImportExportOsmBin::readTagLists(QDataStream& ds)
{
	QList<QString> list;

	ds >> list;
	theDoc->setTagKeys(list);
	ds >> list;
	theDoc->setTagValues(list);

	return true;
}

bool ImportExportOsmBin::readNodes(QDataStream& ds, OsbMapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (TrackPoint* N = TrackPoint::fromBinary(theDoc, aLayer, ds))
			aLayer->add(N);
	}
	return true;
}

bool ImportExportOsmBin::readRoads(QDataStream& ds, OsbMapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (Road* R = Road::fromBinary(theDoc, aLayer, ds))
			aLayer->add(R);
	}
	return true;
}

bool ImportExportOsmBin::readRelations(QDataStream& ds, OsbMapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (Relation* L = Relation::fromBinary(theDoc, aLayer, ds))
			aLayer->add(L);
	}
	return true;
}

bool ImportExportOsmBin::loadRegion(qint32 rg)
{
	if (!theRegionToc.contains(rg))
		return false;

	QDataStream ds(Device);
	Device->seek(theRegionToc[rg]);
	QList< QPair < qint32, quint64 > > aTileList;

	ds >> aTileList;

	for (int i=0; i<aTileList.size(); ++i) {
		theTileToc[aTileList[i].first] = aTileList[i].second;
	}
	theRegionIndex[rg] = aTileList;

	return true;
}

bool ImportExportOsmBin::loadTile(qint32 tile, MapDocument* d, OsbMapLayer* theLayer)
{
	qint8 c;
	quint64 id;
	QDataStream ds(Device);
	MapFeature* F;

	if (!theTileToc.contains(tile))
		return false;

	QList<quint64> aTileList;
	Device->seek(theTileToc[tile]);
	ds >> aTileList;

	for (int i=0; i<aTileList.size(); ++i) {
		quint64 pos = aTileList[i];

		getFeature(d, theLayer, pos);
	}

	return true;
}

bool ImportExportOsmBin::clearTile(qint32 tile, MapDocument* d, OsbMapLayer* theLayer)
{
	qint8 c;
	quint64 id;
	QDataStream ds(Device);
	MapFeature* F;

	int y = int(tile / NUM_TILES);
	int x = (tile % NUM_TILES);
	CoordBox tb = CoordBox(
		Coord (y * (TILE_WIDTH) - INT_MAX, x * TILE_WIDTH - INT_MAX  ),
		Coord (y * (TILE_WIDTH + 1) - INT_MAX, x * (TILE_WIDTH +1) - INT_MAX )
		);

	Q_ASSERT(theTileToc.contains(tile));

	QList<quint64> aTileList;
	Device->seek(theTileToc[tile]);
	ds >> aTileList;

	for (int i=0; i<aTileList.size(); ++i) {
		quint64 pos = aTileList[i];

		Device->seek(pos);
		ds >> c;
		ds >> id;
		F = theLayer->get(QString::number(id), false);
		if (F && tb.contains(F->boundingBox())) {
			unsigned int j=0;
			while (j < F->sizeParents()) {
				if (F->getParent(j)->boundingBox().disjunctFrom(tb)) {
					F->getParent(j)->remove(F);
				} else
					++j;
			}
			if (!F->sizeParents()) {
				theLayer->remove(F);
				delete F;
			}
		}
	}
	return true;
}

MapFeature*  ImportExportOsmBin::getFeature(MapDocument* d, OsbMapLayer* theLayer, quint64 ref)
{
	QDataStream ds(Device);
	MapFeature* F;
	qint8 c;
	quint64 id;
	quint64 cur_pos = Device->pos();

	Device->seek(ref);
	ds >> c;
	ds >> id;
	switch (c) {
		case 'N':
			F = d->getFeature(QString("node_%1").arg(QString::number(id)));
			break;
		case 'R':
			F = d->getFeature(QString("way_%1").arg(QString::number(id)));
			break;
		case 'L':
			F = d->getFeature(QString("rel_%1").arg(QString::number(id)));
			break;
		default:
			Q_ASSERT(false);
	}
	if (F && (F->lastUpdated() != MapFeature::NotYetDownloaded)) {
		Device->seek(cur_pos);
		return F;
	}

	Device->seek(ref);
	switch (c) {
		case 'N':
			F = TrackPoint::fromBinary(d, theLayer, ds);
			tagsFromBinary(F, ds);
			break;
		case 'R':
			F = Road::fromBinary(d, theLayer, ds);
			tagsFromBinary(F, ds);
			break;
		case 'L':
			F = Relation::fromBinary(d, theLayer, ds);
			tagsFromBinary(F, ds);
			break;
		default:
			Q_ASSERT(false);
	}

	Device->seek(cur_pos);
	return F;
}

// export
bool ImportExportOsmBin::export_(const QVector<MapFeature *>& featList)
{
	QDataStream ds(Device);
	//theRegionToc.resize(TILE_WIDTH / REGION_WIDTH);

	if(! IImportExport::export_(featList) ) return false;

	if (! prepare() ) return false;
	if (! writeHeader(ds) ) return false;
	if (! writeTagLists(ds) ) return false;

	if (! writeNodes(ds) ) return false;
	if (! writeRoads(ds) ) return false;
	if (! writeRelations(ds) ) return false;

	if (! writeIndex(ds) ) return false;

	Device->seek(HEADER_SIZE);
	ds << tocPos;
	ds << tagKeysPos;
	ds << tagValuesPos;

	return true;
}

// import the  input
bool ImportExportOsmBin::import(MapLayer* aLayer)
{
	Q_UNUSED(aLayer)

	QDataStream ds(Device);

	if (! readHeader(ds) ) return false;
	if (! readRegionToc(ds) ) return false;
	//if (! readTagLists(ds) ) return false;

	//if (! readNodes(ds, aLayer) ) return false;
	//if (! readRoads(ds, aLayer) ) return false;
	//if (! readRelations(ds, aLayer) ) return false;

	return true;
}
