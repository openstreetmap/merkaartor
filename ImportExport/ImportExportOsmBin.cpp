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

//void ImportExportOsmBin::addTileIndex(MapFeature* F, qint64 pos)
//{
//	QRectF r = F->boundingBox().toQRectF();
//	int x1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH);
//	int y1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
//	int x2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
//	int y2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);
//
//	for (int i=x1; i <= x2; ++i)
//		for (int j=y1; j <= y2; ++j)
//			theTileIndex[j*NUM_TILES+i].push_back(pos);
//}
//
void ImportExportOsmBin::addTileIndex(MapFeature* F)
{
	QRectF r = F->boundingBox().toQRectF();
	int x1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH);
	int y1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
	int x2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
	int y2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);

	int span = (x2 - x1) * (y2 - y1);
	for (int i=x1; i <= x2; ++i)
		for (int j=y1; j <= y2; ++j) {
			qint32 tile = j*NUM_TILES+i;
			qint32 rg = (j * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (i * NUM_REGIONS / NUM_TILES);
			if (span >= TILETOREGION_THRESHOLD)
				tile = -rg;
			if (
					(F->tagValue("highway", "") == "motorway") ||
					(F->tagValue("highway", "") == "trunk") ||
					(F->tagValue("highway", "") == "primary") ||
					(F->tagValue("highway", "") == "secondry") ||
					(F->tagValue("place", "") == "city") ||
					(F->tagValue("place", "") == "town") ||
					(F->tagValue("boundary", "") == "administrative") ||
					(F->tagValue("waterway", "") == "river") ||
					(F->tagValue("waterway", "") == "riverbank") ||
					(F->tagValue("waterway", "") == "canal") ||
					false)
				tile = -rg;

			if (TrackPoint* N = CAST_NODE(F)) {
				if (!theTileNodesIndex[tile].contains(F))
					theTileNodesIndex[tile].push_back(F);
			}
			if (Road* R = CAST_WAY(F)) {
				for (unsigned int k=0; k<R->size(); ++k)
					if (!theTileNodesIndex[tile].contains(R->get(k)))
						theTileNodesIndex[tile].push_back(R->get(k));
				if (!theTileRoadsIndex[tile].contains(F))
					theTileRoadsIndex[tile].push_back(F);
			}
			if (Relation* L = CAST_RELATION(F)) {
				//for (unsigned int k=0; k<L->size(); ++k) {
				//	if (TrackPoint* N2 = CAST_NODE(L->get(k)))
				//		if (!theTileNodesIndex[j*NUM_TILES+i].contains(N2))
				//			theTileNodesIndex[j*NUM_TILES+i].push_back(N2);
				//	if (Road* R2 = CAST_WAY(L->get(k)))
				//		if (!theTileRoadsIndex[j*NUM_TILES+i].contains(R2))
				//			theTileRoadsIndex[j*NUM_TILES+i].push_back(R2);
				//	if (Relation* L2 = CAST_RELATION(L->get(k)))
				//		if (!theTileRelationsIndex[j*NUM_TILES+i].contains(L2))
				//			theTileRelationsIndex[j*NUM_TILES+i].push_back(L2);
				//}
				if (!theTileRelationsIndex[tile].contains(F))
					theTileRelationsIndex[tile].push_back(F);
			}
		}
}

void ImportExportOsmBin::tagsToBinary(MapFeature* F, QDataStream& ds)
{
	qint64 k, v;
	quint8 tagSize = (quint8)qMin(F->tagSize(), (unsigned int) 255);
	if (F->tagValue("created_by", "dummy") != "dummy")
		tagSize--;

	ds << tagSize;
	for (unsigned int i=0; i<F->tagSize(); ++i) {
		if (F->tagKey(i) == "created_by")
			continue;
		k = theTagKeysIndex[F->tagKey(i)];
		Q_ASSERT((k>0));
		v = theTagValuesIndex[F->tagValue(i)];
		Q_ASSERT((v>0));
		ds << k;
		ds << v;
	}
}

