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

OsbTile::OsbTile()
	: isLocked(false), isDeleted(false)
{
}

OsbTile::~OsbTile()
{
}


OsbRegion::OsbRegion(ImportExportOsmBin* osb)
	: region(0), device(0), isWorld(false), theOsb(osb)
{
}

OsbRegion::~OsbRegion()
{
	if (isWorld) {
		if (device)
			device->close();
		delete device;
		delete theOsb;
	}
}

bool OsbRegion::load(qint32 rg, Document* d, OsbLayer* theLayer)
{
	region = rg;

	quint64 tocPos = theOsb->theRegionToc[rg];
	if (tocPos) {
		QDataStream ds(device);
		device->seek(tocPos);
		QList< QPair < qint32, quint64 > > aTileList;

		ds >> aTileList;

		for (int i=0; i<aTileList.size(); ++i) {
			theTileToc[aTileList[i].first] = aTileList[i].second;
		}
		theRegionIndex = aTileList;

		bool OK = true;
		qint32 theRegionTile = theRegionIndex[0].first;
		if (theRegionTile == -rg)
			OK = loadTile(theRegionTile, d, theLayer);

		return OK;
	} else {
		isWorld = true;

		theOsb = new ImportExportOsmBin(NULL);
		QFileInfo fi( *((QFile*)device) );
		QString p = fi.absolutePath();
		device = NULL;
		theOsb->loadFile(QString("%1/%2.osb").arg(p).arg(QString::number(rg)));
		if (!theOsb->import(NULL))
			return false;

		device = new QFile(QString("%1/%2.osb").arg(p).arg(QString::number(rg)));
		device->open(QIODevice::ReadOnly);
		if (!device->isOpen()) {
			SAFE_DELETE(device);
			return false;
		}

		loadRegion(0, d, theLayer);
		return loadRegion(rg, d, theLayer);
	}
}

bool OsbRegion::loadRegion(qint32 rg, Document* d, OsbLayer* theLayer)
{
	device->seek(theOsb->theRegionToc[rg]);
	QList< QPair < qint32, quint64 > > aTileList;

	QDataStream ds(device);
	ds >> aTileList;

	for (int i=0; i<aTileList.size(); ++i) {
		theTileToc[aTileList[i].first] = aTileList[i].second;
	}
	theRegionIndex = aTileList;

	bool OK = true;
	qint32 theRegionTile = theRegionIndex[0].first;
	if (theRegionTile == -rg)
		OK = loadTile(theRegionTile, d, theLayer);

	return OK;
}

bool OsbRegion::loadTile(qint32 tile, Document* d, OsbLayer* theLayer)
{
	if (theTileIndex[tile] && theTileIndex[tile]->isDeleted == true) {
		theTileIndex[tile]->isDeleted = false;
		return true;
	}

	QDataStream ds(device);
	quint32 featCount;

	quint32 pos = theTileToc[tile];
	if (!pos)
		return false;

	device->seek(pos);
	ds >> featCount;

	OsbTile* theTile = new OsbTile;
	for (quint32 i=0; i<featCount; ++i) {
		Feature_ptr F(theOsb->getFeature(this, d, theLayer));
		theTile->theIndex.append(F);
	}
	theTileIndex[tile] = theTile;

	return true;
}

QHash< qint32, OsbTile* >&  OsbRegion::getTileIndex()
{
	return theTileIndex;
}

bool OsbRegion::clearRegion(Document* d, OsbLayer* theLayer)
{
	bool OK = true;
	qint32 theRegionTile = theRegionIndex[0].first;
	if (theRegionTile < 0)
		OK = clearTile(theRegionTile, d, theLayer);

	isDeleted = true;

	return OK;
}

bool OsbRegion::clearTile(qint32 tile, Document* d, OsbLayer* theLayer)
{
	Q_UNUSED(d);
	Q_UNUSED(theLayer);

	if (!theTileIndex[tile]->isLocked)
		theTileIndex[tile]->isDeleted = true;

	return true;
}

ImportExportOsmBin::ImportExportOsmBin(Document* doc)
 : IImportExport(doc)
{
}

ImportExportOsmBin::~ImportExportOsmBin()
{
}

