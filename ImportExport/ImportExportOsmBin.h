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

/**
	@author cbro <cbro@semperpax.com>
*/
class ImportExportOsmBin : public IImportExport
{
public:
    ImportExportOsmBin(MapDocument* doc);

    ~ImportExportOsmBin();

	// import the  input
	virtual bool import(MapLayer* aLayer);

	//export
	virtual bool export_(const QVector<MapFeature *>& featList);

protected:
	bool prepare();
	bool writeHeader(QDataStream& ds);
	bool writeTagLists(QDataStream& ds);
	bool writeNodes(QDataStream& ds);
	bool writeRoads(QDataStream& ds);
	bool writeRelations(QDataStream& ds);

	bool readHeader(QDataStream& ds);
	bool readTagLists(QDataStream& ds);
	bool readNodes(QDataStream& ds, MapLayer* aLayer);
	bool readRoads(QDataStream& ds, MapLayer* aLayer);
	bool readRelations(QDataStream& ds, MapLayer* aLayer);

protected:
	QVector<TrackPoint*> theNodes;
	QVector<Road*> theRoads;
	QVector<Relation*> theRelations;

};

#endif
