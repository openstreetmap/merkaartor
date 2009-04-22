//
// C++ Interface: ImportExportSHP
//
// Description: 
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportSHP_H
#define ImportExportSHP_H

#include <ImportExport/IImportExport.h>

/**
	@author cbro <cbro@semperpax.com>
*/
class ImportExportSHP : public IImportExport
{
public:
    ImportExportSHP(MapDocument* doc);

    ~ImportExportSHP();

	// Specify the input as a QFile
	virtual bool loadFile(QString filename);
	// Specify the output as a QFile
	virtual bool saveFile(QString filename);
	// import the  input
	virtual bool import(MapLayer* aLayer);

	//export
	virtual bool export_(const QList<MapFeature *>& featList);
};

#endif