void ImportExportOsmBin::doAddTileIndex(Feature* F, qint32 tile)
{
	if (/*Node* N = */CAST_NODE(F)) {
		theTileNodesIndex[tile].push_back(F);
	} else
	if (Way* R = CAST_WAY(F)) {
		for (int k=0; k<R->size(); ++k) {
			Node* N = CAST_NODE(R->get(k));
			if(N->sizeParents() > 1)
				if (!theTileNodesIndex[tile].contains(N))
					theTileNodesIndex[tile].push_back(N);
		}
		theTileWaysIndex[tile].push_back(F);
	} else
	if (CAST_RELATION(F)) {
		theTileRelationsIndex[tile].push_back(F);
	}
}

bool ImportExportOsmBin::exists(Feature* F, qint32 tile)
{
	if (CAST_NODE(F)) {
		if (theTileNodesIndex[tile].contains(F))
			return true;
	} else
	if (CAST_WAY(F)) {
		if (theTileWaysIndex[tile].contains(F))
			return true;
	} else
	if (CAST_RELATION(F)) {
		if (theTileRelationsIndex[tile].contains(F))
			return true;
	}

	return false;
}

void ImportExportOsmBin::addTileIndex(Feature* F, qint32 selRegion)
{
	if (exists(F, 0))
		return;

	if (selRegion == 0) {
		doAddTileIndex(F, selRegion);
		return;
	}

	QRectF r = F->boundingBox().toQRectF();
	int x1 = int((r.topLeft().x() + INT_MAX) / TILE_WIDTH);
	int y1 = int((r.topLeft().y() + INT_MAX) / TILE_WIDTH);
	int x2 = int((r.bottomRight().x() + INT_MAX) / TILE_WIDTH);
	int y2 = int((r.bottomRight().y() + INT_MAX) / TILE_WIDTH);

	int span = (x2 - x1 +1) * (y2 - y1 +1);
	if (span > TILETOREGION_THRESHOLD * TILETOREGION_THRESHOLD) {
		doAddTileIndex(F, 0);
		return;
	}
	for (int i=x1; i <= x2; ++i) {
		for (int j=y1; j <= y2; ++j) {
			qint32 tile = j*NUM_TILES+i;
			qint32 rg = (j * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (i * NUM_REGIONS / NUM_TILES);

			if (exists(F, -rg))
				continue;

			if (span >= TILETOREGION_THRESHOLD)
				tile = -rg;
			else if (
					(F->tagValue("highway", "") == "motorway") ||
					(F->tagValue("highway", "") == "trunk") ||
					(F->tagValue("highway", "") == "primary") ||
					(F->tagValue("highway", "") == "secondary") ||
					(F->tagValue("place", "") == "city") ||
					(F->tagValue("place", "") == "town") ||
					(F->tagValue("capital", "") == "yes") ||
					(F->tagValue("capital", "") == "trueyes") ||
					(F->tagValue("state_capital", "") == "yes") ||
					(F->tagValue("state_capital", "") == "trueyes") ||
					(F->tagValue("boundary", "") == "administrative") ||
					(F->tagValue("waterway", "") == "river") ||
					(F->tagValue("waterway", "") == "riverbank") ||
					(F->tagValue("waterway", "") == "canal") ||
					(F->tagValue("natural", "") == "coastline") ||
					//(F->tagValue("landuse", "__NULL__") != "__NULL__") ||
					false)
				tile = -rg;

			if (!exists(F, tile))
				doAddTileIndex(F, tile);
		}
	}
}

void ImportExportOsmBin::tagsToBinary(Feature* F, QDataStream& ds)
{
	qint64 k, v;
	quint8 tagSize = (quint8)qMin(F->tagSize(), (int) 255);
	if (F->tagValue("created_by", "dummy") != "dummy")
		tagSize--;

	ds << tagSize;
	for (int i=0; i<F->tagSize(); ++i) {
		if (F->tagKey(i) == "created_by")
			continue;
		k = theTagKeysIndex[F->tagKey(i)];
		v = theTagValuesIndex[F->tagValue(i)];
		ds << k;
		ds << v;
	}
}

void ImportExportOsmBin::tagsFromBinary(Feature * F, QDataStream& ds)
{
	quint8 numTags;
	quint64 k,v;
	QString K, V;
	quint64 cur_pos;

	ds >> numTags;
	for (int i=0; i < numTags; ++i) {
		ds >> k;
		ds >> v;
		if (F) {
			cur_pos = ds.device()->pos();
			if (k) {
				if (keyTable.contains(k))
					K = keyTable[k];
				else {
					ds.device()->seek(k);
					ds >> K;
				}
			}
			if (v) {
				if (valueTable.contains(v))
					V = valueTable[v];
				else {
					ds.device()->seek(v);
					ds >> V;
				}
			}
			F->setTag(K,V);
			ds.device()->seek(cur_pos);
		}
	}
}

void ImportExportOsmBin::tagsPopularity(Feature * F)
{
	int val;
	for (int i=0; i<F->tagSize(); ++i) {
		val = keyPopularity.value(F->tagKey(i));
		keyPopularity.insert(F->tagKey(i), val + 1);
		val = valuePopularity.value(F->tagValue(i));
		valuePopularity.insert(F->tagValue(i), val + 1);
	}
}

bool ImportExportOsmBin::prepare(qint32 selRegion)
{
	for (int j=0; j< theFeatures.size(); ++j) {
		qint64 idx = theFeatures[j]->idToLong();
		if (Node* N = CAST_NODE(theFeatures[j])) {
			if (!N->isPOI())
				continue;
			theNodes[idx] = N;
		}
		if (Way* R = CAST_WAY(theFeatures[j])) {
			theWays[idx] = R;
		}
		if (Relation* L = CAST_RELATION(theFeatures[j])) {
			theRelations[idx] = L;
		}
		tagsPopularity(theFeatures[j]);
		addTileIndex(theFeatures[j], selRegion);
	}

	qDebug() << "theTileNodesIndex size: " << theTileNodesIndex.size();
	qDebug() << "theTileWaysIndex size: " << theTileWaysIndex.size();
	qDebug() << "theNodes size: " << theNodes.size();
	qDebug() << "theWays size: " << theWays.size();

	return true;
}

#define HEADER_SIZE 5
bool ImportExportOsmBin::writeHeader(QDataStream& ds)
{
	quint16 osbVersion = 3;

	ds << (qint8)'O' << (qint8)'S' << (qint8)'B' << /* ds.version() << */ osbVersion;
	ds << (qint64)0 /* RegionToc offset */ << (qint64)0 /* tagKeys offset */ << (qint64)0 /* tag Values offset */;

	return true;
}

bool ImportExportOsmBin::writeIndex(QDataStream& ds, int selRegion)
{
	QMapIterator<qint32, QList<Feature*> > itN(theTileNodesIndex);
	while(itN.hasNext()) {
		itN.next();
		theTileIndex[itN.key()] += itN.value();
	}
	QMapIterator<qint32, QList<Feature*> > itR(theTileWaysIndex);
	while(itR.hasNext()) {
		itR.next();
		theTileIndex[itR.key()] += itR.value();
	}
	QMapIterator<qint32, QList<Feature*> > itL(theTileRelationsIndex);
	while(itL.hasNext()) {
		itL.next();
		theTileIndex[itL.key()] += itL.value();
	}

	qDebug() << "theTileIndex size: " << theTileIndex.size();

	int featsize = 0;
	int x, y, rg;
	QMapIterator< qint32, QList<Feature*> > it(theTileIndex);
	while (it.hasNext()) {
		it.next();

		featsize += it.value().size();
		if (it.key() < 0)
			rg = -it.key();
		else {
			y = int(it.key() / NUM_TILES);
			x = (it.key() % NUM_TILES);
			rg = (y * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (x * NUM_REGIONS / NUM_TILES);
		}

		if (rg == selRegion || rg == 0 || selRegion == -1) {
			theRegionIndex[rg].append(QPair < qint32, quint64 > (it.key(), Device->pos()));
			writeFeatures(it.value(), ds);
		}
	}

	qDebug() << "featsize: " << featsize;
	qDebug() << "theRegionIndex size: " << theRegionIndex.size();

	QMapIterator < qint32, QList< QPair < qint32, quint64 > > > j(theRegionIndex);
	while (j.hasNext()) {
		j.next();

		theRegionToc[j.key()] = ds.device()->pos();
		ds << j.value();
	}
	tocPos = ds.device()->pos();
	ds << theRegionToc;

	return true;
}

bool ImportExportOsmBin::readWorld(QDataStream& ds)
{
	if (! readHeader(ds) ) return false;
	if (! readRegionToc(ds) ) return false;

	return true;
}

void ImportExportOsmBin::addWorldRegion(int region)
{
	theRegionToc[region] = 0;
}

void ImportExportOsmBin::removeWorldRegion(int region)
{
	theRegionToc.remove(region);
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

bool ImportExportOsmBin::writeFeatures(QList<Feature*> theFeatList, QDataStream& ds)
{
	ds << (qint32)theFeatList.size();
	QListIterator<Feature*> i(theFeatList);
	while (i.hasNext()) {
		Feature* F = i.next();

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
	ds.device()->seek(tocPos);
	ds >> theRegionToc;

	return true;
}

bool ImportExportOsmBin::readPopularTagLists(QDataStream& ds)
{
	qint32 tagKeysSize, tagValuesSize;
	QString s;
	quint64 cur_pos;

	if (!tagKeysPos)
		return true;

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

bool ImportExportOsmBin::loadRegion(qint32 rg, Document* d, OsbLayer* theLayer)
{
//	Q_ASSERT(theRegionToc.contains(rg));

	if (theRegionList[rg] && theRegionList[rg]->isDeleted == true) {
		theRegionList[rg]->isDeleted = false;
		return true;
	}

	OsbRegion* org = new OsbRegion(this);
	org->device = Device;
	if (!org->load(rg, d, theLayer)) {
		delete org;
		return false;
	}
	theRegionList[rg] = org;

	return true;
}

bool ImportExportOsmBin::loadTile(qint32 tile, Document* d, OsbLayer* theLayer)
{
	int y = int(tile / NUM_TILES);
	int x = (tile % NUM_TILES);
	int rg = (y * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (x * NUM_REGIONS / NUM_TILES);
	if (!theRegionList.contains(rg))
		return false;

	return theRegionList[rg]->loadTile(tile, d, theLayer);
}

bool ImportExportOsmBin::clearRegion(qint32 rg, Document* d, OsbLayer* theLayer)
{
//	Q_ASSERT(theRegionList.contains(rg));
	if (theRegionList[rg] && theRegionList[rg]->isLocked)
		return false;

	theRegionList[rg]->clearRegion(d, theLayer);
//	delete theRegionList[rg];
//	theRegionList.remove(rg);

	return true;
}

bool ImportExportOsmBin::clearTile(qint32 tile, Document* d, OsbLayer* theLayer)
{
	int y = int(tile / NUM_TILES);
	int x = (tile % NUM_TILES);
	int rg = (y * NUM_REGIONS / NUM_TILES) * NUM_REGIONS + (x * NUM_REGIONS / NUM_TILES);
//	Q_ASSERT(theRegionList.contains(rg));

	return theRegionList[rg]->clearTile(tile, d, theLayer);
}

Feature* ImportExportOsmBin::getFeature(OsbRegion* osr, Document* d, OsbLayer* theLayer, quint64 ref)
{
	QDataStream ds(osr->device);
	Feature* F = NULL;
	qint8 c;
	quint64 id;
	quint64 cur_pos = osr->device->pos();

	osr->device->seek(ref);
	ds >> c;
	ds >> id;

	if (d) {
		switch (c) {
			case 'N':
				F = d->getFeature(QString("node_%1").arg(QString::number(id)));
				break;
			case 'A':
			case 'R':
				F = d->getFeature(QString("way_%1").arg(QString::number(id)));
				break;
			case 'L':
				F = d->getFeature(QString("rel_%1").arg(QString::number(id)));
				break;
			default:
				F = NULL;
				Q_ASSERT(false);
		}
		if (F && (F->lastUpdated() != Feature::NotYetDownloaded)) {
			osr->device->seek(cur_pos);
			return F;
		}
	}

	switch (c) {
		case 'N':
			F = Node::fromBinary(d, theLayer, ds, c, id);
			if (F) {
				tagsFromBinary(F, ds);
				if (!F->tagSize())
					F->setVisible(false);
				theLayer->add(F);
			}
			break;
		case 'A':
		case 'R':
			F = Way::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'L':
			F = Relation::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		default:
			Q_ASSERT(false);
	}

	return F;
}

Feature* ImportExportOsmBin::getFeature(OsbRegion* osr, Document* d, OsbLayer* theLayer)
{
	QDataStream ds(osr->device);
	Feature* F, * oF;
	qint8 c;
	quint64 id;

	ds >> c;
	ds >> id;

	switch (c) {
		case 'N':
			if (d)
				oF = d->getFeature(QString("node_%1").arg(QString::number(id)));
			else
				oF = ((Layer*)theLayer)->get(QString("node_%1").arg(QString::number(id)));
			break;
		case 'A':
		case 'R':
			if (d)
				oF = d->getFeature(QString("way_%1").arg(QString::number(id)));
			else
				oF = ((Layer*)theLayer)->get(QString("way_%1").arg(QString::number(id)));
			break;
		case 'L':
			if (d)
				oF = d->getFeature(QString("rel_%1").arg(QString::number(id)));
			else
				oF = ((Layer*)theLayer)->get(QString("rel_%1").arg(QString::number(id)));
			break;
		default:
			oF = NULL;
			Q_ASSERT(false);
	}

	if (oF && (!(oF->isNull()))) {
		theLayer = NULL;
	}

	switch (c) {
		case 'N':
			F = Node::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			if (F && theLayer) {
				if (!F->tagSize())
					F->setVisible(false);
				if (F->layer() != theLayer)
					theLayer->add(F);
			}
			break;
		case 'A':
		case 'R':
			F = Way::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		case 'L':
			F = Relation::fromBinary(d, theLayer, ds, c, id);
			tagsFromBinary(F, ds);
			break;
		default:
			F = NULL;
			Q_ASSERT(false);
	}

	if (oF && (!(oF->isNull())))
		return oF;
	else
		return F;
}

// export
bool ImportExportOsmBin::export_(const QList<Feature*>& featList)
{
	QDataStream ds(Device);
	//theRegionToc.resize(TILE_WIDTH / REGION_WIDTH);

	if(! IImportExport::export_(featList) ) return false;

	if (! prepare() ) return false;
	if (! writeHeader(ds) ) return false;
	if (! writeTagLists(ds) ) return false;

	//if (! writeNodes(ds) ) return false;
	//if (! writeWays(ds) ) return false;
	//if (! writeRelations(ds) ) return false;

	if (! writeIndex(ds) ) return false;

	Device->seek(HEADER_SIZE);
	ds << tocPos;
	ds << tagKeysPos;
	ds << tagValuesPos;

	return true;
}

bool ImportExportOsmBin::export_(const QList<Feature*>& featList, quint32 rg)
{
	QDataStream ds(Device);
	//theRegionToc.resize(TILE_WIDTH / REGION_WIDTH);

	if(! IImportExport::export_(featList) ) return false;

	if (! prepare(rg) ) return false;
	if (! writeHeader(ds) ) return false;
	if (! writeTagLists(ds) ) return false;

	//if (! writeNodes(ds) ) return false;
	//if (! writeWays(ds) ) return false;
	//if (! writeRelations(ds) ) return false;

	if (! writeIndex(ds, rg) ) return false;

	Device->seek(HEADER_SIZE);
	ds << tocPos;
	ds << tagKeysPos;
	ds << tagValuesPos;

	return true;
}

// import the  input
bool ImportExportOsmBin::import(Layer* aLayer)
{
	Q_UNUSED(aLayer)

	QDataStream ds(Device);

	if (! readHeader(ds) ) return false;
	if (! readRegionToc(ds) ) return false;
	if (! readPopularTagLists(ds) ) return false;
	//if (! readTagLists(ds) ) return false;

	//if (! readNodes(ds, aLayer) ) return false;
	//if (! readWays(ds, aLayer) ) return false;
	//if (! readRelations(ds, aLayer) ) return false;

	return true;
}

bool ImportExportOsmBin::writeWorld(QDataStream& ds)
{
	theRegionToc[0] = 0;

	if (! writeHeader(ds) ) return false;
	if (! writeIndex(ds) ) return false;

	ds.device()->seek(HEADER_SIZE);
	ds << tocPos;
	ds << (qint64)0;
	ds << (qint64) 0;

	return true;
}

