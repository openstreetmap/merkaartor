//
// C++ Implementation: ImportExportCSV
//
// Description:
//
//
// Author: cbro <cbro@semperpax.com>, (C) 2010
//
// Copyright: See COPYING file that comes with this distribution
//
//

#include <QtGui>
#include <QMessageBox>

#include "../ImportExport/ImportExportCSV.h"
#include "ImportCSVDialog.h"
#include "Maps/Projection.h"

bool parseContainer(QDomElement& e, Layer* aLayer);

ImportExportCSV::ImportExportCSV(Document* doc)
 : IImportExport(doc)
{
}


ImportExportCSV::~ImportExportCSV()
{
}


// export
bool ImportExportCSV::export_(const QList<Feature *>& featList)
{
}

// import the  input
bool ImportExportCSV::import(Layer* aLayer)
{
    ImportCSVDialog* dlg = new ImportCSVDialog(Device);
    if (dlg->exec() == QDialog::Rejected)
        return false;
    dlg->import(aLayer);
}

