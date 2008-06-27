//
// C++ Interface: ImportNMEA
//
// Description: 
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef IMPORTNMEA_H
#define IMPORTNMEA_H

#include <ImportExport/IImportExport.h>

/**
	@author cbro <cbro@semperpax.com>
*/
class ImportNMEA : public IImportExport
{
public:
    ImportNMEA(MapDocument* doc);

    ~ImportNMEA();

	// import the  input
	virtual bool import(MapLayer* aLayer);
	// export
	virtual bool export_(const QVector<MapFeature *>& featList);

private:
	TrackMapLayer* theLayer;

	bool importGSA (QString line);
	bool importGSV (QString line);
	TrackPoint* importGGA (QString line);
	TrackPoint* importRMC (QString line);

};

#endif
