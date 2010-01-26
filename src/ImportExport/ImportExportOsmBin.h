//
// C++ Interface: ImportExportOsmBin
//
// Description: 
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IMPORTEXPORTOSMBIN_H
#define IMPORTEXPORTOSMBIN_H

#include <ImportExport/IImportExport.h>

#define TILE_WIDTH (int(UINT_MAX/40000))
#define REGION_WIDTH (int(UINT_MAX/4000))
#define NUM_TILES (int(UINT_MAX/TILE_WIDTH))
#define NUM_REGIONS (int(UINT_MAX/REGION_WIDTH))
#define TILETOREGION_THRESHOLD 9

class ImportExportOsmBin;

class OsbRegion 
{
	public:
		OsbRegion(ImportExportOsmBin* osb);
		~OsbRegion();

		bool load(qint32 rg, Document* d, OsbLayer* theLayer);
		bool loadRegion(qint32 rg, Document* d, OsbLayer* theLayer);
		bool loadTile(qint32 tile, Document* d, OsbLayer* theLayer);
		bool clearRegion(Document* d, OsbLayer* theLayer);
		bool clearTile(qint32 tile, Document* d, OsbLayer* theLayer);

	public:
		qint32		region;
		QIODevice*	device;
		bool		isWorld;
	
		QList< QPair < qint32, quint64 > > theRegionIndex;
		QMap< qint32, quint64 > theTileToc;

		quint64 tocPos;
		ImportExportOsmBin* theOsb;
};

class OsbTile
{
};


/**
	@author cbro <cbro@semperpax.com>
*/
class ImportExportOsmBin : public IImportExport
{
	friend class OsbLayer;
	friend class WorldOsbManager;
	friend class OsbRegion;
	friend class OsbLayerPrivate;

public:
    ImportExportOsmBin(Document* doc);

    ~ImportExportOsmBin();

	// import the  input
	virtual bool import(Layer* aLayer);

	//export
	virtual bool export_(const QList<Feature *>& featList);
	virtual bool export_(const QList<Feature *>& featList, quint32 rg);

protected:
//	void addTileIndex(MapFeature* F, qint64 pos);
	void doAddTileIndex(Feature* F, qint32 tile);
	bool exists(Feature* F, qint32 tile);
	void addTileIndex(Feature* F);
	void tagsToBinary(Feature* F, QDataStream& ds);
	void tagsFromBinary(Feature* F, QDataStream& ds);
	void tagsPopularity(Feature * F);
	
	bool prepare();
	bool writeHeader(QDataStream& ds);
	bool writeIndex(QDataStream& ds, int selRegion=-1);
	bool writeTagLists(QDataStream& ds);
	//bool writeNodes(QDataStream& ds);
	//bool writeRoads(QDataStream& ds);
	//bool writeRelations(QDataStream& ds);
	bool writeFeatures(QList<Feature*>, QDataStream& ds);

	bool readWorld(QDataStream& ds);
	void addWorldRegion(int region);
	void removeWorldRegion(int region);
	bool writeWorld(QDataStream& ds);

	bool readHeader(QDataStream& ds);
	bool readRegionToc(QDataStream& ds);
	bool readPopularTagLists(QDataStream& ds);
	//bool readTagLists(QDataStream& ds);
	//bool readNodes(QDataStream& ds, OsbMapLayer* aLayer);
	//bool readRoads(QDataStream& ds, OsbMapLayer* aLayer);
	//bool readRelations(QDataStream& ds, OsbMapLayer* aLayer);

	bool loadRegion(qint32 rg, Document* d, OsbLayer* theLayer);
	bool loadTile(qint32 tile, Document* d, OsbLayer* theLayer);
	bool clearRegion(qint32 rg, Document* d, OsbLayer* theLayer);
	bool clearTile(qint32 tile, Document* d, OsbLayer* theLayer);
	Feature* getFeature(OsbRegion* osr, Document* d, OsbLayer* theLayer, quint64 ref);
	Feature* getFeature(OsbRegion* osr, Document* d, OsbLayer* theLayer);

protected:
	QMap <QString, qint32> keyPopularity;
	QMap <QString, qint32> valuePopularity;
	QMap <quint64, QString> keyTable;
	QMap <quint64, QString> valueTable;

	// k=region# v=offset
	QMap< qint32, quint64 > theRegionToc;
	QMap< qint32, OsbRegion* > theRegionList;

	QMap< qint32, QList<Feature*> > theTileIndex;
	QMap< qint32, QList< QPair < qint32, quint64 > > > theRegionIndex;
	QMap< qint32, QList<Feature*> > theTileNodesIndex;
	QMap< qint32, QList<Feature*> > theTileRoadsIndex;
	QMap< qint32, QList<Feature*> > theTileRelationsIndex;

	QHash <QString, quint64> theFeatureIndex;

	QMap<quint64, Node*> theNodes;
	QMap<quint64,Way*> theRoads;
	QMap<quint64, Relation*> theRelations;

	QMap <QString, quint64> theTagKeysIndex;
	QMap <QString, quint64> theTagValuesIndex;

	qint64 tocPos;
	qint64 tagKeysPos;
	qint64 tagValuesPos;
};

#endif
