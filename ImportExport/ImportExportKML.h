//
// C++ Interface: ImportExportKML
//
// Description: 
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportKML_H
#define ImportExportKML_H

#include <ImportExport/IImportExport.h>

class QDomDocument;
/**
	@author cbro <cbro@semperpax.com>
*/
class ImportExportKML : public IImportExport
{
public:
    ImportExportKML(MapDocument* doc);

    ~ImportExportKML();

	// import the  input
	virtual bool import(MapLayer* aLayer);

	//export
	virtual bool export_(const QVector<MapFeature *>& featList);
};

#endif
