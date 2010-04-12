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
#ifndef ImportExportCSV_H
#define ImportExportCSV_H

#include <ImportExport/IImportExport.h>

class QDomDocument;
/**
    @author cbro <cbro@semperpax.com>
*/
class ImportExportCSV : public IImportExport
{
public:
    ImportExportCSV(Document* doc);

    ~ImportExportCSV();

    // import the  input
    virtual bool import(Layer* aLayer);

    //export
    virtual bool export_(const QList<Feature *>& featList);
};

#endif
