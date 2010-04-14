//
// C++ Interface: ImportExportKML
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef ImportExportOSC_H
#define ImportExportOSC_H

#include <ImportExport/IImportExport.h>

class QDomDocument;
/**
    @author cbro <cbro@semperpax.com>
*/
class ImportExportOSC : public IImportExport
{
public:
    ImportExportOSC(Document* doc);

    ~ImportExportOSC();

    // import the  input
    virtual bool import(Layer* aLayer);

    //export
    virtual bool export_(const QList<Feature *>& featList = QList<Feature *>());
};

#endif