void ImportExportOsmBin::tagsFromBinary(MapFeature * F, QDataStream& ds)
{
	quint8 numTags;
	quint64 k,v;
	QString K, V;
	quint64 cur_pos;

	ds >> numTags;
	for (unsigned int i=0; i < numTags; ++i) {
		ds >> k;
		ds >> v;
		cur_pos = Device->pos();
		if (keyTable.contains(k))
			K = keyTable[k];
		else {
			Device->seek(k);
			ds >> K;
		}
		if (valueTable.contains(v))
			V = valueTable[v];
		else {
			Device->seek(v);
			ds >> V;
		}
		F->setTag(K,V);
		Device->seek(cur_pos);
	}
}

void ImportExportOsmBin::tagsPopularity(MapFeature * F)
{
	int val;
	for (unsigned int i=0; i<F->tagSize(); ++i) {
		val = keyPopularity.value(F->tagKey(i));
		keyPopularity.insert(F->tagKey(i), val + 1);
		val = valuePopularity.value(F->tagValue(i));
		valuePopularity.insert(F->tagValue(i), val + 1);
	}
}

bool ImportExportOsmBin::prepare()
{
	for (int j=0; j< theFeatures.size(); ++j) {
		qint64 idx = theFeatures[j]->idToLong();
		tagsPopularity(theFeatures[j]);
		addTileIndex(theFeatures[j]);
		if (TrackPoint* N = CAST_NODE(theFeatures[j])) {
			theNodes[idx] = N;
		}
		if (Road* R = CAST_WAY(theFeatures[j])) {
			theRoads[idx] = R;
		}
		if (Relation* L = CAST_RELATION(theFeatures[j])) {
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
	QMapIterator<qint32, QList<MapFeature*> > itN(theTileNodesIndex);
	while(itN.hasNext()) {
		itN.next();
		theTileIndex[itN.key()] += itN.value();
	}
	QMapIterator<qint32, QList<MapFeature*> > itR(theTileRoadsIndex);
	while(itR.hasNext()) {
		itR.next();
		theTileIndex[itR.key()] += itR.value();
	}
	QMapIterator<qint32, QList<MapFeature*> > itL(theTileRelationsIndex);
	while(itL.hasNext()) {
		itL.next();
		theTileIndex[itL.key()] += itL.value();
	}

	int x, y, rg;
	QMapIterator< qint32, QList<MapFeature*> > it(theTileIndex);
	while (it.hasNext()) {
		it.next();

		if (it.key() < 0)
			rg = -it.key();
		else {
			y = int(it.key() / NUM_TILES);
			x = (it.key() % NUM_TILES);
			rg = (y * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (x * NUM_REGIONS / NUM_TILES);
		}

		theRegionIndex[rg].append(QPair < qint32, quint64 > (it.key(), Device->pos()));
		writeFeatures(it.value(), ds);
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
	QMultiMap <qint32, QString> popularityKey;
	QMultiMap <qint32, QString> popularityValue;

	QMapIterator<QString, qint32> kpi(keyPopularity);
	while (kpi.hasNext()) {
		kpi.next();
		popularityKey.insert(kpi.value(), kpi.key());
	}

	tagKeysPos = Device->pos();
	ds << (qint32) theDoc->getTagKeys().size();

	QMapIterator<qint32, QString> ki(popularityKey);
	ki.toBack();
	while (ki.hasPrevious()) {
		ki.previous();

		theTagKeysIndex[ki.value()] = Device->pos();
		ds << ki.value();
	}

	QMapIterator<QString, qint32> vpi(valuePopularity);
	while (vpi.hasNext()) {
		vpi.next();
		popularityValue.insert(vpi.value(), vpi.key());
	}

	tagValuesPos = Device->pos();
	ds << (qint32) theDoc->getTagValues().size();

	QMapIterator<qint32, QString> vi(popularityValue);
	vi.toBack();
	while (vi.hasPrevious()) {
		vi.previous();

		theTagValuesIndex[vi.value()] = Device->pos();
		ds << vi.value();
	}

	return true;
}

//bool ImportExportOsmBin::writeNodes(QDataStream& ds)
//{
//	ds << (qint32)theNodes.size();
//	QMapIterator<quint64, TrackPoint*> i(theNodes);
//	i.toBack();
//	while (i.hasPrevious()) {
//		i.previous();
//
//		addTileIndex(i.value(), Device->pos());
//		theFeatureIndex["N" +  QString::number(i.value()->idToLong())] = Device->pos();
//		i.value()->toBinary(ds, theFeatureIndex);
//		tagsToBinary(i.value(), ds);
//	}
//	return true;
//}
//
//bool ImportExportOsmBin::writeRoads(QDataStream& ds)
//{
//	ds << (qint32)theRoads.size();
//	QMapIterator<quint64,Road*> i(theRoads);
//	i.toBack();
//	while (i.hasPrevious()) {
//		i.previous();
//
//		addTileIndex(i.value(), Device->pos());
//		theFeatureIndex["R" + QString::number(i.value()->idToLong())] = Device->pos();
//		i.value()->toBinary(ds, theFeatureIndex);
//		tagsToBinary(i.value(), ds);
//	}
//	return true;
//}
//
//bool ImportExportOsmBin::writeRelations(QDataStream& ds)
//{
//	ds << (qint32)theRelations.size();
//	QMapIterator<quint64,Relation*> i(theRelations);
//	i.toBack();
//	while (i.hasPrevious()) {
//		i.previous();
//
//		addTileIndex(i.value(), Device->pos());
//		theFeatureIndex["L" + QString::number(i.value()->idToLong())] = Device->pos();
//		i.value()->toBinary(ds, theFeatureIndex);
//		tagsToBinary(i.value(), ds);
//	}
//	return true;
//}
//
bool ImportExportOsmBin::writeFeatures(QList<MapFeature*> theFeatList, QDataStream& ds)
{
	ds << (qint32)theFeatList.size();
	QListIterator<MapFeature*> i(theFeatList);
	while (i.hasNext()) {
		MapFeature* F = i.next();

		F->toBinary(ds, theFeatureIndex);
		tagsToBinary(F, ds);
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

bool ImportExportOsmBin::readPopularTagLists(QDataStream& ds)
{
	qint32 tagKeysSize, tagValuesSize;
	QString s;
	quint64 cur_pos;
	
	Device->seek(tagKeysPos);
	ds >> tagKeysSize;

	for (int i=0; i<20 && i<tagKeysSize; ++i) {
		cur_pos = Device->pos();
		ds >> s;
		keyTable.insert(cur_pos, s);
	}
	
	Device->seek(tagValuesPos);
	ds >> tagValuesSize;

	for (int i=0; i<20 && i<tagValuesSize; ++i) {
		cur_pos = Device->pos();
		ds >> s;
		valueTable.insert(cur_pos, s);
	}

	return true;
}

//bool ImportExportOsmBin::readTagLists(QDataStream& ds)
//{
//	QList<QString> list;
//
//	ds >> list;
//	theDoc->setTagKeys(list);
//	ds >> list;
//	theDoc->setTagValues(list);
//
//	return true;
//}
//
//bool ImportExportOsmBin::readNodes(QDataStream& ds, OsbMapLayer* aLayer)
//{
//	int fSize;
//
//	ds >> fSize;
//	for (int i=0; i< fSize; ++i) {
//		if (TrackPoint* N = TrackPoint::fromBinary(theDoc, aLayer, ds))
//			aLayer->add(N);
//	}
//	return true;
//}
//
//bool ImportExportOsmBin::readRoads(QDataStream& ds, OsbMapLayer* aLayer)
//{
//	int fSize;
//
//	ds >> fSize;
//	for (int i=0; i< fSize; ++i) {
//		if (Road* R = Road::fromBinary(theDoc, aLayer, ds))
//			aLayer->add(R);
//	}
//	return true;
//}
//
//bool ImportExportOsmBin::readRelations(QDataStream& ds, OsbMapLayer* aLayer)
//{
//	int fSize;
//
//	ds >> fSize;
//	for (int i=0; i< fSize; ++i) {
//		if (Relation* L = Relation::fromBinary(theDoc, aLayer, ds))
//			aLayer->add(L);
//	}
//	return true;
//}

bool ImportExportOsmBin::loadRegion(qint32 rg, MapDocument* d, OsbMapLayer* theLayer)
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

	bool OK;
	qint32 theRegionTile = theRegionIndex[rg][0].first;
	if (theRegionTile < 0)
		OK = loadTile(theRegionTile, d, theLayer);

	return OK;
}

bool ImportExportOsmBin::loadTile(qint32 tile, MapDocument* d, OsbMapLayer* theLayer)
{
	QDataStream ds(Device);
	quint32 featCount;

	if (!theTileToc.contains(tile))
		return false;

	quint32 pos = theTileToc[tile];
	Device->seek(pos);
	ds >> featCount;

	for (quint32 i=0; i<featCount; ++i) {
		MapFeature* F = getFeature(d, theLayer);
		theTileIndex[tile].push_back(F);
		featRefCount[F]++;
	}

	//qDebug() << "--------- " << tile;
	//QMapIterator<MapFeature*, quint32> itN(featRefCount);
	//while(itN.hasNext()) {
	//	itN.next();

	//	if (itN.value() > 1)
	//		qDebug() << itN.key()->id() << " - " << itN.value();
	//}

	return true;
}

bool ImportExportOsmBin::clearRegion(qint32 rg, MapDocument* d, OsbMapLayer* theLayer)
{
	bool OK;
	qint32 theRegionTile = theRegionIndex[rg][0].first;
	if (theRegionTile < 0)
		OK = clearTile(theRegionTile, d, theLayer);
	theRegionIndex.remove(rg);

	return OK;
}

bool ImportExportOsmBin::clearTile(qint32 tile, MapDocument* d, OsbMapLayer* theLayer)
{
	Q_UNUSED(d);

	QDataStream ds(Device);
	bool OK = true;

	for (qint32 i=0; i<theTileIndex[tile].size(); ++i) {
		MapFeature* F = theTileIndex[tile][i];
		if (F->layer() == theLayer) {
			int theCount = --featRefCount[F];
			if (!(theCount)) {
				theLayer->remove(F);
				delete F;
				featRefCount.remove(F);
			}
		} else {
			featRefCount.remove(F);
		}
	}
	theTileIndex.remove(tile);

	//qDebug() << "--------- " << tile;
	//QMapIterator<MapFeature*, quint32> itN(featRefCount);
	//while(itN.hasNext()) {
	//	itN.next();

	//	if (itN.value() > 1)
	//		qDebug() << itN.key()->id() << " - " << itN.value();
	//}

	return OK;
}

MapFeature* ImportExportOsmBin::getFeature(MapDocument* d, OsbMapLayer* theLayer, quint64 ref)
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

	switch (c) {
		case 'N':
			F = TrackPoint::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'R':
			F = Road::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'L':
			F = Relation::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		default:
			Q_ASSERT(false);
	}

	//Device->seek(cur_pos);
	return F;
}

MapFeature* ImportExportOsmBin::getFeature(MapDocument* d, OsbMapLayer* theLayer)
{
	QDataStream ds(Device);
	MapFeature* F;
	qint8 c;
	quint64 id;

	ds >> c;
	ds >> id;
	//switch (c) {
	//	case 'N':
	//		F = d->getFeature(QString("node_%1").arg(QString::number(id)));
	//		break;
	//	case 'R':
	//		F = d->getFeature(QString("way_%1").arg(QString::number(id)));
	//		break;
	//	case 'L':
	//		F = d->getFeature(QString("rel_%1").arg(QString::number(id)));
	//		break;
	//	default:
	//		Q_ASSERT(false);
	//}
	//if (F && (F->lastUpdated() != MapFeature::NotYetDownloaded)) {
	//	Device->seek(cur_pos);
	//	return F;
	//}
#ifndef NDEBUG
	quint32 pos = Device->pos(); //1097543
#endif
	switch (c) {
		case 'N':
			F = TrackPoint::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'R':
			F = Road::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'L':
			F = Relation::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		default:
			Q_ASSERT(false);
	}

	//Device->seek(cur_pos);
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

	//if (! writeNodes(ds) ) return false;
	//if (! writeRoads(ds) ) return false;
	//if (! writeRelations(ds) ) return false;

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
	if (! readPopularTagLists(ds) ) return false;
	//if (! readTagLists(ds) ) return false;

	//if (! readNodes(ds, aLayer) ) return false;
	//if (! readRoads(ds, aLayer) ) return false;
	//if (! readRelations(ds, aLayer) ) return false;

	return true;
}
