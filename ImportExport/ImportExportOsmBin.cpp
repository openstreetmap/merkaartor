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

bool ImportExportOsmBin::prepare()
{
	for (int i=0; i< theFeatures.size(); ++i) {
		if (TrackPoint* N = dynamic_cast <TrackPoint*> (theFeatures[i]))
			theNodes.append(N);
		if (Road* R = dynamic_cast <Road*> (theFeatures[i]))
			theRoads.append(R);
		if (Relation* L = dynamic_cast <Relation*> (theFeatures[i]))
			theRelations.append(L);
	}
	return true;
}

bool ImportExportOsmBin::writeHeader(QDataStream& ds)
{
	quint16 osbVersion = 1;

	ds << (qint8)'O' << (qint8)'S' << (qint8)'B' << /* ds.version() << */ osbVersion;

	return true;
}

bool ImportExportOsmBin::writeTagLists(QDataStream& ds)
{
	ds << theDoc->getTagKeys();
	ds << theDoc->getTagValues();

	return true;
}

bool ImportExportOsmBin::writeNodes(QDataStream& ds)
{
	ds << (qint32)theNodes.size();
	for (int i=0; i< theNodes.size(); ++i) {
		theNodes[i]->toBinary(ds);
	}
	return true;
}

bool ImportExportOsmBin::writeRoads(QDataStream& ds)
{
	ds << (qint32)theRoads.size();
	for (int i=0; i< theRoads.size(); ++i) {
		theRoads[i]->toBinary(ds);
	}
	return true;
}

bool ImportExportOsmBin::writeRelations(QDataStream& ds)
{
	ds << (qint32)theRelations.size();
	for (int i=0; i< theRelations.size(); ++i) {
		theRelations[i]->toBinary(ds);
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

bool ImportExportOsmBin::readNodes(QDataStream& ds, MapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (TrackPoint* N = TrackPoint::fromBinary(theDoc, aLayer, ds))
			aLayer->add(N);
	}
	return true;
}

bool ImportExportOsmBin::readRoads(QDataStream& ds, MapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (Road* R = Road::fromBinary(theDoc, aLayer, ds))
			aLayer->add(R);
	}
	return true;
}

bool ImportExportOsmBin::readRelations(QDataStream& ds, MapLayer* aLayer)
{
	int fSize;

	ds >> fSize;
	for (int i=0; i< fSize; ++i) {
		if (Relation* L = Relation::fromBinary(theDoc, aLayer, ds))
			aLayer->add(L);
	}
	return true;
}


// export
bool ImportExportOsmBin::export_(const QVector<MapFeature *>& featList)
{
	QDataStream ds(Device);

	if(! IImportExport::export_(featList) ) return false;

	if (! prepare() ) return false;
	if (! writeHeader(ds) ) return false;
	if (! writeTagLists(ds) ) return false;

	if (! writeNodes(ds) ) return false;
	if (! writeRoads(ds) ) return false;
	if (! writeRelations(ds) ) return false;

	return true;
}

// import the  input
bool ImportExportOsmBin::import(MapLayer* aLayer)
{
	QDataStream ds(Device);

	if (! readHeader(ds) ) return false;
	if (! readTagLists(ds) ) return false;

	if (! readNodes(ds, aLayer) ) return false;
	if (! readRoads(ds, aLayer) ) return false;
	if (! readRelations(ds, aLayer) ) return false;

	return true;
}
